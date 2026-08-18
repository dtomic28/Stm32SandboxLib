#!/usr/bin/env python3
"""GUI editor for can_messages.yaml.

Add/remove messages and their signals in an editable grid, validate
against the exact same schema rules gen_can_database.py uses, and save
back to the YAML file. This tool only edits and validates the YAML — it
does not generate C code itself. CMake regenerates the C code
automatically on the next configure/build (see can_database/src/CMakeLists.txt),
so just save here and rebuild.

Run:
    py can_message_editor.py [path/to/can_messages.yaml]

Defaults to ../can_messages.yaml (relative to this script) if no path
is given.
"""
import copy
import os
import sys
import tkinter as tk
from tkinter import messagebox, ttk

import yaml

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_can_database as gen  # noqa: E402
from gen_can_database import discover_bus_names  # noqa: E402

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_YAML = os.path.normpath(os.path.join(TOOLS_DIR, "..", "can_messages.yaml"))
CAN_FRAME_H = os.path.normpath(os.path.join(TOOLS_DIR, "..", "include", "can_database", "can_frame.h"))

MESSAGE_COLUMNS = ("name", "id", "bus", "dlc", "description")
SIGNAL_COLUMNS = ("name", "type", "wire_type", "bits", "scale", "offset", "unit")

NEW_MESSAGE = {"name": "NewMessage", "id": 0, "bus": "CAN_BUS_SENSOR", "dlc": 8,
               "description": "", "signals": []}
NEW_SIGNAL = {"name": "new_signal", "type": "uint8_t", "wire_type": "uint8_t",
              "bits": 8, "scale": 1, "offset": 0, "unit": ""}


class EditableTreeview(ttk.Treeview):
    """A Treeview where double-clicking a cell opens an inline editor —
    a Combobox for columns with a fixed choice list, an Entry otherwise.
    Commits on Enter, dropdown selection, or losing focus."""

    def __init__(self, master, columns, combo_values=None, on_commit=None, **kw):
        super().__init__(master, columns=columns, show="headings", **kw)
        self.combo_values = combo_values or {}
        self.on_commit = on_commit
        for col in columns:
            self.heading(col, text=col)
            self.column(col, width=100, anchor="w")
        self.bind("<Double-1>", self._begin_edit)
        self._editor = None

    def _begin_edit(self, event):
        row = self.identify_row(event.y)
        col = self.identify_column(event.x)
        if not row or not col:
            return
        col_name = self["columns"][int(col.replace("#", "")) - 1]
        bbox = self.bbox(row, col)
        if not bbox:
            return
        x, y, w, h = bbox
        value = self.set(row, col_name)

        if self._editor is not None:
            self._editor.destroy()
            self._editor = None

        if col_name in self.combo_values:
            editor = ttk.Combobox(self, values=self.combo_values[col_name], state="readonly")
            editor.set(value)
        else:
            editor = ttk.Entry(self)
            editor.insert(0, value)
            editor.select_range(0, tk.END)

        editor.place(x=x, y=y, width=w, height=h)
        editor.focus_set()

        committed = [False]

        def commit(_event=None):
            if committed[0]:
                return
            committed[0] = True
            new_value = editor.get()
            self.set(row, col_name, new_value)
            editor.destroy()
            if self._editor is editor:
                self._editor = None
            if self.on_commit:
                self.on_commit(row, col_name, new_value)

        editor.bind("<Return>", commit)
        editor.bind("<<ComboboxSelected>>", commit)
        editor.bind("<FocusOut>", commit)
        editor.bind("<Escape>", lambda e: editor.destroy())
        self._editor = editor


