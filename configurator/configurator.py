#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import locale
import platform
import struct
import sys
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Tuple

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parent))

import gi

gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gio, Gdk, GLib, Gtk, Pango

try:
    from .model import ConfigStore, EntryRecord, ValueState, box64_arch_from_machine
    from .usage import EnvOption, load_default_usage_catalog, load_usage_catalog
except ImportError:
    from model import ConfigStore, EntryRecord, ValueState, box64_arch_from_machine
    from usage import EnvOption, load_default_usage_catalog, load_usage_catalog


APP_ID = "org.box64.Configurator"


_EMBEDDED_TEXTS: Optional[Dict[str, Dict[str, str]]] = globals().get("_EMBEDDED_TEXTS")
_EMBEDDED_UI: Optional[str] = globals().get("_EMBEDDED_UI")


def load_texts() -> Dict[str, Dict[str, str]]:
    if _EMBEDDED_TEXTS is not None:
        return _EMBEDDED_TEXTS

    directory = Path(__file__).resolve().parent
    texts: Dict[str, Dict[str, str]] = {}
    for language in ("en", "zh"):
        path = directory / f"text_{language}.json"
        with path.open("r", encoding="utf-8") as file:
            texts[language] = json.load(file)
    return texts


def load_ui(builder: Gtk.Builder) -> None:
    if _EMBEDDED_UI is not None:
        builder.add_from_string(_EMBEDDED_UI)
        return
    builder.add_from_file(str(Path(__file__).resolve().parent / "configurator.ui"))


TEXT = load_texts()


CSS = b"""
.configurator-window:backdrop,
.configurator-window *:backdrop {
    color: @theme_fg_color;
}
headerbar:backdrop {
    background-color: @theme_bg_color;
    background-image: none;
    color: @theme_fg_color;
}
headerbar:backdrop button,
headerbar:backdrop combobox,
headerbar:backdrop label,
headerbar:backdrop image {
    color: @theme_fg_color;
    opacity: 1;
    -gtk-icon-effect: none;
}
.sidebar {
    background: @theme_base_color;
}
.entry-title {
    font-weight: 600;
}
.entry-subtitle {
    opacity: 0.72;
    font-size: 0.88em;
}
.detail-title {
    font-size: 1.45em;
    font-weight: 700;
}
.section-heading {
    font-size: 1.08em;
    font-weight: 700;
    margin-top: 18px;
    margin-bottom: 6px;
}
.option-row {
    background: transparent;
    border-left: 3px solid transparent;
    border-bottom: 1px solid alpha(@theme_fg_color, 0.18);
}
.option-row:hover {
    background: alpha(@theme_selected_bg_color, 0.10);
}
.option-row:focus {
    box-shadow: inset 0 0 0 1px alpha(@theme_selected_bg_color, 0.55);
}
.option-row.selected-option {
    background: alpha(@theme_selected_bg_color, 0.18);
    border-left-color: @theme_selected_bg_color;
    border-bottom-color: alpha(@theme_selected_bg_color, 0.40);
}
.option-row.selected-option:hover {
    background: alpha(@theme_selected_bg_color, 0.24);
}
.option-row.read-only-option:hover {
    background: transparent;
}
.option-row.read-only-option.default-row:hover {
    background: alpha(@theme_fg_color, 0.035);
}
.default-row {
    color: alpha(@theme_fg_color, 0.62);
    background: alpha(@theme_fg_color, 0.035);
}
.choice-doc {
    opacity: 0.78;
    font-size: 0.9em;
}
.status-note {
    border-radius: 6px;
    padding: 8px 10px;
    background: alpha(@theme_selected_bg_color, 0.12);
}
.badge {
    border-radius: 999px;
    padding: 2px 8px;
    background: alpha(@theme_fg_color, 0.12);
    font-size: 0.85em;
}
.option-value {
    font-size: 1.35em;
    font-weight: 700;
}
.selected-option {
    background: alpha(@theme_selected_bg_color, 0.10);
}
"""


def tr(language: str, key: str) -> str:
    return TEXT[language].get(key, TEXT["en"][key])


def detect_language() -> str:
    code = locale.getlocale()[0] or ""
    return "zh" if code.lower().startswith("zh") else "en"


def add_style(widget: Gtk.Widget, *classes: str) -> Gtk.Widget:
    style = widget.get_style_context()
    for css_class in classes:
        style.add_class(css_class)
    return widget


def set_style(widget: Gtk.Widget, css_class: str, active: bool) -> None:
    style = widget.get_style_context()
    (style.add_class if active else style.remove_class)(css_class)


def make_label(
    text: str = "",
    css_class: Optional[str] = None,
    *,
    selectable: bool = False,
    wrap: bool = False,
    ellipsize: bool = False,
) -> Gtk.Label:
    label = Gtk.Label(label=text)
    label.set_xalign(0)
    label.set_selectable(selectable)
    label.set_line_wrap(wrap)
    if ellipsize:
        label.set_ellipsize(Pango.EllipsizeMode.END)
    if css_class:
        add_style(label, css_class)
    return label


