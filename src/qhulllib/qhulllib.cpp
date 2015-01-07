/*----------------------------------------------------------------------
    Copyright (c) 2004 Open Dynamics Framework Group
          www.physicstools.org
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided
    that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions
    and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    Neither the name of the Open Dynamics Framework Group nor the names of its contributors may
    be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE INTEL OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
    IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <stdarg.h>
#include <setjmp.h>

#define USE_PRINTF 0 // true if we want to print some info to the console.
#include "qhulllib.h"

/* MESH_OPTIMIZE
#include <include/meshoptimizeinterface.h>
*/

#include "qinterface.h"


QhullInterface *gInterface=0;      // hooks so qhull can call back to our interface to send the results.
jmp_buf         gErrExit;          // jump buffer for when qhull wants to error exit.

static int gSequenceCount=0;

// 16 byte header in front of all memory allocations to keep track of memory that Qhull did not free.
class MemStamp
{
public:
  MemStamp(void)
  {
    Init(0,0);
  }

  void Init(unsigned int size,unsigned int index)
  {
    mSize = size;
    mId[0] = 'J';
    mId[1] = 'O';
    mId[2] = 'H';
    mId[3] = 'N';
    mId[4] = 'R';
    mId[5] = 'A';
    mId[6] = 'T';
    mId[7] = 0;
    mSize  = size;
    mIndex = index;
  }

  unsigned int GetSize(void) const
  {

    unsigned int ret = 0;

    assert( mId[0] == 'J');
    assert( mId[1] == 'O');
    assert( mId[2] == 'H');
    assert( mId[3] == 'N');
    assert( mId[4] == 'R');
    assert( mId[5] == 'A');
    assert( mId[6] == 'T');
    assert( mId[7] == 0);

    if ( mId[0] == 'J' && mId[1] == 'O' && mId[2] == 'H' && mId[3] == 'N' )
    {
      ret = mSize;
    }

    return ret;
  }

  unsigned int GetIndex(void) const { return mIndex; };

  unsigned char mId[8];         // identifier.
  unsigned int  mSize;          // size of allocation.
  unsigned int  mIndex;         // index number of allocation.
};


#define MAX_ALLOCATIONS 4096 // maximum number of memory allocations!

class QhullParser : public QhullInterface
{
public:
  QhullParser(QhullUserAllocator *useralloc)
  {
    mUserAlloc    = useralloc;
    mVerts        = 0;
    mUsed         = 0;
    mAllocCount   = 0;
    mAllocSize    = 0;
    mMaxAllocSize = 0;
    mAllocIndex   = 0;
    mIndices      = 0;
    mOutV         = 0;
    mOutCount     = 0;
    mIcount       = 0;
    for (int i=0; i<MAX_ALLOCATIONS; i++)
    {
      mStamps[i] = 0;
    }
    mArea = 0;
    mVolume = 0;
  }

  ~QhullParser(void)
  {
    Release();

    // Free up any allocations Qhull forgot to delete!
    for (unsigned int i=0; i<MAX_ALLOCATIONS; i++)
    {
      if ( mStamps[i] )
      {
        MemStamp *ms = mStamps[i];
        assert( ms->GetIndex() == i );
        ms++;
        HullFree(ms);
        assert(mStamps[i] == 0 );
      }
    }
    assert( mAllocCount == 0 );
    assert( mAllocSize  == 0 );

  }

  void Release(void)
  {
    if ( mVerts )
    {
      HullFree(mVerts);
      mVerts = 0;
    }
    if ( mUsed  )
    {
      HullFree(mUsed);
      mUsed = 0;
    }
    if ( mOutV )
    {
      HullFree(mOutV);
      mOutV = 0;
      mOutCount = 0;
    }
    if ( mIndices )
    {
      HullFree(mIndices);
      mIndices = 0;
      mIcount = 0;
    }
    mVertCount = 0;
  }

  virtual void HullBegin(unsigned int dimensions,
                         unsigned int vcount,
                         unsigned int fcount,
                         unsigned int /*totneighbors*/)
  {


    assert( mVerts == 0 );
    assert( dimensions == 3 );

    Release();

    mDimensions = dimensions;
    mVertCount  = vcount;
    mFaceCount  = fcount;
    mTriCount   = 0;
    mVindex     = 0; // counter as vertices come back to us from Qhull.
    mFindex     = 0;
    mOutCount   = 0;
    mIcount     = 0;
    mMaxIndices = 0;

    if ( mVertCount )
    {

      mVerts = (float *)HullMalloc( sizeof(float)*3*mVertCount );
      mUsed  = (unsigned int *)HullMalloc( mVertCount*sizeof(unsigned int) );
      memset(mUsed,0,sizeof(unsigned int)*mVertCount);
      memset(mVerts,0,sizeof(float)*3*mVertCount);

      mOutV = (float *) HullMalloc( sizeof(float)*3*mVertCount );
      mMaxIndices = mVertCount*16;
      mIndices    = (unsigned int *) HullMalloc( sizeof(unsigned int)*mMaxIndices );

    }
  }

