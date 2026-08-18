#!/usr/bin/env python3
"""Tests for gen_can_database.py itself — the deterministic part of this
pipeline. Pure Python, no compiler needed.

Two kinds of coverage:
  - Unit tests for the small pure functions (signal_bounds, is_fast_path,
    screaming_snake) and for load_messages()'s schema validation.
  - A golden-file test: generate from test_fixtures/sample_messages.yaml
    and diff every output file byte-for-byte against test_fixtures/golden/.
    Any change in codegen behavior — intentional or not — shows up as a
    failing diff here, so it has to be a deliberate decision (regenerate
    the golden files and review the diff) rather than a silent drift.

Run: py -m unittest discover -s can_database/tools -p "test_*.py" -v
"""
import filecmp
import os
import sys
import tempfile
import unittest

import yaml

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_can_database as gen  # noqa: E402

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
FIXTURE_YAML = os.path.join(TOOLS_DIR, "test_fixtures", "sample_messages.yaml")
CAN_FRAME_H = os.path.join(TOOLS_DIR, "..", "include", "can_database", "can_frame.h")
GOLDEN_DIR = os.path.join(TOOLS_DIR, "test_fixtures", "golden")


def write_yaml(path, messages):
    with open(path, "w", encoding="utf-8") as f:
        yaml.safe_dump({"messages": messages}, f)


class ScreamingSnakeTests(unittest.TestCase):
    def test_known_names(self):
        cases = {
            "WheelSpeeds": "WHEEL_SPEEDS",
            "PowertrainCtrl": "POWERTRAIN_CTRL",
            "BatteryStatus": "BATTERY_STATUS",
            "MotorStatus": "MOTOR_STATUS",
            "IMUAccel": "IMU_ACCEL",
            "DashboardCtrl": "DASHBOARD_CTRL",
        }
        for name, expected in cases.items():
            with self.subTest(name=name):
                self.assertEqual(gen.screaming_snake(name), expected)


class SignalBoundsTests(unittest.TestCase):
    def test_unsigned_8bit(self):
        self.assertEqual(gen.signal_bounds({"bits": 8, "is_signed_wire": False}), (0, 255))

    def test_signed_16bit(self):
        self.assertEqual(gen.signal_bounds({"bits": 16, "is_signed_wire": True}), (-32768, 32767))

    def test_signed_24bit(self):
        self.assertEqual(gen.signal_bounds({"bits": 24, "is_signed_wire": True}), (-8388608, 8388607))

    def test_unsigned_24bit(self):
        self.assertEqual(gen.signal_bounds({"bits": 24, "is_signed_wire": False}), (0, 16777215))


class IsFastPathTests(unittest.TestCase):
    def _sig(self, **overrides):
        base = {"start_bit": 0, "needs_conversion": False, "bits": 8, "wire_type": "uint8_t"}
        base.update(overrides)
        return base

    def test_byte_aligned_full_width_unscaled_is_fast(self):
        self.assertTrue(gen.is_fast_path(self._sig()))
        self.assertTrue(gen.is_fast_path(self._sig(start_bit=24, bits=16, wire_type="uint16_t")))

    def test_not_byte_aligned_is_slow(self):
        self.assertFalse(gen.is_fast_path(self._sig(start_bit=3)))

    def test_scaled_signal_is_slow(self):
        self.assertFalse(gen.is_fast_path(self._sig(needs_conversion=True)))

    def test_narrower_than_wire_type_is_slow(self):
        # 24 bits packed into a uint32_t is never "full width" -- must
        # go through the general bit-packer.
        self.assertFalse(gen.is_fast_path(self._sig(bits=24, wire_type="uint32_t")))


