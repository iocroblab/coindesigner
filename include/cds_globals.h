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

#ifndef _CDS_GLOBALS_H_
#define _CDS_GLOBALS_H_

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

//Definimos el prefijo por defecto de la aplicacion
#ifndef PREFIX
//#error Debe definir un valor para PREFIX
#define PREFIX
#endif

//Definimos el directorio de la aplicacion
#ifdef _WIN32
#define CDS_DIR (".") 
#else
#define CDS_DIR PREFIX"/share/coindesigner-"VERSION
#endif

//Directorio de coindesigner
extern const char *cds_dir;

#include <stdio.h>
#include <Inventor/nodes/SoSeparator.h>

/******DEFINICION DEL TIPO ivPadre_t PARA LA LISTA DE COMPONENTES******/
typedef struct  {
    char *clase;
    char *padre;
} ivPadre_t;

/*************Distintas configuraciones de componentes *****************/
extern ivPadre_t comp_coin230 []; 
extern ivPadre_t comp_simple []; 
extern ivPadre_t comp_coin230_plano []; 
extern ivPadre_t comp_SGI [];
#ifdef USE_VOLEON
extern ivPadre_t comp_simVoleon [];
#endif

/*************DEFINICION DEL TIPO YYSTYPE PARA EL PARSER****************/
#define YYSTYPE_IS_DECLARED 1
typedef union { 
        int entero;
        float real;
        char * pchar;
      } YYSTYPE;

/******************DEFINICION DE LAS FUNCIONES YY DEL PARSER**************/
void yyerror(char *s);
int yylex (void);
int yyparse (void);
void yyrestart (FILE *input_file);

/************** Variables de comunicación con el parser *******************/

/* Fichero del que se leeran los datos a cargar */
extern FILE *yyin;

/* Variable que contiene el numero de linea actual. Mantenida en scanner.l */
extern unsigned yylinenum;

/* Separator de donde colgamos el resultado de la lectura del parser*/
extern SoSeparator * yyGeometry;

/**************DEFINICION DE LAS FUNCIONES AUXILIARES DE DEPURACION**********/
#define __chivato__ fprintf(stderr, "%s:%d\n",__FILE__,__LINE__);

#endif
