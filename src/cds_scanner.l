%option noyywrap

%{
/*
    This file is part of coindesigner.

    coindesigner is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    coindesigner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with coindesigner; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifdef __TURBOC__
#   pragma warn -pro
#   pragma warn -rch
#   pragma warn -ccc
#endif

#include <string.h>
#include <assert.h>
#include "cds_globals.h"
#include "cds_parser.h"

#ifndef yylval
extern YYSTYPE yylval;
#endif

//Esto es para evitar un error con visual C 
#define YY_NO_UNISTD_H 1
int isatty ( int ) {return 0;}

// Numero de linea actual
unsigned yylinenum = 1;

//Indicamos si debemos pasar los saltos de linea al parser 
int yy_ver_LF = 0;

void yyerror(const char *s);
%}

ENTERO    ([+-])?([0-9])+
REAL      ({ENTERO}("."([0-9])*)?([eE]{ENTERO})?)

%%

COFF            return _COFF;
OFF             return _OFF;
NOFF            return _NOFF;
CNOFF           return _NCOFF;
NCOFF           return _NCOFF;
STOFF           return _STOFF;
LIST            return _LIST;
APPEARANCE      return _APPEARANCE;
FILE      	return _FILE;
MTLLIB		return _MTLLIB;
USEMTL		return _USEMTL;
BEGIN		return _BEGIN;
END		return _END;

solid		return _SOLID;
ascii		return _ASCII;
facet		return _FACET;
normal		return _NORMAL;
outer		return _OUTER;
loop		return _LOOP;
vertex		return _VERTEX;
endloop		return _ENDLOOP;
endfacet	return _ENDFACET;
endsolid	return _ENDSOLID;

([+-])?([0-9])+  { 
                   yylval.entero = atol(yytext);
                   return _ENTERO;
                 }

([+-])?([0-9])*("."([0-9])*)?([eE]([+-])?([0-9])+)?           { 
                   yylval.real = (float)atof(yytext);
                   return _REAL;
                 }

\"[^"\n]*\" |
\'[^'\n]*\'      {yylval.pchar = strdup(yytext);
                  assert (yylval.pchar != NULL) ;
                  return _CADENA;}

\#[^\n]*         /* Comentario de oogl */
[ \t]+           /* Texto blanco */

[\r]*\n          {     yylinenum++;
                       /* Si estamos vigilando saltos de linea */
                       if (yy_ver_LF)
                          return _LF;
                 }

<<EOF>>          return (0);

\' |
\"             {yyerror( "Cadena inacabada.");
                return (-101);}

.              return (*yytext);
%%

/* Esto solo sirve para eliminar un warning del compilador */
void funcionQueNoSeEjecutaNunca(void) { yyunput(0,"");}

