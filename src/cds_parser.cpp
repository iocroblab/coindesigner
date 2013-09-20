/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 1 "src/cds_parser.y"

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

/* Gramatica de Bison que genera escenas de openInventor a partir de
   ficheros en los siguientes formatos:
     -Geomview: soporta ficheros OFF, COFF, NOFF, NCOFF, STOFF
     -SMF, OBJ
     -XYZ : Nubes de puntos (3 coordenadas por línea de fichero)
     -SPH : Arboles de esferas 
*/

#define YYERROR_VERBOSE
#define YYSTYPE_IS_DECLARED 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/cds_globals.h"

/* Nodos de openInventor que necesitamos */
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoIndexedPointSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoTextureCoordinate2.h>
#include <Inventor/nodes/SoBlinker.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoTranslation.h>

/* Variables para almacenar la información */
SoSeparator *yyGeometry;
SoCoordinate3 *yyCoordinate3;
SoIndexedPointSet *yyIndexedPointSet;
SoIndexedLineSet *yyIndexedLineSet;
SoIndexedFaceSet *yyIndexedFaceSet;
SoFaceSet *yyFaceSet;
SoNormal *yyNormal;
SoMaterial *yyMaterial;
SoTextureCoordinate2 *yyTextureCoordinate2;
SoMaterialBinding *yyMaterialBinding;
SoBlinker *yyBlinker;

/* numero de puntos y de facetas */
unsigned int yyNumeroFacetas;
unsigned int yyNumeroPuntos;

/* variable que almacena el numero de puntos leidos */
int num_puntos=0;

/*variable que almacena el numero de facetas leidas*/
int num_facetas=0;

/* Depth y brach de los arboles de esferas .sph */
int yy_sph_depth;
int yy_sph_branch;
int yy_sph_num;
int yy_sph_l;
SoSeparator *yy_sph_l_sep;

bool yy_outerloop=false;
unsigned yy_loopsize=false;

/* Variable para almacenar coordenadas de textura de ficheros .obj */
SoMFVec2f *yy_texture_coord = NULL;

/* Las siguientes funciones se implementan al final de este fichero */

void yyerror(const char *s);

/* Funcion que lee del fichero hasta un salto de linea */
void ignora_resto_linea();

/* Funcion que lee un numero del fichero, sea entero o real */
float LeeReal ();

/* Funcion que lee un numero del fichero, obligatoriamente entero. */
int LeeEntero ();

/* Función que trata un bloque OFF/COFF/NOFF/NCOFF/STOFF */
void LeeBloqueOFF (int nPuntos, int nCaras, int tipoBloqueOFF);

/* Variable que indica al scanner si debe mostrar los saltos de linea */
extern int yy_ver_LF;


/* Line 268 of yacc.c  */
#line 182 "y.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     _OFF = 258,
     _COFF = 259,
     _NOFF = 260,
     _NCOFF = 261,
     _STOFF = 262,
     _LIST = 263,
     _APPEARANCE = 264,
     _FILE = 265,
     _LF = 266,
     _MTLLIB = 267,
     _USEMTL = 268,
     _BEGIN = 269,
     _END = 270,
     _SOLID = 271,
     _ASCII = 272,
     _FACET = 273,
     _NORMAL = 274,
     _OUTER = 275,
     _LOOP = 276,
     _VERTEX = 277,
     _ENDLOOP = 278,
     _ENDFACET = 279,
     _ENDSOLID = 280,
     _ENTERO = 281,
     _REAL = 282,
     _CADENA = 283
   };
