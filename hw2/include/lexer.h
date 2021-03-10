/*
  Notation program
  @(#)lexer.h	3.9 (C) Henry Thomas   Release 3     Dated 12/10/91
 */
#ifndef _HEADER_LEXER
#define _HEADER_LEXER

extern int column;
extern int lineno;

extern FILE * yyin;
extern FILE * yyout;

#ifdef __STDC__
extern int yylex(/*void*/); //*void*/
#endif
extern int yylex_destroy(/*void*/);
#endif
