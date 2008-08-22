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
#ifndef  _CDS_UTIL_H
#define  _CDS_UTIL_H

#include <Inventor/SoPath.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoSFImage.h>
#include <Inventor/SbBox.h>
#include <Inventor/VRMLnodes/SoVRMLIndexedFaceSet.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoIndexedTriangleStripSet.h>
#include <string>

#if defined(QT_GUI_LIB)
#include <QImage>
#endif

///Cuenta el numero de facetas de un VRMLIndexedFaceSet
int count_facets (const SoVRMLIndexedFaceSet *node);

///Cuenta el numero de facetas de un IndexedFaceSet
int count_facets (const SoIndexedFaceSet *node);

///Elimina una faceta de un SoIndexedFaceSet 
int ifs_remove_facet(const SoPath *path, int num_facet);

///Cambia el sentido de una faceta de un SoIndexedFaceSet 
int ifs_flip_facet(const SoPath *path, int num_facet);

///Intentar parchear un hueco de un solo triangulo de un SoIndexedFaceSet, a partir de dos facetas que lo rodean
int ifs_patch_hole2(const SoPath *path, int f1, int f2);

///Intentar parchear un hueco de un solo triangulo de un SoIndexedFaceSet, a partir de las tres facetas que lo rodean
int ifs_patch_hole(const SoPath *path, int f1, int f2, int f3);

///Calcula las normales por faceta y vertice de un SoIndexedFaceSet 
void ifs_normals(const SoPath *path, SoMFVec3f &normals_face, SoMFVec3f &normals_vertex);

///Calcula el recubrimiento con radio r de una malla de triangulos
SoSeparator *ifs_recubrimiento(const SoPath *path, const float r);

///Calcula el recubrimiento con radio r de una malla de triangulos
SoSeparator *ifs_recubrimiento2(const SoPath *path, const float r);

//! Volcado de un VRMLIndexedFaceSet a fichero SMF
int VRMLIndexedFaceSet_to_SMF (SoVRMLIndexedFaceSet *node, FILE *out);

//! Volcado de un VRMLIndexedFaceSet a fichero OFF
int VRMLIndexedFaceSet_to_OFF (SoVRMLIndexedFaceSet *node, FILE *out);

//! Volcado de un VRMLIndexedFaceSet a fichero STL (StereoLitography)
int VRMLIndexedFaceSet_to_STL (SoVRMLIndexedFaceSet *ifs, FILE *out, bool with_normals=false);

//! Volcado de un IndexedFaceSet a fichero SMF
int IndexedFaceSet_to_SMF (SoPath *path, FILE *out, bool with_normals=false);

//! Volcado de un IndexedFaceSet a std::string SMF
int IndexedFaceSet_to_SMF (SoPath *path, std::string &out, bool with_normals=false);

//! Volcado de un IndexedFaceSet a fichero OFF
int IndexedFaceSet_to_OFF (SoPath *path, FILE *out);

//! Volcado de un SoIndexedFaceSet a fichero STL (StereoLitography)
int IndexedFaceSet_to_STL (SoPath *path, FILE *out, bool with_normals=false);

//! Volcado de un Coordinate3 a fichero XYZ
int SoMFVec3f_to_XYZ (const SoMFVec3f &coords, FILE *out);

//! Importacion de un complejo simplicial en formato .neutral (netgen)
SoSeparator *import_netgen_neutral(FILE *in);

//! Importacion de un complejo simplicial en formato .node .faces (tetgen)
SoSeparator *import_tetgen (const char *nodeFilename);

//! Busqueda del ultimo nodo SoCoordinate3 de un path 
SoNode *buscaCoordenadas (const SoPath *path, SoMFVec3f &coords);

//! Calcula la mímima BBox no orientada con los ejes
SbXfBox3f measure_XfBBox(SoPath *path);

//! Calcula el centro de masas de un vector de puntos
SbVec3f centroid (const SoMFVec3f &coords);

//! Modifica el origen de un vector de puntos 
void center_new (SoMFVec3f &coords, const SbVec3f center);

//! Triangulado de un IndexedFaceSet. Devuelve el numero de triangulos
int IndexedFaceSet_triangulate (SoMFInt32 &coordIndex);

//! Cambia la orientación de todas las facetas de un IndexedFaceSet
void IndexedFaceSet_change_orientation (SoMFInt32 &coordIndex);

//! Conversion de un IndexedTriangleStripSet a IndexedFaceSet
SoIndexedFaceSet *IndexedTriangleStripSet_to_IndexedFaceSet (const SoIndexedTriangleStripSet * itss);

//! Calcula el cierre convexo de una nube de puntos. Devuelve numero de facetas en cierre
int convex_hull (const SoMFVec3f &coords, SoMFVec3f &ch_coords, SoMFInt32 &ch_coordIndex);

//!Exporta un subarbol de escena a un buffer de memoria
char *cds_export_string (SoNode *node);

//!Exporta un subarbol de escena a una clase de c++ en un fichero .h
bool cds_export_hppFile (SoNode *node, const char *className, const char *filename);

//!Crea una aplicacion para visualizar una clase creada con cds_export_hppFile
bool cds_export_cppFile (const char *className, const char *filename);

//! Busca el ultimo nodo de un tipo dado en un path
SoNode *buscaUltimoNodo(SoPath *p, SoType t);

//! Remove all nodes of a given type
void strip_node(SoType type, SoNode * root);

#ifdef QIMAGE_H
  ///Convert a SoSFImage object into a QImage
  QImage * SoSFImage_to_QImage(const SoSFImage *sfimage);
#endif

#endif

