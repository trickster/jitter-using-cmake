// run with ./bin/izmirvm-minimal ../vm-tests.txt
#include <stdio.h>
#include <stdlib.h>
#include <jitter/jitter.h>
#include "izmirvm-vm.h"

int main(int argc, char **argv) {
    // Parse command-line arguments
    const char *input_file = NULL;

    if (argc > 1) {
        input_file = argv[1];
    }

    izmirvm_initialize();
    struct izmirvm_state s;
    izmirvm_state_initialize (& s);

    struct izmirvm_mutable_routine *r = izmirvm_make_mutable_routine();

    // Parse the mutable routine either from stdin or from the specified file
    struct izmirvm_routine_parse_error *parse_error_p = NULL;
        // Read from a specified file
    fprintf(stdout, "Parsing file ...\n");
    parse_error_p = izmirvm_parse_mutable_routine_from_file(input_file, r);

    // If parsing was successful, make the routine executable
    if (parse_error_p == NULL) {
        struct izmirvm_executable_routine *er = izmirvm_make_executable_routine(r);

        // Use the executable routine as needed here
        // ...
        fprintf(stdout, "Interpreting...\n");
        izmirvm_execute_executable_routine(er, & s);

        // Clean up the executable routine
        izmirvm_destroy_executable_routine(er);
    } else {
        // Handle the parsing error
        fprintf(stderr, "%s:%i near \"%s\": %s\n",
                parse_error_p->file_name, parse_error_p->error_line_no,
                parse_error_p->error_token_text,
                izmirvm_routine_edit_status_to_string(parse_error_p->status));

        izmirvm_routine_parse_error_destroy(parse_error_p);
    }

    izmirvm_destroy_mutable_routine(r);
    izmirvm_state_finalize(&s);
    izmirvm_finalize();

    return parse_error_p != NULL ? EXIT_FAILURE : EXIT_SUCCESS;
}
