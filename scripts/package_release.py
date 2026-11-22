#!/usr/bin/env python3
"""Create portable release archives for Windows, Linux, and macOS."""

import argparse
import os
import shutil
import stat
import sys
import tarfile
import zipfile
from pathlib import Path

EXCLUDED_DIR_NAMES = {"UserData", "Logs"}
EXCLUDED_EXTENSIONS = {".pdb", ".ilk", ".exp", ".lib", ".obj", ".iobj", ".ipdb"}


def copy_clean_tree(src: Path, dst: Path) -> None:
    for root, dirs, files in os.walk(src):
        rel_root = Path(root).relative_to(src)
        dirs[:] = [d for d in dirs if d not in EXCLUDED_DIR_NAMES]
        target_dir = dst / rel_root
        target_dir.mkdir(parents=True, exist_ok=True)
        for filename in files:
            src_file = Path(root) / filename
            if src_file.suffix.lower() in EXCLUDED_EXTENSIONS:
                continue
            shutil.copy2(src_file, target_dir / filename)


def make_zip(stage_dir: Path, archive_path: Path) -> None:
    archive_path.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(archive_path, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        for file_path in sorted(stage_dir.rglob("*")):
            if file_path.is_file():
                zf.write(file_path, file_path.relative_to(stage_dir.parent))


def make_tar_gz(stage_dir: Path, archive_path: Path) -> None:
    archive_path.parent.mkdir(parents=True, exist_ok=True)
    with tarfile.open(archive_path, "w:gz") as tf:
        tf.add(stage_dir, arcname=stage_dir.name)


def ensure_executable(path: Path) -> None:
    if path.is_file() and os.name != "nt":
        mode = path.stat().st_mode
        path.chmod(mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def main() -> int:
    parser = argparse.ArgumentParser(description="Package portable pandaeSokonium release archive.")
    parser.add_argument("--platform", choices=["windows", "linux", "macos"], required=True)
    parser.add_argument("--build-dir", required=True, help="Directory containing built executable / .app bundle")
    parser.add_argument("--out-dir", required=True, help="Output directory for release archive")
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parent.parent
    build_dir = Path(args.build_dir).resolve()
    out_dir = Path(args.out_dir).resolve()

    if not build_dir.is_dir():
        print(f"ERROR: Build directory does not exist: {build_dir}", file=sys.stderr)
        return 1

    stage_root = out_dir / "_stage"
    if stage_root.exists():
        shutil.rmtree(stage_root)
    package_dir = stage_root / f"pandaeSokonium-{args.platform}"
    package_dir.mkdir(parents=True, exist_ok=True)

    try:
        if args.platform == "windows":
            exe = build_dir / "Sokoban-app.exe"
            if not exe.is_file():
                raise RuntimeError(f"Missing executable: {exe}")
            shutil.copy2(exe, package_dir / exe.name)
            for dll in build_dir.glob("*.dll"):
                shutil.copy2(dll, package_dir / dll.name)
            copy_clean_tree(build_dir / "Assets", package_dir / "Assets")
            tools_dir = build_dir / "Tools"
            if tools_dir.is_dir():
                copy_clean_tree(tools_dir, package_dir / "Tools")
            shutil.copy2(repo_root / "THIRD_PARTY_LICENSES.txt", package_dir / "THIRD_PARTY_LICENSES.txt")
            shutil.copy2(repo_root / "README.md", package_dir / "README.md")

            # セルフチェック
            if not (package_dir / "Assets" / "fonts" / "BIZUDPGothic-Regular.ttf").is_file():
                raise RuntimeError("Font asset missing from packaged output")
            if (package_dir / "UserData").exists() or (package_dir / "Assets" / "Logs").exists():
                raise RuntimeError("Forbidden runtime directory found in package")

            archive_path = out_dir / "pandaeSokonium-windows-x64.zip"
            make_zip(package_dir, archive_path)

        elif args.platform == "linux":
            exe = build_dir / "Sokoban-app"
            if not exe.is_file():
                raise RuntimeError(f"Missing executable: {exe}")
            shutil.copy2(exe, package_dir / exe.name)
            ensure_executable(package_dir / exe.name)
            copy_clean_tree(build_dir / "Assets", package_dir / "Assets")
            tools_dir = build_dir / "Tools"
            if tools_dir.is_dir():
                copy_clean_tree(tools_dir, package_dir / "Tools")
                ensure_executable(package_dir / "Tools" / "festival")
            shutil.copy2(repo_root / "THIRD_PARTY_LICENSES.txt", package_dir / "THIRD_PARTY_LICENSES.txt")
            shutil.copy2(repo_root / "README.md", package_dir / "README.md")

            archive_path = out_dir / "pandaeSokonium-linux-x64.tar.gz"
            make_tar_gz(package_dir, archive_path)

        else:  # macos
            app_bundle = build_dir / "Sokoban-app.app"
            if not app_bundle.is_dir():
                raise RuntimeError(f"Missing macOS .app bundle: {app_bundle}")
            copy_clean_tree(app_bundle, package_dir / "Sokoban-app.app")
            ensure_executable(package_dir / "Sokoban-app.app" / "Contents" / "MacOS" / "Sokoban-app")
            ensure_executable(package_dir / "Sokoban-app.app" / "Contents" / "Resources" / "Tools" / "festival")
            shutil.copy2(repo_root / "THIRD_PARTY_LICENSES.txt", package_dir / "THIRD_PARTY_LICENSES.txt")
            shutil.copy2(repo_root / "README.md", package_dir / "README.md")

            archive_path = out_dir / "pandaeSokonium-macos.tar.gz"
            make_tar_gz(package_dir, archive_path)

        print(f"Created release archive: {archive_path} ({archive_path.stat().st_size} bytes)")
        return 0
    finally:
        if stage_root.exists():
            shutil.rmtree(stage_root, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
