#!/usr/bin/env python3
"""Generates can_database's structs, IDs, codec, publish API, and .dbc
file from can_messages.yaml.

Do not hand-edit the generated output — edit can_messages.yaml and
regenerate. CMake runs this automatically at configure time and again
at build time whenever can_messages.yaml changes.
"""
import argparse
import os
import re
import sys

import yaml

SIGNED_TYPES = {"int8_t", "int16_t", "int32_t"}
UNSIGNED_TYPES = {"uint8_t", "uint16_t", "uint32_t"}
INT_TYPES = SIGNED_TYPES | UNSIGNED_TYPES
FLOAT_TYPES = {"float", "double"}
TYPE_WIDTH = {"uint8_t": 8, "int8_t": 8, "uint16_t": 16, "int16_t": 16,
              "uint32_t": 32, "int32_t": 32}

HEADER_NOTE = (
    "/* GENERATED FILE — do not edit by hand.\n"
    " * Source: can_messages.yaml, generator: tools/gen_can_database.py\n"
    " */\n\n"
)


class SchemaError(ValueError):
    """Raised for invalid can_messages.yaml content. Callers importing this
    module (e.g. the GUI editor) can catch this directly; running as a
    script instead prints it and exits non-zero (see main())."""


def die(msg):
    raise SchemaError(msg)


def load_messages(yaml_path):
    with open(yaml_path, "r", encoding="utf-8") as f:
        data = yaml.safe_load(f)

    messages = data.get("messages") or []
    if not messages:
        die(f"{yaml_path}: no messages defined")

    for msg in messages:
        for field in ("name", "id", "bus", "dlc", "signals"):
            if field not in msg:
                die(f"message missing required field '{field}': {msg}")

        total_bits = 0
        for sig in msg["signals"]:
            for field in ("name", "type", "bits"):
                if field not in sig:
                    die(f"{msg['name']}: signal missing required field '{field}': {sig}")

            sig.setdefault("wire_type", sig["type"])
            sig.setdefault("scale", 1)
            sig.setdefault("offset", 0)
            sig.setdefault("unit", "")

            ftype = sig["type"]
            wtype = sig["wire_type"]

            if wtype not in INT_TYPES:
                die(f"{msg['name']}.{sig['name']}: wire_type '{wtype}' must be one of {sorted(INT_TYPES)}")
            if ftype in FLOAT_TYPES and wtype == ftype:
                die(f"{msg['name']}.{sig['name']}: type '{ftype}' requires an explicit integer wire_type")
            if ftype not in FLOAT_TYPES and ftype not in INT_TYPES:
                die(f"{msg['name']}.{sig['name']}: unsupported type '{ftype}'")

            bits = sig["bits"]
            if not (1 <= bits <= 32):
                die(f"{msg['name']}.{sig['name']}: bits must be 1-32, got {bits}")

            sig["needs_conversion"] = (wtype != ftype)
            sig["is_signed_wire"] = wtype in SIGNED_TYPES
            sig["start_bit"] = total_bits
            total_bits += bits

        if total_bits > msg["dlc"] * 8:
            die(f"{msg['name']}: signals use {total_bits} bits but dlc={msg['dlc']} "
                f"only provides {msg['dlc'] * 8} bits")

        msg.setdefault("description", "")

    return messages


def is_fast_path(sig):
    """True when a signal can be encoded/decoded as a plain byte-aligned
    load/store instead of going through the general bit-packer: it starts
    on a byte boundary, has no scale/offset conversion, and its bit width
    exactly matches its wire type's natural width (so there's no partial-
    byte truncation to handle)."""
    return (sig["start_bit"] % 8 == 0
            and not sig["needs_conversion"]
            and sig["bits"] == TYPE_WIDTH[sig["wire_type"]])


def screaming_snake(name):
    """WheelSpeeds -> WHEEL_SPEEDS, IMUAccel -> IMU_ACCEL. Used only for the
    CAN_ID_*/CAN_BUS_* enum constants, matching this project's original
    hand-written naming convention (types/functions stay PascalCase)."""
    s1 = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    s2 = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", s1)
    return s2.upper()


