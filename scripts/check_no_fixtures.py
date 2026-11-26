#!/usr/bin/env python3
"""Fail if a RAM dump captured from a game (test-fixtures/README.md) is tracked by git."""
import re
import subprocess
import sys

DUMP = re.compile(r"(^|/)(mainram|ewram|iwram|wram|wramx|sram|hram)\.bin$")
ROOTS = ("test-fixtures/resources/fixtures/", "app/src/test/resources/fixtures/")

files = subprocess.run(["git", "ls-files", "-z"], capture_output=True, text=True, check=True).stdout.split("\0")
bad = [f for f in files if f and (f.startswith(ROOTS) or (DUMP.search(f) and "fixtures/" in f))]
if bad:
    print("Game RAM dumps must not be committed (test-fixtures/README.md):")
    print("\n".join("  " + f for f in bad[:20]))
    sys.exit(1)