  virtual void HullVertex(float x,float y,float z)
  {
    if ( mVindex < mVertCount )
    {
      float *dest = &mVerts[mVindex*3];
      dest[0] = x;
      dest[1] = y;
      dest[2] = z;
      mVindex++;
      assert(mVindex <= mVertCount );
    }
  }

  virtual void HullPolygon(unsigned int pcount,unsigned int *vertices)
  {
    assert( mFindex < mFaceCount );
    if ( mFindex < mFaceCount )
    {

      mTriCount+=(pcount-2);

      PushIndex(pcount);
      for (unsigned int i=0; i<pcount; i++)
      {
        unsigned int v = vertices[i];
        assert( v >= 0 && v < mVertCount );
        if ( v >= 0 && v < mVertCount )
        {
          unsigned int vi = mUsed[v];

          if ( vi == 0 )
          {
            const float *source = &mVerts[v*3];
            float       *dest   = &mOutV[mOutCount*3];

            dest[0] = source[0];
            dest[1] = source[1];
            dest[2] = source[2];

            mOutCount++;

            mUsed[v] = mOutCount;
            vi = mOutCount;
          }
          vi--;
          PushIndex(vi);
        }
      }
      mFindex++;
    }
  }

  void PushIndex(unsigned int index)
  {
    if ( mIcount < mMaxIndices )
    {
      mIndices[mIcount] = index;
      mIcount++;
    }
  }

  bool Complete(QhullResult *qprop,bool triangles,bool reverseorder)
  {

    bool ret = false;

    qprop->mNumOutputVertices = 0;
    qprop->mOutputVertices    = 0;
    qprop->mNumFaces          = 0;
    qprop->mNumTriangles      = 0;
    qprop->mNumIndices        = 0;
    qprop->mIndices           = 0;
    qprop->mPolygons          = true;

    if ( mVerts && mUsed && mOutCount && mIcount ) // if we have a solution.
    {
      qprop->mNumOutputVertices = mOutCount;
      qprop->mOutputVertices = (float *) MemAlloc( sizeof(float)*3*qprop->mNumOutputVertices );
      memcpy( qprop->mOutputVertices, mOutV, sizeof(float)*3*qprop->mNumOutputVertices );

      qprop->mNumFaces       = mFaceCount;
      qprop->mNumTriangles   = mTriCount;
      qprop->mNumIndices     = mIcount;

      if ( triangles )
      {
        qprop->mPolygons   = false;
        qprop->mIndices    = (unsigned int *) MemAlloc(sizeof(unsigned int)*mTriCount*3 );
        qprop->mNumIndices = mTriCount*3;

        unsigned int *idx  = mIndices;
        unsigned int *dest = qprop->mIndices;

        for (unsigned int i=0; i<mFaceCount; i++)
        {
          unsigned int pcount = *idx++;

          unsigned int i1 = *idx++;
          unsigned int i2 = *idx++;
          unsigned int i3 = *idx++;

          if ( reverseorder )
          {
            *dest++ = i3;
            *dest++ = i2;
            *dest++ = i1;
          }
          else
          {
            *dest++ = i1;
            *dest++ = i2;
            *dest++ = i3;
          }


          pcount-=3;

          while ( pcount )
          {
            i2 = i3;
            i3 = *idx++;

            if ( reverseorder )
            {
              *dest++ = i3;
              *dest++ = i2;
              *dest++ = i1;
            }
            else
            {
              *dest++ = i1;
              *dest++ = i2;
              *dest++ = i3;
            }

            pcount--;
          }

        }

      }
      else
      {
        qprop->mPolygons = true;
        qprop->mIndices = (unsigned int *) MemAlloc(sizeof(unsigned int)*mIcount );
        memcpy(qprop->mIndices, mIndices,sizeof(unsigned int)*mIcount);
      }

      ret  = true; // we have hull data!
    }

    Release(); // release all memory consumed in the process of generating the hull, except for the results we are returning in QhullProperties.

    return ret;
  }

