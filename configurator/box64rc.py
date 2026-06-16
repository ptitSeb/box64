from __future__ import annotations

import os
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple


@dataclass(frozen=True)
class SectionKey:
    kind: str
    name: str
    arch: str = ""

    def sort_key(self) -> Tuple[int, str, str]:
        order = {"shared": 0, "exact": 1, "wildcard": 2, "file": 3}
        return (order.get(self.kind, 9), self.name.lower(), self.arch.lower())


@dataclass(frozen=True)
class Assignment:
    key: str
    value: str
    line_index: int


@dataclass(frozen=True)
class RcSection:
    header: str
    start: int
    end: int
    key: SectionKey
    assignments: Tuple[Assignment, ...]

    def last_value(self, key: str) -> Optional[str]:
        return next((item.value for item in reversed(self.assignments) if item.key == key), None)


def normalize_arch(value: Optional[str]) -> str:
    value = (value or "").strip().strip("`")
    if value == "<empty>":
        return ""
    return value.lower()


def section_key_from_header(header: str, arch: Optional[str] = None) -> SectionKey:
    name = header.strip()
    arch_value = normalize_arch(arch)
    if name == "*":
        return SectionKey("shared", "*", arch_value)
    if name.startswith("*") and name.endswith("*") and len(name) > 2:
        return SectionKey("wildcard", name[1:-1].strip().lower(), arch_value)
    if name.startswith("/"):
        return SectionKey("file", name.lower(), arch_value)
    return SectionKey("exact", name.lower(), arch_value)


def normalize_section_input(value: str) -> str:
    name = value.strip()
    if name.startswith("[") or name.endswith("]"):
        if not (name.startswith("[") and name.endswith("]")):
            raise ValueError("Invalid section name")
        name = name[1:-1].strip()

    if not name or any(char in name for char in "\r\n[]#"):
        raise ValueError("Invalid section name")
    return name


def header_from_key(key: SectionKey) -> str:
    if key.kind == "shared":
        return "*"
    if key.kind == "wildcard":
        return f"*{key.name}*"
    return key.name


def semantic_line(line: str) -> str:
    return line.rstrip("\n").split("#", 1)[0].strip()


def parse_section_header(line: str) -> Optional[str]:
    semantic = semantic_line(line)
    return semantic[1 : semantic.find("]")].strip() if semantic.startswith("[") and "]" in semantic else None


def parse_assignment(line: str) -> Optional[Tuple[str, str]]:
    semantic = semantic_line(line)
    if "=" not in semantic:
        return None
    key, value = semantic.split("=", 1)
    key = key.strip()
    return (key, value.strip()) if key else None


def render_assignment_line(original: str, key: str, value: str) -> str:
    text = original.rstrip("\n")
    newline = "\n" if original.endswith("\n") else ""
    indent = text[: len(text) - len(text.lstrip())]
    comment = ""
    comment_start = text.find("#")
    if comment_start >= 0:
        comment = "  " + text[comment_start:].strip()
    return f"{indent}{key}={value}{comment}{newline or os.linesep}"


class RcDocument:
    def __init__(self, path: Path) -> None:
        self.path = path
        self.lines: List[str] = []
        self.sections: List[RcSection] = []
        self.sections_by_key: Dict[SectionKey, RcSection] = {}
        self.reload()

    def reload(self) -> None:
        self.lines = self.path.read_text(encoding="utf-8").splitlines(True) if self.path.is_file() else []
        self._parse()

    def _parse(self) -> None:
        builders: List[Dict[str, object]] = []
        current: Optional[Dict[str, object]] = None

        for index, line in enumerate(self.lines):
            header = parse_section_header(line)
            if header is not None:
                if current is not None:
                    current["end"] = index
                current = {
                    "header": header,
                    "start": index,
                    "end": len(self.lines),
                    "assignments": [],
                }
                builders.append(current)
                continue

            if current is None:
                continue

            parsed = parse_assignment(line)
            if parsed is None:
                continue
            key, value = parsed
            current["assignments"].append(Assignment(key, value, index))

        if current is not None:
            current["end"] = len(self.lines)

        sections: List[RcSection] = []
        for builder in builders:
            assignments = tuple(builder["assignments"])
            arch = next((item.value for item in reversed(assignments) if item.key == "BOX64_ARCH"), None)
            key = section_key_from_header(str(builder["header"]), arch)
            sections.append(
                RcSection(
                    header=str(builder["header"]),
                    start=int(builder["start"]),
                    end=int(builder["end"]),
                    key=key,
                    assignments=assignments,
                )
            )

        self.sections = sections
        self.sections_by_key = {section.key: section for section in sections}

    def section_for_key(self, key: SectionKey) -> Optional[RcSection]:
        return self.sections_by_key.get(key)

    def add_section(self, header: str, assignments: Optional[Dict[str, str]] = None) -> RcSection:
        if self.lines and self.lines[-1].strip():
            self.lines.append(os.linesep)
        self.lines.append(f"[{header}]{os.linesep}")
        for key, value in (assignments or {}).items():
            self.lines.append(f"{key}={value}{os.linesep}")
        self._parse()
        return self.sections[-1]

    def delete_section(self, key: SectionKey) -> None:
        section = self.section_for_key(key)
        if section is None:
            return
        delete_end = section.end
        if section.assignments:
            delete_end = section.assignments[-1].line_index + 1
        del self.lines[section.start : delete_end]
        self._parse()

    def set_assignment(self, section_key: SectionKey, key: str, value: str) -> SectionKey:
        section = self.section_for_key(section_key)
        if section is None:
            raise KeyError(section_key)

        matching = [assignment for assignment in section.assignments if assignment.key == key]
        if matching:
            line_index = matching[-1].line_index
            self.lines[line_index] = render_assignment_line(self.lines[line_index], key, value)
            start = section.start
        else:
            insert_at = section.end
            self.lines.insert(insert_at, f"{key}={value}{os.linesep}")
            start = section.start

        self._parse()
        for updated in self.sections:
            if updated.start == start:
                return updated.key
        raise KeyError(section_key)

    def remove_assignment(self, section_key: SectionKey, key: str) -> SectionKey:
        section = self.section_for_key(section_key)
        if section is None:
            raise KeyError(section_key)

        indexes = [assignment.line_index for assignment in section.assignments if assignment.key == key]
        start = section.start
        for line_index in reversed(indexes):
            del self.lines[line_index]

        self._parse()
        for updated in self.sections:
            if updated.start == start:
                return updated.key
        return section_key

    def save(self) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True)
        data = "".join(self.lines)
        tmp_path = self.path.with_name(f"{self.path.name}.tmp")
        tmp_path.write_text(data, encoding="utf-8")
        if self.path.exists():
            mode = self.path.stat().st_mode
            os.chmod(tmp_path, mode)
        os.replace(tmp_path, self.path)


def copy_assignments(section: RcSection) -> Dict[str, str]:
    return {assignment.key: assignment.value for assignment in section.assignments}


def direct_box64_assignments(section: Optional[RcSection]) -> Iterable[Assignment]:
    return () if section is None else tuple(
        assignment for assignment in section.assignments if assignment.key.startswith("BOX64_")
    )
