#ifndef QINTERFACE_H
#define QINTERFACE_H

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h>

// This is the interface that the Qhull code uses to report back results to the application.
class QhullInterface
{
public:

  virtual void HullBegin(unsigned int dimensions,
                         unsigned int vcount,
                         unsigned int fcount,
                         unsigned int totneighbors) = 0;

  virtual void HullVertex(float x,float y,float z) = 0;
  virtual void HullPolygon(unsigned int pcount,unsigned int *vertices) = 0;
  virtual void HullAreaVolume(float area,float volume) = 0;

  virtual void HullPrintf(FILE *fph,const char *fmt, ... ) = 0;

  virtual void * HullMalloc(unsigned int size) = 0;
  virtual void   HullFree(void *mem) = 0;
  virtual void   HullIsNarrow(void) = 0; // warning message that the hull is 'narrow'.
  virtual void   HullErrorExit(int exitcode) = 0;

private:
};

extern QhullInterface *gInterface;

#endif