  // Traps any output from Qhull code that we don't yet parse correctly.
  // If this was valid data you were interested in, then backtrackup the callstack and replace the 'HullPrintf' calls with calls to
  // the interface class to return the results as you need.
  virtual void HullPrintf(FILE * /*fph*/ ,const char *fmt, ... )
  {
    char wbuff[8192];
    va_list args;
    va_start(args,fmt);
    //vsprintf(wbuff, fmt, (char *)(&fmt+1));
    vsprintf(wbuff, fmt, args);
    HullErrorExit(1);
  }

  virtual void * HullMalloc(unsigned int size)
  {
    assert( size );
    //JESPA assert( size < 4000000 ); // never allocate more than 4mb

    MemStamp *ret = (MemStamp *) MemAlloc( size+sizeof(MemStamp) ); // fullfill allocation request, plus the size of the memory block.

    assert(ret); // assert that the allocation was successful.

    if ( ret )
    {


      assert( mAllocCount < MAX_ALLOCATIONS ); // assert that we have less than 'MAX_ALLOCATIONS' total outstanding allocs at this time.

      unsigned int search = mAllocIndex;  // begin searching at this current index number.

      for (unsigned int i=0; i<MAX_ALLOCATIONS; i++) // search for any free slot.
      {
        if ( mStamps[search] == 0 ) break;  // if this slot is free, break.
        search++; // advance the search index.
        if ( search == MAX_ALLOCATIONS ) search = 0; // wrap back to zero.
      }

      assert( mStamps[search] == 0 ); // assert that we found a free slot.

      if ( mStamps[search] == 0 )  // if the slot if free.
      {
        mAllocCount++;        // increment active allocation count.
        mAllocSize+=size;     // total current allocation size.
        ret->Init(size,search); // init it
        mStamps[search] = ret;
        ret++;
        if ( mAllocSize > mMaxAllocSize )
          mMaxAllocSize = mAllocSize;
      }
      else
      {
        free(ret); // free the allocated memory.
        ret = 0;
      }
    }

    return ret;
  }

  virtual void   HullFree(void *mem)
  {
    assert(mem);

    if ( mem )
    {
      MemStamp *ms = (MemStamp *) mem;
      ms--;
      unsigned int size = ms->GetSize();
      assert(size);
      if ( size )
      {
        unsigned int index = ms->GetIndex();
        assert( index >= 0 && index < MAX_ALLOCATIONS );
        assert( mStamps[index] == ms );
        mStamps[index] = 0;
        mAllocCount--;
        mAllocSize-=size;
        MemFree(ms);
      }
    }
  }

  void * MemAlloc(unsigned int size)
  {
    void * ret = 0;
    if (mUserAlloc )
    {
      ret = mUserAlloc->QhullAlloc(size);
    }
    else
    {
      ret = malloc(size);
    }
    return ret;
  }

  void MemFree(void *mem)
  {
    if ( mUserAlloc )
    {
      mUserAlloc->QhullFree(mem);
    }
    else
    {
      free(mem);
    }
  }


  virtual void HullAreaVolume(float area,float volume)
  {
    mArea   = area;
    mVolume = volume;
  }

  float GetArea(void)   const { return mArea; };
  float GetVolume(void) const { return mVolume; };


  void AddPoint(unsigned int &vcount,float *p,float x,float y,float z)
  {
    float *dest = &p[vcount*3];

    dest[0] = x;
    dest[1] = y;
    dest[2] = z;

    vcount++;

  }

  float GetDist(float px,float py,float pz,const float *p2)
  {

    float dx = px - p2[0];
    float dy = py - p2[1];
    float dz = pz - p2[2];

    return dx*dx+dy*dy+dz*dz;
  }

