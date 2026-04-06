# game2048_dynamic

Python ctypes wrapper for `game2048_dynamic_export.dll`.

## Build DLL

From project root with CMake target:

```powershell
cmake --build cmake-build-debug --target game2048_dynamic_export
```

## Install Python Package

```powershell
cd python
pip install -e .
```

## Quick Start

```python
from game2048_dynamic import Logic2048DynamicU64

b = Logic2048DynamicU64()
b.configure([4, 4])
b.reset_and_seed(2)
changed = b.operate_and_spawn(0, positive=False)
print(changed, b.data())
```
