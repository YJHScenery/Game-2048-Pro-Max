import ctypes
from typing import Iterable, List, Sequence

from .loader import load_library

MOVE_NEGATIVE = -1
MOVE_POSITIVE = 1


class _BaseBoard:
    _prefix = ""
    _ctype = None

    def __init__(self, dll_path: str | None = None):
        self._lib = load_library(dll_path)
        self._bind_functions()
        self._h = self._create()
        if not self._h:
            raise RuntimeError(f"{self._prefix}_create failed")

    def __del__(self):
        if hasattr(self, "_h") and self._h:
            self._destroy(self._h)
            self._h = None

    def _bind_functions(self):
        p = self._prefix

        self._create = getattr(self._lib, f"{p}_create")
        self._create.restype = ctypes.c_void_p

        self._destroy = getattr(self._lib, f"{p}_destroy")
        self._destroy.argtypes = [ctypes.c_void_p]

        self._configure = getattr(self._lib, f"{p}_configure")
        self._configure.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_size_t), ctypes.c_size_t]
        self._configure.restype = ctypes.c_bool

        self._reset = getattr(self._lib, f"{p}_reset_and_seed")
        self._reset.argtypes = [ctypes.c_void_p, ctypes.c_size_t]

        self._operate = getattr(self._lib, f"{p}_operate")
        self._operate.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_int32]
        self._operate.restype = ctypes.c_bool

        self._operate_and_spawn = getattr(self._lib, f"{p}_operate_and_spawn")
        self._operate_and_spawn.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_int32]
        self._operate_and_spawn.restype = ctypes.c_bool

        self._total = getattr(self._lib, f"{p}_total_elements")
        self._total.argtypes = [ctypes.c_void_p]
        self._total.restype = ctypes.c_size_t

        self._rank = getattr(self._lib, f"{p}_rank")
        self._rank.argtypes = [ctypes.c_void_p]
        self._rank.restype = ctypes.c_size_t

        self._get_sizes = getattr(self._lib, f"{p}_get_sizes")
        self._get_sizes.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_size_t), ctypes.c_size_t]
        self._get_sizes.restype = ctypes.c_bool

        self._set_data = getattr(self._lib, f"{p}_set_data")
        self._set_data.argtypes = [ctypes.c_void_p, ctypes.POINTER(self._ctype), ctypes.c_size_t]
        self._set_data.restype = ctypes.c_bool

        self._get_data = getattr(self._lib, f"{p}_get_data")
        self._get_data.argtypes = [ctypes.c_void_p, ctypes.POINTER(self._ctype), ctypes.c_size_t]
        self._get_data.restype = ctypes.c_bool

        self._hash = getattr(self._lib, f"{p}_hash")
        self._hash.argtypes = [ctypes.c_void_p]
        self._hash.restype = ctypes.c_uint64

    def configure(self, dims: Sequence[int]) -> bool:
        arr = (ctypes.c_size_t * len(dims))(*dims)
        return bool(self._configure(self._h, arr, len(dims)))

    def reset_and_seed(self, initial_tile_count: int = 2) -> None:
        self._reset(self._h, initial_tile_count)

    def operate(self, dim: int, positive: bool) -> bool:
        d = MOVE_POSITIVE if positive else MOVE_NEGATIVE
        return bool(self._operate(self._h, dim, d))

    def operate_and_spawn(self, dim: int, positive: bool) -> bool:
        d = MOVE_POSITIVE if positive else MOVE_NEGATIVE
        return bool(self._operate_and_spawn(self._h, dim, d))

    def total_elements(self) -> int:
        return int(self._total(self._h))

    def rank(self) -> int:
        return int(self._rank(self._h))

    def sizes(self) -> List[int]:
        r = self.rank()
        arr = (ctypes.c_size_t * r)()
        ok = self._get_sizes(self._h, arr, r)
        if not ok:
            raise RuntimeError("get_sizes failed")
        return [int(v) for v in arr]

    def set_data(self, data: Iterable):
        values = list(data)
        arr = (self._ctype * len(values))(*values)
        ok = self._set_data(self._h, arr, len(values))
        if not ok:
            raise RuntimeError("set_data failed")

    def data(self) -> List:
        n = self.total_elements()
        arr = (self._ctype * n)()
        ok = self._get_data(self._h, arr, n)
        if not ok:
            raise RuntimeError("get_data failed")
        return list(arr)

    def hash(self) -> int:
        return int(self._hash(self._h))


class Logic2048DynamicU64(_BaseBoard):
    _prefix = "g2048_u64"
    _ctype = ctypes.c_uint64


class Logic2048DynamicI64(_BaseBoard):
    _prefix = "g2048_i64"
    _ctype = ctypes.c_int64


class Logic2048DynamicF32(_BaseBoard):
    _prefix = "g2048_f32"
    _ctype = ctypes.c_float


class Logic2048DynamicF64(_BaseBoard):
    _prefix = "g2048_f64"
    _ctype = ctypes.c_double