  bool  CleanupVertices(unsigned int svcount,
                        const float *svertices,
                        unsigned int stride,
                        unsigned int &vcount,       // output number of vertices
                        float *vertices,                 // location to store the results.
                        float  normalepsilon,
                        float *scale,
                        bool removeduplicates)
  {

    if ( svcount == 0 ) return false;


    #define EPSILON 0.000001f // close enough to consider two floating point numbers to be 'the same'.

    vcount = 0;

    float recip[3];

    if ( scale )
    {
      scale[0] = 1;
      scale[1] = 1;
      scale[2] = 1;
    }

    float bmin[3] = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
    float bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    const char *vtx = (const char *) svertices;

    if ( 1 )
    {
      for (unsigned int i=0; i<svcount; i++)
      {
        const float *p = (const float *) vtx;

        vtx+=stride;

        for (int j=0; j<3; j++)
        {
          if ( p[j] < bmin[j] ) bmin[j] = p[j];
          if ( p[j] > bmax[j] ) bmax[j] = p[j];
        }
      }
    }

    float dx = bmax[0] - bmin[0];
    float dy = bmax[1] - bmin[1];
    float dz = bmax[2] - bmin[2];

    float center[3];

    center[0] = dx*0.5f + bmin[0];
    center[1] = dy*0.5f + bmin[1];
    center[2] = dz*0.5f + bmin[2];

    if ( dx < EPSILON || dy < EPSILON || dz < EPSILON || svcount < 3 )
    {

      float len = FLT_MAX;

      if ( dx > EPSILON && dx < len ) len = dx;
      if ( dy > EPSILON && dy < len ) len = dy;
      if ( dz > EPSILON && dz < len ) len = dz;

      if ( len == FLT_MAX )
      {
        dx = dy = dz = 0.01f; // one centimeter
      }
      else
      {
        if ( dx < EPSILON ) dx = len * 0.05f; // 1/5th the shortest non-zero edge.
        if ( dy < EPSILON ) dy = len * 0.05f;
        if ( dz < EPSILON ) dz = len * 0.05f;
      }

      float x1 = center[0] - dx;
      float x2 = center[0] + dx;

      float y1 = center[1] - dy;
      float y2 = center[1] + dy;

      float z1 = center[2] - dz;
      float z2 = center[2] + dz;

      AddPoint(vcount,vertices,x1,y1,z1);
      AddPoint(vcount,vertices,x2,y1,z1);
      AddPoint(vcount,vertices,x2,y2,z1);
      AddPoint(vcount,vertices,x1,y2,z1);
      AddPoint(vcount,vertices,x1,y1,z2);
      AddPoint(vcount,vertices,x2,y1,z2);
      AddPoint(vcount,vertices,x2,y2,z2);
      AddPoint(vcount,vertices,x1,y2,z2);

      #if USE_PRINTF
      printf("Too few input vertices, converting to a bounding box.\r\n");
      #endif

      return true; // return cube


    }
    else
    {
      if ( scale )
      {
        scale[0] = dx;
        scale[1] = dy;
        scale[2] = dz;

        recip[0] = 1 / dx;
        recip[1] = 1 / dy;
        recip[2] = 1 / dz;

        center[0]*=recip[0];
        center[1]*=recip[1];
        center[2]*=recip[2];

      }

    }



    vtx = (const char *) svertices;

    for (unsigned int i=0; i<svcount; i++)
    {

      const float *p = (const float *)vtx;
      vtx+=stride;

      float px = p[0];
      float py = p[1];
      float pz = p[2];

      if ( scale )
      {
        px = px*recip[0]; // normalize
        py = py*recip[1]; // normalize
        pz = pz*recip[2]; // normalize
      }

      if ( removeduplicates )
      {
        unsigned int j;

        for (j=0; j<vcount; j++)
        {
          float *v = &vertices[j*3];

          float x = v[0];
          float y = v[1];
          float z = v[2];

          float dx = fabsf(x - px );
          float dy = fabsf(y - py );
          float dz = fabsf(z - pz );

          if ( dx < normalepsilon && dy < normalepsilon && dz < normalepsilon )
          {
            // ok, it is close enough to the old one
            // now let us see if it is further from the center of the point cloud than the one we already recorded.
            // in which case we keep this one instead.

            float dist1 = GetDist(px,py,pz,center);
            float dist2 = GetDist(v[0],v[1],v[2],center);

            if ( dist1 > dist2 )
            {
              v[0] = px;
              v[1] = py;
              v[2] = pz;
            }

            break;
          }
        }

        if ( j == vcount )
        {
          float *dest = &vertices[vcount*3];
          dest[0] = px;
          dest[1] = py;
          dest[2] = pz;
          vcount++;
        }
      }
      else
      {
        float *dest = &vertices[vcount*3];
        dest[0] = px;
        dest[1] = py;
        dest[2] = pz;
        vcount++;
      }
    }

    // ok..now make sure we didn't prune so many vertices it is now invalid.
    if ( 1 )
    {
      float bmin[3] = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
      float bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

      for (unsigned int i=0; i<vcount; i++)
      {
        const float *p = &vertices[i*3];
        for (int j=0; j<3; j++)
        {
          if ( p[j] < bmin[j] ) bmin[j] = p[j];
          if ( p[j] > bmax[j] ) bmax[j] = p[j];
        }
      }

      float dx = bmax[0] - bmin[0];
      float dy = bmax[1] - bmin[1];
      float dz = bmax[2] - bmin[2];

      if ( dx < EPSILON || dy < EPSILON || dz < EPSILON || vcount < 3)
      {
        float cx = dx*0.5f + bmin[0];
        float cy = dy*0.5f + bmin[1];
        float cz = dz*0.5f + bmin[2];

        float len = FLT_MAX;

        if ( dx >= EPSILON && dx < len ) len = dx;
        if ( dy >= EPSILON && dy < len ) len = dy;
        if ( dz >= EPSILON && dz < len ) len = dz;

        if ( len == FLT_MAX )
        {
          dx = dy = dz = 0.01f; // one centimeter
        }
        else
        {
          if ( dx < EPSILON ) dx = len * 0.05f; // 1/5th the shortest non-zero edge.
          if ( dy < EPSILON ) dy = len * 0.05f;
          if ( dz < EPSILON ) dz = len * 0.05f;
        }

        float x1 = cx - dx;
        float x2 = cx + dx;

        float y1 = cy - dy;
        float y2 = cy + dy;

        float z1 = cz - dz;
        float z2 = cz + dz;

        vcount = 0; // add box

        AddPoint(vcount,vertices,x1,y1,z1);
        AddPoint(vcount,vertices,x2,y1,z1);
        AddPoint(vcount,vertices,x2,y2,z1);
        AddPoint(vcount,vertices,x1,y2,z1);
        AddPoint(vcount,vertices,x1,y1,z2);
        AddPoint(vcount,vertices,x2,y1,z2);
        AddPoint(vcount,vertices,x2,y2,z2);
        AddPoint(vcount,vertices,x1,y2,z2);

        #if USE_PRINTF
        printf("Too few input vertices, converting to a bounding box.\r\n");
        #endif

        return true;
      }
    }

    #if USE_PRINTF
    printf("Reduced point cloud from %d vertices in to %d vertices out, removed %d\r\n", svcount, vcount, svcount - vcount );
    #endif


    return true;

  }

