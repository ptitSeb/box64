import json
import os
import re

script_dir = os.path.dirname(os.path.abspath(__file__))
usage_file = os.path.join(script_dir, "usage.json")

PADDING = 30
INCLUDE_BLANK = False


def get_usage_entry(usage, define):
    entry = list(e for e in usage if e["name"] == define)
    if len(entry) == 0:
        print(f"{define:<{PADDING}}: missing usage.json entry")
    elif len(entry) > 1:
        print(f"{define:<{PADDING}}: multiple usage entries found", len(entry))
    else:
        return entry[0]
    return None


# load usage.json
with open(usage_file, "r") as file:
    usage = json.load(file)

# check `default` uniqueness
for entry in usage:
    i = list(d["default"] for d in entry["options"]).count(True)
    if i > 1:
        print(f"{entry["name"]:<{PADDING}}: multiple default values usage.json")


# regex to match env.h C code
regex = {
    "INTEGER": re.compile(
        r"^\s*INTEGER\((?P<define>\w+), (?P<name>\w+), (?P<default>\w+), (?P<min>\w+), (?P<max>\w+)\)"
    ),
    "INTEGER64": re.compile(
        r"^\s*INTEGER64\((?P<define>\w+), (?P<name>\w+), (?P<default>\w+)\)"
    ),
    "BOOLEAN": re.compile(
        r"^\s*BOOLEAN\((?P<define>\w+), (?P<name>\w+), (?P<default>\w+)\)"
    ),
    "ADDRESS": re.compile(r"^\s*ADDRESS\((?P<define>\w+), (?P<name>\w+)\)"),
    "STRING": re.compile(r"^\s*STRING\((?P<define>\w+), (?P<name>\w+)\)"),
}

env_file = os.path.join(script_dir, "../../src/include/env.h")
with open(env_file, "r") as file:
    matches = {}
    for line in file.readlines():
        for t, r in regex.items():
            m = r.search(line)
            if m:
                # filter out comments and other non useful code
                if m.group("define") == "NAME":
                    continue

                if t not in matches:
                    matches[t] = {}
                matches[t][m.group("define")] = m
                break

for define, m in matches["INTEGER"].items():
    name = m.group("name")
    default = (
        0 if m.group("default") == "DEFAULT_LOG_LEVEL" else int(m.group("default"))
    )
    min = int(m.group("min"))
    max = int(m.group("max"))

    # Check default in valid range
    if default < min or default > max:
        print(f"{define:<{PADDING}}: default lays outside of min/max range")

    # Check consistency with usage.json
    if e := get_usage_entry(usage, define):
        # guess min/max values if possible
        min2 = max2 = None
        # blank means that the entry has an 'XXXX' entry which usually indicated that arbitrary values are valid
        blank = False
        default2 = None
        for o in e["options"]:
            if o["key"] == "XXXX":
                blank = True
                continue

            val = int(o["key"])  # not supposed to fail

            min2 = min2 if min2 is not None and min2 < val else val
            max2 = max2 if max2 is not None and max2 > val else val

            if o["default"]:
                default2 = val

        if min2 and min2 != min:
            if not blank or (blank and INCLUDE_BLANK):
                print(
                    f"{define:<{PADDING}}: min value mismatch: env.h={min}, usage.json={min2}{(' (possible false positive)' if blank else '')}"
                )
        if max2 and max2 != max:
            if not blank or (blank and INCLUDE_BLANK):
                print(
                    f"{define:<{PADDING}}: max value mismatch: env.h={max}, usage.json={max2}{(' (possible false positive)' if blank else '')}"
                )
        if default2 and default2 != default:
            print(
                f"{define:<{PADDING}}: default value mismatch: env.h={default}, usage.json={default2}"
            )

for define, m in matches["INTEGER64"].items():
    # similar to INTEGER but without min/max
    name = m.group("name")
    default = int(m.group("default"))

    # Check consistency with usage.json
    if e := get_usage_entry(usage, define):
        default2 = None
        for o in e["options"]:
            if o["key"] == "XXXX":
                continue

            val = int(o["key"])

            if o["default"]:
                default2 = val

        if default2 and default2 != default:
            print(
                f"{define:<{PADDING}}: default value mismatch: env.h={default}, usage.json={default2}"
            )

for define, m in matches["BOOLEAN"].items():
    name = m.group("name")
    default = bool(m.group("default"))

    # Check consistency with usage.json
    if e := get_usage_entry(usage, define):
        default2 = None
        for o in e["options"]:
            try:
                val = bool(o["key"])
            except ValueError:
                print(f"{define:<{PADDING}}: failed to parse boolean {o["key"]}")

            if o["default"]:
                default2 = val

        if default2 and default2 != default:
            print(
                f"{define:<{PADDING}}: default value mismatch: env.h={default}, usage.json={default2}"
            )

# ADDRESS and STRING are not that interesting, just check that they exist in usage.json
for define, m in matches["ADDRESS"].items():
    _ = get_usage_entry(usage, define)
for define, m in matches["STRING"].items():
    # skip BOX64_ENV[1-5] entries, they mismatch but this is fine
    if define.startswith("BOX64_ENV"):
        continue
    _ = get_usage_entry(usage, define)

# check that everything from usage.json is in env.h
for e in usage:
    define = e["name"]

    # skip BOX64_ENV[1-5] entries, they mismatch but this is fine
    if define.startswith("BOX64_ENV"):
        continue

    found = False
    for t in matches:
        for d in matches[t]:
            if d == define:
                found = True

    if not found:
        print(f"{define:<{PADDING}}: missing env.h entry")
