/* Izmir langauge Bison parser.

   Copyright (C) 2016, 2017, 2019, 2020, 2021 Luca Saiu
   Copyright (C) 2022 Luca Saiu
   Written by Luca Saiu

   This file is part of the İzmir example, based on the Jitter
   structured-language example which is distributed along with
   GNU Jitter under its same license.

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


/* This code does not go to the generated header. */
%{
#include <stdio.h>
#include <ctype.h>

#include <jitter/jitter-malloc.h>
#include <jitter/jitter-fatal.h>
#include <jitter/jitter-parse-int.h>
#include <jitter/jitter-string.h>

#include "izmir-syntax.h"
#include "izmir-parser.h"
#include "izmir-scanner.h"

/* This is currently a fatal error.  I could longjmp away instead. */
static void
izmir_error (YYLTYPE *locp, struct izmir_program *p,
                  yyscan_t scanner, char *message)
  __attribute__ ((noreturn));

#define IZMIR_PARSE_ERROR(message)                            \
  do                                                               \
    {                                                              \
      izmir_error (izmir_get_lloc (izmir_scanner),  \
                        p, izmir_scanner, message);           \
    }                                                              \
  while (false)

/* What would be yytext in a non-reentrant scanner. */
#define IZMIR_TEXT \
  (izmir_get_text (izmir_scanner))

 /* What would be yylineno in a non-reentrant scanner. */
#define IZMIR_LINENO \
  (izmir_get_lineno (izmir_scanner))

/* A copy of what would be yytext in a non-reentrant scanner. */
#define IZMIR_TEXT_COPY \
  (jitter_clone_string (IZMIR_TEXT))

/* Initialise the fields of the pointed program, except for the main statement.
   The name will be copied.  */
static void izmir_initialize_program (struct izmir_program *p,
                               const char *file_name)
{
  p->source_file_name = jitter_clone_string (file_name);
  p->procedures = NULL;
  p->procedure_no = 0;
  /* Do not initialise p->main_statement . */
}

/* Return a pointer to a fresh malloc-allocated expression of the given case.
   No field is initialized but case_. */
static struct izmir_expression*
izmir_make_expression (enum izmir_expression_case case_)
{
  struct izmir_expression* res = jitter_xmalloc (sizeof (struct izmir_expression));
  res->case_ = case_;

  return res;
}

/* Return a pointer to a fresh malloc-allocated expression of the primitive
   case, with the given binary primitive and operands.  Every field is
   initalized. */
static struct izmir_expression* izmir_make_binary (enum izmir_primitive primitive, struct izmir_expression *operand_0, struct izmir_expression *operand_1)
{
  struct izmir_expression* res = izmir_make_expression (izmir_expression_case_primitive);
  res->primitive = primitive;
  res->primitive_operand_0 = operand_0;
  res->primitive_operand_1 = operand_1;
  return res;
}

/* Return a pointer to a fresh malloc-allocated expression of the primitive
   case, with the given nullary primitive.  Every field is initalized. */
static struct izmir_expression* izmir_make_nullary (enum izmir_primitive primitive)
{
  return izmir_make_binary (primitive, NULL, NULL);
}

static struct izmir_expression* izmir_make_unary (enum izmir_primitive primitive, struct izmir_expression *operand_0)
{
  return izmir_make_binary (primitive, operand_0, NULL);
}

static struct izmir_statement* izmir_make_statement (enum izmir_statement_case case_)
{
  struct izmir_statement* res = jitter_xmalloc (sizeof (struct izmir_statement));
  res->case_ = case_;

  return res;
}

/* Return a pointer to a fresh malloc-allocated statement containing a sequence
   setting the given variable to the pointed expression, and then the pointed
   statement. */
static struct izmir_statement* izmir_make_block (izmir_variable v, struct izmir_expression *e, struct izmir_statement *body)
{
  struct izmir_statement *sequence
    = izmir_make_statement (izmir_statement_case_sequence);
  struct izmir_statement *assignment
    = izmir_make_statement (izmir_statement_case_assignment);
  assignment->assignment_variable = v;
  assignment->assignment_expression = e;
  sequence->sequence_statement_0 = assignment;
  sequence->sequence_statement_1 = body;
  return sequence;
}

/* Add an element at the end of the pointed array of pointers, which is
   currently allocated with malloc and of size *element_no (in elements), by
   using realloc.  Add new_pointer as the new value at the end.  Increment the
   pointed size. */
static void izmir_append_pointer (void ***pointers, size_t *element_no,
                           void *new_pointer)
{
  * pointers = jitter_xrealloc (* pointers,
                                sizeof (void *) * ((* element_no) + 1));
  (* pointers) [* element_no] = new_pointer;
  (* element_no) ++;
}

static struct izmir_procedure* izmir_make_procedure (const char *procedure_name)
{
  struct izmir_procedure *res = jitter_xmalloc (sizeof (struct izmir_procedure));
  res->procedure_name = jitter_clone_string (procedure_name);
  res->formals = NULL;
  res->formal_no = 0;
  /* Do not initialise res->body . */
  return res;
}

static void izmir_program_append_procedure (struct izmir_program *p,
                                     const char *procedure_name)
{
  izmir_append_pointer ((void ***) & p->procedures, & p->procedure_no,
                             izmir_make_procedure (procedure_name));
}

static void izmir_procedure_append_formal (struct izmir_procedure *p,
                                    const char *new_formal_name)
{
  int i;
  for (i = 0; i < p->formal_no; i ++)
    if (! strcmp (p->formals [i], new_formal_name))
      jitter_fatal ("duplicated formal name %s in %s",
                    p->procedure_name, new_formal_name);
  izmir_append_pointer ((void ***) & p->formals, & p->formal_no,
                             jitter_clone_string (new_formal_name));
}

static struct izmir_procedure* izmir_last_procedure (struct izmir_program *p)
{
  if (p->procedure_no == 0)
    jitter_fatal ("izmir_last_procedure: no procedure exists");
  return p->procedures [p->procedure_no - 1];
}

/* These are used internally, when parsing sequences. */
struct izmir_sequence
{
  void **pointers;
  size_t pointer_no;
};

static void izmir_initialize_sequence(struct izmir_sequence *s)
{
  s->pointers = NULL;
  s->pointer_no = 0;
}

static struct izmir_sequence* izmir_make_sequence (void)
{
  struct izmir_sequence *res = jitter_xmalloc (sizeof (struct izmir_sequence));
  izmir_initialize_sequence (res);
  return res;
}


%}