class MessageEditorApp:
    def __init__(self, root, yaml_path):
        self.root = root
        self.yaml_path = yaml_path
        self.messages = []
        self.selected_message_iid = None

        root.title(f"CAN Message Editor — {os.path.basename(yaml_path)}")
        root.geometry("1000x650")

        bus_names = discover_bus_names(CAN_FRAME_H)
        type_names = sorted(gen.INT_TYPES | gen.FLOAT_TYPES)

        msg_frame = ttk.LabelFrame(root, text="Messages (double-click a cell to edit)")
        msg_frame.pack(fill="both", expand=True, padx=8, pady=(8, 4))

        self.msg_tree = EditableTreeview(
            msg_frame, MESSAGE_COLUMNS,
            combo_values={"bus": bus_names},
            on_commit=self._on_message_cell_edit,
        )
        self.msg_tree.pack(fill="both", expand=True, side="left")
        self.msg_tree.bind("<<TreeviewSelect>>", self._on_select_message)

        msg_btns = ttk.Frame(msg_frame)
        msg_btns.pack(side="left", fill="y", padx=4)
        ttk.Button(msg_btns, text="Add Message", command=self._add_message).pack(fill="x", pady=2)
        ttk.Button(msg_btns, text="Remove Message", command=self._remove_message).pack(fill="x", pady=2)

        sig_frame = ttk.LabelFrame(root, text="Signals for selected message")
        sig_frame.pack(fill="both", expand=True, padx=8, pady=(4, 4))

        self.sig_tree = EditableTreeview(
            sig_frame, SIGNAL_COLUMNS,
            combo_values={"type": type_names, "wire_type": type_names},
            on_commit=self._on_signal_cell_edit,
        )
        self.sig_tree.pack(fill="both", expand=True, side="left")

        sig_btns = ttk.Frame(sig_frame)
        sig_btns.pack(side="left", fill="y", padx=4)
        ttk.Button(sig_btns, text="Add Signal", command=self._add_signal).pack(fill="x", pady=2)
        ttk.Button(sig_btns, text="Remove Signal", command=self._remove_signal).pack(fill="x", pady=2)

        bottom = ttk.Frame(root)
        bottom.pack(fill="x", padx=8, pady=8)
        self.status = tk.StringVar(value="")
        ttk.Label(bottom, textvariable=self.status).pack(side="left")
        ttk.Button(bottom, text="Save && Validate", command=self._save).pack(side="right")

        self._load()

    # --- load / refresh ---

    def _load(self):
        try:
            with open(self.yaml_path, "r", encoding="utf-8") as f:
                data = yaml.safe_load(f) or {}
        except (OSError, yaml.YAMLError) as e:
            messagebox.showerror("Failed to load", str(e))
            data = {}

        self.messages = data.get("messages") or []
        for msg in self.messages:
            msg.setdefault("description", "")
            for sig in msg.get("signals", []):
                sig.setdefault("wire_type", sig.get("type", ""))
                sig.setdefault("scale", 1)
                sig.setdefault("offset", 0)
                sig.setdefault("unit", "")
        self._refresh_messages()

    def _refresh_messages(self):
        self.msg_tree.delete(*self.msg_tree.get_children())
        for i, msg in enumerate(self.messages):
            msg_id = msg.get("id", 0)
            id_display = hex(msg_id) if isinstance(msg_id, int) else msg_id
            self.msg_tree.insert("", "end", iid=str(i), values=(
                msg.get("name", ""), id_display, msg.get("bus", ""),
                msg.get("dlc", ""), msg.get("description", ""),
            ))
        self.selected_message_iid = None
        self.sig_tree.delete(*self.sig_tree.get_children())

    def _refresh_signals(self):
        self.sig_tree.delete(*self.sig_tree.get_children())
        if self.selected_message_iid is None:
            return
        msg = self.messages[int(self.selected_message_iid)]
        for i, sig in enumerate(msg.get("signals", [])):
            self.sig_tree.insert("", "end", iid=str(i), values=(
                sig.get("name", ""), sig.get("type", ""), sig.get("wire_type", ""),
                sig.get("bits", ""), sig.get("scale", ""), sig.get("offset", ""), sig.get("unit", ""),
            ))

    def _on_select_message(self, _event=None):
        sel = self.msg_tree.selection()
        self.selected_message_iid = sel[0] if sel else None
        self._refresh_signals()

    # --- cell edit commits ---

    def _on_message_cell_edit(self, row_iid, col_name, new_value):
        msg = self.messages[int(row_iid)]
        if col_name == "id":
            try:
                msg["id"] = int(new_value, 0)
            except ValueError:
                messagebox.showerror("Invalid ID", f"'{new_value}' is not a valid integer (try 0x100)")
                self._refresh_messages()
                return
            self.msg_tree.set(row_iid, "id", hex(msg["id"]))
        elif col_name == "dlc":
            try:
                msg["dlc"] = int(new_value)
            except ValueError:
                messagebox.showerror("Invalid DLC", f"'{new_value}' is not an integer")
                self._refresh_messages()
                return
        else:
            msg[col_name] = new_value

    def _on_signal_cell_edit(self, row_iid, col_name, new_value):
        if self.selected_message_iid is None:
            return
        msg = self.messages[int(self.selected_message_iid)]
        sig = msg["signals"][int(row_iid)]
        if col_name == "bits":
            try:
                sig["bits"] = int(new_value)
            except ValueError:
                messagebox.showerror("Invalid bits", f"'{new_value}' is not an integer")
                self._refresh_signals()
                return
        elif col_name in ("scale", "offset"):
            try:
                sig[col_name] = float(new_value)
            except ValueError:
                messagebox.showerror(f"Invalid {col_name}", f"'{new_value}' is not a number")
                self._refresh_signals()
                return
        else:
            sig[col_name] = new_value

    # --- add/remove rows ---

    def _add_message(self):
        self.messages.append(copy.deepcopy(NEW_MESSAGE))
        self._refresh_messages()

    def _remove_message(self):
        sel = self.msg_tree.selection()
        if not sel:
            return
        del self.messages[int(sel[0])]
        self._refresh_messages()

    def _add_signal(self):
        if self.selected_message_iid is None:
            messagebox.showinfo("No message selected", "Select a message first")
            return
        msg = self.messages[int(self.selected_message_iid)]
        msg.setdefault("signals", []).append(copy.deepcopy(NEW_SIGNAL))
        self._refresh_signals()

    def _remove_signal(self):
        if self.selected_message_iid is None:
            return
        sel = self.sig_tree.selection()
        if not sel:
            return
        msg = self.messages[int(self.selected_message_iid)]
        del msg["signals"][int(sel[0])]
        self._refresh_signals()

    # --- save / validate ---

    def _save(self):
        data = {"messages": copy.deepcopy(self.messages)}
        tmp_path = self.yaml_path + ".tmp"
        with open(tmp_path, "w", encoding="utf-8") as f:
            yaml.safe_dump(data, f, sort_keys=False, default_flow_style=False)

        try:
            gen.load_messages(tmp_path)
        except gen.SchemaError as e:
            os.remove(tmp_path)
            messagebox.showerror("Validation failed", str(e))
            self.status.set(f"Not saved — {e}")
            return

        os.replace(tmp_path, self.yaml_path)
        self.status.set(f"Saved {self.yaml_path} — valid. Rebuild to regenerate C code.")


def main():
    yaml_path = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_YAML
    if not os.path.exists(yaml_path):
        print(f"can_message_editor.py: error: {yaml_path} not found", file=sys.stderr)
        sys.exit(1)

    root = tk.Tk()
    MessageEditorApp(root, yaml_path)
    root.mainloop()


if __name__ == "__main__":
    main()
