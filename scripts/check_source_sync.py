#!/usr/bin/env python3
"""Verify that source lists in CMakeLists.txt and Sokoban-app.vcxproj stay in sync."""

import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


def load_cmake_sources(cmake_path: Path) -> set[str]:
    text = cmake_path.read_text(encoding="utf-8")
    match = re.search(r"set\s*\(\s*PANDAE_SOURCES(.*?)\)", text, re.DOTALL)
    if not match:
        raise RuntimeError("PANDAE_SOURCES block not found in CMakeLists.txt")
    sources: set[str] = set()
    for line in match.group(1).splitlines():
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        if line.startswith("Sokoban-app/"):
            sources.add(line[len("Sokoban-app/"):])
        else:
            sources.add(line)
    return sources


def load_vcxproj_sources(vcxproj_path: Path) -> set[str]:
    tree = ET.parse(vcxproj_path)
    root = tree.getroot()
    ns = {"ms": "http://schemas.microsoft.com/developer/msbuild/2003"}
    sources: set[str] = set()
    for elem in root.findall(".//ms:ClCompile", ns):
        include = elem.attrib.get("Include")
        if include:
            sources.add(include.replace("\\", "/"))
    return sources


def main() -> int:
    repo_root = Path(__file__).resolve().parent.parent
    cmake_sources = load_cmake_sources(repo_root / "CMakeLists.txt")
    vcxproj_sources = load_vcxproj_sources(repo_root / "Sokoban-app" / "Sokoban-app.vcxproj")

    only_cmake = sorted(cmake_sources - vcxproj_sources)
    only_vcxproj = sorted(vcxproj_sources - cmake_sources)

    if only_cmake or only_vcxproj:
        print("ERROR: Source file lists are out of sync!", file=sys.stderr)
        if only_cmake:
            print("  Only in CMakeLists.txt:", ", ".join(only_cmake), file=sys.stderr)
        if only_vcxproj:
            print("  Only in Sokoban-app.vcxproj:", ", ".join(only_vcxproj), file=sys.stderr)
        return 1

    print(f"OK: {len(cmake_sources)} source files are in sync between CMakeLists.txt and Sokoban-app.vcxproj.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