%require "3.0"
%define api.prefix {izmir_}
%defines
%define api.pure

%locations

/* The parser and scanner functions both have additional parameters. */
%lex-param { izmir_scan_t izmir_scanner }
%parse-param { struct izmir_program *p }
%parse-param { void* izmir_scanner }

%code requires {
void izmir_scan_error (void *izmir_scanner) __attribute__ ((noreturn));
struct izmir_program* izmir_parse_file_star (FILE *input_file);
struct izmir_program* izmir_parse_file (const char *input_file_name);
} /* end of %code requires */

%union
{
  long literal;
  char* variable; /* izmir_variable */
  struct izmir_expression *expression;
  struct izmir_statement *statement;
  struct izmir_sequence *pointers;
}

%token PROCEDURE
%token RETURN
%token BEGIN_ END
%token SKIP
%token VAR
%token PRINT
%token INPUT
%token SET_TO
%token SEMICOLON
%token COMMA
%token IF THEN ELSE ELIF
%token WHILE DO
%token REPEAT UNTIL
%token OPEN_PAREN CLOSE_PAREN
%token UNDEFINED
       VARIABLE
       /*BINARY_LITERAL OCTAL_LITERAL*/ DECIMAL_LITERAL /*HEXADECIMAL_LITERAL*/
       TRUE FALSE
%left PLUS MINUS
%left TIMES
%left DIVIDED REMAINDER
%left EQUAL DIFFERENT LESS LESS_OR_EQUAL GREATER GREATER_OR_EQUAL
%left LOGICAL_OR
%left LOGICAL_AND
%left LOGICAL_NOT
%precedence UNARY_MINUS

