/* Jittery structured language example: main.

   Copyright (C) 2017, 2019, 2020, 2021, 2022 Luca Saiu
   Copyright (C) 2022 ???
   Written by Luca Saiu and ???

   This file is part of the İzmir example, based on the Jitter
   structured-language example which is distributed along with GNU Jitter under
   its same license.

   GNU Jitter is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   GNU Jitter is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Jitter.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jitter/jitter-fatal.h>

#include "izmir-parser.h"
#include "izmir-syntax.h"

/* Why this source file parses argc and argv directly.
 * ************************************************************************** */

/* This example does not use argp, differently from the rest of Jitter, in order
   to keep portability without requiring Gnulib.  The intent here is to simplify
   the example build system for users looking at how to adopt Jitter for their
   own code, even at a small cost in complexity within this source file.

   The command-line interface of the structued program is simple, but civilized
   enough to respect the GNU conventions.

   This example uses routines through the unified API. */

/* Global variables.
 * ************************************************************************** */

/* The program name as it was invoked from the shell, or in other words a copy
   of the pointer in argv [0] , globally visible. */
static char *izmir_program_name;

/* See the comment about help_section_indentation in jitter-config.in . */
#define IZMIR_HELP_SECTION_INDENTATION ""

/* Utility functions for the command line.
 * ************************************************************************** */

/* Print a fatal error message and exit with failure, in response to an
   incorrect command line.  The other_information string is printed right
   after the error message, with no preceeding space.  It is a crude but
   convenient way of providing an "argument" to error messages. */
static void izmir_usage(char *error_message, char *other_information) {
  fprintf(stderr, "%s: %s%s.\n", izmir_program_name, error_message,
          other_information);
  fprintf(stderr, "Try '%s --help' for more information.\n",
          izmir_program_name);

  exit(EXIT_FAILURE);
}

/* Print a section heading in --help , with the given heading title. */
static void izmir_help_section(const char *title) {
  printf("\n" IZMIR_HELP_SECTION_INDENTATION "%s:\n", title);
}

/* Print command-line interface help and exit with success. */
static void izmir_help(void) {
  printf("Usage: %s [OPTION...] FILE.izmir\n", izmir_program_name);
  printf("   or: %s [OPTION...] -\n", izmir_program_name);
  printf("Print the İzmirVM translation of an İzmir-language program.");

  izmir_help_section("Common GNU-style options");
  printf("      --help                       give this help list and exit\n");
  printf("      --version                    print program version and exit\n");

  printf("\n");
  printf("An \"--\" argument terminates option processing.\n");

  printf("\n");
  printf(JITTER_PACKAGE_NAME " home page: <" JITTER_PACKAGE_URL ">.\n");
  printf("\n");
  printf("Report bugs to <" JITTER_PACKAGE_BUGREPORT ">.\n");
  printf("General help using GNU software: <https://www.gnu.org/gethelp/>.\n");

  exit(EXIT_SUCCESS);
}

/* Print version information and exit with success. */
static void izmir_version(void) {
  printf("İzmir-to-İzmirVM translator");
  printf(
      "Copyright (C) 2022 Luca Saiu, ???.\n"
      "GNU Jitter comes with ABSOLUTELY NO WARRANTY.\n"
      "You may redistribute copies of GNU Jitter under the terms of the GNU\n"
      "General Public License, version 3 or any later version published\n"
      "by the Free Software Foundation.  For more information see the\n"
      "file named COPYING.\n"
      "\n"
      "Written by Luca Saiu <http://ageinghacker.net> and ???.\n");

  exit(EXIT_SUCCESS);
}

/* Command-line handling.
 * ************************************************************************** */

/* A specifier for the code generator which is being used. */
enum izmir_code_generator {
  /* Generate stack-based instructions */
  izmir_code_generator_stack,

  /* Generate register-based instructions */
  izmir_code_generator_register
};

/* What to print about defective specialised instructions. */
enum izmir_print_defect_what {
  izmir_print_defect_what_summary,
  izmir_print_defect_what_list,
  izmir_print_defect_what_replacements,
  izmir_print_defect_what_no
};

/* The state encoded in a user command line. */
struct izmir_command_line {
  /* True iff we should print back the VM routine. */
  bool print;