class LoadMessagesValidationTests(unittest.TestCase):
    def setUp(self):
        self.tmpdir = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmpdir.cleanup)
        self.path = os.path.join(self.tmpdir.name, "test.yaml")

    def test_valid_message_is_accepted(self):
        write_yaml(self.path, [{
            "name": "Test", "id": 0x100, "bus": "CAN_BUS_SENSOR", "dlc": 8,
            "signals": [{"name": "x", "type": "uint16_t", "bits": 16}],
        }])
        messages = gen.load_messages(self.path)
        self.assertEqual(messages[0]["name"], "Test")
        self.assertEqual(messages[0]["signals"][0]["start_bit"], 0)

    def test_bit_budget_overflow_is_rejected(self):
        write_yaml(self.path, [{
            "name": "Test", "id": 0x100, "bus": "CAN_BUS_SENSOR", "dlc": 1,
            "signals": [{"name": "x", "type": "uint32_t", "bits": 32}],
        }])
        with self.assertRaises(gen.SchemaError):
            gen.load_messages(self.path)

    def test_invalid_wire_type_is_rejected(self):
        write_yaml(self.path, [{
            "name": "Test", "id": 0x100, "bus": "CAN_BUS_SENSOR", "dlc": 8,
            "signals": [{"name": "x", "type": "uint16_t", "wire_type": "not_a_type", "bits": 16}],
        }])
        with self.assertRaises(gen.SchemaError):
            gen.load_messages(self.path)

    def test_float_without_explicit_wire_type_is_rejected(self):
        write_yaml(self.path, [{
            "name": "Test", "id": 0x100, "bus": "CAN_BUS_SENSOR", "dlc": 8,
            "signals": [{"name": "x", "type": "float", "bits": 16}],
        }])
        with self.assertRaises(gen.SchemaError):
            gen.load_messages(self.path)

    def test_bits_out_of_range_is_rejected(self):
        write_yaml(self.path, [{
            "name": "Test", "id": 0x100, "bus": "CAN_BUS_SENSOR", "dlc": 8,
            "signals": [{"name": "x", "type": "uint32_t", "bits": 33}],
        }])
        with self.assertRaises(gen.SchemaError):
            gen.load_messages(self.path)

    def test_no_messages_is_rejected(self):
        write_yaml(self.path, [])
        with self.assertRaises(gen.SchemaError):
            gen.load_messages(self.path)


class GoldenFileTests(unittest.TestCase):
    """Regenerates from the fixture YAML and diffs every output file
    against the checked-in golden copy. A failing diff means codegen
    behavior changed -- if that's intentional, regenerate the golden
    files (see test_fixtures/sample_messages.yaml's docstring) and
    review the diff like any other code change."""

    def test_generated_output_matches_golden(self):
        with tempfile.TemporaryDirectory() as tmp:
            include_out = os.path.join(tmp, "include")
            src_out = os.path.join(tmp, "src")
            dbc_out_dir = os.path.join(tmp, "dbc")

            messages = gen.load_messages(FIXTURE_YAML)
            dbc_paths = gen.generate_all(messages, include_out, src_out, dbc_out_dir, CAN_FRAME_H)

            expected_files = {
                os.path.join(include_out, "can_database", "can_messages.h"):
                    os.path.join(GOLDEN_DIR, "include", "can_database", "can_messages.h"),
                os.path.join(include_out, "can_database", "can_ids.h"):
                    os.path.join(GOLDEN_DIR, "include", "can_database", "can_ids.h"),
                os.path.join(include_out, "can_database", "can_codec.h"):
                    os.path.join(GOLDEN_DIR, "include", "can_database", "can_codec.h"),
                os.path.join(include_out, "can_database", "can_publish.h"):
                    os.path.join(GOLDEN_DIR, "include", "can_database", "can_publish.h"),
                os.path.join(src_out, "can_codec.c"):
                    os.path.join(GOLDEN_DIR, "src", "can_codec.c"),
                os.path.join(src_out, "can_publish.c"):
                    os.path.join(GOLDEN_DIR, "src", "can_publish.c"),
            }
            for dbc_path in dbc_paths:
                expected_files[dbc_path] = os.path.join(GOLDEN_DIR, "dbc", os.path.basename(dbc_path))

            for actual, golden in expected_files.items():
                with self.subTest(file=os.path.basename(golden)):
                    self.assertTrue(os.path.exists(golden), f"missing golden file: {golden}")
                    same = filecmp.cmp(actual, golden, shallow=False)
                    if not same:
                        with open(actual, encoding="utf-8") as f:
                            actual_text = f.read()
                        with open(golden, encoding="utf-8") as f:
                            golden_text = f.read()
                        self.assertEqual(
                            actual_text, golden_text,
                            f"generated output for {os.path.basename(golden)} no longer "
                            f"matches the golden file -- if this is intentional, regenerate "
                            f"test_fixtures/golden/ and review the diff"
                        )


if __name__ == "__main__":
    unittest.main()