  void DumpObj(QhullResult &answer)
  {
    char scratch[512];

    gSequenceCount++;
    sprintf(scratch,"QHULL_OK_%04d.obj", gSequenceCount );

    FILE *fph = fopen(scratch,"wb");

    if ( fph )
    {
      #if USE_PRINTF
      printf("Saving %s with %d vertices and %d faces.\r\n", scratch, answer.mNumOutputVertices, answer.mNumFaces );
      #endif

      fprintf(fph,"#Vertices: %8d\r\n",    answer.mNumOutputVertices );
      fprintf(fph,"#Faces   : %8d\r\n",    answer.mNumFaces );

      for (unsigned int i=0; i<answer.mNumOutputVertices; i++)
      {
        const float *p = &answer.mOutputVertices[i*3];
        fprintf(fph,"v %0.9f %0.9f %0.9f\r\n", p[0], p[1], p[2] );
      }

      if ( 1 )
      {
        unsigned int *indices = answer.mIndices;

        if ( answer.mPolygons )
        {
          for (unsigned int i=0; i<answer.mNumFaces; i++)
          {
            fprintf(fph,"f ");

            unsigned int pcount = *indices++;

            if ( 1 )
            {
              for (int j=pcount-1; j>=0; j--)
              {
                unsigned int k = indices[j];
                fprintf(fph,"%d ", k+1 );
              }
            }


            indices+=pcount;

            fprintf(fph,"\r\n");
          }
        }
        else
        {
          for (unsigned int i=0; i<answer.mNumTriangles; i++)
          {
            unsigned int i1 = *indices++;
            unsigned int i2 = *indices++;
            unsigned int i3 = *indices++;

            fprintf(fph,"f %d %d %d\r\n", i3+1, i2+1, i1+1);
          }
        }

      }
      fclose(fph);
    }
  }