  /* True iff we should use the cross-disassembler rather than the native
     disassembler for the VM routine.  If false, use the native
     disassembler.  */
  bool cross_disassemble;

  /* True iff we should disassemble the VM routine. */
  bool disassemble;

  /* True iff we should print information about defective specialised
     instructions and their replacements. */
  enum izmir_print_defect_what print_defects;

  /* True iff we should print profiling information, respectively for
     specialised and unspecialised instructions. */
  bool profile_specialized;
  bool profile_unspecialized;

  /* True iff we should print data locations. */
  bool print_locations;

  /* True iff we should not actually run the VM routine. */
  bool dry_run;

  /* True iff we should disable fast literals, for benchmarking a worst-case
     scenario or for comparing with some other implementation. */
  bool slow_literals_only;

  /* Like slow_literals_only, but for fast registers. */
  bool slow_registers_only;

  /* True iff we should enable optimization rewriting. */
  bool optimization_rewriting;

  /* Which code generator is being used. */
  enum izmir_code_generator code_generator;

  /* Pathname of the program source to be loaded. */
  char *program_path;
};

/* Inizialize the command-line state to sensible defaults; make the program path
   intentionally invalid to catch errors. */
static void izmir_initialize_command_line(struct izmir_command_line *cl) {
  cl->print = false;
  cl->cross_disassemble = false;
  cl->disassemble = false;
  cl->print_defects = izmir_print_defect_what_no;
  cl->profile_specialized = false;
  cl->profile_unspecialized = false;
  cl->print_locations = false;
  cl->dry_run = false;
  cl->optimization_rewriting = true;
  cl->slow_literals_only = false;
  cl->slow_registers_only = false;
  cl->code_generator = izmir_code_generator_register;
  cl->program_path = NULL;
}

/* Set the program name in the pointed command line structure to the given
   value, or fail fatally if the name was already set. */
static void izmir_set_command_line_program(struct izmir_command_line *cl,
                                           char *arg) {
  if (cl->program_path != NULL)
    izmir_usage("more than one program given; the second is ", arg);
  cl->program_path = arg;
}

/* Fill the pointed command-line data structure with information from the
   actual command line. */
