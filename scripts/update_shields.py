#!/usr/bin/env python3
"""Update documentation and test coverage badges in README.md."""

import os
import re
import subprocess
import sys
import urllib.parse


def get_color(pct_str):
    """Return shield color string for a percentage string."""
    try:
        pct = float(pct_str.rstrip("%"))
        if pct >= 90:
            return "brightgreen"
        if pct >= 80:
            return "green"
        if pct >= 70:
            return "yellowgreen"
        if pct >= 60:
            return "yellow"
        return "red"
    except Exception:
        return "brightgreen"


def get_test_coverage():
    """Calculate test line coverage using gcovr or ctest."""
    candidate_dirs = ["build_gcc", "cmake-build-debug", "build"]
    build_dir = None

    for d in candidate_dirs:
        if os.path.isdir(d):
            for _, _, files in os.walk(d):
                if any(f.endswith(".gcda") or f.endswith(".gcno") for f in files):
                    build_dir = d
                    break
            if build_dir:
                break

    if not build_dir:
        build_dir = "build_gcc"
        try:
            subprocess.check_call(
                [
                    "cmake",
                    "-S",
                    ".",
                    "-B",
                    build_dir,
                    "-DCMAKE_BUILD_TYPE=Debug",
                    "-DBUILD_TESTING=ON",
                    "-DCMAKE_C_FLAGS=--coverage",
                    "-DCMAKE_EXE_LINKER_FLAGS=--coverage",
                ]
            )
            subprocess.check_call(["cmake", "--build", build_dir])
            subprocess.check_call(
                [
                    "ctest",
                    "--test-dir",
                    build_dir,
                    "-C",
                    "Debug",
                    "--output-on-failure",
                ]
            )
        except Exception as e:
            print(f"Build failed: {e}", file=sys.stderr)

    if build_dir and os.path.isdir(build_dir):
        has_gcda = False
        for _, _, files in os.walk(build_dir):
            if any(f.endswith(".gcda") for f in files):
                has_gcda = True
                break
        if not has_gcda:
            try:
                subprocess.check_call(
                    [
                        "ctest",
                        "--test-dir",
                        build_dir,
                        "-C",
                        "Debug",
                        "--output-on-failure",
                    ]
                )
            except Exception as e:
                print(f"ctest run failed: {e}", file=sys.stderr)

        try:
            res = subprocess.run(
                [
                    "gcovr",
                    "-r",
                    ".",
                    "--print-summary",
                    "-e",
                    ".*tests.*",
                    "-e",
                    ".*CMakeFiles.*",
                    "-e",
                    ".*greatest.*",
                    "-e",
                    ".*_deps.*",
                    "-e",
                    ".*rhash_include.*",
                    "-e",
                    ".*amalgamation.*",
                    build_dir,
                ],
                capture_output=True,
                text=True,
            )
            if res.returncode == 0:
                match = re.search(r"lines:\s+([0-9.]+)%", res.stdout)
                if match:
                    return f"{float(match.group(1)):.2f}%"
        except Exception as e:
            print(f"gcovr failed: {e}", file=sys.stderr)

    return "0.00%"


def get_doc_coverage():
    """Calculate Doxygen documentation coverage across public headers."""
    headers = [
        "rhasheq/rhasheq.h",
        "rhasheq/rhasheq_stdbool.h",
        "rhasheq/no_discard.h",
    ]
    total = 0
    documented = 0
    for h in headers:
        if not os.path.exists(h):
            continue
        with open(h, "r", encoding="utf-8") as f:
            content = f.read()
        impl_split = content.split("#ifdef RHASHEQ_IMPLEMENTATION")[0]
        parts = re.split(
            r"(?m)^(?:\s*(?:RHASHEQ_)?NO_DISCARD\s+)?RHASHEQ_EXPORT\b", impl_split
        )
        file_total = len(parts) - 1
        total += file_total
        for i in range(file_total):
            text_before = parts[i]
            last_comment_end = text_before.rfind("*/")
            if last_comment_end != -1:
                last_comment_start = text_before.rfind("/**", 0, last_comment_end)
                if last_comment_start != -1:
                    documented += 1
    if total == 0:
        return "100.00%"
    return f"{(documented / total) * 100:.2f}%"


def update_readme():
    """Update README.md with doc and test coverage badges."""
    test_cov = get_test_coverage()
    doc_cov = get_doc_coverage()

    test_color = get_color(test_cov)
    doc_color = get_color(doc_cov)

    test_cov_enc = urllib.parse.quote(test_cov)
    doc_cov_enc = urllib.parse.quote(doc_cov)

    test_shield = f"![Test Coverage](https://img.shields.io/badge/test__coverage-{test_cov_enc}-{test_color})"
    doc_shield = f"![Doc Coverage](https://img.shields.io/badge/doc__coverage-{doc_cov_enc}-{doc_color})"

    with open("README.md", "r", encoding="utf-8") as f:
        readme = f.read()

    readme = re.sub(
        r"\[?!\[(?:Test|test)[ _]?Coverage\]\([^)]+\)\]?(?:\([^)]+\))?\n?",
        "",
        readme,
    )
    readme = re.sub(
        r"\[?!\[(?:Doc|doc)[ _]?Coverage\]\([^)]+\)\]?(?:\([^)]+\))?\n?",
        "",
        readme,
    )

    license_shield = (
        r"(\[!\[License\]\(https://img\.shields\.io/badge/license-[^)]+\)\]\([^)]+\))"
    )
    new_shields = "\\1\n" + doc_shield + "\n" + test_shield

    new_readme = re.sub(license_shield, new_shields, readme, count=1)

    with open("README.md", "w", encoding="utf-8") as f:
        f.write(new_readme)


if __name__ == "__main__":
    update_readme()
