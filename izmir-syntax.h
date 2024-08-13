/* Izmir langauge: abstract syntax.

   Copyright (C) 2017, 2019 Luca Saiu
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


// #ifndef JITTER_IZMIR_SYNTAX_H_
// #define JITTER_IZMIR_SYNTAX_H_

#ifndef IZMIR_SYNTAX_H_
#define IZMIR_SYNTAX_H_

#include <stdlib.h>

#include <jitter/jitter.h>
#include <jitter/jitter-fatal.h>


/* About AST data structures and heap-allocation.
 * ************************************************************************** */

/* This headers defines C data types representing a high-level AST data
   structure for a izmir program.

   Unboxed AST data structures are all heap-allocated with malloc .  There is no
   sharing within an AST (no two parents ever point to the same children) and in
   particular every text string is allocated independently, even when the text
   it contains is identical to the text of another string occurring elsewhere in
   the same AST.

   All the allocation, right now, occurs within the parser rules.  There is no
   explicit facility to free ASTs, but that would be trivial to add if needed in
   the future. */




/* Izmir-language ASTs.
 * ************************************************************************** */

/* The case of an AST expression. */
enum izmir_expression_case
  {
    izmir_expression_case_undefined,
    izmir_expression_case_literal,
    izmir_expression_case_variable,
    izmir_expression_case_if_then_else,
    izmir_expression_case_primitive,
    izmir_expression_case_call
  };

/* An identifier for a izmir-language primitive.  Primitives always work on
   values (one or two), and always produce one result.  In other words a
   primitive call is always an expression taking other expressions as arguments.
   There are no statement-like primitives. */
enum izmir_primitive
  {
    izmir_primitive_plus,
    izmir_primitive_minus,
    izmir_primitive_times,
    izmir_primitive_divided,
    izmir_primitive_remainder,
    izmir_primitive_unary_minus,
    izmir_primitive_equal,
    izmir_primitive_different,
    izmir_primitive_less,
    izmir_primitive_less_or_equal,
    izmir_primitive_greater,
    izmir_primitive_greater_or_equal,
    izmir_primitive_logical_not,
    izmir_primitive_is_nonzero,
    izmir_primitive_input
  };

/* A variable is represented as a pointer to a malloc-allocated C string holding
   the variable name.  There is no sharing: each instance of the same variable
   is allocated separately. */
typedef char* izmir_variable;

/* A izmir-language expression AST.  Whenever an expression is contained
   within a statement or a larger super-expresison the parent points to a struct
   of this type. */
struct izmir_expression
{
  /* The expression case. */
  enum izmir_expression_case case_;

  /* Expression fields, as an anonymous union.  Some fields of the anonymous
     union are anonymous structs. */
  union
  {
    /* An integer. */
    jitter_int literal;

    /* A variable. */
    izmir_variable variable;

    /* If-then-else fields. */
    struct
    {
      /* A pointer to the condition expression, as a malloc-allocated struct. */
      struct izmir_expression *if_then_else_condition;

      /* A pointer to the then-branch expression, as a malloc-allocated
         struct. */
      struct izmir_expression *if_then_else_then_branch;

      /* A pointer to the else-branch expression, as a malloc-allocated
         struct. */
      struct izmir_expression *if_then_else_else_branch;
    };

    /* Primitive fields. */
    struct
    {
      /* Primitive identifier. */
      enum izmir_primitive primitive;

      /* Pointer to a malloc-allocated first operand structure as an expression;
         NULL if there is no first operand. */
      struct izmir_expression *primitive_operand_0;

      /* Pointer to a malloc-allocated second operand structure as an
         expression; NULL if there is no second operand. */
      struct izmir_expression *primitive_operand_1;
    };

    /* Call fields. */
    struct
    {
      izmir_variable callee;
      struct izmir_expression **actuals;
      size_t actual_no;
    };
  }; /* end of the anonymous union. */
};

/* The case of an AST statement. */
enum izmir_statement_case
  {
    izmir_statement_case_skip,
    izmir_statement_case_block,
    izmir_statement_case_assignment,
    izmir_statement_case_print,
    izmir_statement_case_sequence,
    izmir_statement_case_if_then_else,
    izmir_statement_case_repeat_until,
    izmir_statement_case_return,
    izmir_statement_case_call
  };




/* Izmir-language statements.
 * ************************************************************************** */

