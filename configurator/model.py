from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Tuple

try:
    from .box64rc import (
        Assignment,
        RcDocument,
        RcSection,
        SectionKey,
        copy_assignments,
        direct_box64_assignments,
        header_from_key,
        normalize_section_input,
        section_key_from_header,
    )
    from .usage import EnvOption, UsageCatalog
except ImportError:
    from box64rc import (
        Assignment,
        RcDocument,
        RcSection,
        SectionKey,
        copy_assignments,
        direct_box64_assignments,
        header_from_key,
        normalize_section_input,
        section_key_from_header,
    )
    from usage import EnvOption, UsageCatalog


ARCH_ALIASES = {
    "aarch64": "arm64",
    "arm64": "arm64",
    "amd64": "x86_64",
    "loong64": "la64",
    "loongarch64": "la64",
    "powerpc64le": "ppc64le",
    "ppc64le": "ppc64le",
    "x86_64": "x86_64",
}


def box64_arch_from_machine(machine: Optional[str]) -> str:
    value = (machine or "").strip().lower().replace("-", "_")
    if value.startswith("riscv64"):
        return "rv64"
    return ARCH_ALIASES.get(value, value)


@dataclass(frozen=True)
class EntryRecord:
    key: SectionKey
    display_name: str
    system_section: Optional[RcSection]
    user_section: Optional[RcSection]

    @property
    def editable_source(self) -> str:
        if self.user_section is not None:
            return "user"
        if self.system_section is not None:
            return "system"
        return "new"

    @property
    def arch(self) -> str:
        return self.key.arch


@dataclass(frozen=True)
class ValueState:
    option: EnvOption
    value: str
    source: str


SectionIdentity = Tuple[str, str]


OFTEN_USED_OPTION_NAMES = {
    "BOX64_DYNAREC_BIGBLOCK",
    "BOX64_DYNAREC_CALLRET",
    "BOX64_DYNAREC_DIRTY",
    "BOX64_DYNAREC_FASTNAN",
    "BOX64_DYNAREC_FASTROUND",
    "BOX64_DYNAREC_SAFEFLAGS",
    "BOX64_DYNAREC_STRONGMEM",
}