def default_system_rc() -> Path:
    etc_path = Path("/etc/box64.box64rc")
    if etc_path.is_file():
        return etc_path
    repo_sample = Path(__file__).resolve().parent.parent / "system" / "box64.box64rc"
    return repo_sample


def default_user_rc() -> Path:
    return Path.home() / ".box64rc"


def detect_supported_executable(path: Path) -> Optional[str]:
    with path.open("rb") as file:
        header = file.read(64)

        if len(header) >= 20 and header.startswith(b"\x7fELF"):
            elf_class = header[4]
            elf_data = header[5]
            if elf_data == 1:
                machine = struct.unpack_from("<H", header, 18)[0]
            elif elf_data == 2:
                machine = struct.unpack_from(">H", header, 18)[0]
            else:
                return None

            if elf_class == 1 and machine == 3:
                return "x86 ELF"
            if elf_class == 2 and machine == 62:
                return "x86_64 ELF"
            return None

        if len(header) >= 64 and header.startswith(b"MZ"):
            pe_offset = struct.unpack_from("<I", header, 0x3C)[0]
            if pe_offset > 1024 * 1024:
                return None
            file.seek(pe_offset)
            pe_header = file.read(6)
            if len(pe_header) < 6 or not pe_header.startswith(b"PE\0\0"):
                return None

            machine = struct.unpack_from("<H", pe_header, 4)[0]
            if machine == 0x014C:
                return "x86 PE"
            if machine == 0x8664:
                return "x86_64 PE"

    return None


def dropped_executable_entry(path: Path) -> str:
    resolved = path.expanduser().resolve(strict=True)
    if not resolved.is_file():
        raise ValueError(f"Not a file: {path}")
    if detect_supported_executable(resolved) is None:
        raise ValueError(f"Unsupported file: {path}")
    return resolved.name


class EntryRow(Gtk.ListBoxRow):
    def __init__(self, entry: EntryRecord, language: str) -> None:
        super().__init__()
        self.entry = entry
        self.language = language
        self.add_events(Gdk.EventMask.BUTTON_PRESS_MASK)

        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=3)
        box.set_border_width(8)
        self.event_box = Gtk.EventBox()
        self.event_box.set_visible_window(False)
        self.event_box.set_above_child(True)
        self.event_box.add_events(Gdk.EventMask.BUTTON_PRESS_MASK)

        self.title = make_label(css_class="entry-title", ellipsize=True)
        box.pack_start(self.title, False, False, 0)

        self.subtitle = make_label(css_class="entry-subtitle", ellipsize=True)
        box.pack_start(self.subtitle, False, False, 0)

        self.event_box.add(box)
        self.add(self.event_box)
        self.update(entry, language)

    def update(self, entry: EntryRecord, language: str) -> None:
        self.entry = entry
        self.language = language
        self.title.set_text(self._title())
        self.subtitle.set_text(self._subtitle())

    def _title(self) -> str:
        if self.entry.key.kind == "shared":
            return "[*]"
        return self.entry.display_name

    def _subtitle(self) -> str:
        pieces: List[str] = []
        if self.entry.key.kind == "wildcard":
            pieces.append("*pattern*")
        elif self.entry.key.kind == "file":
            pieces.append("file")
        if self.entry.arch:
            pieces.append(self.entry.arch)
        pieces.append(tr(self.language, f"source_{self.entry.editable_source}"))
        return " · ".join(pieces)


