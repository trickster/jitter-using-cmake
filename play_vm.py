import sys
import argparse
from ctypes import CDLL, c_char_p, c_int, c_void_p, Structure, POINTER, c_bool, byref
import pathlib

# Load the shared library
lib = CDLL(f"{pathlib.Path.cwd()}/build/libizmirvmlib.so")

class IzmirVMState(Structure):
    pass

class IzmirvmExitStatus(c_int):
    pass

# Define C structures and enums
class JitterMutableRoutine(Structure):
    pass

class JitterExecutableRoutine(Structure):
    pass

class JitterRoutineParseError(Structure):
    _fields_ = [
        ("file_name", c_char_p),
        ("error_line_no", c_int),
        ("error_token_text", c_char_p),
        ("status", c_int)
    ]


def main():
    parser = argparse.ArgumentParser(description="Run a routine encoded as a text file on the Izmirvm VM")
    args = parser.parse_args()

    lib.izmirvm_initialize.argtypes = []
    lib.izmirvm_initialize.restype = None

    lib.izmirvm_make_mutable_routine.argtypes = []
    lib.izmirvm_make_mutable_routine.restype = POINTER(JitterMutableRoutine)

    lib.jitter_make_executable_routine.argtypes = [POINTER(JitterMutableRoutine)]
    lib.jitter_make_executable_routine.restype = POINTER(JitterExecutableRoutine)

    lib.izmirvm_state_initialize.argtypes = [POINTER(IzmirVMState)]
    lib.izmirvm_state_initialize.restype = None

    lib.izmirvm_state_finalize.argtypes = [POINTER(IzmirVMState)]
    lib.izmirvm_state_finalize.restype = None

    lib.jitter_destroy_executable_routine.argtypes = [POINTER(JitterExecutableRoutine)]
    lib.jitter_destroy_executable_routine.restype = None

    lib.jitter_destroy_mutable_routine.argtypes = [POINTER(JitterMutableRoutine)]
    lib.jitter_destroy_mutable_routine.restype = None

    lib.izmirvm_finalize.argtypes = []
    lib.izmirvm_finalize.restype = None


    print("Initializing...", file=sys.stderr)

    lib.izmirvm_initialize()

    r = lib.izmirvm_make_mutable_routine()

    parse_error = lib.izmirvm_parse_mutable_routine_from_file("vm-tests.txt".encode(), r)

    if parse_error:
        print(f"{parse_error.contents.file_name.decode()}:{parse_error.contents.error_line_no} near \"{parse_error.contents.error_token_text.decode()}\": {parse_error.contents.status}")
        return 1

    print("Making executable...", file=sys.stderr)

    er = lib.jitter_make_executable_routine(r)
    print(er)
    print("Initializing VM state...", file=sys.stderr)

    vm_state = IzmirVMState()
    lib.izmirvm_state_initialize(vm_state)

    lib.izmirvm_execute_executable_routine.argtypes = [POINTER(JitterExecutableRoutine), POINTER(IzmirVMState)]
    lib.izmirvm_execute_executable_routine.restype = IzmirvmExitStatus

    print("Interpreting...", file=sys.stderr)
    lib.izmirvm_execute_executable_routine(er, vm_state)

    # print("Destroying the routine data structure...", file=sys.stderr)
    # lib.jitter_destroy_executable_routine(er)
    # lib.jitter_destroy_mutable_routine(r)

    # print("Finalizing VM state...", file=sys.stderr)

    # # somthing wrong here
    # lib.izmirvm_state_finalize(byref(vm_state))

    print("finalizing...", file=sys.stderr)
    lib.izmirvm_finalize()

if __name__ == "__main__":
    sys.exit(main())