  void DumpObj(unsigned int vcount,const float *vertices,unsigned int stride)
  {
    char scratch[512];

    gSequenceCount++;
    sprintf(scratch,"QHULL_FAIL_%04d.obj", gSequenceCount );

    FILE *fph = fopen(scratch,"wb");

    if ( fph )
    {
      #if USE_PRINTF
      printf("Qhull failed with %d input vertices.  Output file: %s\n", vcount, scratch );
      #endif

      fprintf(fph,"#Vertices: %8d\r\n", vcount );

      const char *vtx = (const char *) vertices;

      for (unsigned int i=0; i<vcount; i++)
      {
        const float *p = (const float *)vtx;

        float vx = p[0];
        float vy = p[1];
        float vz = p[2];

        fprintf(fph,"v %0.9f %0.9f %0.9f\r\n", vx, vy, vz );
        vtx+=stride;
      }

      fclose(fph);
    }

  }

  virtual void HullIsNarrow(void)
  {
  }

  virtual void HullErrorExit(int exitcode)
  {
    Release();
    longjmp(gErrExit,exitcode);
  }

  void BoundingBox(unsigned int &vcount,float *vertices)
  {
    float bmin[3] = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
    float bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (unsigned int i=0; i<vcount; i++)
    {
      const float *p = (const float *) &vertices[i*3];
      for (int j=0; j<3; j++)
      {
        if ( p[j] < bmin[j] ) bmin[j] = p[j];
        if ( p[j] > bmax[j] ) bmax[j] = p[j];
      }
    }

    float dx = bmax[0] - bmin[0];
    float dy = bmax[1] - bmin[1];
    float dz = bmax[2] - bmin[2];

    float center[3];

    center[0] = dx*0.5f + bmin[0];
    center[1] = dy*0.5f + bmin[1];
    center[2] = dz*0.5f + bmin[2];

    float x1 = center[0] - dx;
    float x2 = center[0] + dx;
    float y1 = center[1] - dy;
    float y2 = center[1] + dy;
    float z1 = center[2] - dz;
    float z2 = center[2] + dz;

    vcount = 0;

    AddPoint(vcount,vertices,x1,y1,z1);
    AddPoint(vcount,vertices,x2,y1,z1);
    AddPoint(vcount,vertices,x2,y2,z1);
    AddPoint(vcount,vertices,x1,y2,z1);
    AddPoint(vcount,vertices,x1,y1,z2);
    AddPoint(vcount,vertices,x2,y1,z2);
    AddPoint(vcount,vertices,x2,y2,z2);
    AddPoint(vcount,vertices,x1,y2,z2);

  }

private:
  unsigned int      mDimensions;
  unsigned int     *mUsed;
  float            *mVerts;
  unsigned int      mVertCount;
  unsigned int      mVindex;
  unsigned int      mFindex; // received number of faces.

  unsigned int      mFaceCount;
  unsigned int      mTriCount;
  unsigned int      mAllocCount;
  unsigned int      mAllocSize;
  unsigned int      mMaxAllocSize;

  unsigned int      mAllocIndex;
  MemStamp         *mStamps[MAX_ALLOCATIONS];

  // Results...
  unsigned int      mOutCount;   // number of output vertices.
  float            *mOutV;       // output vertices.
  unsigned int      mIcount;     // current index count.
  unsigned int      mMaxIndices; // the maximum number of indices we support!
  unsigned int     *mIndices;
  QhullUserAllocator *mUserAlloc;

  float               mArea;
  float               mVolume;

};



int QhullMain(int argc, char const*argv[],int numverts,const float *verts);