class OptionRow(Gtk.EventBox):
    KEY_RETURN = 65293
    KEY_KP_ENTER = 65421
    KEY_SPACE = 32

    def __init__(self, owner: "ConfiguratorWindow", option: EnvOption) -> None:
        super().__init__()
        self.owner = owner
        self.option = option
        self.entry: Optional[EntryRecord] = None
        self.state: Optional[ValueState] = None
        self.language = ""
        self.is_selected = False

        add_style(self, "option-row")
        self.set_visible_window(True)
        self.set_can_focus(True)
        self.connect("button-press-event", self._on_button_press)
        self.connect("key-press-event", self._on_key_press)

        box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=14)
        box.set_border_width(14)
        self.add(box)

        text_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=7)
        box.pack_start(text_box, True, True, 0)

        title_line = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        text_box.pack_start(title_line, False, False, 0)

        self.name = make_label(option.name, "entry-title", selectable=True)
        title_line.pack_start(self.name, False, False, 0)

        self.badge = make_label(css_class="badge")
        title_line.pack_start(self.badge, False, False, 0)

        self.description = make_label(css_class="choice-doc", ellipsize=True)
        text_box.pack_start(self.description, False, False, 0)

        value_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=10)
        box.pack_end(value_box, False, False, 0)

        self.value = make_label(css_class="option-value", selectable=True, ellipsize=True)
        self.value.set_xalign(1)
        self.value.set_max_width_chars(28)
        value_box.pack_start(self.value, False, False, 0)

        self.chevron = Gtk.Image.new_from_icon_name("pan-end-symbolic", Gtk.IconSize.MENU)
        value_box.pack_start(self.chevron, False, False, 0)

    def update(
        self,
        entry: EntryRecord,
        state: ValueState,
        default_row: bool,
        language: str,
        selected: bool,
    ) -> None:
        self.entry = entry
        self.state = state

        set_style(self, "default-row", default_row)
        editable = entry.user_section is not None
        selected = selected and editable
        set_style(self, "read-only-option", not editable)
        self.set_can_focus(editable)
        self.chevron.set_visible(editable)

        source_label = tr(language, f"source_{state.source}")
        if state.source == "default" and not state.value:
            value_label = tr(language, "default_dynamic")
        else:
            value_label = state.value if state.value != "" else "<empty>"
        self.badge.set_text(source_label)
        self.value.set_text(value_label)

        if language != self.language:
            self.language = language
            self.description.set_text(self.option.description(language))
        self.set_tooltip_text(tr(language, "edit") if editable else None)

        self.set_selected(selected)

    def set_selected(self, selected: bool) -> None:
        set_style(self, "selected-option", selected)
        self.is_selected = selected
        self.chevron.set_from_icon_name(
            "pan-down-symbolic" if selected else "pan-end-symbolic", Gtk.IconSize.MENU
        )

    def _activate(self) -> None:
        if self.entry is None or self.state is None or self.entry.user_section is None:
            return
        self.owner.toggle_option(self.entry, self.state.option.name)

    def _on_button_press(self, _widget: Gtk.Widget, event) -> bool:
        if event.button != 1 or self.entry is None or self.entry.user_section is None:
            return False
        self.grab_focus()
        self._activate()
        return True

    def _on_key_press(self, _widget: Gtk.Widget, event) -> bool:
        if (
            event.keyval not in (self.KEY_RETURN, self.KEY_KP_ENTER, self.KEY_SPACE)
            or self.entry is None
            or self.entry.user_section is None
        ):
            return False
        self._activate()
        return True