def signal_bounds(sig):
    bits = sig["bits"]
    if sig["is_signed_wire"]:
        return -(1 << (bits - 1)), (1 << (bits - 1)) - 1
    return 0, (1 << bits) - 1


def fmt_num(value, as_float):
    return f"{float(value)}f" if as_float else f"{value}"


def emit_struct(messages):
    # No #pragma pack: Encode/Decode work signal-by-signal (never memcpy or
    # sizeof() the whole struct), so wire layout is fully decoupled from
    # struct layout. Leaving the struct naturally aligned lets the compiler
    # generate normal aligned loads/stores instead of forcing unaligned
    # access — and avoids a latent misalignment fault risk on strict-
    # alignment cores for any future signal ordering that packing would
    # otherwise silently allow.
    lines = [HEADER_NOTE, "#pragma once", "#include <stdint.h>", ""]
    for msg in messages:
        lines.append("typedef struct {")
        for sig in msg["signals"]:
            lines.append(f"    {sig['type']} {sig['name']};  /* {sig['unit'] or 'raw'} */")
        lines.append(f"}} CanMsg_{msg['name']}_t;")
        lines.append("")
    return "\n".join(lines) + "\n"


def emit_ids(messages):
    lines = [HEADER_NOTE, "#pragma once", '#include "can_database/can_frame.h"', ""]
    for msg in messages:
        const_name = screaming_snake(msg["name"])
        lines.append(f"enum {{ CAN_ID_{const_name} = {hex(msg['id'])}, "
                      f"CAN_BUS_{const_name} = {msg['bus']} }};")
    return "\n".join(lines) + "\n"


def emit_codec_header(messages):
    lines = [HEADER_NOTE, "#pragma once", "#include <stdint.h>",
             '#include "can_database/can_messages.h"', ""]
    for msg in messages:
        name, dlc = msg["name"], msg["dlc"]
        lines.append(f"void CanCodec_{name}_Encode(const CanMsg_{name}_t *m, uint8_t data[{dlc}]);")
        lines.append(f"CanMsg_{name}_t CanCodec_{name}_Decode(const uint8_t data[{dlc}]);")
        lines.append("")
    return "\n".join(lines) + "\n"