class ConfigStore:
    def __init__(
        self, system_path: Path, user_path: Path, catalog: UsageCatalog, machine_arch: str = ""
    ) -> None:
        self.catalog = catalog
        self.machine_arch = machine_arch
        self.system = RcDocument(system_path)
        self.user = RcDocument(user_path)
        self.dirty = False
        self._entries: Tuple[EntryRecord, ...] = ()
        self._entries_by_key: Dict[SectionKey, EntryRecord] = {}
        self._system_sections_by_identity: Dict[SectionIdentity, RcSection] = {}
        self._user_sections_by_identity: Dict[SectionIdentity, RcSection] = {}
        self._rebuild_entry_cache()

    def reload(self) -> None:
        self.system.reload()
        self.user.reload()
        self.dirty = False
        self._rebuild_entry_cache()

    def save(self) -> None:
        self.user.save()
        self.reload()

    def entries(self) -> Tuple[EntryRecord, ...]:
        return self._entries

    def entry_for_key(self, key: SectionKey) -> Optional[EntryRecord]:
        return self._entries_by_key.get(key)

    def _rebuild_entry_cache(self) -> None:
        self._system_sections_by_identity = self._effective_sections(self.system)
        self._user_sections_by_identity = self._effective_sections(self.user)
        identities = set(self._system_sections_by_identity)
        identities.update(self._user_sections_by_identity)

        entries: List[EntryRecord] = []
        for identity in identities:
            system_section = self._system_sections_by_identity.get(identity)
            user_section = self._user_sections_by_identity.get(identity)
            display_section = user_section or system_section
            if display_section is None:
                continue
            display_name = display_section.header if display_section else header_from_key(display_section.key)
            entries.append(EntryRecord(display_section.key, display_name, system_section, user_section))

        self._entries = tuple(
            sorted(entries, key=lambda entry: (entry.key.sort_key(), entry.display_name.lower()))
        )
        self._entries_by_key = {entry.key: entry for entry in self._entries}

    def _effective_sections(self, document: RcDocument) -> Dict[SectionIdentity, RcSection]:
        sections: Dict[SectionIdentity, RcSection] = {}
        for section in document.sections:
            if not self._section_matches_arch(section):
                continue

            identity = self._identity(section.key)
            current = sections.get(identity)
            if current is None or self._section_replaces(current, section):
                sections[identity] = section
        return sections

    def _section_matches_arch(self, section: RcSection) -> bool:
        return not section.key.arch or section.key.arch == self.machine_arch

    def _section_replaces(self, current: RcSection, candidate: RcSection) -> bool:
        if candidate.key.arch:
            return candidate.key.arch == self.machine_arch
        return not current.key.arch

    def _identity(self, key: SectionKey) -> SectionIdentity:
        return key.kind, key.name

    def _mark_dirty(self) -> None:
        self.dirty = True
        self._rebuild_entry_cache()

    def create_entry(self, header: str) -> SectionKey:
        normalized = normalize_section_input(header)
        requested_key = section_key_from_header(normalized)
        user_section = self.user.section_for_key(requested_key)
        if user_section is not None:
            return user_section.key

        section = self.user.add_section(normalized)
        self._mark_dirty()
        return section.key

    def delete_user_entry(self, key: SectionKey) -> None:
        self.user.delete_section(key)
        self._mark_dirty()

    def fork_entry(self, entry: EntryRecord) -> Tuple[SectionKey, bool]:
        section, forked = self._ensure_user_section(entry)
        if forked:
            self._mark_dirty()
        return section.key, forked

    def set_option(self, entry: EntryRecord, option_name: str, value: str) -> Tuple[SectionKey, bool]:
        section, forked = self._ensure_user_section(entry)
        new_key = self.user.set_assignment(section.key, option_name, value)
        self._mark_dirty()
        return new_key, forked

    def reset_option(self, entry: EntryRecord, option_name: str) -> SectionKey:
        user_section = entry.user_section
        if user_section is None:
            return entry.key

        system_value = None
        if entry.system_section is not None:
            system_value = entry.system_section.last_value(option_name)

        new_key = (
            self.user.set_assignment(user_section.key, option_name, system_value)
            if system_value is not None
            else self.user.remove_assignment(user_section.key, option_name)
        )
        self._mark_dirty()
        return new_key

    def unsupported_assignments(self, entry: EntryRecord) -> Tuple[Assignment, ...]:
        section = entry.user_section or entry.system_section
        return tuple(
            assignment for assignment in direct_box64_assignments(section) if assignment.key not in self.catalog
        )

    def effective_values(
        self, entry: EntryRecord
    ) -> Tuple[List[ValueState], List[ValueState], List[ValueState]]:
        states: Dict[str, ValueState] = {}

        if entry.key.kind != "shared":
            self._apply_shared(states)
            self._apply_matching_wildcards(states, entry)

        self._apply_direct(states, entry)

        active: List[ValueState] = []
        often_used: List[ValueState] = []
        defaults: List[ValueState] = []
        for option in self.catalog.options:
            state = states.get(option.name)
            if state is None:
                default_state = ValueState(
                    option=option,
                    value=option.default_value,
                    source="default",
                )
                if option.name in OFTEN_USED_OPTION_NAMES:
                    often_used.append(default_state)
                else:
                    defaults.append(default_state)
            else:
                active.append(state)

        return active, often_used, defaults

    def _ensure_user_section(self, entry: EntryRecord) -> Tuple[RcSection, bool]:
        user_section = self.user.section_for_key(entry.key)
        if user_section is not None:
            return user_section, False

        if entry.system_section is not None:
            assignments = copy_assignments(entry.system_section)
            user_section = self.user.add_section(entry.system_section.header, assignments)
            return user_section, True

        user_section = self.user.add_section(entry.display_name)
        return user_section, True

    def _apply_shared(self, states: Dict[str, ValueState]) -> None:
        for section, source in (
            (self._system_sections_by_identity.get(("shared", "*")), "shared-system"),
            (self._user_sections_by_identity.get(("shared", "*")), "shared-user"),
        ):
            if section is not None:
                self._apply_assignments(states, section, source)

    def _apply_matching_wildcards(self, states: Dict[str, ValueState], entry: EntryRecord) -> None:
        if entry.key.kind not in ("exact", "file"):
            return

        target = entry.key.name.lower()
        wildcard_identities = {
            identity
            for identity in (
                *self._system_sections_by_identity.keys(),
                *self._user_sections_by_identity.keys(),
            )
            if identity[0] == "wildcard" and identity[1] in target
        }

        for identity in sorted(wildcard_identities, key=lambda item: item[1]):
            user_section = self._user_sections_by_identity.get(identity)
            system_section = self._system_sections_by_identity.get(identity)
            if user_section is not None:
                self._apply_assignments(states, user_section, "wildcard-user")
            elif system_section is not None:
                self._apply_assignments(states, system_section, "wildcard-system")

    def _apply_direct(self, states: Dict[str, ValueState], entry: EntryRecord) -> None:
        if entry.user_section is not None:
            self._apply_assignments(states, entry.user_section, "user")
        elif entry.system_section is not None:
            self._apply_assignments(states, entry.system_section, "system")

    def _apply_assignments(
        self, states: Dict[str, ValueState], section: RcSection, source: str
    ) -> None:
        for assignment in section.assignments:
            option = self.catalog.by_name.get(assignment.key)
            if option is None:
                continue
            states[assignment.key] = ValueState(
                option=option,
                value=assignment.value,
                source=source,
            )