#endif
/* Tokens.  */
#define _OFF 258
#define _COFF 259
#define _NOFF 260
#define _NCOFF 261
#define _STOFF 262
#define _LIST 263
#define _APPEARANCE 264
#define _FILE 265
#define _LF 266
#define _MTLLIB 267
#define _USEMTL 268
#define _BEGIN 269
#define _END 270
#define _SOLID 271
#define _ASCII 272
#define _FACET 273
#define _NORMAL 274
#define _OUTER 275
#define _LOOP 276
#define _VERTEX 277
#define _ENDLOOP 278
#define _ENDFACET 279
#define _ENDSOLID 280
#define _ENTERO 281
#define _REAL 282
#define _CADENA 283




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 279 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   157

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  42
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  68
/* YYNRULES -- Number of states.  */
#define YYNSTATES  148

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   283

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    41,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    35,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,    34,    38,     2,     2,     2,     2,    33,     2,
      37,    39,    32,     2,     2,    40,    36,     2,    31,     2,
       2,     2,     2,    29,     2,    30,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     7,     8,    11,    12,    15,    17,
      19,    21,    23,    25,    27,    29,    31,    33,    36,    39,
      43,    46,    48,    51,    56,    61,    66,    71,    76,    79,
      81,    85,    90,    96,    99,   103,   108,   118,   126,   138,
     143,   149,   155,   160,   162,   164,   166,   168,   170,   172,
     175,   177,   182,   183,   188,   191,   193,   199,   200,   206,
     209,   211,   215,   223,   224,   228,   232,   235,   237
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      43,     0,    -1,    -1,    44,    48,    -1,    -1,    45,    58,
      -1,    -1,    46,    56,    -1,    60,    -1,    64,    -1,    27,
      -1,    26,    -1,    51,    -1,    52,    -1,    53,    -1,    54,
      -1,    55,    -1,    50,    48,    -1,     8,    49,    -1,    29,
      48,    30,    -1,    48,    49,    -1,    48,    -1,     9,    29,
      -1,     3,    26,    26,    26,    -1,     4,    26,    26,    26,
      -1,     5,    26,    26,    26,    -1,     6,    26,    26,    26,
      -1,     7,    26,    26,    26,    -1,    56,    57,    -1,    57,
      -1,    14,    56,    15,    -1,    31,    47,    47,    47,    -1,
      31,    47,    47,    47,    47,    -1,    32,    26,    -1,    33,
      26,    26,    -1,    34,    26,    26,    26,    -1,    34,    26,
      35,    35,    26,    26,    35,    35,    26,    -1,    34,    26,
      35,    26,    26,    35,    26,    -1,    34,    26,    35,    26,
      35,    26,    26,    35,    26,    35,    26,    -1,    31,    36,
      47,    47,    -1,    31,    36,    47,    47,    47,    -1,    31,
      37,    47,    47,    47,    -1,    31,    32,    47,    47,    -1,
      38,    -1,    39,    -1,    40,    -1,    12,    -1,    13,    -1,
      41,    -1,    58,    59,    -1,    59,    -1,    47,    47,    47,
      11,    -1,    -1,    26,    26,    61,    62,    -1,    62,    63,
      -1,    63,    -1,    47,    47,    47,    47,    47,    -1,    -1,
      65,    16,    17,    66,    25,    -1,    66,    67,    -1,    67,
      -1,    18,    68,    24,    -1,    18,    19,    47,    47,    47,
      68,    24,    -1,    -1,    20,    69,    68,    -1,    21,    70,
      23,    -1,    70,    71,    -1,    71,    -1,    22,    47,    47,
      47,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   127,   127,   127,   139,   139,   162,   162,   218,   221,
     225,   226,   229,   230,   231,   232,   233,   234,   235,   236,
     239,   240,   243,   275,   282,   289,   296,   303,   310,   311,
     312,   315,   321,   327,   356,   390,   428,   469,   511,   565,
     572,   584,   591,   596,   601,   605,   609,   613,   617,   624,
     625,   628,   637,   636,   674,   675,   678,   726,   726,   743,
     744,   747,   748,   751,   751,   752,   760,   761,   764
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "_OFF", "_COFF", "_NOFF", "_NCOFF",
  "_STOFF", "_LIST", "_APPEARANCE", "_FILE", "_LF", "_MTLLIB", "_USEMTL",
  "_BEGIN", "_END", "_SOLID", "_ASCII", "_FACET", "_NORMAL", "_OUTER",
  "_LOOP", "_VERTEX", "_ENDLOOP", "_ENDFACET", "_ENDSOLID", "_ENTERO",
  "_REAL", "_CADENA", "'{'", "'}'", "'v'", "'p'", "'l'", "'f'", "'/'",
  "'t'", "'n'", "'g'", "'o'", "'s'", "'$'", "$accept", "fich_geom", "$@1",
  "$@2", "$@3", "REAL", "bloque_oogl", "_bloques_oogl",
  "bloque_appearance", "bloque_off", "bloque_coff", "bloque_noff",
  "bloque_ncoff", "bloque_stoff", "bloque_SMF", "linea_SMF", "bloque_XYZ",
  "linea_XYZ", "fichero_SPH", "$@4", "bloque_SPH", "linea_SPH",
  "fichero_STL", "$@5", "bloque_STL", "facet_STL", "loop_STL", "$@6",
  "bloque_loop_STL", "vertex_STL", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   123,
     125,   118,   112,   108,   102,    47,   116,   110,   103,   111,
     115,    36
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    42,    44,    43,    45,    43,    46,    43,    43,    43,
      47,    47,    48,    48,    48,    48,    48,    48,    48,    48,
      49,    49,    50,    51,    52,    53,    54,    55,    56,    56,
      56,    57,    57,    57,    57,    57,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    57,    58,
      58,    59,    61,    60,    62,    62,    63,    65,    64,    66,
      66,    67,    67,    69,    68,    68,    70,    70,    71
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     0,     2,     0,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     3,
       2,     1,     2,     4,     4,     4,     4,     4,     2,     1,
       3,     4,     5,     2,     3,     4,     9,     7,    11,     4,
       5,     5,     4,     1,     1,     1,     1,     1,     1,     2,
       1,     4,     0,     4,     2,     1,     5,     0,     5,     2,
       1,     3,     7,     0,     3,     3,     2,     1,     4
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       6,     0,     0,     0,     0,     0,     8,     9,     0,    52,
       1,     0,     0,     0,     0,     0,     0,     0,     0,     3,
       0,    12,    13,    14,    15,    16,    11,    10,     0,     5,
      50,    46,    47,     0,     0,     0,     0,     0,    43,    44,
      45,    48,     7,    29,     0,     0,     0,     0,     0,     0,
       0,    21,    18,    22,     0,    17,     0,    49,     0,     0,
       0,     0,     0,    33,     0,     0,    28,     0,     0,    53,
      55,     0,     0,     0,     0,     0,    20,    19,     0,    30,
       0,     0,     0,     0,    34,     0,     0,     0,     0,    60,
       0,    54,    23,    24,    25,    26,    27,    51,    42,    39,
       0,    31,    35,     0,     0,     0,    63,     0,     0,    58,
      59,     0,    40,    41,    32,     0,     0,     0,     0,     0,
       0,     0,    67,    61,     0,     0,     0,     0,     0,    64,
       0,    65,    66,    56,    37,     0,     0,     0,     0,     0,
       0,     0,    68,     0,    36,    62,     0,    38
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     2,     3,     4,     5,    28,    51,    52,    20,    21,
      22,    23,    24,    25,    42,    43,    29,    30,     6,    45,
      69,    70,     7,     8,    88,    89,   108,   119,   121,   122
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -119
static const yytype_int8 yypact[] =
{
     124,   -24,     8,     6,    17,    25,  -119,  -119,   -12,  -119,
    -119,    -3,    19,    22,    34,    35,     6,    38,     6,  -119,
       6,  -119,  -119,  -119,  -119,  -119,  -119,  -119,    17,    17,
    -119,  -119,  -119,    25,    42,    44,    49,    50,  -119,  -119,
    -119,  -119,    85,  -119,    13,    17,    54,    55,    56,    58,
      61,     6,  -119,  -119,    59,  -119,    17,  -119,    73,    17,
      17,    17,    17,  -119,    65,   -19,  -119,    75,    17,    17,
    -119,    68,    69,    77,    82,    83,  -119,  -119,    88,  -119,
      17,    17,    17,    17,  -119,    89,     1,     5,     4,  -119,
      17,  -119,  -119,  -119,  -119,  -119,  -119,  -119,  -119,    17,
      17,    17,  -119,    16,    94,    17,  -119,    79,    97,  -119,
    -119,    17,  -119,  -119,  -119,    87,   108,   109,    17,    26,
      17,    27,  -119,  -119,    17,   110,   111,   103,    17,  -119,
      17,  -119,  -119,  -119,  -119,   104,   106,    26,    17,   116,
     117,   120,  -119,   112,  -119,  -119,   119,  -119
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -119,  -119,  -119,  -119,  -119,   -28,     0,    95,  -119,  -119,
    -119,  -119,  -119,  -119,   115,   -37,  -119,   123,  -119,  -119,
    -119,    80,  -119,  -119,  -119,    66,  -118,  -119,  -119,    36
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -58
static const yytype_int16 yytable[] =
{
      56,   129,     9,    19,    44,    66,    62,    85,    10,    11,
      12,    13,    14,    15,    16,    17,    86,    68,    54,   141,
      55,    66,    87,    46,   105,   106,   107,   103,    78,   109,
      67,    80,    81,    82,    83,    18,   104,    31,    32,    33,
      90,    68,   115,    26,    27,    47,   106,   107,    48,   120,
     131,   116,    98,    99,   100,   101,    34,    35,    36,    37,
      49,    50,   111,    38,    39,    40,    41,    53,    26,    27,
      63,   112,   113,   114,    59,    64,    65,   118,    60,    61,
      71,    72,    73,   124,    74,    31,    32,    75,    79,    77,
     128,    84,   130,    87,    92,    93,   133,    31,    32,    97,
     137,   120,   138,    94,    34,    35,    36,    37,    95,    96,
     142,    38,    39,    40,    41,   102,    34,    35,    36,    37,
     117,   123,   125,    38,    39,    40,    41,    -2,    -2,    -2,
      -2,    -2,    -2,    -2,   126,   127,   134,   135,   136,   139,
     -57,   140,   143,   144,   145,   147,    76,   146,    58,    91,
       1,    -4,    57,    -2,   110,     0,     0,   132
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-119))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      28,   119,    26,     3,    16,    42,    34,    26,     0,     3,
       4,     5,     6,     7,     8,     9,    35,    45,    18,   137,
      20,    58,    18,    26,    19,    20,    21,    26,    56,    25,
      17,    59,    60,    61,    62,    29,    35,    12,    13,    14,
      68,    69,    26,    26,    27,    26,    20,    21,    26,    22,
      23,    35,    80,    81,    82,    83,    31,    32,    33,    34,
      26,    26,    90,    38,    39,    40,    41,    29,    26,    27,
      26,    99,   100,   101,    32,    26,    26,   105,    36,    37,
      26,    26,    26,   111,    26,    12,    13,    26,    15,    30,
     118,    26,   120,    18,    26,    26,   124,    12,    13,    11,
     128,    22,   130,    26,    31,    32,    33,    34,    26,    26,
     138,    38,    39,    40,    41,    26,    31,    32,    33,    34,
      26,    24,    35,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    26,    26,    26,    26,    35,    35,
      16,    35,    26,    26,    24,    26,    51,    35,    33,    69,
      26,    27,    29,    29,    88,    -1,    -1,   121
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    26,    43,    44,    45,    46,    60,    64,    65,    26,
       0,     3,     4,     5,     6,     7,     8,     9,    29,    48,
      50,    51,    52,    53,    54,    55,    26,    27,    47,    58,
      59,    12,    13,    14,    31,    32,    33,    34,    38,    39,
      40,    41,    56,    57,    16,    61,    26,    26,    26,    26,
      26,    48,    49,    29,    48,    48,    47,    59,    56,    32,
      36,    37,    47,    26,    26,    26,    57,    17,    47,    62,
      63,    26,    26,    26,    26,    26,    49,    30,    47,    15,
      47,    47,    47,    47,    26,    26,    35,    18,    66,    67,
      47,    63,    26,    26,    26,    26,    26,    11,    47,    47,
      47,    47,    26,    26,    35,    19,    20,    21,    68,    25,
      67,    47,    47,    47,    47,    26,    35,    26,    47,    69,
      22,    70,    71,    24,    47,    35,    26,    26,    47,    68,
      47,    23,    71,    47,    26,    26,    35,    47,    47,    35,
      35,    68,    47,    26,    26,    24,    35,    26
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1806 of yacc.c  */
#line 127 "src/cds_parser.y"
    {
    /* Preparamos para leer OOGL */
    yyGeometry = new SoSeparator();
    yyNumeroFacetas = 0;
    yyNumeroPuntos = 0;
    yylinenum = 1;
  }
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 135 "src/cds_parser.y"
    {
  }
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 139 "src/cds_parser.y"
    {
     /* Preparamos para leer XYZ */
     yyGeometry = new SoSeparator();
     yyCoordinate3 = new SoCoordinate3();

     /* Añadimos los puntos al resultado */
     yyGeometry->addChild(yyCoordinate3);
     yyGeometry->addChild(new SoPointSet() );

     yyNumeroFacetas = 0;
     yyNumeroPuntos = 0;
     yylinenum = 1;

     //Indicamos al scanner que queremos ver los saltos de linea
     yy_ver_LF = 1;
  }
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 156 "src/cds_parser.y"
    {
     //Indicamos al scanner que no queremos ver los saltos de linea
     yy_ver_LF = 0;
  }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 162 "src/cds_parser.y"
    {
     /* Preparamos para leer SMF */
     yyGeometry = new SoSeparator();
     yyCoordinate3 = new SoCoordinate3();
     yyIndexedPointSet = new SoIndexedPointSet();
     yyIndexedPointSet->coordIndex.setNum(0);
     yyIndexedLineSet = new SoIndexedLineSet();
     yyIndexedLineSet->coordIndex.setNum(0);
     yyIndexedFaceSet = new SoIndexedFaceSet();
     yyIndexedFaceSet->coordIndex.setNum(0);
     yyTextureCoordinate2 = new SoTextureCoordinate2;
     yyNormal = new SoNormal();

     yyGeometry->addChild(yyCoordinate3);
     yyGeometry->addChild(yyNormal);
     yyGeometry->addChild(yyTextureCoordinate2);
     yyGeometry->addChild(yyIndexedPointSet);
     yyGeometry->addChild(yyIndexedLineSet);
     yyGeometry->addChild(yyIndexedFaceSet);

     //En ocasiones los fichero contienen información de textura
     //Pero los indices son diferentes a openInventor, por lo que
     //necesitamos una tabla auxiliar
     yy_texture_coord = new SoMFVec2f() ;

     yyNumeroFacetas = 0;
     yyNumeroPuntos = 0;
     yylinenum = 1;
  }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 192 "src/cds_parser.y"
    {
     //Si el fichero no contenia coordenadas de textura, lo eliminamos
     if (yy_texture_coord->getNum() < 1)
       yyGeometry->removeChild(yyTextureCoordinate2);

     //Si el fichero no contenia coordenadas de normal, lo eliminamos
     if (yyNormal->vector.getNum() < 1)
       yyGeometry->removeChild(yyNormal);

     //Si el fichero no contenia puntos, lo eliminamos
     if (yyIndexedPointSet->coordIndex.getNum() < 1)
       yyGeometry->removeChild(yyIndexedPointSet);

     //Si el fichero no contenia lineas, lo eliminamos
     if (yyIndexedLineSet->coordIndex.getNum() < 1)
       yyGeometry->removeChild(yyIndexedLineSet);

     //Si el fichero no contenia facetas, lo eliminamos
     if (yyIndexedFaceSet->coordIndex.getNum() < 1)
       yyGeometry->removeChild(yyIndexedFaceSet);

     //Liberamos espacio de información de textura
     delete yy_texture_coord;
  }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 225 "src/cds_parser.y"
    {(yyval.real) = (yyvsp[(1) - (1)].real); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 226 "src/cds_parser.y"
    {(yyval.real) = (yyvsp[(1) - (1)].entero);}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 244 "src/cds_parser.y"
    {
     /* Ignoramos el contenido por ahora */
     int tipo, num_llaves;
     num_llaves = 1;
     while (num_llaves > 0)
     {
       tipo = yylex();
       if (tipo == '{') num_llaves++;
       else if (tipo == '}') num_llaves--;
       else if (tipo == _FILE)
       {
          tipo = yylex();
          if (tipo == _CADENA)
          {
             //Eliminamos las comillas del final
             yylval.pchar[strlen(yylval.pchar)-1] = 0;

             //Creamos un nodo Texture2
             SoTexture2 *yyTexture2 = new SoTexture2;
             yyGeometry->addChild(yyTexture2);

             //Metemos el nombre del fichero, sin las comillas iniciales
             yyTexture2->filename.setValue(yylval.pchar+1);

          }
       }
     } // while
  }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 276 "src/cds_parser.y"
    {
     /* leemos este bloque, indicando el tipo adecuado */
     LeeBloqueOFF ((yyvsp[(2) - (4)].entero), (yyvsp[(3) - (4)].entero), _OFF);
  }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 283 "src/cds_parser.y"
    {
     /* leemos este bloque, indicando el tipo adecuado */
     LeeBloqueOFF ((yyvsp[(2) - (4)].entero), (yyvsp[(3) - (4)].entero), _COFF);
  }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 290 "src/cds_parser.y"
    {
     /* leemos este bloque, indicando el tipo adecuado */
     LeeBloqueOFF ((yyvsp[(2) - (4)].entero), (yyvsp[(3) - (4)].entero), _NOFF);
  }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 297 "src/cds_parser.y"
    {
     /* leemos este bloque, indicando el tipo adecuado */
     LeeBloqueOFF ((yyvsp[(2) - (4)].entero), (yyvsp[(3) - (4)].entero), _NCOFF);
  }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 304 "src/cds_parser.y"
    {
     /* leemos este bloque, indicando el tipo adecuado */
     LeeBloqueOFF ((yyvsp[(2) - (4)].entero), (yyvsp[(3) - (4)].entero), _STOFF);
  }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 316 "src/cds_parser.y"
    {
     // Almacenamos las coordenadas cartesianas.
     yyCoordinate3->point.set1Value(yyNumeroPuntos++, (yyvsp[(2) - (4)].real), (yyvsp[(3) - (4)].real), (yyvsp[(4) - (4)].real));
  }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 322 "src/cds_parser.y"
    {
     // Almacenamos las coordenadas racionales.
     yyCoordinate3->point.set1Value(yyNumeroPuntos++, (yyvsp[(2) - (5)].real)/(yyvsp[(5) - (5)].real), (yyvsp[(3) - (5)].real)/(yyvsp[(5) - (5)].real), (yyvsp[(4) - (5)].real)/(yyvsp[(5) - (5)].real));
  }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 328 "src/cds_parser.y"
    {
     // generamos un nuevo punto
     int k = yyIndexedPointSet->coordIndex.getNum();
     yyIndexedPointSet->coordIndex.set1Value(k++, (yyvsp[(2) - (2)].entero) -1);

     //Miramos si hay más vertices en esta nube de puntos

     //Indicamos al scanner que queremos ver los saltos de linea
     yy_ver_LF = 1;

     //Leemos hasta el fin de linea, insertando los vertices
     int tipo = yylex();
     while (tipo == _ENTERO)
     {
        yyIndexedPointSet->coordIndex.set1Value(k++,yylval.entero-1);
        tipo = yylex();
     }

     if (tipo != _LF)
     {
        yyerror("Format error in integer number.\n");
        return -1;
     }

     //Indicamos al scanner que no queremos ver los saltos de linea
     yy_ver_LF = 0;
  }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 357 "src/cds_parser.y"
    {
     // generamos una nueva polilinea de 2 o mas vertices

     int k = yyIndexedLineSet->coordIndex.getNum();
     yyIndexedLineSet->coordIndex.set1Value(k++, (yyvsp[(2) - (3)].entero) -1);
     yyIndexedLineSet->coordIndex.set1Value(k++, (yyvsp[(3) - (3)].entero) -1);

     //Miramos si hay más vertices en esta polilinea

     //Indicamos al scanner que queremos ver los saltos de linea
     yy_ver_LF = 1;

     //Leemos hasta el fin de linea, insertando los vertices
     int tipo = yylex();
     while (tipo == _ENTERO)
     {
        yyIndexedLineSet->coordIndex.set1Value(k++,yylval.entero-1);
        tipo = yylex();
     }

     if (tipo != _LF)
     {
        yyerror("Format error in integer number.\n");
        return -1;
     }

     //Insertamos la marca de fin de polilinea
     yyIndexedLineSet->coordIndex.set1Value(k++, -1);

     //Indicamos al scanner que no queremos ver los saltos de linea
     yy_ver_LF = 0;
  }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 391 "src/cds_parser.y"
    {
     // generamos una nueva faceta de 3 o mas vertices

     int k = yyIndexedFaceSet->coordIndex.getNum();
     yyIndexedFaceSet->coordIndex.set1Value(k++, (yyvsp[(2) - (4)].entero) -1);
     yyIndexedFaceSet->coordIndex.set1Value(k++, (yyvsp[(3) - (4)].entero) -1);
     yyIndexedFaceSet->coordIndex.set1Value(k++, (yyvsp[(4) - (4)].entero) -1);

     //Miramos si hay más vertices en esta faceta

     //Indicamos al scanner que queremos ver los saltos de linea
     yy_ver_LF = 1;

     //Leemos hasta el fin de linea, insertando los vertices
     int tipo = yylex();
     while (tipo == _ENTERO)
     {
        yyIndexedFaceSet->coordIndex.set1Value(k++,yylval.entero-1);
        tipo = yylex();
     }

     if (tipo != _LF)
     {
        yyerror("Format error in integer number.\n");
        return -1;
     }

     //Insertamos la marca de fin de faceta
     yyIndexedFaceSet->coordIndex.set1Value(k++, -1);

     //Incrementamos la cuenta de facetas
     yyNumeroFacetas++;

     //Indicamos al scanner que no queremos ver los saltos de linea
     yy_ver_LF = 0;
  }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 429 "src/cds_parser.y"
    {
     /* generamos una nueva faceta, ignorando resto de informacion */
     int k = yyIndexedFaceSet->coordIndex.getNum();

     //Por ahora hemos leido 2 valores
     yyIndexedFaceSet->coordIndex.set1Value(k++, (yyvsp[(2) - (9)].entero) -1);
     yyIndexedFaceSet->coordIndex.set1Value(k++, (yyvsp[(6) - (9)].entero) -1);

     //Salvamos las coordenadas de normal
     yyIndexedFaceSet->normalIndex.set1Value(k-2, (yyvsp[(5) - (9)].entero) -1);
     yyIndexedFaceSet->normalIndex.set1Value(k-1, (yyvsp[(9) - (9)].entero) -1);

     //Miramos si hay más vertices en esta faceta

     //Indicamos al scanner que queremos ver los saltos de linea
     yy_ver_LF = 1;

     //Leemos hasta el fin de linea
     while (yylex() == _ENTERO)
     {
        yyIndexedFaceSet->coordIndex.set1Value(k++, yylval.entero -1);

        //Ignoramos los 2 campos siguientes ( / / )
        yylex();
        yylex();

        //Leemos la coordenada de normal (vn)
        int idn = LeeEntero() - 1;
        yyIndexedFaceSet->normalIndex.set1Value(k-1, idn);

     }

     //Indicamos al scanner que no queremos ver los saltos de linea
     yy_ver_LF = 0;

     //Cerramos la faceta actual
     yyIndexedFaceSet->coordIndex.set1Value(k++, -1);
     yyIndexedFaceSet->normalIndex.set1Value(k-1, -1);
     yyNumeroFacetas++;
  }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 470 "src/cds_parser.y"
    {
     /* generamos una nueva faceta, ignorando resto de informacion */
     int k = yyIndexedFaceSet->coordIndex.getNum();

     //Por ahora hemos leido 2 valores
     yyIndexedFaceSet->coordIndex.set1Value(k++, (yyvsp[(2) - (7)].entero) -1);
     yyIndexedFaceSet->coordIndex.set1Value(k++, (yyvsp[(5) - (7)].entero) -1);

     //Salvamos las coordenadas de textura
     yyTextureCoordinate2->point.set1Value((yyvsp[(2) - (7)].entero)-1, (*yy_texture_coord)[(yyvsp[(4) - (7)].entero)-1] );
     yyTextureCoordinate2->point.set1Value((yyvsp[(5) - (7)].entero)-1, (*yy_texture_coord)[(yyvsp[(7) - (7)].entero)-1] );

     //Miramos si hay más vertices en esta faceta

     //Indicamos al scanner que queremos ver los saltos de linea
     yy_ver_LF = 1;

     //Leemos hasta el fin de linea
     while (yylex() == _ENTERO)
     {
        //Salvamos el indice del vértice
        int idx = yylval.entero-1;
        yyIndexedFaceSet->coordIndex.set1Value(k++, idx);

        //Ignoramos la barra
        yylex();

        //Leemos la coordenada de textura
        int idt = LeeEntero() - 1;
        yyTextureCoordinate2->point.set1Value(idx, (*yy_texture_coord)[idt]);

     }// while (yylex() == _ENTERO)

     //Indicamos al scanner que no queremos ver los saltos de linea
     yy_ver_LF = 0;

     //Cerramos la faceta actual
     yyIndexedFaceSet->coordIndex.set1Value(k++, -1);
     yyNumeroFacetas++;
  }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 512 "src/cds_parser.y"
    {
     /* generamos una nueva faceta, ignorando resto de informacion */
     int k = yyIndexedFaceSet->coordIndex.getNum();

     //Por ahora hemos leido 2 valores
     yyIndexedFaceSet->coordIndex.set1Value(k++, (yyvsp[(2) - (11)].entero) -1);
     yyIndexedFaceSet->coordIndex.set1Value(k++, (yyvsp[(7) - (11)].entero) -1);

     //Salvamos las coordenadas de normal
     yyIndexedFaceSet->normalIndex.set1Value(k-2, (yyvsp[(6) - (11)].entero) -1);
     yyIndexedFaceSet->normalIndex.set1Value(k-1, (yyvsp[(11) - (11)].entero)-1);

     //Salvamos las coordenadas de textura
     yyTextureCoordinate2->point.set1Value((yyvsp[(2) - (11)].entero)-1, (*yy_texture_coord)[(yyvsp[(4) - (11)].entero)-1] );
     yyTextureCoordinate2->point.set1Value((yyvsp[(7) - (11)].entero)-1, (*yy_texture_coord)[(yyvsp[(9) - (11)].entero)-1] );

     //Miramos si hay más vertices en esta faceta

     //Indicamos al scanner que queremos ver los saltos de linea
     yy_ver_LF = 1;

     //Leemos hasta el fin de linea
     while (yylex() == _ENTERO)
     {
        //Salvamos el indice del vértice
        int idx = yylval.entero-1;
        yyIndexedFaceSet->coordIndex.set1Value(k++, idx);

        //Ignoramos el campo siguiente (/)
        yylex();

        //Leemos la coordenada de textura
        int idt = LeeEntero() - 1;
        yyTextureCoordinate2->point.set1Value(idx, (*yy_texture_coord)[idt]);

        //Ignoramos el campo siguiente (/)
        yylex();

        //Leemos la coordenada de normal (vn)
        int idn = LeeEntero() - 1;
        yyIndexedFaceSet->normalIndex.set1Value(k-1, idn);

     }//while (yylex() == _ENTERO)

     //Indicamos al scanner que no queremos ver los saltos de linea
     yy_ver_LF = 0;

     //Cerramos la faceta actual
     yyIndexedFaceSet->coordIndex.set1Value(k++, -1);
     yyIndexedFaceSet->normalIndex.set1Value(k-1, -1);
     yyNumeroFacetas++;
  }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 566 "src/cds_parser.y"
    {
     /* Almacenamos la informacion de textura */
      int idx = yy_texture_coord->getNum();
      yy_texture_coord->set1Value(idx,(yyvsp[(3) - (4)].real), (yyvsp[(4) - (4)].real));
  }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 573 "src/cds_parser.y"
    {
     /* Almacenamos la informacion de textura */
      int idx = yy_texture_coord->getNum();

      //Impedimos una hipotetica division por cero
      if ((yyvsp[(5) - (5)].real) != 0.0f)
         yy_texture_coord->set1Value(idx,(yyvsp[(3) - (5)].real)/(yyvsp[(5) - (5)].real), (yyvsp[(4) - (5)].real)/(yyvsp[(5) - (5)].real));
      else
         yy_texture_coord->set1Value(idx, 0, 0);
  }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 585 "src/cds_parser.y"
    {
     /* Almacenamos la informacion de normal */
      int idx = yyNormal->vector.getNum();
      yyNormal->vector.set1Value(idx,(yyvsp[(3) - (5)].real), (yyvsp[(4) - (5)].real), (yyvsp[(5) - (5)].real));
  }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 592 "src/cds_parser.y"
    {
     /* Ignoramos la informacion */
  }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 597 "src/cds_parser.y"
    {
    ignora_resto_linea();
  }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 602 "src/cds_parser.y"
    {
    ignora_resto_linea();
  }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 606 "src/cds_parser.y"
    {
    ignora_resto_linea();
  }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 610 "src/cds_parser.y"
    {
    ignora_resto_linea();
  }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 614 "src/cds_parser.y"
    {
    ignora_resto_linea();
  }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 618 "src/cds_parser.y"
    {
    //En ocasiones hay comentarios marcados con $
    ignora_resto_linea();
  }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 629 "src/cds_parser.y"
    {
     /* Almacenamos el punto. */
     yyCoordinate3->point.set1Value(yyNumeroPuntos++, (yyvsp[(1) - (4)].real), (yyvsp[(2) - (4)].real), (yyvsp[(3) - (4)].real));
  }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 637 "src/cds_parser.y"
    {
    int i;

    //Leemos el depth y branch
    yy_sph_depth = (yyvsp[(1) - (2)].entero);
    yy_sph_branch = (yyvsp[(2) - (2)].entero);

    //Introducimos el arbol de esferas en un blinker 
     yyGeometry = new SoSeparator();
     yyBlinker = new SoBlinker();
     yyBlinker->speed = 0.3f;
     yyGeometry->addChild(yyBlinker);

    //Colgamos un separator por cada nivel
    for (i=0; i < yy_sph_depth; i++)
    {
       char buf[16];
       SoSeparator *sep = new SoSeparator();
       yyBlinker->addChild (sep);
       //Le damos un nombre al nivel
       sprintf(buf, "L%d", i);
       sep->setName(buf);
    }

    //Nivel que vamos a leer
    yy_sph_l      = 0;
    yy_sph_l_sep  = (SoSeparator *)yyBlinker->getChild(0);

    //Numero de esferas en el nivel
    yy_sph_num    = 1;

  }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 670 "src/cds_parser.y"
    {
  }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 679 "src/cds_parser.y"
    {
    int i;
    //fprintf(stderr, "l=%d quedan=%d radio=%g\n",yy_sph_l,yy_sph_num,$4);

    //Añadimos la esfera al nivel actual, con una traslacion a su sitio
    SoSeparator *sep = new SoSeparator ();
    yy_sph_l_sep->addChild (sep);

    SoTranslation *trl = new SoTranslation();
    trl->translation.setValue ((yyvsp[(1) - (5)].real), (yyvsp[(2) - (5)].real), (yyvsp[(3) - (5)].real));
    sep->addChild(trl);

    SoSphere *sph = new SoSphere();
    sph->radius = (yyvsp[(4) - (5)].real);
    sep->addChild (sph);

    //decrementamos el numero de esferas restantes por leer del nivel
    yy_sph_num--;

    //Comprobamos si se ha llenado este nivel
    if (yy_sph_num == 0)
    {
       //Aumentamos el nivel que vamos a leer
       yy_sph_l++;

       //Comprobamos el numero de niveles leidos y cambiamos al separador 
       //del nivel actual, o bien terminamos la lectura de fichero
       if (yy_sph_l < yyBlinker->getNumChildren())
          yy_sph_l_sep  = (SoSeparator *)yyBlinker->getChild(yy_sph_l);
       else
       {
          //Hemos leido todas las esferas de todos los niveles
          return 0;
       }

       //Calculamos el numero de esferas restantes
       yy_sph_num = 1;
       for (i=0; i<yy_sph_l; i++)
           yy_sph_num *= yy_sph_branch;

    }// if (yy_sph_num == 0)

  }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 726 "src/cds_parser.y"
    {
     /* Preparamos para leer STL */
     yyGeometry = new SoSeparator();
     yyCoordinate3 = new SoCoordinate3();
     yyFaceSet = new SoFaceSet();
     //yyNormal = new SoNormal();

     yyGeometry->addChild(yyCoordinate3);
     //yyGeometry->addChild(yyNormal);
     yyGeometry->addChild(yyFaceSet);

     yyNumeroFacetas = 0;
     yyNumeroPuntos = 0;
     yylinenum = 1;
     yy_loopsize=0;
  }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 751 "src/cds_parser.y"
    {yy_outerloop=true;}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 753 "src/cds_parser.y"
    {
       yyFaceSet->numVertices.set1Value(yyNumeroFacetas++, yy_loopsize);
       yy_loopsize=0;
       yy_outerloop=false;
    }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 765 "src/cds_parser.y"
    {
     yyCoordinate3->point.set1Value(yyNumeroPuntos++, (yyvsp[(2) - (4)].real), (yyvsp[(3) - (4)].real), (yyvsp[(4) - (4)].real));
     yy_loopsize++;
}
    break;



/* Line 1806 of yacc.c  */
#line 2385 "y.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 771 "src/cds_parser.y"


void yyerror(const char *s)
{
  fprintf(stderr, "\nLinea %u: %s\n", yylinenum, s);
  /* No hay piedad con los errores */
  //exit(-1);
}

void yyerror(char *s)
{
  fprintf(stderr, "\nLinea %u: %s\n", yylinenum, s);
  /* No hay piedad con los errores */
  //exit(-1);
}

/* Funcion que lee del fichero hasta un salto de linea */
void ignora_resto_linea()
{
   //Indicamos al scanner que queremos ver los saltos de linea
   yy_ver_LF = 1;

   //Leemos hasta el fin de linea
   while (yylex() != _LF) {}

   //Indicamos al scanner que no queremos ver los saltos de linea
   yy_ver_LF = 0;
}

/* Funcion que lee un numero del fichero, independientemente de si
   es real o entero */
float LeeReal ()
{
   int tipo;
   tipo = yylex();
   if (tipo == _REAL)
     return yylval.real;
   else if (tipo == _ENTERO)
     return (float)yylval.entero;
   else
   {
     fprintf(stderr, "\nLine %u: Format error in real number.\n", yylinenum);
     exit (-1);
   }
}

/* Funcion que lee un numero del fichero, obligatoriamente entero */
int LeeEntero ()
{
   int tipo;
   tipo = yylex();
   if (tipo == _ENTERO)
     return yylval.entero;
   else
   {
     fprintf(stderr, "\nLine %u: Format error in integer number.\n", yylinenum);
     exit (-1);
   }
}


/* Función que trata un bloque OFF/COFF/NOFF/NCOFF/STOFF ignorando la
   información de color y normal. El tercer argumento indica
   cuantos valores deben ignorarse despues de leer cada punto 3D */
void LeeBloqueOFF (int nPuntos, int nCaras, int tipoBloqueOFF)
{
   int nVertCara, i,j;
   float x, y, z;
   float r, g, b, a;
   float nx, ny, nz;
   float tx, ty;

   /* Reservamos espacio para las coordenadas de los vértices */
   SoCoordinate3 *coordinate3 = new SoCoordinate3();
   yyGeometry->addChild(coordinate3);

   /* Si hay información de color, creamos un material */
   if (tipoBloqueOFF == _COFF || tipoBloqueOFF == _NCOFF )
   {
     yyMaterialBinding=new SoMaterialBinding();
     yyMaterialBinding->value=SoMaterialBinding::PER_VERTEX_INDEXED;
     yyGeometry->addChild(yyMaterialBinding);

     yyMaterial=new SoMaterial();
     yyGeometry->addChild(yyMaterial);
   }

   /* Si hay información de normales, creamos un SoNormal */
   if (tipoBloqueOFF == _NOFF || tipoBloqueOFF == _NCOFF )
   {
     yyNormal=new SoNormal();
     yyGeometry->addChild(yyNormal);
   }

   /* Si hay información de textura, creamos un SoTextureCoordinate2 */
   if (tipoBloqueOFF == _STOFF)
   {
     yyTextureCoordinate2 = new SoTextureCoordinate2;
     yyGeometry->addChild(yyTextureCoordinate2);
   }

   /* Reservamos espacio para las facetas de los vértices */
   SoIndexedPointSet *yyIndexedPointSet = new SoIndexedPointSet();
   yyGeometry->addChild(yyIndexedPointSet);
   SoIndexedLineSet *yyIndexedLineSet = new SoIndexedLineSet();
   yyGeometry->addChild(yyIndexedLineSet);
   SoIndexedFaceSet *yyIndexedFaceSet = new SoIndexedFaceSet();
   yyGeometry->addChild(yyIndexedFaceSet);

   /* Lectura de la informacion de los puntos */
   for (i=0; i<nPuntos; i++)
   {
     /* Leemos x,y,z mediante yylex() */
     x = LeeReal();
     y = LeeReal();
     z = LeeReal();

     /* Almacenamos el punto. */
     coordinate3->point.set1Value(i,x,y,z);

     /* Si hay información de normales, leemos la información */
     if (tipoBloqueOFF == _NOFF || tipoBloqueOFF == _NCOFF )
     {

       /* Leemos nx,ny,nz mediante yylex() */
       nx = LeeReal();
       ny = LeeReal();
       nz = LeeReal();

       /* Almacenamos la normal. */
       yyNormal->vector.set1Value(i, nx, ny, nz);

     }//if normal

     /* Si hay información de color, leemos la información */
     if (tipoBloqueOFF == _COFF || tipoBloqueOFF == _NCOFF )
     {
       r = LeeReal();
       g = LeeReal();
       b = LeeReal();
       a = LeeReal();

       /* Almacenamos el color. */
       yyMaterial->diffuseColor.set1Value(i,r,g,b);

       /* Almacenamos la transparencia */
       yyMaterial->transparency.set1Value(i, 1.0 - a);

     }//if color

     /* Si hay información de textura, leemos la información */
     if (tipoBloqueOFF == _STOFF)
     {
       /* Leemos nx,ny,nz mediante yylex() */
       tx = LeeReal();
       ty = LeeReal();

       /* Almacenamos la normal. */
       yyTextureCoordinate2->point.set1Value(i, tx, ty);

     }//if textura

   } /* for */

   /* Aumentamos la cuenta de vertices */
   yyNumeroPuntos += nPuntos;


   /* Lectura de la informacion de las facetas */
   int k,v;
   k=0;
   int np=0;
   int nl=0;

   for (i=0; i<nCaras; i++)
   {
     /* Leemos el numero de vertices de esta faceta */
     nVertCara = LeeEntero();

     /* Ignoramos facetas de menos de 3 vértices */
     if (nVertCara <= 0)
     {
       fprintf(stderr, "\nLine %u: Size of face cannot be %d\n", yylinenum, nVertCara);
       /* Continuamos con la siguiente faceta */
       continue;
     }

     if (nVertCara == 1)
     {
       /* Leemos el indices y creamos un punto visible */
       v = LeeEntero();
       yyIndexedPointSet->coordIndex.set1Value(np++, v);
       continue;
     }

     if (nVertCara == 2)
     {
       /* Leemos dos indices y creamos una arista */
       v = LeeEntero();
       yyIndexedLineSet->coordIndex.set1Value(nl++, v);
       v = LeeEntero();
       yyIndexedLineSet->coordIndex.set1Value(nl++, v);
       yyIndexedLineSet->coordIndex.set1Value(nl++, -1);

       /* Continuamos con la siguiente faceta */
       continue;
     }

     for (j=0; j<nVertCara; j++)
     {
       /* Leemos el índice del vértice */
       v = LeeEntero();

       /* Almacenamos la coordenada. */
       yyIndexedFaceSet->coordIndex.set1Value(k++, v);

     } /* for */

     /* Almacenamos la marca de fin de faceta. */
     yyIndexedFaceSet->coordIndex.set1Value(k++, -1);

     //Si hay información de color por faceta, la ignoramos
     ignora_resto_linea();

     /* Aumentamos la cuenta de facetas */
     yyNumeroFacetas++;

   } /* for */

   /* Elimina objetos no utilizados */
   if (np <=0)
      yyGeometry->removeChild(yyIndexedPointSet);
   if (nl <=0)
      yyGeometry->removeChild(yyIndexedLineSet);
   if (k <=0)
      yyGeometry->removeChild(yyIndexedFaceSet);


} /* LeeBloqueOFF (nPuntos, nCaras, tipoBloqueOFF) */