QhullError QhullLibrary::CreateConvexHull(const QhullDesc       &desc,           // describes the input request
                                          QhullResult           &result)         // contains the resulst
{
  QhullError ret = QE_FAIL;

  #if USE_PRINTF
  printf("Submitting %d vertices to Qhull\r\n", desc.mVcount );
  #endif

  char const*argv[16];

  argv[0] = "qhull";
  argv[1] = "o";   // produce off file output...

  QhullParser qp(mAllocator);

  gInterface = &qp; // expose the interface as a global variable.

  unsigned int count = desc.mVcount;
  if ( count < 8 ) count = 8;
  count++;

  float scale[3];
  float *nvertices = (float *)gInterface->HullMalloc( sizeof(float)*3*count );

  // Normalize the vertices to a unit cube and remove duplicates.
  unsigned int nvcount;

  float *usescale = 0;

  if ( desc.HasQhullFlag(QF_NORMALIZE) )
  {
    usescale = scale;
  }

  bool ok = qp.CleanupVertices(desc.mVcount,desc.mVertices,desc.mVertexStride,nvcount,nvertices,desc.mNormalEpsilon,usescale, desc.HasQhullFlag(QF_REMOVE_DUPLICATES) );


  if ( ok )
  {
    bool first = true;

    unsigned int code = setjmp(gErrExit);

    if ( code == 0 )
    {
      QhullMain(2,argv,nvcount,nvertices); // build hull from normalized and cleaned up vertice input
    }
    else
    {
      if ( first )
      {
        first = false;

        if ( desc.HasQhullFlag(QF_SAVE_OBJ_FAIL) )
        {
          qp.DumpObj(nvcount,nvertices,sizeof(float)*3);
        }

        qp.BoundingBox(nvcount,nvertices); // just keep the bounding box as the hull!
        QhullMain(2,argv,nvcount,nvertices); // build hull from normalized and cleaned up vertice input
      }
      else
      {
        assert(0); // this just should freaking never happen!
      }
    }


    bool ok = qp.Complete(&result, desc.HasQhullFlag(QF_TRIANGLES), desc.HasQhullFlag(QF_REVERSE_ORDER) ); // capture the results

    if ( ok )
    {
      ret = QE_OK;
      // Scale answers back into their original space.
      if ( usescale )
      {
        for (unsigned int i=0; i<result.mNumOutputVertices; i++)
        {
          float *p = &result.mOutputVertices[i*3];
          p[0]*=scale[0];
          p[1]*=scale[1];
          p[2]*=scale[2];
        }
      }

      /* MESH_OPTIMIZE
       
      if ( mMeshInterface && desc.HasQhullFlag(QF_MESH_OPTIMIZE) )
      {
        #if USE_PRINTF
        printf("Mesh Optimizing convex hull with %d vertices and %d faces\r\n", result.mNumOutputVertices, result.mNumFaces );
        #endif

        MeshOutput answer;
        bool ok;

        if ( result.mPolygons )
          ok = mMeshInterface->MeshOptimizePolygons(answer,desc.mMeshTolerance, result.mNumOutputVertices, result.mOutputVertices, sizeof(float)*3, result.mNumFaces, result.mIndices );
        else
          ok = mMeshInterface->MeshOptimizeTriangles(answer,desc.mMeshTolerance, result.mNumOutputVertices, result.mOutputVertices, sizeof(float)*3, result.mNumFaces, result.mIndices );

        if ( ok )
        {
          #if USE_PRINTF
          printf("Mesh Optimized Convex Hull from %d input vertices to %d output vertices, %d triangles. Saved %d verts.\r\n", result.mNumOutputVertices, answer.mVcount, answer.mTriCount, result.mNumOutputVertices - answer.mVcount );
          #endif

          if ( result.mPolygons )
          {
            ok = mMeshInterface->TrianglesToPolygons(answer); // if the app wants polygons not triangles.
          }

          if ( ok )
          {
            qp.MemFree( result.mIndices );
            qp.MemFree( result.mOutputVertices );

            result.mNumOutputVertices = answer.mVcount;
            result.mOutputVertices    = (float *)qp.MemAlloc( sizeof(float)*3* answer.mVcount );
            memcpy(result.mOutputVertices, answer.mVertices, sizeof(float)*3* answer.mVcount );
            result.mNumFaces          = answer.mTriCount;
            result.mNumTriangles      = answer.mTriCount;
            result.mIndices           = (unsigned int *)qp.MemAlloc( sizeof(unsigned int)*answer.mIndexCount );
            memcpy(result.mIndices, answer.mIndices,  sizeof(unsigned int)*answer.mIndexCount );
          }

          mMeshInterface->ReleaseMeshOutput(answer);
        }
        else
        {
          #if USE_PRINTF
          printf("Unable to optmize convex hull\r\n");
          #endif
        }
      }
      MESHOPTIMIZE */

    }
  }

  if ( ret == QE_OK )
  {
    if ( desc.HasQhullFlag(QF_SAVE_OBJ_OK) )
    {
      qp.DumpObj(result);
    }
  }
  else
  {
    if ( desc.HasQhullFlag(QF_SAVE_OBJ_FAIL) )
    {
      qp.DumpObj(desc.mVcount, desc.mVertices, desc.mVertexStride );
    }
  }

  assert( ret == QE_OK );

  gInterface->HullFree(nvertices);


  return ret;
}



QhullError QhullLibrary::ReleaseResult(QhullResult &result) // release memory allocated for this result, we are done with it.
{
  if ( result.mOutputVertices )
  {
    if ( mAllocator )
      mAllocator->QhullFree(result.mOutputVertices);
    else
      free(result.mOutputVertices);
    result.mOutputVertices = 0;
  }
  if ( result.mIndices )
  {
    if ( mAllocator )
      mAllocator->QhullFree(result.mIndices);
    else
      free(result.mIndices);
    result.mIndices = 0;
  }
  return QE_OK;
}