class ConfiguratorWindow(Gtk.ApplicationWindow):
    def __init__(self, app: Gtk.Application, store: ConfigStore) -> None:
        super().__init__(application=app)
        self.store = store
        self.language = detect_language()
        self.get_style_context().add_class("configurator-window")
        self.current_key = None
        self.rows_by_key: Dict[object, EntryRow] = {}
        self.status_context = 0
        self._filter_text = ""
        self._entry_filter_mode = "user"
        self._building = False
        self._pending_refresh_key = None
        self._pending_status = ""
        self._pending_rebuild_entries = False
        self._pending_refresh_source = 0
        self.option_rows: Dict[str, OptionRow] = {}
        self.selected_option_name: Optional[str] = None
        self.selected_option_editor: Optional[Gtk.Widget] = None
        self.entry_menu: Optional[Gtk.Menu] = None

        self.set_default_size(1120, 740)
        self.set_title(tr(self.language, "title"))
        self.connect("delete-event", self._on_delete_event)
        self.connect("key-press-event", self._on_key_press)

        self._install_css()
        self._build_ui()
        self._refresh_view()
        self.show_all()

    def _install_css(self) -> None:
        provider = Gtk.CssProvider()
        provider.load_from_data(CSS)
        Gtk.StyleContext.add_provider_for_screen(
            self.get_screen(), provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

    def _build_ui(self) -> None:
        builder = Gtk.Builder()
        load_ui(builder)

        self.header = builder.get_object("header")
        self.set_titlebar(self.header)
        self.header.props.title = tr(self.language, "title")

        root = builder.get_object("root_box")
        self.add(root)

        self.new_button = builder.get_object("new_button")
        self.reload_button = builder.get_object("reload_button")
        self.save_button = builder.get_object("save_button")
        self.language_combo = builder.get_object("language_combo")
        self.search = builder.get_object("search_entry")
        self.mode_box = builder.get_object("mode_box")
        self.user_button = builder.get_object("user_button")
        self.all_button = builder.get_object("all_button")
        self.status = builder.get_object("statusbar")

        list_scroll = builder.get_object("list_scroll")
        list_scroll.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        for child in list_scroll.get_children():
            list_scroll.remove(child)
        self.entry_list = Gtk.ListBox()
        self.entry_list.set_selection_mode(Gtk.SelectionMode.SINGLE)
        list_scroll.add(self.entry_list)

        detail_scroll = builder.get_object("detail_scroll")
        detail_scroll.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        for child in detail_scroll.get_children():
            detail_scroll.remove(child)
        self.detail_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        self.detail_box.set_border_width(18)
        self.detail_header = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        self.effective_group = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.often_used_group = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.unsupported_group = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.defaults_group = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.detail_box.pack_start(self.detail_header, False, False, 0)
        self.detail_box.pack_start(self.effective_group, False, False, 0)
        self.detail_box.pack_start(self.often_used_group, False, False, 0)
        self.detail_box.pack_start(self.unsupported_group, False, False, 0)
        self.detail_box.pack_start(self.defaults_group, False, False, 0)
        detail_scroll.add(self.detail_box)

        self._configure_header_button(self.new_button, "document-new-symbolic", "new", self._on_new_entry)
        self.new_button.set_label(tr(self.language, "new"))
        self.new_button.set_always_show_image(True)
        self._configure_header_button(self.reload_button, "view-refresh-symbolic", "reload", self._on_reload)
        self._configure_header_button(self.save_button, "document-save-symbolic", "save", self._on_save)
        save_action = Gio.SimpleAction.new("save", None)
        save_action.connect("activate", self._on_save_action)
        self.add_action(save_action)
        self.get_application().set_accels_for_action("win.save", ["<Primary>s"])

        self.language_combo.append("en", "English")
        self.language_combo.append("zh", "中文")
        self.language_combo.set_active_id(self.language)
        self.language_combo.connect("changed", self._on_language_changed)

        self.search.set_can_focus(True)
        self.search.set_placeholder_text(tr(self.language, "search"))
        self.search.connect("search-changed", self._on_search_changed)

        self.user_button.set_can_focus(True)
        self.all_button.set_can_focus(True)
        self.user_button.set_label(tr(self.language, "filter_user"))
        self.all_button.set_label(tr(self.language, "filter_all"))
        self.user_button.connect("toggled", self._on_mode_toggled, "user")
        self.all_button.connect("toggled", self._on_mode_toggled, "all")

        self.entry_list.set_can_focus(True)
        self.entry_list.add_events(Gdk.EventMask.BUTTON_PRESS_MASK)
        self.entry_list.set_filter_func(self._filter_entry_row)
        self.entry_list.connect("row-selected", self._on_entry_selected)
        self.entry_list.connect("button-press-event", self._on_entry_list_button_press)

        self.language_combo.set_can_focus(True)

        self.status_context = self.status.get_context_id("main")
        self._enable_file_drop(self, root, self.entry_list, self.detail_box, list_scroll, detail_scroll)

    def _configure_header_button(
        self, button: Gtk.Button, icon: str, tooltip_key: str, callback
    ) -> None:
        button.set_image(Gtk.Image.new_from_icon_name(icon, Gtk.IconSize.BUTTON))
        button.set_tooltip_text(tr(self.language, tooltip_key))
        button.connect("clicked", callback)

    def _enable_file_drop(self, *widgets: Gtk.Widget) -> None:
        for widget in widgets:
            widget.drag_dest_set(Gtk.DestDefaults.ALL, [], Gdk.DragAction.COPY)
            widget.drag_dest_add_uri_targets()
            widget.connect("drag-data-received", self._on_drag_data_received)

    def _on_drag_data_received(
        self,
        _widget: Gtk.Widget,
        context: Gdk.DragContext,
        _x: int,
        _y: int,
        selection: Gtk.SelectionData,
        _info: int,
        time_: int,
    ) -> None:
        keys = []
        errors = []
        for uri in selection.get_uris() or ():
            path_text = Gio.File.new_for_uri(uri).get_path()
            if path_text is None:
                errors.append(f"Unsupported drop URI: {uri}")
                continue

            try:
                keys.append(self.store.create_entry(dropped_executable_entry(Path(path_text))))
            except Exception as error:
                errors.append(str(error))

        Gtk.drag_finish(context, bool(keys), False, time_)
        if keys:
            self.current_key = keys[-1]
            self.selected_option_name = None
            self._refresh_view(self.current_key)
            if self.store.dirty:
                self._push_status(tr(self.language, "dirty"))
        elif errors:
            self._show_error(ValueError("\n".join(errors)))

    def _on_entry_list_button_press(self, _listbox: Gtk.ListBox, event) -> bool:
        if event.button != 3:
            return False

        row = self.entry_list.get_row_at_y(int(event.y))
        if not isinstance(row, EntryRow):
            return False

        return self._show_entry_menu_for_row(row, event)

    def _on_entry_button_press(self, row: EntryRow, event) -> bool:
        if event.button != 3:
            return False

        return self._show_entry_menu_for_row(row, event)

    def _show_entry_menu_for_row(self, row: EntryRow, event) -> bool:
        self._show_entry_menu(row.entry, event)
        return True

    def _show_entry_menu(self, entry: EntryRecord, event) -> None:
        menu = Gtk.Menu()
        if entry.user_section is not None:
            item = Gtk.MenuItem(label=tr(self.language, "delete_action"))
            item.connect("activate", lambda _item: self._delete_entry(entry))
        elif entry.system_section is not None:
            item = Gtk.MenuItem(label=tr(self.language, "fork_action"))
            item.connect("activate", lambda _item: self._fork_entry(entry))
        else:
            return

        menu.append(item)
        menu.show_all()
        self.entry_menu = menu
        menu.attach_to_widget(self, None)
        if hasattr(menu, "popup_at_pointer"):
            menu.popup_at_pointer(event)
        else:
            menu.popup(None, None, None, None, event.button, event.time)

    def _filter_entry_row(self, row: EntryRow) -> bool:
        if self._entry_filter_mode == "user" and row.entry.user_section is None:
            return False
        text = self._filter_text
        if not text:
            return True
        title = "[*]" if row.entry.key.kind == "shared" else row.entry.display_name
        haystack = f"{title} {row.entry.display_name} {row.entry.key.arch}".lower()
        return text in haystack

    def _refresh_entries(self, select_key=None) -> None:
        self._building = True
        selected_key = select_key if select_key is not None else self.current_key
        for child in self.entry_list.get_children():
            self.entry_list.remove(child)

        self.rows_by_key.clear()
        for entry in self.store.entries():
            row = EntryRow(entry, self.language)
            row.connect("button-press-event", self._on_entry_button_press)
            row.event_box.connect(
                "button-press-event",
                lambda _widget, event, row=row: self._on_entry_button_press(row, event),
            )
            self.rows_by_key[entry.key] = row
            self.entry_list.add(row)

        self.entry_list.show_all()
        self.entry_list.invalidate_filter()

        selected_row = self.rows_by_key.get(selected_key)
        if selected_row is not None:
            self.entry_list.select_row(selected_row)
            self.current_key = selected_key
        else:
            self._clear_entry_selection()

        self._building = False
        self._update_actions()

    def _refresh_entry_row(self, key) -> None:
        row = self.rows_by_key.get(key)
        entry = self.store.entry_for_key(key)
        if row is None or entry is None:
            self._refresh_entries(select_key=key)
            return

        row.update(entry, self.language)
        row.show_all()
        self.entry_list.invalidate_filter()

    def _refresh_view(self, select_key=None) -> None:
        self._refresh_entries(select_key=select_key)
        self._refresh_details()

    def _refresh_details(self) -> None:
        self.selected_option_editor = None
        self.option_rows.clear()
        for container in (
            self.detail_header,
            self.effective_group,
            self.often_used_group,
            self.unsupported_group,
            self.defaults_group,
        ):
            for child in container.get_children():
                child.destroy()

        self.effective_group.hide()
        self.often_used_group.hide()
        self.unsupported_group.hide()
        self.defaults_group.hide()

        if self.current_key is None:
            self.detail_header.pack_start(
                make_label(tr(self.language, "select_entry")), False, False, 0
            )
            self.detail_header.show_all()
            return

        entry = self.store.entry_for_key(self.current_key)
        if entry is None:
            return

        self._add_entry_header(entry)
        active, often_used, defaults = self.store.effective_values(entry)
        visible_names = {state.option.name for state in (*active, *often_used, *defaults)}
        if self.selected_option_name not in visible_names:
            self.selected_option_name = None
        self._add_option_group(
            self.effective_group, tr(self.language, "effective"), active, entry, default_rows=False
        )
        if often_used:
            self._add_option_group(
                self.often_used_group,
                tr(self.language, "often_used"),
                often_used,
                entry,
                default_rows=True,
            )
        self._add_unsupported(entry)
        self._add_option_group(
            self.defaults_group, tr(self.language, "defaults"), defaults, entry, default_rows=True
        )

        self.detail_header.show_all()
        self.effective_group.show_all()
        if often_used:
            self.often_used_group.show_all()
        else:
            self.often_used_group.hide()
        self.defaults_group.show_all()
        if self.unsupported_group.get_children():
            self.unsupported_group.show_all()
        else:
            self.unsupported_group.hide()
        self._update_actions()

    def _add_entry_header(self, entry: EntryRecord) -> None:
        title = "[*]" if entry.key.kind == "shared" else entry.display_name
        self.detail_header.pack_start(
            make_label(title, "detail-title", selectable=True), False, False, 0
        )

        source_text = (
            tr(self.language, "user_note") if entry.user_section else tr(self.language, "system_note")
        )
        if entry.system_section is None:
            source_text = tr(self.language, "user_note")
        if entry.arch:
            source_text = f"{source_text}  BOX64_ARCH={entry.arch}"

        self.detail_header.pack_start(
            make_label(source_text, "status-note", wrap=True), False, False, 0
        )

    def _build_option_editor(self, entry: EntryRecord, state: ValueState) -> Gtk.Box:
        option = state.option
        panel = add_style(Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8), "status-note")
        panel.pack_start(make_label(option.name, "entry-title"), False, False, 0)
        panel.pack_start(make_label(option.description(self.language), wrap=True), False, False, 0)

        controls = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        panel.pack_start(controls, False, False, 0)

        if option.allows_custom_value:
            value_entry = Gtk.Entry()
            value_entry.set_text(state.value)
            value_entry.set_width_chars(28)
            controls.pack_start(value_entry, False, False, 0)

            apply_button = Gtk.Button(label=tr(self.language, "apply"))
            apply_button.connect(
                "clicked",
                lambda _button: self.change_option(entry, option.name, value_entry.get_text()),
            )
            controls.pack_start(apply_button, False, False, 0)
        else:
            combo = Gtk.ComboBoxText()
            values: List[str] = []
            active_index = -1
            for index, choice in enumerate(option.choices):
                combo.append(str(index), choice.label)
                values.append(choice.value)
                if choice.value == state.value:
                    active_index = index

            if active_index < 0 and state.value:
                active_index = len(values)
                values.append(state.value)
                combo.append(str(active_index), state.value)

            combo.set_active(active_index)
            combo.connect("changed", self._on_editor_choice_changed, entry, option.name, values)
            controls.pack_start(combo, False, False, 0)

        reset = Gtk.Button(label=tr(self.language, "reset"))
        reset.set_sensitive(entry.user_section is not None)
        reset.connect("clicked", lambda _button: self.reset_option(entry, option.name))
        controls.pack_start(reset, False, False, 0)

        choice_lines = []
        for choice in option.choices:
            suffix = f" [{tr(self.language, 'default_marker')}]" if choice.is_default else ""
            choice_lines.append(f"{choice.label}: {choice.description(self.language)}{suffix}")

        if choice_lines:
            panel.pack_start(
                make_label("\n".join(choice_lines), "choice-doc", wrap=True), False, False, 0
            )

        return panel

    def _add_option_group(
        self,
        container: Gtk.Box,
        title_text: str,
        states: Sequence[ValueState],
        entry: EntryRecord,
        default_rows: bool,
    ) -> None:
        container.pack_start(
            make_label(f"{title_text} ({len(states)})", "section-heading"), False, False, 0
        )

        if not states:
            return

        for state in states:
            selected = entry.user_section is not None and state.option.name == self.selected_option_name
            row = self._option_row_for(
                entry,
                state,
                default_rows,
                selected=selected,
            )
            container.pack_start(row, False, False, 0)
            if selected:
                self._insert_option_editor_after(container, row, entry, state)

    def _option_row_for(
        self, entry: EntryRecord, state: ValueState, default_row: bool, selected: bool
    ) -> OptionRow:
        row = self.option_rows.get(state.option.name)
        if row is None:
            row = OptionRow(self, state.option)
            self.option_rows[state.option.name] = row
        row.update(entry, state, default_row, self.language, selected)
        return row

    def _add_unsupported(self, entry: EntryRecord) -> None:
        unsupported = self.store.unsupported_assignments(entry)
        if not unsupported:
            return

        self.unsupported_group.pack_start(
            make_label(
                f"{tr(self.language, 'unsupported')} ({len(unsupported)})", "section-heading"
            ),
            False,
            False,
            0,
        )
        self.unsupported_group.pack_start(
            make_label(tr(self.language, "unsupported_help"), wrap=True), False, False, 0
        )

        for assignment in unsupported:
            self.unsupported_group.pack_start(
                make_label(
                    f"{assignment.key}={assignment.value}", "choice-doc", selectable=True
                ),
                False,
                False,
                0,
            )

    def toggle_option(self, entry: EntryRecord, option_name: str) -> None:
        if entry.user_section is None:
            return
        if entry.key != self.current_key:
            self.current_key = entry.key
            self.selected_option_name = option_name
            self._refresh_details()
            return

        row = self.option_rows.get(option_name)
        state = self._state_for_option(entry, option_name)
        if row is None or state is None:
            self.selected_option_name = option_name
            self._refresh_details()
            return

        if self.selected_option_name == option_name:
            self.selected_option_name = None
            row.set_selected(False)
            self._remove_selected_option_editor()
            return

        previous_name = self.selected_option_name
        if previous_name is not None:
            previous_row = self.option_rows.get(previous_name)
            if previous_row is not None:
                previous_row.set_selected(False)
        self._remove_selected_option_editor()

        self.selected_option_name = option_name
        row.set_selected(True)
        self._insert_option_editor_after(row.get_parent(), row, entry, state)

    def _state_for_option(self, entry: EntryRecord, option_name: str) -> Optional[ValueState]:
        active, often_used, defaults = self.store.effective_values(entry)
        for state in (*active, *often_used, *defaults):
            if state.option.name == option_name:
                return state
        return None

    def _remove_selected_option_editor(self) -> None:
        editor = self.selected_option_editor
        self.selected_option_editor = None
        if editor is None:
            return
        parent = editor.get_parent()
        if parent is not None:
            parent.remove(editor)
        editor.destroy()

    def _insert_option_editor_after(
        self, container: Gtk.Box, row: OptionRow, entry: EntryRecord, state: ValueState
    ) -> None:
        panel = self._build_option_editor(entry, state)
        self.selected_option_editor = panel
        container.pack_start(panel, False, False, 8)
        try:
            position = container.get_children().index(row) + 1
        except ValueError:
            position = -1
        if position >= 0:
            container.reorder_child(panel, position)
        panel.show_all()

    def change_option(self, entry: EntryRecord, option_name: str, value: str) -> None:
        if entry.user_section is None:
            return
        self.selected_option_name = option_name
        try:
            new_key, forked = self.store.set_option(entry, option_name, value)
        except Exception as error:
            self._show_error(error)
            return

        self._queue_refresh(
            new_key,
            tr(self.language, "forked" if forked else "dirty"),
            rebuild_entries=(new_key != entry.key),
        )

    def reset_option(self, entry: EntryRecord, option_name: str) -> None:
        self.selected_option_name = option_name
        try:
            new_key = self.store.reset_option(entry, option_name)
        except Exception as error:
            self._show_error(error)
            return
        self._queue_refresh(new_key, tr(self.language, "dirty"), rebuild_entries=(new_key != entry.key))

    def _queue_refresh(self, key, status: str, rebuild_entries: bool = False) -> None:
        self.current_key = key
        self._pending_refresh_key = key
        self._pending_status = status
        self._pending_rebuild_entries = self._pending_rebuild_entries or rebuild_entries
        if not self._pending_refresh_source:
            self._pending_refresh_source = GLib.idle_add(self._flush_queued_refresh)

    def _flush_queued_refresh(self) -> bool:
        key = self._pending_refresh_key
        status = self._pending_status
        rebuild_entries = self._pending_rebuild_entries
        self._pending_refresh_key = None
        self._pending_status = ""
        self._pending_rebuild_entries = False
        self._pending_refresh_source = 0
        if rebuild_entries:
            self._refresh_entries(select_key=key)
        else:
            self._refresh_entry_row(key)
        self._refresh_details()
        if status:
            self._push_status(status)
        return GLib.SOURCE_REMOVE

    def _on_editor_choice_changed(
        self,
        combo: Gtk.ComboBoxText,
        entry: EntryRecord,
        option_name: str,
        values: Sequence[str],
    ) -> None:
        index = combo.get_active()
        if index < 0 or index >= len(values):
            return
        self.change_option(entry, option_name, values[index])

    def _on_search_changed(self, search: Gtk.SearchEntry) -> None:
        self._filter_text = search.get_text().strip().lower()
        self.entry_list.invalidate_filter()

    def _on_mode_toggled(self, button: Gtk.ToggleButton, mode: str) -> None:
        if not button.get_active():
            return
        self._entry_filter_mode = mode
        if mode == "user":
            self.all_button.set_active(False)
        else:
            self.user_button.set_active(False)
        self.entry_list.invalidate_filter()
        self._refresh_details()

    def _clear_entry_selection(self) -> None:
        self.entry_list.unselect_all()
        self.current_key = None
        self.selected_option_name = None

    def _on_entry_selected(self, _listbox: Gtk.ListBox, row: Optional[EntryRow]) -> None:
        if self._building or row is None:
            return
        is_new_entry = row.entry.key != self.current_key
        self.current_key = row.entry.key
        if is_new_entry:
            self.selected_option_name = None
            self._clear_status()
        self._refresh_details()

    def _on_key_press(self, _widget: Gtk.Widget, event) -> bool:
        if event.keyval != Gdk.KEY_Escape:
            return False
        self._clear_entry_selection()
        self._clear_status()
        self._refresh_details()
        return True

    def _on_language_changed(self, combo: Gtk.ComboBoxText) -> None:
        active = combo.get_active_id()
        if active not in ("en", "zh"):
            return
        self.language = active
        self.header.props.title = tr(self.language, "title")
        self.set_title(tr(self.language, "title"))
        self.search.set_placeholder_text(tr(self.language, "search"))
        self.user_button.set_label(tr(self.language, "filter_user"))
        self.all_button.set_label(tr(self.language, "filter_all"))
        self.new_button.set_label(tr(self.language, "new"))
        self.new_button.set_tooltip_text(tr(self.language, "new"))
        self.reload_button.set_tooltip_text(tr(self.language, "reload"))
        self.save_button.set_tooltip_text(tr(self.language, "save"))
        self._refresh_view()

    def _on_new_entry(self, _button: Gtk.Button) -> None:
        dialog = Gtk.Dialog(
            title=tr(self.language, "new_title"),
            transient_for=self,
            modal=True,
            destroy_with_parent=True,
        )
        dialog.add_button(tr(self.language, "cancel"), Gtk.ResponseType.CANCEL)
        dialog.add_button(tr(self.language, "create"), Gtk.ResponseType.OK)
        dialog.set_default_response(Gtk.ResponseType.OK)

        content = dialog.get_content_area()
        content.set_spacing(8)
        content.set_border_width(12)
        content.pack_start(make_label(tr(self.language, "new_prompt")), False, False, 0)
        entry = Gtk.Entry()
        entry.set_activates_default(True)
        content.pack_start(entry, False, False, 0)
        content.pack_start(
            make_label(tr(self.language, "new_drop_hint"), "choice-doc", wrap=True), False, False, 0
        )
        content.pack_start(
            make_label(tr(self.language, "new_global_hint"), "choice-doc", wrap=True), False, False, 0
        )
        dialog.show_all()

        response = dialog.run()
        name = entry.get_text().strip()
        dialog.destroy()
        if response != Gtk.ResponseType.OK or not name:
            return

        try:
            new_key = self.store.create_entry(name)
        except Exception as error:
            self._show_error(error)
            return

        self.current_key = new_key
        self._refresh_view(new_key)
        if self.store.dirty:
            self._push_status(tr(self.language, "dirty"))

    def _fork_entry(self, entry: EntryRecord) -> None:
        try:
            new_key, forked = self.store.fork_entry(entry)
        except Exception as error:
            self._show_error(error)
            return

        self.current_key = new_key
        self.selected_option_name = None
        self._refresh_view(new_key)
        if forked:
            self._push_status(tr(self.language, "forked"))

    def _delete_entry(self, entry: EntryRecord) -> None:
        if entry.user_section is None:
            return
        if self._message(
            Gtk.MessageType.WARNING,
            tr(self.language, "delete_title"),
            tr(self.language, "delete_body"),
            ((tr(self.language, "cancel"), Gtk.ResponseType.CANCEL),
             (tr(self.language, "delete_action"), Gtk.ResponseType.OK)),
        ) != Gtk.ResponseType.OK:
            return

        self.store.delete_user_entry(entry.key)
        if self.current_key == entry.key:
            self.current_key = entry.key if entry.system_section else None
            self.selected_option_name = None
        self._refresh_view(self.current_key)
        self._push_status(tr(self.language, "dirty"))

    def _on_reload(self, _button: Gtk.Button) -> None:
        if self.store.dirty and not self._confirm_discard():
            return
        self.store.reload()
        self._refresh_view(self.current_key)

    def _on_save(self, _button: Gtk.Button) -> None:
        try:
            self.store.save()
        except Exception as error:
            self._show_error(error)
            return
        self._refresh_view(self.current_key)
        self._push_status(tr(self.language, "saved"))

    def _on_save_action(self, _action: Gio.SimpleAction, _parameter) -> None:
        if self.store.dirty:
            self._on_save(self.save_button)

    def _on_delete_event(self, _widget: Gtk.Widget, _event) -> bool:
        if not self.store.dirty:
            return False

        response = self._message(
            Gtk.MessageType.QUESTION,
            tr(self.language, "save_changes_title"),
            tr(self.language, "save_changes_body"),
            ((tr(self.language, "discard"), Gtk.ResponseType.NO),
             (tr(self.language, "cancel"), Gtk.ResponseType.CANCEL),
             (tr(self.language, "save"), Gtk.ResponseType.YES)),
        )
        if response == Gtk.ResponseType.YES:
            self._on_save(self.save_button)
            return self.store.dirty
        if response == Gtk.ResponseType.NO:
            return False
        return True

    def _confirm_discard(self) -> bool:
        return self._message(
            Gtk.MessageType.QUESTION,
            tr(self.language, "save_changes_title"),
            tr(self.language, "save_changes_body"),
            ((tr(self.language, "cancel"), Gtk.ResponseType.CANCEL),
             (tr(self.language, "discard"), Gtk.ResponseType.OK)),
        ) == Gtk.ResponseType.OK

    def _update_actions(self) -> None:
        self.save_button.set_sensitive(self.store.dirty)

    def _push_status(self, text: str) -> None:
        self.status.pop(self.status_context)
        self.status.push(self.status_context, text)

    def _clear_status(self) -> None:
        self.status.pop(self.status_context)

    def _show_error(self, error: Exception) -> None:
        self._message(Gtk.MessageType.ERROR, tr(self.language, "error"), str(error))

    def _message(
        self,
        message_type: Gtk.MessageType,
        text: str,
        secondary: str = "",
        buttons: Sequence[Tuple[str, Gtk.ResponseType]] = (),
    ) -> int:
        dialog = Gtk.MessageDialog(
            transient_for=self,
            modal=True,
            destroy_with_parent=True,
            message_type=message_type,
            buttons=Gtk.ButtonsType.NONE if buttons else Gtk.ButtonsType.CLOSE,
            text=text,
        )
        if secondary:
            dialog.format_secondary_text(secondary)
        for label, response in buttons:
            dialog.add_button(label, response)
        response = dialog.run()
        dialog.destroy()
        return response