/* A izmir-language statement AST.  Whenever a statement is contained
   within a larger super-statement or directly within the programa AST, the
   parent points to a struct of this type. */
struct izmir_statement
{
  /* The statement case. */
  enum izmir_statement_case case_;

  /* Statement fields, as an anonymous union.  Some fields of the anonymous
     union are anonymous structs. */
  union
  {
    /* There are no fields for the skip case. */

    /* Block fields. */
    struct
    {
      /* The variable being declared. */
      izmir_variable block_variable;
      struct izmir_statement *block_body;
    };

    /* Assignmenet fields. */
    struct
    {
      /* The set variable. */
      izmir_variable assignment_variable;

      /* A pointer to the expression whose value will be set into the
         variable, as a malloc-allocated struct. */
      struct izmir_expression *assignment_expression;
    };

    /* A pointer to the expression to be printed, as a malloc-allocated
       struct. */
    struct izmir_expression *print_expression;

    /* Sequence fields. */
    struct
    {
      /* A pointer to the first statement in the sequence, as a malloc-allocated
         struct.  The parser will nest sequences on the right, but there is no
         deep reason why the pointed first statement could not be a sequence as
         well. */
      struct izmir_statement *sequence_statement_0;

      /* A pointer to the second statement in the sequence, as a malloc-allocated
         struct.  The second statement may be another sequence. */
      struct izmir_statement *sequence_statement_1;
    };

    /* If-then-else fields. */
    struct
    {
      /* A pointer to the condition expression, as a malloc-allocated struct. */
      struct izmir_expression *if_then_else_condition;

      /* A pointer to the then-branch statement, as a malloc-allocated struct. */
      struct izmir_statement *if_then_else_then_branch;

      /* A pointer to the else-branch statement, as a malloc-allocated struct. */
      struct izmir_statement *if_then_else_else_branch;
    };

    /* If-then fields. */
    struct
    {
      /* A pointer to the condition expression, as a malloc-allocated struct. */
      struct izmir_expression *if_then_condition;

      /* A pointer to the then-branch statement, as a malloc-allocated struct. */
      struct izmir_statement *if_then_then_branch;
    };

    /* While-do fields. */
    struct
    {
      /* A pointer to the guard expression, as a malloc-allocated struct. */
      struct izmir_expression *while_do_guard;

      /* A pointer to the body statement, as a malloc-allocated struct. */
      struct izmir_statement *while_do_body;
    };

    /* Repeat-until fields. */
    struct
    {
      /* A pointer to the body statement, as a malloc-allocated struct. */
      struct izmir_statement *repeat_until_body;

      /* A pointer to the guard expression, as a malloc-allocated struct. */
      struct izmir_expression *repeat_until_guard;
    };

    /* Return fields. */
    struct
    {
      /* A pointer to the return expression, as a malloc-allocated struct. */
      struct izmir_expression *return_result;
    };

    /* Call fields. */
    struct
    {
      izmir_variable callee;
      struct izmir_expression **actuals;
      size_t actual_no;
    };
  }; /* end of the anonymous union. */
};

struct izmir_procedure
{
  /* A pointer to the procedure name as a malloc-allocated C string. */
  char *procedure_name;

  /* A malloc-allocated array of malloc-allocated C strings containing formal
     parameter names. */
  char **formals;

  /* The number of formal parameters. */
  size_t formal_no;

  /* The procedure body. */
  struct izmir_statement *body;
};

/* A izmir program AST.  Right now a program consists of a single
   statement. */
struct izmir_program
{
  /* A pointer to the source file pathname as a malloc-allocated C string. */
  char *source_file_name;

  /* A malloc-allocated array of pointers to malloc-allocated procedures. */
  struct izmir_procedure **procedures;

  /* The number of procedures. */
  size_t procedure_no;

  /* A pointer to the main statement, as a malloc-allocated struct. */
  struct izmir_statement *main_statement;
};




/* Boolean primitives.
 * ************************************************************************** */

/* Given the case for a primitive, return non-false iff the primitive is a
   comparison primitive. */
bool
izmir_is_comparison_primitive (enum izmir_primitive p);

/* Given the case for a comparison primitive returning a boolean, return the
   case of the opposite primitive.  For example, the reverse of less is
   greater_or_equal.
   Fail if reversing is not defined on the given primitive. */
enum izmir_primitive
izmir_reverse_comparison_primitive (enum izmir_primitive p);


#endif // #ifndef JITTER_IZMIR_SYNTAX_H_