QhullError QhullLibrary::CreateTriangleMesh(QhullResult &answer,ConvexHullTriangleInterface *iface)
{
  QhullError ret = QE_FAIL;


  const float *p            = answer.mOutputVertices;
  const unsigned int   *idx = answer.mIndices;
  unsigned int fcount       = answer.mNumFaces;

  if ( p && idx && fcount )
  {
    ret = QE_OK;

    for (unsigned int i=0; i<fcount; i++)
    {
      unsigned int pcount = *idx++;

      unsigned int i1 = *idx++;
      unsigned int i2 = *idx++;
      unsigned int i3 = *idx++;

      const float *p1 = &p[i1*3];
      const float *p2 = &p[i2*3];
      const float *p3 = &p[i3*3];

      AddConvexTriangle(iface,p1,p2,p3);

      pcount-=3;
      while ( pcount )
      {
        i3 = *idx++;
        p2 = p3;
        p3 = &p[i3*3];

        AddConvexTriangle(iface,p1,p2,p3);
        pcount--;
      }

    }
  }

  return ret;
}

void QhullLibrary::AddConvexTriangle(ConvexHullTriangleInterface *callback,const float *p1,const float *p2,const float *p3)
{
  ConvexHullVertex v1,v2,v3;


  #define TSCALE1 (1.0f/4.0f)

  v1.mPos[0] = p1[0];
  v1.mPos[1] = p1[1];
  v1.mPos[2] = p1[2];

  v2.mPos[0] = p2[0];
  v2.mPos[1] = p2[1];
  v2.mPos[2] = p2[2];

  v3.mPos[0] = p3[0];
  v3.mPos[1] = p3[1];
  v3.mPos[2] = p3[2];

  float n[3];
  ComputeNormal(n,p1,p2,p3);

  v1.mNormal[0] = n[0];
  v1.mNormal[1] = n[1];
  v1.mNormal[2] = n[2];

  v2.mNormal[0] = n[0];
  v2.mNormal[1] = n[1];
  v2.mNormal[2] = n[2];

  v3.mNormal[0] = n[0];
  v3.mNormal[1] = n[1];
  v3.mNormal[2] = n[2];

  const float *tp1 = p1;
  const float *tp2 = p2;
  const float *tp3 = p3;

  int i1 = 0;
  int i2 = 0;

  float nx = fabsf(n[0]);
  float ny = fabsf(n[1]);
  float nz = fabsf(n[2]);

  if ( nx <= ny && nx <= nz ) i1 = 0;
  if ( ny <= nx && ny <= nz ) i1 = 1;
  if ( nz <= nx && nz <= ny ) i1 = 2;

  switch ( i1 )
  {
    case 0:
      if ( ny < nz )
        i2 = 1;
      else
        i2 = 2;
      break;
    case 1:
      if ( nx < nz )
        i2 = 0;
      else
        i2 = 2;
      break;
    case 2:
      if ( nx < ny )
        i2 = 0;
      else
        i2 = 1;
      break;
  }

  v1.mTexel[0] = tp1[i1]*TSCALE1;
  v1.mTexel[1] = tp1[i2]*TSCALE1;

  v2.mTexel[0] = tp2[i1]*TSCALE1;
  v2.mTexel[1] = tp2[i2]*TSCALE1;

  v3.mTexel[0] = tp3[i1]*TSCALE1;
  v3.mTexel[1] = tp3[i2]*TSCALE1;



  callback->ConvexHullTriangle(v3,v2,v1);

}


float QhullLibrary::ComputeNormal(float *n,const float *A,const float *B,const float *C)
{

  float vx,vy,vz,wx,wy,wz,vw_x,vw_y,vw_z,mag;

  vx = (B[0] - C[0]);
  vy = (B[1] - C[1]);
  vz = (B[2] - C[2]);

  wx = (A[0] - B[0]);
  wy = (A[1] - B[1]);
  wz = (A[2] - B[2]);

  vw_x = vy * wz - vz * wy;
  vw_y = vz * wx - vx * wz;
  vw_z = vx * wy - vy * wx;

  mag = sqrtf((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

  if ( mag < 0.000001f )
  {
    mag = 0;
  }
  else
  {
    mag = 1.0f/mag;
  }

  n[0] = vw_x * mag;
  n[1] = vw_y * mag;
  n[2] = vw_z * mag;

  return mag;
}


