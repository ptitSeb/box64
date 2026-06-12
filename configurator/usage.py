from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Tuple


PLACEHOLDER_MARKERS = (
    "XXXX",
    "YYYY",
    "ZZZZ",
    "xxx",
    "yyy",
    "zzz",
    "symbolname",
)

_EMBEDDED_USAGE_DATA: Optional[List[Dict[str, Any]]] = globals().get("_EMBEDDED_USAGE_DATA")
_EMBEDDED_USAGE_CN_DATA: Optional[Dict[str, Any]] = globals().get("_EMBEDDED_USAGE_CN_DATA")


def clean_choice_value(raw_key: str) -> str:
    value = str(raw_key).strip()
    if value.startswith("`") and value.endswith("`") and len(value) >= 2:
        value = value[1:-1]
    return "" if value == "<empty>" else value


def is_placeholder_value(value: str) -> bool:
    return bool(value) and (
        any(marker in value for marker in PLACEHOLDER_MARKERS) or value.startswith("0x") and "X" in value
    )


def localized(language: str, english: str, chinese: str) -> str:
    return (chinese or english) if language == "zh" else english


@dataclass(frozen=True)
class Choice:
    value: str
    label: str
    description_en: str
    description_cn: str
    is_default: bool
    is_placeholder: bool

    def description(self, language: str) -> str:
        return localized(language, self.description_en, self.description_cn)


@dataclass(frozen=True)
class EnvOption:
    name: str
    description_en: str
    description_cn: str
    choices: Tuple[Choice, ...]

    @property
    def allows_custom_value(self) -> bool:
        return any(choice.is_placeholder for choice in self.choices)

    @property
    def default_value(self) -> str:
        return next((choice.value for choice in self.choices if choice.is_default), "")

    def description(self, language: str) -> str:
        return localized(language, self.description_en, self.description_cn)


class UsageCatalog:
    def __init__(self, options: Iterable[EnvOption]) -> None:
        self.options: Tuple[EnvOption, ...] = tuple(options)
        self.by_name: Dict[str, EnvOption] = {option.name: option for option in self.options}

    def __contains__(self, name: str) -> bool:
        return name in self.by_name


def find_usage_paths(start: Optional[Path] = None) -> Tuple[Path, Path]:
    here = (start or Path(__file__)).resolve()
    for parent in (here.parent, *here.parents):
        for directory in (parent / "docs" / "gen", parent / "share" / "box64"):
            usage = directory / "usage.json"
            usage_cn = directory / "usage_cn.json"
            if usage.is_file() and usage_cn.is_file():
                return usage, usage_cn

    raise FileNotFoundError("Could not find usage.json and usage_cn.json")


def load_default_usage_catalog() -> UsageCatalog:
    if _EMBEDDED_USAGE_DATA is not None and _EMBEDDED_USAGE_CN_DATA is not None:
        return load_usage_catalog_from_data(_EMBEDDED_USAGE_DATA, _EMBEDDED_USAGE_CN_DATA)

    usage_path, usage_cn_path = find_usage_paths()
    return load_usage_catalog(usage_path, usage_cn_path)


def load_usage_catalog(usage_path: Path, usage_cn_path: Path) -> UsageCatalog:
    with usage_path.open("r", encoding="utf-8") as file:
        english_data = json.load(file)
    with usage_cn_path.open("r", encoding="utf-8") as file:
        chinese_data = json.load(file)
    return load_usage_catalog_from_data(english_data, chinese_data)


def load_usage_catalog_from_data(
    english_data: Iterable[Dict[str, Any]], chinese_data: Dict[str, Any]
) -> UsageCatalog:
    options: List[EnvOption] = []
    for entry in english_data:
        if not entry.get("configurator", False):
            continue

        name = entry["name"]
        chinese_entry = chinese_data.get(name, {})
        chinese_choices = chinese_entry.get("options", {})
        choices = []

        for option in entry.get("options", []):
            raw_key = str(option["key"])
            value = clean_choice_value(raw_key)
            choices.append(
                Choice(
                    value=value,
                    label="<empty>" if value == "" else value,
                    description_en=option.get("description", ""),
                    description_cn=chinese_choices.get(raw_key, ""),
                    is_default=bool(option.get("default", False)),
                    is_placeholder=is_placeholder_value(value),
                )
            )

        options.append(
            EnvOption(
                name=name,
                description_en=entry.get("description", ""),
                description_cn=chinese_entry.get("description", ""),
                choices=tuple(choices),
            )
        )

    return UsageCatalog(options)
