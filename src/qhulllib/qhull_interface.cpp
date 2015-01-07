
#include "qinterface.h"
/*<html><pre>  -<a                             href="qh-user.htm"
  >-------------------------------</a><a name="TOP">-</a>
*/


//--- Include qhull, so it works from with in a C++ source file
//---
//--- In MVC one cannot just do:
//---
//---    extern "C"
//---    {
//---      #include "qhull_a.h"
//---    }
//---
//--- Because qhull_a.h includes math.h, which can not appear
//--- inside a extern "C" declaration.
//---
//--- Maybe that why Numerical recipes in C avoid this problem, by removing
//--- standard include headers from its header files and add them in the
//--- respective source files instead.
//---
//--- [K. Erleben]

#include <stdio.h>
#include <stdlib.h>
#include "qhull.h"
#include "qh_mem.h"
#include "qh_qset.h"
#include "qh_geom.h"
#include "qh_merge.h"
#include "qh_poly.h"
#include "qh_io.h"
#include "qh_stat.h"



int QhullMain(int argc, char const *argv[],int numpoints,const float *verts)
{
  int exitcode,dim;
  coordT *points;
  boolT ismalloc;

  exitcode = 0;


  qh_init_A (stdin, stdout, stderr, argc, argv);  /* sets qh qhull_command */

  qh_initflags (qh qhull_command);

  dim = 3;

  points = (double *)gInterface->HullMalloc(sizeof(coordT)*numpoints*dim);

  if ( 1 )
  {
    for (int i=0; i<numpoints*dim; i++)
    {
      points[i] = verts[i];
    }
  }

  ismalloc = 1;


  qh_init_B (points, numpoints, dim, ismalloc);
  qh_qhull();
  qh_check_output();
  qh_produce_output();

  gInterface->HullFree(points);

  return exitcode;
} /* main */