%type <literal> literal;
%type <variable> variable;
%type <expression> expression;
%type <statement> statement;
%type <statement> statements;
%type <statement> one_or_more_statements;
%type <statement> block;
%type <statement> block_rest;
%type <statement> if_statement;
%type <statement> if_statement_rest;
%type <expression> optional_initialization;
%type <expression> if_expression;
%type <expression> if_expression_rest;
%type <pointers> actuals;
%type <pointers> non_empty_actuals;

%%

program:
  statements
  { p->main_statement = $1; }
| procedure_definition program
;

/* FIXME: use the style of actuals for formals and procedures. */
formals:
  /* nothing */
| non_empty_formals
;

non_empty_formals:
  variable
    { izmir_procedure_append_formal (izmir_last_procedure (p), $1); }
| variable COMMA
    { izmir_procedure_append_formal (izmir_last_procedure (p), $1); }
  non_empty_formals
;

actuals:
  /* nothing */
  { $$ = izmir_make_sequence (); }
| non_empty_actuals
  { $$ = $1; }
  ;

non_empty_actuals:
  expression
  { $$ = izmir_make_sequence ();
    izmir_append_pointer ((void ***) & $$->pointers, & $$->pointer_no,
                               $1); }
| non_empty_actuals COMMA expression
  { izmir_append_pointer ((void ***) & $$->pointers, & $$->pointer_no,
                               $3); }
;

procedure_definition:
  PROCEDURE variable
    { izmir_program_append_procedure (p, $2); }
  OPEN_PAREN formals CLOSE_PAREN statements END SEMICOLON
    { izmir_last_procedure (p)->body = $7; }
;

statement:
  optional_skip SEMICOLON
  { $$ = izmir_make_statement (izmir_statement_case_skip); }
| variable SET_TO expression SEMICOLON
  { $$ = izmir_make_statement (izmir_statement_case_assignment);
    $$->assignment_variable = $1;
    $$->assignment_expression = $3; }
| RETURN expression SEMICOLON
  { $$ = izmir_make_statement (izmir_statement_case_return);
    $$->return_result = $2; }
| RETURN SEMICOLON
  { $$ = izmir_make_statement (izmir_statement_case_return);
    struct izmir_expression *e
      = izmir_make_expression (izmir_expression_case_undefined);
    $$->return_result = e; }
| PRINT expression SEMICOLON
  { $$ = izmir_make_statement (izmir_statement_case_print);
    $$->print_expression = $2; }
| begin statements end
  { $$ = $2; }
| IF if_statement
  { $$ = $2; }
| WHILE expression DO statements end
  { /* Parse "while A do B end" as "if A then repeat B until not A else
       skip". */
    struct izmir_statement *r
      = izmir_make_statement (izmir_statement_case_repeat_until);
    r->repeat_until_body = $4;
    /* FIXME: clone $2 into a separate heap object, if I want to be able to free
       ASTs. */
    r->repeat_until_guard
      = izmir_make_unary (izmir_primitive_logical_not, $2);
    $$ = izmir_make_statement (izmir_statement_case_if_then_else);
    $$->if_then_else_condition = $2;
    $$->if_then_else_then_branch = r;
    $$->if_then_else_else_branch
      = izmir_make_statement (izmir_statement_case_skip); }
| REPEAT statements UNTIL expression SEMICOLON
  { $$ = izmir_make_statement (izmir_statement_case_repeat_until);
    $$->repeat_until_body = $2;
    $$->repeat_until_guard = $4; }
| variable OPEN_PAREN actuals CLOSE_PAREN
    { $$ = izmir_make_statement (izmir_statement_case_call);
      $$->callee = $1;
      $$->actuals = (struct izmir_expression **) $3->pointers;
      $$->actual_no = $3->pointer_no;
      /* FIXME: I could free $3 if I cared about not leaking memory at
         parsing time. */}
  ;

if_statement:
  expression THEN statements if_statement_rest
  { $$ = izmir_make_statement (izmir_statement_case_if_then_else);
    $$->if_then_else_condition = $1;
    $$->if_then_else_then_branch = $3;
    $$->if_then_else_else_branch = $4; }
;

