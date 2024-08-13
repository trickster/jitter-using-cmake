/* Izmir langauge Flex scanner.

   Copyright (C) 2016, 2017 Luca Saiu
   Copyright (C) 2022 Luca Saiu
   Updated in 2019 and 2020 and 2021 by Luca Saiu

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


/* Preliminary C code.  This is not included in the generated header.
 * ************************************************************************** */

%{
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "izmir-syntax.h"
#include "izmir-parser.h"


/* Re-declare two automatically-defined flex functions with
   __attribute__((unused)), to prevent an annoying GCC warning. */
static int input  (yyscan_t yyscanner)
  __attribute__ ((unused));
static void yyunput (int c, char * yy_bp , yyscan_t yyscanner)
  __attribute__ ((unused));

/* Provide aliases for a few identifiers not renamed by %option prefix. */
#define YYSTYPE IZMIR_STYPE
#define YYLTYPE IZMIR_LTYPE
%}


/* Flex options.
 * ************************************************************************** */

%option bison-bridge bison-locations nodefault noyywrap prefix="izmir_"
%option reentrant yylineno


/* Fundamental regular expression definitions.
 * ************************************************************************** */

DECIMAL_NATURAL      (0|([1-9][0-9]*))

DECIMAL_INTEGER      [-+]?{DECIMAL_NATURAL}

IDENTIFIER           [_a-zA-Z][-+._~@/\\a-zA-Z0-9]*

WHITESPACE           [\ \t\n\r\f]+
NEWLINE              [\n\r\f]
COMMENT              "//".*{NEWLINE}
/* STRING             \"([^"]|(\\\"))*\" */


/* End of the definition section.
 * ************************************************************************** */

%%

 /* Rules section (comments in this section cannot start at column 0).
 * ************************************************************************** */

({WHITESPACE}|{COMMENT})+ { /* Do nothing. */ }
"skip"                    { return SKIP; }
":="                      { return SET_TO; }
"print"                   { return PRINT; }
"input"                   { return INPUT; }
";"                       { return SEMICOLON; }
","                       { return COMMA; }
"begin"                   { return BEGIN_; }
"end"                     { return END; }
"if"                      { return IF; }
"then"                    { return THEN; }
"else"                    { return ELSE; }
"elif"                    { return ELIF; }
"while"                   { return WHILE; }
"do"                      { return DO; }
"repeat"                  { return REPEAT; }
"until"                   { return UNTIL; }
"return"                  { return RETURN; }
"("                       { return OPEN_PAREN; }
")"                       { return CLOSE_PAREN; }
"undefined"               { return UNDEFINED; }
{DECIMAL_INTEGER}         { return DECIMAL_LITERAL; }
"true"                    { return TRUE; }
"false"                   { return FALSE; }
"+"                       { return PLUS; }
"-"                       { return MINUS; }
"*"                       { return TIMES; }
"/"                       { return DIVIDED; }
"mod"                     { return REMAINDER; }
"="                       { return EQUAL; }
"<>"                      { return DIFFERENT; }
"<"                       { return LESS; }
"<="                      { return LESS_OR_EQUAL; }
">"                       { return GREATER; }
">="                      { return GREATER_OR_EQUAL; }
"and"                     { return LOGICAL_AND; }
"or"                      { return LOGICAL_OR; }
"not"                     { return LOGICAL_NOT; }
"var"                     { return VAR; }
"procedure"               { return PROCEDURE; }
{IDENTIFIER}              { return VARIABLE; }
.                         { izmir_scan_error (yyscanner); }

%%
