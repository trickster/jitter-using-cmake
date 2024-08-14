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

    # print("Interpreting...", file=sys.stderr)
    lib.izmirvm_execute_executable_routine(er, vm_state)


    print("Destroying the routine data structure...", file=sys.stderr)
    lib.jitter_destroy_executable_routine(er)
    lib.jitter_destroy_mutable_routine(r)

    print("Finalizing VM state...", file=sys.stderr)

    # somthing wrong here
    lib.izmirvm_state_finalize(byref(vm_state))

    lib.izmirvm_finalize()

if __name__ == "__main__":
    sys.exit(main())



"""
izmirvm_asm_footers
izmirvm_asm_headers
izmirvm_branch_to_program_point
izmirvm_call_related_specialized_instruction_id_no
izmirvm_call_related_specialized_instruction_ids
izmirvm_data_locations
izmirvm_data_locations_size_in_bytes
izmirvm_defect_descriptors
izmirvm_defect_descriptors_correct_displacement
izmirvm_defect_descriptors_size_in_bytes
izmirvm_defect_print
izmirvm_defect_print_replacement_table
izmirvm_defect_print_summary
izmirvm_defective_specialized_instructions
izmirvm_dump_data_locations
izmirvm_ensure_enough_slow_registers_for_executable_routine
izmirvm_ensure_enough_slow_registers_for_routine
izmirvm_execute_executable_routine
izmirvm_execute_routine
izmirvm_finalize
izmirvm_initialize
izmirvm_initialize_threads
izmirvm_make_mutable_routine
izmirvm_make_place_for_slow_registers
izmirvm_meta_instruction_hash
izmirvm_meta_instructions
izmirvm_parse_mutable_routine_from_file
izmirvm_parse_mutable_routine_from_file_star
izmirvm_parse_mutable_routine_from_string
izmirvm_patch_in_descriptors
izmirvm_patch_in_descriptors_size_in_bytes
izmirvm_profile_runtime_clear
izmirvm_profile_runtime_destroy
izmirvm_profile_runtime_make
izmirvm_profile_runtime_merge_from
izmirvm_profile_runtime_merge_from_state
izmirvm_profile_runtime_print_specialized
izmirvm_profile_runtime_print_unspecialized
izmirvm_profile_specialized_from_runtime
izmirvm_profile_unspecialized_from_runtime
izmirvm_register_class_character_to_register_class
izmirvm_regiter_classes
izmirvm_replacement_table
izmirvm_rewrite
izmirvm_specialize_instruction
izmirvm_specialized_instruction_call_relateds
izmirvm_specialized_instruction_callees
izmirvm_specialized_instruction_callers
izmirvm_specialized_instruction_fast_label_bitmasks
izmirvm_specialized_instruction_label_bitmasks
izmirvm_specialized_instruction_names
izmirvm_specialized_instruction_relocatables
izmirvm_specialized_instruction_residual_arities
izmirvm_specialized_instruction_to_unspecialized_instruction
izmirvm_state_destroy
izmirvm_state_finalize
izmirvm_state_initialize
izmirvm_state_initialize_with_slow_registers
izmirvm_state_make
izmirvm_state_make_with_slow_registers
izmirvm_state_profile_runtime
izmirvm_state_reset
izmirvm_states
izmirvm_thread_ends
izmirvm_thread_sizes
izmirvm_threads
izmirvm_vm
izmirvm_vm_configuration
izmirvm_worst_case_replacement_table
jitter_native_snippet_names
jitter_native_snippet_pointers
jitter_native_snippet_sizes
"""