if_statement_rest:
  end
  { /* Parse "if A then B end" as "if A then B else skip end". */
    $$ = izmir_make_statement (izmir_statement_case_skip); }
| ELIF expression THEN statements if_statement_rest
  { $$ = izmir_make_statement (izmir_statement_case_if_then_else);
    $$->if_then_else_condition = $2;
    $$->if_then_else_then_branch = $4;
    $$->if_then_else_else_branch = $5; }
| ELSE statements end
  { $$ = $2; }
;

statements:
  /* nothing */
  { $$ = izmir_make_statement (izmir_statement_case_skip); }
| one_or_more_statements
  { $$ = $1; }
  ;

one_or_more_statements:
  statement
  { $$ = $1; }
| statement one_or_more_statements
  { $$ = izmir_make_statement (izmir_statement_case_sequence);
    $$->sequence_statement_0 = $1;
    $$->sequence_statement_1 = $2; }
| VAR block
  { $$ = $2; }
  ;

block:
  variable optional_initialization block_rest
  { $$ = izmir_make_statement (izmir_statement_case_block);
    $$->block_variable = $1;
    $$->block_body = izmir_make_block ($1, $2, $3); }
  ;

block_rest:
  SEMICOLON statements
  { $$ = $2; }
| COMMA block
  { $$ = $2; }
  ;

optional_initialization:
  /* nothing*/
  { $$ = izmir_make_expression (izmir_expression_case_undefined); }
| EQUAL expression
  { $$ = $2; }
  ;

expression:
  UNDEFINED
  { $$ = izmir_make_expression (izmir_expression_case_undefined); }
| literal
  { $$ = izmir_make_expression (izmir_expression_case_literal);
    $$->literal = $1; }
| variable
  { $$ = izmir_make_expression (izmir_expression_case_variable);
    $$->variable = $1; }
| OPEN_PAREN expression CLOSE_PAREN
  { $$ = $2; }
| IF if_expression
  { $$ = $2; }
| expression PLUS expression
  { $$ = izmir_make_binary (izmir_primitive_plus, $1, $3); }
| expression MINUS expression
  { $$ = izmir_make_binary (izmir_primitive_minus, $1, $3); }
| MINUS expression %prec UNARY_MINUS
  { $$ = izmir_make_unary (izmir_primitive_unary_minus, $2); }
| expression TIMES expression
  { $$ = izmir_make_binary (izmir_primitive_times, $1, $3); }
| expression DIVIDED expression
  { $$ = izmir_make_binary (izmir_primitive_divided, $1, $3); }
| expression REMAINDER expression
  { $$ = izmir_make_binary (izmir_primitive_remainder, $1, $3); }
| expression EQUAL expression
  { $$ = izmir_make_binary (izmir_primitive_equal, $1, $3); }
| expression DIFFERENT expression
  { $$ = izmir_make_binary (izmir_primitive_different, $1, $3); }
| expression LESS expression
  { $$ = izmir_make_binary (izmir_primitive_less, $1, $3); }
| expression LESS_OR_EQUAL expression
  { $$ = izmir_make_binary (izmir_primitive_less_or_equal, $1, $3); }
| expression GREATER expression
  { $$ = izmir_make_binary (izmir_primitive_greater, $1, $3); }
| expression GREATER_OR_EQUAL expression
  { $$ = izmir_make_binary (izmir_primitive_greater_or_equal, $1, $3); }
| expression LOGICAL_AND expression
  { /* Parse "A and B" as "if A then B else false end". */
    $$ = izmir_make_expression (izmir_expression_case_if_then_else);
    $$->if_then_else_condition = $1;
    $$->if_then_else_then_branch = $3;
    $$->if_then_else_else_branch
      = izmir_make_expression (izmir_expression_case_literal);
    $$->if_then_else_else_branch->literal = 0; }
| expression LOGICAL_OR expression
  { /* Parse "A or B" as "if A then true else B end". */
    $$ = izmir_make_expression (izmir_expression_case_if_then_else);
    $$->if_then_else_condition = $1;
    $$->if_then_else_then_branch
      = izmir_make_expression (izmir_expression_case_literal);
    $$->if_then_else_then_branch->literal = 1;
    $$->if_then_else_else_branch = $3; }
