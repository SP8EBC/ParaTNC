#!/usr/bin/env python3
"""
Generates JSON description of all events logged by ParaMETEO firmware.

Every call to event_log_sync / event_log_sync_triple found in C sources is
turned into a single entry of "events" array:

  severity, source  - integer value of event_log_severity_t / event_log_source_t
                      enumerator (include/event_log_t.h)
  eventId           - integer value of EVENTS_xxx define (include/events_definitions/*.h)
  eventString       - string returned by event_id_to_str (src/event_log_to_string.c)
                      for given source and event id define. The string itself is
                      taken from include/event_log_strings.h
  param ... lparam2 - source text of each argument, as written in a call.
                      wparam3 is empty for event_log_sync (non triple)
  asHexParam ...    - bool, if a parameter should be displayed in hexadecimal.
  asHexLparam2        Always generated as true, to be adjusted manually

Usage:
  tools/generate_event_log_description.py [-o output.json] [--with-location] [repo_root]

Problems which can't be resolved automatically (unknown define, event id not
handled in event_log_to_string.c, etc.) are reported on stderr.
"""

import argparse
import datetime
import json
import os
import re
import sys

SOURCE_DIRS = ["src", "system/src"]

EVENT_LOG_T_H = "include/event_log_t.h"
EVENT_DEFINITIONS_DIR = "include/events_definitions"
EVENT_LOG_STRINGS_H = "include/event_log_strings.h"
EVENT_LOG_TO_STRING_C = "src/event_log_to_string.c"

# names of parameters in order, after severity, source and event_id
PARAMS_SYNC = ["param", "param2", "wparam", "wparam2", "lparam", "lparam2"]
PARAMS_TRIPLE = ["param", "param2", "wparam", "wparam2", "wparam3", "lparam", "lparam2"]
PARAMS_OUTPUT_ORDER = ["param", "param2", "wparam", "wparam2", "wparam3", "lparam", "lparam2"]

CALL_RE = re.compile(r"\bevent_log_sync(_triple)?\s*\(")


def warn(msg):
    print("WARNING: " + msg, file=sys.stderr)