def emit_encode_signal(sig):
    bits = sig["bits"]
    wtype = sig["wire_type"]
    start_bit = sig["start_bit"]
    as_float = sig["type"] == "float"

    if sig["needs_conversion"]:
        round_fn = "lroundf" if as_float else "lround"
        offset_lit = fmt_num(sig["offset"], as_float)
        scale_lit = fmt_num(sig["scale"], as_float)
        raw_expr = f"({wtype}){round_fn}((m->{sig['name']} - {offset_lit}) / {scale_lit})"
    else:
        raw_expr = f"({wtype})m->{sig['name']}"

    if is_fast_path(sig):
        # Byte-aligned, full-width, no scaling: plain little-endian byte
        # store — no bit-packer call, no range check needed (the value
        # already fits its type exactly, nothing to truncate).
        byte_off = start_bit // 8
        lines = [f"    {{ {wtype} _raw = {raw_expr};"]
        for i in range(bits // 8):
            shift = f" >> {i * 8}" if i else ""
            lines.append(f" data[{byte_off + i}] = (uint8_t)(_raw{shift});")
        lines.append(" }")
        return "".join(lines)

    # The bounds check only tells us anything when bits is narrower than
    # the wire type's own range (e.g. 24 bits packed into a uint32_t) —
    # when they're equal it's tautologically true (and MSVC/GCC will say
    # so via -Wtype-limits), so skip it entirely in that case. And for an
    # unsigned wire type, ">= 0" is *always* tautologically true (_raw is
    # unsigned before the cast to int64_t), regardless of bits — only the
    # upper bound is ever real information there.
    assert_line = ""
    if bits < TYPE_WIDTH[wtype]:
        lo, hi = signal_bounds(sig)
        if sig["is_signed_wire"]:
            assert_line = f"\n        assert((int64_t)_raw >= {lo} && (int64_t)_raw <= {hi});"
        else:
            assert_line = f"\n        assert((int64_t)_raw <= {hi});"

    return (
        f"    {{\n"
        f"        {wtype} _raw = {raw_expr};{assert_line}\n"
        f"        CanBits_Pack(data, (uint32_t)_raw, {start_bit}, {bits});\n"
        f"    }}"
    )


def emit_decode_signal(sig):
    bits = sig["bits"]
    start_bit = sig["start_bit"]
    as_float = sig["type"] == "float"

    if is_fast_path(sig):
        byte_off = start_bit // 8
        wtype = sig["wire_type"]
        terms = [f"(uint32_t)data[{byte_off + i}] << {i * 8}" if i else f"data[{byte_off}]"
                 for i in range(bits // 8)]
        raw_expr = f"({wtype})({' | '.join(terms)})"
        return f"    msg.{sig['name']} = ({sig['type']})({raw_expr});"

    unpack = f"CanBits_Unpack(data, {start_bit}, {bits})"
    raw_expr = f"CanBits_SignExtend({unpack}, {bits})" if sig["is_signed_wire"] else unpack

    if sig["needs_conversion"]:
        offset_lit = fmt_num(sig["offset"], as_float)
        scale_lit = fmt_num(sig["scale"], as_float)
        value_expr = f"({raw_expr}) * {scale_lit} + {offset_lit}"
    else:
        value_expr = raw_expr

    return f"    msg.{sig['name']} = ({sig['type']})({value_expr});"


def emit_codec_source(messages):
    lines = [HEADER_NOTE,
             '#include "can_database/can_codec.h"',
             '#include "can_database/can_bits.h"',
             "#include <string.h>",
             "#include <math.h>",
             "#include <assert.h>", ""]
    for msg in messages:
        name, dlc = msg["name"], msg["dlc"]

        lines.append(f"void CanCodec_{name}_Encode(const CanMsg_{name}_t *m, uint8_t data[{dlc}]) {{")
        lines.append(f"    memset(data, 0, {dlc});")
        for sig in msg["signals"]:
            lines.append(emit_encode_signal(sig))
        lines.append("}")
        lines.append("")

        lines.append(f"CanMsg_{name}_t CanCodec_{name}_Decode(const uint8_t data[{dlc}]) {{")
        lines.append(f"    CanMsg_{name}_t msg;")
        for sig in msg["signals"]:
            lines.append(emit_decode_signal(sig))
        lines.append("    return msg;")
        lines.append("}")
        lines.append("")
    return "\n".join(lines) + "\n"


def emit_publish_header(messages):
    lines = [HEADER_NOTE, "#pragma once", '#include "can_database/can_messages.h"', ""]
    for msg in messages:
        lines.append(f"void CanDb_{msg['name']}_Publish(const CanMsg_{msg['name']}_t *m);")
    return "\n".join(lines) + "\n"


def emit_publish_source(messages):
    lines = [HEADER_NOTE,
             '#include "can_database/can_publish.h"',
             '#include "can_database/can_codec.h"',
             '#include "can_database/can_ids.h"',
             '#include "can_database_internal.h"', ""]
    for msg in messages:
        name, dlc = msg["name"], msg["dlc"]
        const_name = screaming_snake(name)
        lines.append(f"void CanDb_{name}_Publish(const CanMsg_{name}_t *m) {{")
        lines.append("    CanFrame_t frame;")
        lines.append(f"    frame.id  = CAN_ID_{const_name};")
        lines.append(f"    frame.dlc = {dlc};")
        lines.append(f"    CanCodec_{name}_Encode(m, frame.data);")
        lines.append(f"    CanDb_Send(CAN_BUS_{const_name}, &frame);")
        lines.append("}")
        lines.append("")
    return "\n".join(lines) + "\n"


def discover_bus_names(can_frame_h_path):
    """Pull CAN_BUS_* names out of can_frame.h's CanBusId_t enum. Shared
    by DBC-per-bus generation and the GUI editor's Bus dropdown, so
    there's one place that knows how to find real bus names."""
    try:
        with open(can_frame_h_path, "r", encoding="utf-8") as f:
            text = f.read()
    except OSError:
        return ["CAN_BUS_POWERTRAIN", "CAN_BUS_SENSOR", "CAN_BUS_DASHBOARD"]
    names = re.findall(r"\bCAN_BUS_(\w+)\b", text)
    return [f"CAN_BUS_{n}" for n in dict.fromkeys(names) if n != "COUNT"]


def bus_dbc_filename(bus_name):
    """CAN_BUS_SENSOR -> can_database_sensor.dbc"""
    suffix = bus_name[len("CAN_BUS_"):].lower() if bus_name.startswith("CAN_BUS_") else bus_name.lower()
    return f"can_database_{suffix}.dbc"


def emit_dbc_for_bus(messages, bus_name):
    """DBC text for just the messages assigned to one bus. Real CAN
    tooling (CANoe, CANalyzer, cantools) loads one database per physical
    bus/channel, not one file mixing every bus — different buses can
    reuse the same CAN ID, so a single combined file isn't meaningful."""
    lines = ['VERSION ""', "", "NS_ :", "", "BS_:", "", "BU_: Vector__XXX", ""]
    for msg in messages:
        if msg["bus"] != bus_name:
            continue
        lines.append(f"BO_ {msg['id']} {msg['name']}: {msg['dlc']} Vector__XXX")
        for sig in msg["signals"]:
            sign_char = "-" if sig["is_signed_wire"] else "+"
            lines.append(
                f' SG_ {sig["name"]} : {sig["start_bit"]}|{sig["bits"]}@1{sign_char} '
                f'({sig["scale"]},{sig["offset"]}) [0|0] "{sig["unit"]}"  Vector__XXX'
            )
        lines.append("")
    return "\n".join(lines) + "\n"


def write_if_changed(path, content):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if os.path.exists(path):
        with open(path, "r", encoding="utf-8") as f:
            if f.read() == content:
                return
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


def generate_all(messages, include_out, src_out, dbc_out_dir, can_frame_h):
    """Write every generated artifact for already-loaded/validated messages.
    Shared by the CLI (main, below) and other callers (e.g. the GUI editor)
    so there's one place that knows the output file layout.

    Returns the list of .dbc file paths written, one per bus known to
    can_frame.h (written even for a bus with zero messages, so the set of
    output filenames stays predictable regardless of the YAML's content)."""
    include_dir = os.path.join(include_out, "can_database")
    write_if_changed(os.path.join(include_dir, "can_messages.h"), emit_struct(messages))
    write_if_changed(os.path.join(include_dir, "can_ids.h"), emit_ids(messages))
    write_if_changed(os.path.join(include_dir, "can_codec.h"), emit_codec_header(messages))
    write_if_changed(os.path.join(include_dir, "can_publish.h"), emit_publish_header(messages))

    write_if_changed(os.path.join(src_out, "can_codec.c"), emit_codec_source(messages))
    write_if_changed(os.path.join(src_out, "can_publish.c"), emit_publish_source(messages))

    dbc_paths = []
    for bus_name in discover_bus_names(can_frame_h):
        dbc_path = os.path.join(dbc_out_dir, bus_dbc_filename(bus_name))
        write_if_changed(dbc_path, emit_dbc_for_bus(messages, bus_name))
        dbc_paths.append(dbc_path)
    return dbc_paths


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--yaml", required=True)
    parser.add_argument("--include-out", required=True,
                         help="output dir for generated headers (a can_database/ subfolder is created inside)")
    parser.add_argument("--src-out", required=True, help="output dir for generated .c files")
    parser.add_argument("--dbc-out-dir", required=True,
                         help="output dir for the generated .dbc files, one per bus")
    parser.add_argument("--can-frame-h", required=True,
                         help="path to can_frame.h, to discover known CAN_BUS_* names")
    args = parser.parse_args()

    try:
        messages = load_messages(args.yaml)
        generate_all(messages, args.include_out, args.src_out, args.dbc_out_dir, args.can_frame_h)
    except SchemaError as e:
        print(f"gen_can_database.py: error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