static void izmir_parse_command_line(struct izmir_command_line *cl, int argc,
                                     char **argv) {
  izmir_program_name = argv[0];
  izmir_initialize_command_line(cl);

  int i;
  bool handle_options = true;
  for (i = 1; i < argc; i++) {
    /* Get the current argument, be it option or non-option. */
    char *arg = argv[i];

    /* If we are still handling options but the current argument is "--" then
       stop doing that, and don't handle the current argument any further. */
    if (handle_options && !strcmp(arg, "--")) {
      handle_options = false;
      continue;
    }

    /* Handle arg, as an option or a pathname. */
    if (handle_options && !strcmp(arg, "--help"))
      izmir_help();
    else if (handle_options && !strcmp(arg, "--version"))
      izmir_version();
    else if (handle_options && !strcmp(arg, "--disassemble"))
      cl->disassemble = true;
    else if (handle_options && !strcmp(arg, "--cross-disassemble")) {
      cl->cross_disassemble = true;
      cl->disassemble = true;
    } else if (handle_options && !strcmp(arg, "--print-defects=summary"))
      cl->print_defects = izmir_print_defect_what_summary;
    else if (handle_options && !strcmp(arg, "--print-defects=list"))
      cl->print_defects = izmir_print_defect_what_list;
    else if (handle_options && !strcmp(arg, "--print-defects=replacements"))
      cl->print_defects = izmir_print_defect_what_replacements;
    else if (handle_options && !strcmp(arg, "--print-defects=no"))
      cl->print_defects = izmir_print_defect_what_no;
    else if (handle_options && !strcmp(arg, "--profile-specialized"))
      cl->profile_specialized = true;
    else if (handle_options && !strcmp(arg, "--profile-unspecialized"))
      cl->profile_unspecialized = true;
    else if (handle_options && !strcmp(arg, "--no-print-defects"))
      cl->print_defects = izmir_print_defect_what_no;
    else if (handle_options && !strcmp(arg, "--no-profile-unspecialized"))
      cl->profile_unspecialized = false;
    else if (handle_options && !strcmp(arg, "--no-profile-specialized"))
      cl->profile_specialized = false;
    else if (handle_options && !strcmp(arg, "--print-locations"))
      cl->print_locations = true;
    else if (handle_options && !strcmp(arg, "--no-print-locations"))
      cl->print_locations = false;
    else if (handle_options && !strcmp(arg, "--slow-literals-only"))
      cl->slow_literals_only = true;
    else if (handle_options && !strcmp(arg, "--no-slow-literals-only"))
      cl->slow_literals_only = false;
    else if (handle_options && !strcmp(arg, "--slow-registers-only"))
      cl->slow_registers_only = true;
    else if (handle_options && !strcmp(arg, "--no-slow-registers-only"))
      cl->slow_registers_only = false;
    else if (handle_options && !strcmp(arg, "--slow-only")) {
      cl->slow_literals_only = true;
      cl->slow_registers_only = true;
    } else if (handle_options && !strcmp(arg, "--no-slow-only")) {
      cl->slow_literals_only = false;
      cl->slow_registers_only = false;
    } else if (handle_options && !strcmp(arg, "--optimization-rewriting"))
      cl->optimization_rewriting = true;
    else if (handle_options && !strcmp(arg, "--no-optimization-rewriting"))
      cl->optimization_rewriting = false;
    else if (handle_options && !strcmp(arg, "--stack"))
      cl->code_generator = izmir_code_generator_stack;
    else if (handle_options && !strcmp(arg, "--register"))
      cl->code_generator = izmir_code_generator_register;
    else if (handle_options &&
             (!strcmp(arg, "--print") || !strcmp(arg, "--print-routine")))
      cl->print = true;
    else if (handle_options &&
             (!strcmp(arg, "--no-print") || !strcmp(arg, "--no-print-routine")))
      cl->print = false;
    else if (handle_options && !strcmp(arg, "--dry-run"))
      cl->dry_run = true;
    else if (handle_options && !strcmp(arg, "--no-dry-run"))
      cl->dry_run = false;
    else if (handle_options && strlen(arg) > 1 && arg[0] == '-')
      izmir_usage("unrecognized option ", arg);
    else if (handle_options && strlen(arg) > 1 && arg[0] != '-')
      izmir_set_command_line_program(cl, arg);
    else
      izmir_set_command_line_program(cl, arg);
  }

  /* Still not having a program name at the end is an error. */
  if (cl->program_path == NULL)
    izmir_usage("program name missing", "");
}

static void izmir_compile_expression(struct izmir_expression *exp) {
  switch (exp->case_)
  {
    case izmir_expression_case_literal:
      printf("pushconstant %li\n", (long)exp->literal);
      break;
    default:
      printf("NOT SUPPORTED\n");
      break;
  }
  // printf("pushconstant 42\n");
}

static void izmir_compile_statement(struct izmir_statement *st) {
  switch (st->case_) {
  case izmir_statement_case_skip:
    printf("# skip\n");
    break;
  case izmir_statement_case_assignment:
    printf("Case not implemented\n");
    break;
  case izmir_statement_case_print:
    // this is where I am for print(1 + 2);
    izmir_compile_expression(st->print_expression);
    printf("print\n");
    break;
  case izmir_statement_case_sequence:
    izmir_compile_statement(st->sequence_statement_0);
    izmir_compile_statement(st->sequence_statement_1);
    break;
  default:
    // printf("nothing default");
    exit(EXIT_FAILURE);
  }
}

static void izmir_compile_program(struct izmir_program *p) {
  izmir_compile_statement(p->main_statement);
  // printf("not anything useful yet\n");
}

/* Execute what the command line says.
 * ************************************************************************** */

/* Do what the pointed command line data structure says. */
static void izmir_work(struct izmir_command_line *cl) {
  /* Parse a izmir-language program into an AST. */
  struct izmir_program *p;
  if (!strcmp(cl->program_path, "-"))
    p = izmir_parse_file_star(stdin);
  else
    p = izmir_parse_file(cl->program_path);

  izmir_compile_program(p);
}

/* Main function.
 * ************************************************************************** */

int main(int argc, char **argv) {
  /* Parse the command-line arguments, including options. */
  struct izmir_command_line cl;
  izmir_parse_command_line(&cl, argc, argv);

  /* Do what was requested on the command line. */
  izmir_work(&cl);

  /* Exit with success, if we're still alive. */
  return EXIT_SUCCESS;
}