def read(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read()


def strip_comments(text):
    """
    Removes C and C++ comments, keeping string / char literals intact and
    preserving newlines, so line numbers remain valid.
    """
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            while i < n and text[i] != "\n":
                i += 1
        elif c == "/" and i + 1 < n and text[i + 1] == "*":
            end = text.find("*/", i + 2)
            end = n if end < 0 else end + 2
            out.append("\n" * text.count("\n", i, end))
            i = end
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                if text[j] == "\\":
                    j += 1
                j += 1
            out.append(text[i:j + 1])
            i = j + 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


def parse_int(expr):
    """Converts C integer literal, possibly in parentheses and with suffix, to int"""
    expr = expr.strip()
    while expr.startswith("(") and expr.endswith(")"):
        expr = expr[1:-1].strip()
    m = re.fullmatch(r"(0[xX][0-9a-fA-F]+|\d+)[uUlL]*", expr)
    if not m:
        return None
    return int(m.group(1), 0)


def parse_enums(text):
    """Returns dict enumerator name -> integer value for all enums in text"""
    values = {}
    for body in re.findall(r"\benum\b[^{;]*\{(.*?)\}", text, re.S):
        current = -1
        for item in body.split(","):
            item = item.strip()
            if not item:
                continue
            if "=" in item:
                name, val = [x.strip() for x in item.split("=", 1)]
                v = parse_int(val)
                current = v if v is not None else values.get(val, current + 1)
            else:
                name = item
                current += 1
            values[name] = current
    return values


def parse_defines(paths):
    """Returns dict define name -> integer value (only numeric object-like defines)"""
    raw = {}
    for p in paths:
        for m in re.finditer(r"^[ \t]*#[ \t]*define[ \t]+(\w+)[ \t]+(.+)$",
                             strip_comments(read(p)), re.M):
            raw[m.group(1)] = m.group(2).strip()

    resolved = {}

    def resolve(name, depth=0):
        if name in resolved:
            return resolved[name]
        if name not in raw or depth > 10:
            return None
        v = parse_int(raw[name])
        if v is None:
            v = resolve(raw[name].strip("() \t"), depth + 1)
        resolved[name] = v
        return v

    for name in raw:
        resolve(name)
    return {k: v for k, v in resolved.items() if v is not None}


def parse_strings(text):
    """Returns dict variable name -> string for 'static const char *name = "..."' """
    strings = {}
    for m in re.finditer(r"\bconst\s+char\s*\*\s*(\w+)\s*=\s*((?:\s*\"(?:[^\"\\]|\\.)*\")+)\s*;",
                         text):
        literal = "".join(re.findall(r"\"((?:[^\"\\]|\\.)*)\"", m.group(2)))
        # strings in firmware are explicitly null terminated
        literal = literal.split("\\0")[0]
        strings[m.group(1)] = literal.encode("utf-8").decode("unicode_escape")
    return strings


def matching_paren(text, open_pos):
    """Returns index of a parenthesis closing the one at open_pos"""
    depth = 0
    i = open_pos
    while i < len(text):
        c = text[i]
        if c in "\"'":
            j = i + 1
            while j < len(text) and text[j] != c:
                if text[j] == "\\":
                    j += 1
                j += 1
            i = j
        elif c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


def split_args(args):
    """Splits a function call arguments on top level commas"""
    out = []
    depth = 0
    current = []
    i = 0
    while i < len(args):
        c = args[i]
        if c in "\"'":
            j = i + 1
            while j < len(args) and args[j] != c:
                if args[j] == "\\":
                    j += 1
                j += 1
            current.append(args[i:j + 1])
            i = j + 1
            continue
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == "," and depth == 0:
            out.append("".join(current))
            current = []
        else:
            current.append(c)
        i += 1
    out.append("".join(current))
    return [" ".join(a.split()) for a in out]


def parse_event_id_to_str(text, enums):
    """
    Parses event_id_to_str function body. Returns dict
    (source enumerator name, event id define name) -> string variable name
    """
    m = re.search(r"\bevent_id_to_str\s*\([^)]*\)\s*\{", text)
    if not m:
        warn("event_id_to_str not found in " + EVENT_LOG_TO_STRING_C)
        return {}
    end = matching_paren(text, m.end() - 1)
    body = text[m.end():end]

    mapping = {}
    # split function body on 'case EVENT_SRC_xxx:' labels
    parts = re.split(r"\bcase\s+(\w+)\s*:", body)
    for i in range(1, len(parts), 2):
        source = parts[i]
        if source not in enums:
            warn("unknown source in event_id_to_str: " + source)
        for cond, var in re.findall(
                r"event_id\s*==\s*(\w+)\s*\)\s*\{?\s*out\s*=\s*(\w+)\s*;", parts[i + 1]):
            mapping[(source, cond)] = var
    return mapping


def find_calls(path, root):
    """Yields (relative path, line, is_triple, args list) for every call in a file"""
    text = strip_comments(read(path))
    rel = os.path.relpath(path, root)
    for m in CALL_RE.finditer(text):
        # skip definition and declaration of the function itself
        line_start = text.rfind("\n", 0, m.start()) + 1
        if re.search(r"\b(int8_t|void)\s*$", text[line_start:m.start()]):
            continue
        close = matching_paren(text, m.end() - 1)
        if close < 0:
            warn("%s: unbalanced parenthesis in call" % rel)
            continue
        line = text.count("\n", 0, m.start()) + 1
        yield rel, line, m.group(1) is not None, split_args(text[m.end():close])


def to_number(expr, values, what, location):
    if expr in values:
        return values[expr]
    v = parse_int(expr)
    if v is not None:
        return v
    warn("%s: can't resolve %s '%s' to a number, storing as text" % (location, what, expr))
    return expr


def generate(root, with_location):
    enums = parse_enums(strip_comments(read(os.path.join(root, EVENT_LOG_T_H))))

    defs_dir = os.path.join(root, EVENT_DEFINITIONS_DIR)
    defines = parse_defines(sorted(os.path.join(defs_dir, f)
                                   for f in os.listdir(defs_dir) if f.endswith(".h")))

    strings = parse_strings(strip_comments(read(os.path.join(root, EVENT_LOG_STRINGS_H))))
    id_to_str = parse_event_id_to_str(
        strip_comments(read(os.path.join(root, EVENT_LOG_TO_STRING_C))), enums)

    events = []
    for src_dir in SOURCE_DIRS:
        for dirpath, dirnames, filenames in os.walk(os.path.join(root, src_dir)):
            dirnames.sort()
            for fn in sorted(filenames):
                if not fn.endswith(".c"):
                    continue
                for rel, line, triple, args in find_calls(os.path.join(dirpath, fn), root):
                    location = "%s:%d" % (rel, line)
                    names = PARAMS_TRIPLE if triple else PARAMS_SYNC
                    if len(args) != 3 + len(names):
                        warn("%s: unexpected number of arguments (%d), skipping"
                             % (location, len(args)))
                        continue

                    severity, source, event_id = args[0:3]
                    var = id_to_str.get((source, event_id))
                    if var is None:
                        warn("%s: %s / %s not handled in event_id_to_str"
                             % (location, source, event_id))
                        event_string = ""
                    elif var not in strings:
                        warn("%s: string variable %s not found in %s"
                             % (location, var, EVENT_LOG_STRINGS_H))
                        event_string = ""
                    else:
                        event_string = strings[var]

                    entry = {
                        "severity": to_number(severity, enums, "severity", location),
                        "source": to_number(source, enums, "source", location),
                        "eventId": to_number(event_id, defines, "event id", location),
                        "eventString": event_string,
                    }
                    params = dict(zip(names, args[3:]))
                    for p in PARAMS_OUTPUT_ORDER:
                        entry[p] = params.get(p, "")
                    # if parameter value should be displayed in hexadecimal
                    for p in PARAMS_OUTPUT_ORDER:
                        entry["asHex" + p[0].upper() + p[1:]] = True
                    if with_location:
                        entry["file"] = rel
                        entry["line"] = line
                    events.append(entry)

    return {
        "name": "ParaMETEO events log description",
        "lastEditUtcISO8601":
            datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%S+0000"),
        "events": events,
    }


def main():
    default_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("root", nargs="?", default=default_root, help="repository root")
    ap.add_argument("-o", "--output", help="output file, stdout if not given")
    ap.add_argument("--with-location", action="store_true",
                    help="add 'file' and 'line' of each call to entries")
    a = ap.parse_args()

    result = json.dumps(generate(a.root, a.with_location), indent=2, ensure_ascii=False)
    if a.output:
        with open(a.output, "w", encoding="utf-8") as f:
            f.write(result + "\n")
        print("Written %d events to %s" % (result.count('"eventId"'), a.output),
              file=sys.stderr)
    else:
        print(result)


if __name__ == "__main__":
    main()
