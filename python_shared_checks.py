from ctypes import CDLL
import pathlib

lib = CDLL(f"{pathlib.Path.cwd()}/build/libizmirvmlib.so")

lib.izmirvm_initialize.argtypes = []
lib.izmirvm_initialize.restype = None

lib.izmirvm_finalize.argtypes = []
lib.izmirvm_finalize.restype = None

try:
    lib.izmirvm_initialize()
    lib.izmirvm_finalize()
    print("ALL DONE")
except Exception as _:
    print("SOMETHING WRONG")
