import ctypes
import pathlib
from typing import Optional


def _candidate_paths() -> list[pathlib.Path]:
    root = pathlib.Path(__file__).resolve().parents[2]
    package_dir = pathlib.Path(__file__).resolve().parent
    return [
        package_dir / "game2048_dynamic_export.dll",
        root / "build" / "game2048_dynamic_export.dll",
        root / "cmake-build-debug" / "game2048_dynamic_export.dll",
        root / "cmake-build-release" / "game2048_dynamic_export.dll",
        root / "build" / "Desktop_Qt_6_9_3_MSVC2022_64bit-Debug" / "game2048_dynamic_export.dll",
    ]


def load_library(dll_path: Optional[str] = None) -> ctypes.CDLL:
    if dll_path:
        return ctypes.CDLL(dll_path)

    for p in _candidate_paths():
        if p.exists():
            return ctypes.CDLL(str(p))

    raise FileNotFoundError(
        "game2048_dynamic_export.dll not found. "
        "Build target 'game2048_dynamic_export' first or pass explicit dll_path."
    )
