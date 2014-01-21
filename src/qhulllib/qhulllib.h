#ifndef QHULL_LIB_H

#define QHULL_LIB_H


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


struct QhullResult
{
  bool                    mPolygons;                  // true if indices represents polygons, false indices are triangles
  unsigned int            mNumOutputVertices;         // number of vertices in the output hull
  float                  *mOutputVertices;            // array of vertices, 3 floats each x,y,z
  unsigned int            mNumFaces;                  // the number of faces produced
  unsigned int            mNumTriangles;              // the number of triangles produced
  unsigned int            mNumIndices;                // the total number of indices
  unsigned int           *mIndices;                   // pointer to indices.
// If triangles, then indices are array indexes into the vertex list.
// If polygons, indices are in the form (number of points in face) (p1, p2, p3, ..) etc..
};

class QhullUserAllocator // optional user provided memory allocator.
{
public:
  virtual ~QhullUserAllocator();
  virtual void * QhullAlloc(unsigned int size) = 0;
  virtual void   QhullFree(void *mem)          = 0;
};

enum QhullFlag
{
  QF_REMOVE_DUPLICATES = (1<<0),             // remove duplicate vertices from the input stream. always recommended!
  QF_NORMALIZE         = (1<<1),             // normalize the input vertices before removing duplicates. always recommended!
  QF_MESH_OPTIMIZE     = (1<<3),             // optimize the output mesh, almost always useful if meshoptimize interface is available.
  QF_TRIANGLES         = (1<<4),             // report results as triangles, not polygons.
  QF_REVERSE_ORDER     = (1<<5),             // reverse order of the triangle indices.
  QF_SAVE_OBJ_OK       = (1<<6),             // If it succeeds, save an OBJ file with the mesh.
  QF_SAVE_OBJ_FAIL     = (1<<7),             // If it fails, save an OBJ with the vertices we objected to.
  QF_DEFAULT           = (QF_REMOVE_DUPLICATES | QF_NORMALIZE | QF_MESH_OPTIMIZE )
};


class QhullDesc
{
public:
  QhullDesc(void)
  {
    mFlags          = QF_DEFAULT;
    mVcount         = 0;
    mVertices       = 0;
    mVertexStride   = 0;
    mNormalEpsilon  = 0.00001f;
    mMeshTolerance  = 0.8f;
  };

  QhullDesc(QhullFlag flag,
             unsigned int vcount,
             const float *vertices,
             unsigned int stride)
  {
    mFlags          = flag;
    mVcount         = vcount;
    mVertices       = vertices;
    mVertexStride   = stride;
    mNormalEpsilon  = 0.00001f;
    mMeshTolerance  = 0.8f;
  }

  bool HasQhullFlag(QhullFlag flag) const
  {
    if ( mFlags & flag ) return true;
    return false;
  }

  void SetQhullFlag(QhullFlag flag)
  {
    mFlags|=flag;
  }

  void ClearQhullFlag(QhullFlag flag)
  {
    mFlags&=~flag;
  }

  unsigned int      mFlags;           // flags to use when generating the convex hull.
  unsigned int      mVcount;          // number of vertices in the input point cloud
  const float      *mVertices;        // the array of vertices.
  unsigned int      mVertexStride;    // the stride of each vertex, in bytes.
  float             mNormalEpsilon;   // the epsilon for removing duplicates.  This is a normalized value, if normalized bit is on.
  float             mMeshTolerance;   // fit tolerance level for mesh optimization.
};

enum QhullError
{
  QE_OK,            // success!
  QE_FAIL           // failed.
};

// This class is used when converting a convex hull into a triangle mesh.
class ConvexHullVertex
{
public:
  float         mPos[3];
  float         mNormal[3];
  float         mTexel[2];
};

// A virtual interface to receive the triangles from the convex hull.
class ConvexHullTriangleInterface
{
public:
  virtual void ConvexHullTriangle(const ConvexHullVertex &v1,const ConvexHullVertex &v2,const ConvexHullVertex &v3) = 0;
  virtual ~ConvexHullTriangleInterface();
};


class QhullLibrary
{
public:
  QhullLibrary(QhullUserAllocator *user)
  {
    mAllocator = user;
  }

  QhullError CreateConvexHull(const QhullDesc       &desc,           // describes the input request
                              QhullResult           &result);        // contains the resulst

  QhullError ReleaseResult(QhullResult &result); // release memory allocated for this result, we are done with it.

  // Utility function to convert the output convex hull as a renderable set of triangles. Unfolds the polygons into
  // individual triangles, compute the vertex normals, and projects some texture co-ordinates.
  QhullError CreateTriangleMesh(QhullResult &answer,ConvexHullTriangleInterface *iface);

private:
  void    AddConvexTriangle(ConvexHullTriangleInterface *callback,const float *p1,const float *p2,const float *p3);
  float   ComputeNormal(float *n,const float *A,const float *B,const float *C);

  QhullUserAllocator    *mAllocator;
};


#endif