class ConfiguratorApplication(Gtk.Application):
    def __init__(self, store: ConfigStore) -> None:
        super().__init__(application_id=APP_ID, flags=Gio.ApplicationFlags.FLAGS_NONE)
        self.store = store
        self.window: Optional[ConfiguratorWindow] = None

    def do_activate(self) -> None:
        if self.window is None:
            self.window = ConfiguratorWindow(self, self.store)
        self.window.present()


def parse_args(argv: Optional[Sequence[str]]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Box64 graphical rcfile configurator")
    parser.add_argument("--usage", type=Path)
    parser.add_argument("--usage-cn", type=Path)
    parser.add_argument("--system-rc", type=Path, default=default_system_rc())
    parser.add_argument("--user-rc", type=Path, default=default_user_rc())
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    if args.usage is not None or args.usage_cn is not None:
        if args.usage is None or args.usage_cn is None:
            raise SystemExit("--usage and --usage-cn must be used together")
        catalog = load_usage_catalog(args.usage, args.usage_cn)
    else:
        catalog = load_default_usage_catalog()
    store = ConfigStore(args.system_rc, args.user_rc, catalog, box64_arch_from_machine(platform.machine()))
    app = ConfiguratorApplication(store)
    return app.run([sys.argv[0]])


if __name__ == "__main__":
    raise SystemExit(main())