| LOGICAL_NOT expression
  { $$ = izmir_make_unary (izmir_primitive_logical_not, $2); }
| INPUT
  { $$ = izmir_make_nullary (izmir_primitive_input); }
| variable OPEN_PAREN actuals CLOSE_PAREN
    { $$ = izmir_make_expression (izmir_expression_case_call);
      $$->callee = $1;
      $$->actuals = (struct izmir_expression **) $3->pointers;
      $$->actual_no = $3->pointer_no;
      /* FIXME: I could free $3 if I cared about not leaking memory at
         parsing time. */}
  ;

if_expression:
  expression THEN expression if_expression_rest
  { $$ = izmir_make_expression (izmir_expression_case_if_then_else);
    $$->if_then_else_condition = $1;
    $$->if_then_else_then_branch = $3;
    $$->if_then_else_else_branch = $4; }
;

if_expression_rest:
  /* For expressions there is no if..then..end without else; however elif
     clauses are permitted. */
  ELIF expression THEN expression if_expression_rest
  { $$ = izmir_make_expression (izmir_expression_case_if_then_else);
    $$->if_then_else_condition = $2;
    $$->if_then_else_then_branch = $4;
    $$->if_then_else_else_branch = $5; }
| ELSE expression END
  { $$ = $2; }
;

literal:
  DECIMAL_LITERAL
  { $$ = jitter_string_to_long_long_unsafe (IZMIR_TEXT); }
| TRUE
  { $$ = 1; }
| FALSE
  { $$ = 0; }
  ;

variable:
  VARIABLE
  { $$ = IZMIR_TEXT_COPY; }
  ;

optional_skip:
  /* nothing */
| SKIP
  ;

/* No need for optional semicolons after BEGIN_: semicolons after it will be
   parsed as skip statements in any statement sequence opened by BEGIN_. */
begin:
  BEGIN_
  ;

/* No need for optional semicolons after END: any semicolons after it will be
   parsed as skip statements, since any context where END may occur accepts a
   statement sequence, and not just a statement. */
end:
  END
  ;


%%

void
izmir_error (YYLTYPE *locp, struct izmir_program *p, yyscan_t izmir_scanner,
                 char *message)
{
  printf ("%s:%i: %s near \"%s\".\n",
          (p != NULL) ? p->source_file_name : "<INPUT>",
          izmir_get_lineno (izmir_scanner), message, IZMIR_TEXT);
  exit (EXIT_FAILURE);
}

void
izmir_scan_error (void *izmir_scanner)
{
  struct izmir_program *p = NULL; /* A little hack to have p in scope. */
  IZMIR_PARSE_ERROR("scan error");
}

static struct izmir_program *
izmir_parse_file_star_with_name (FILE *input_file, const char *file_name)
{
  yyscan_t scanner;
  izmir_lex_init (&scanner);
  izmir_set_in (input_file, scanner);

  struct izmir_program *res
    = jitter_xmalloc (sizeof (struct izmir_program));
  izmir_initialize_program (res, file_name);
  /* FIXME: if I ever make parsing errors non-fatal, call izmir_lex_destroy before
     returning, and finalize the program -- which might be incomplete! */
  if (izmir_parse (res, scanner))
    izmir_error (izmir_get_lloc (scanner), res, scanner, "parse error");
  izmir_set_in (NULL, scanner);
  izmir_lex_destroy (scanner);

  return res;
}

struct izmir_program *
izmir_parse_file_star (FILE *input_file)
{
  return izmir_parse_file_star_with_name (input_file, "<stdin>");
}

struct izmir_program *
izmir_parse_file (const char *input_file_name)
{
  FILE *f;
  if ((f = fopen (input_file_name, "r")) == NULL)
    jitter_fatal ("failed opening file %s", input_file_name);

  /* FIXME: if I ever make parse errors non-fatal, I'll need to close the file
     before returning. */
  struct izmir_program *res
    = izmir_parse_file_star_with_name (f, input_file_name);
  fclose (f);
  return res;
}
