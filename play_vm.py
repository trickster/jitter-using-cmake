import sys
import argparse
from ctypes import CDLL, CFUNCTYPE, c_char_p, c_int, c_void_p, Structure, POINTER, c_bool, byref, cast
import pathlib

# Load the shared library
lib = CDLL(f"{pathlib.Path.cwd()}/build/libizmirvmlib.so")

class IzmirVMState(Structure):
    pass

class IzmirvmExitStatus(c_int):
    pass


class JitterPrintContextStruct(Structure):
    _fields_ = []



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

jitter_print_context = POINTER(JitterPrintContextStruct)

lib.izmirvm_initialize.argtypes = []
lib.izmirvm_initialize.restype = None

lib.izmirvm_make_mutable_routine.argtypes = []
lib.izmirvm_make_mutable_routine.restype = POINTER(JitterMutableRoutine)

lib.jitter_make_executable_routine.argtypes = [POINTER(JitterMutableRoutine)]
lib.jitter_make_executable_routine.restype = POINTER(JitterExecutableRoutine)

lib.izmirvm_execute_executable_routine.argtypes = [POINTER(JitterExecutableRoutine), POINTER(IzmirVMState)]
lib.izmirvm_execute_executable_routine.restype = IzmirvmExitStatus

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


# Define the FILE type from libc
FILE = c_void_p

# Load the C library
libc = CDLL('libc.so.6')  # Assuming you're on a Unix-like system

# Get the stdout FILE pointer from libc
stdout = cast(libc.stdout, FILE)
jitter_print_context_make_file_star = CFUNCTYPE(jitter_print_context, FILE)(("jitter_print_context_make_file_star", lib))
jpc = jitter_print_context_make_file_star(stdout)


def main():
    parser = argparse.ArgumentParser(description="Run a routine encoded as a text file on the Izmirvm VM")
    args = parser.parse_args()

    print("Initializing....")
    lib.izmirvm_initialize()
    vm_state = IzmirVMState()
    lib.izmirvm_state_initialize(vm_state)

    print("Making mutable routine...")
    r = lib.izmirvm_make_mutable_routine()

    print("parsing from file...")
    parse_error = lib.izmirvm_parse_mutable_routine_from_file("vm-tests.txt".encode(), r)

    if parse_error:
        print(f"{parse_error.contents.file_name.decode()}:{parse_error.contents.error_line_no} near \"{parse_error.contents.error_token_text.decode()}\": {parse_error.contents.status}")
        return 1

    print("Making executable...", file=sys.stderr)
    er = lib.jitter_make_executable_routine(r)
    print(er)

    print("Interpreting...", file=sys.stderr)
    lib.izmirvm_execute_executable_routine(er, byref(vm_state))


    # print("Destroying the routine data structure...", file=sys.stderr)
    lib.jitter_destroy_executable_routine(er)
    lib.jitter_destroy_mutable_routine(r)
    lib.izmirvm_state_finalize(byref(vm_state))

    print("finalizing...", file=sys.stderr)
    lib.izmirvm_finalize()

if __name__ == "__main__":
    sys.exit(main())
