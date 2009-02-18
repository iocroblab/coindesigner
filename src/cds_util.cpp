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

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include "cds_util.h"
#include "qhulllib.h"
#include <Inventor/nodes/SoNodes.h>
#include <Inventor/actions/SoActions.h>
#include <Inventor/VRMLnodes/SoVRMLNodes.h>
#include <simage.h>

#include <map>
#include <iostream>
#include <fstream>
#include <string>

#if defined(QT_GUI_LIB)
#include <qprogressdialog.h>
#include <QMessageBox>
#endif

///Cuenta el numero de facetas de un VRMLIndexedFaceSet
int count_facets (const SoVRMLIndexedFaceSet *node)
{
		int numFac = 0;
		for (int i=0; i<node->coordIndex.getNum(); i++)
		{
			while (node->coordIndex[i] > -1) 
				i++;
			numFac++;
		}

		return numFac;
}

///Cuenta el numero de facetas de un IndexedFaceSet
int count_facets (const SoIndexedFaceSet *node)
{
		int numFac = 0;
		for (int i=0; i<node->coordIndex.getNum(); i++)
		{
			while (node->coordIndex[i] > -1) 
				i++;
			numFac++;
		}

		return numFac;
} //int count_facets (const SoIndexedFaceSet *node)


///Elimina una faceta de un SoIndexedFaceSet 
int ifs_remove_facet(const SoPath *path, int num_facet)
{
   //Extraemos el IndexedFaceSet de la cola del path
   SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)path->getTail();

   //Comprobamos que apunta a un indexedFaceset
   if (!ifs->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId() ) )
	   return -1;

   //Buscamos la faceta numero facetIndex dentro del IndexedFaceSet
   int i;
   int fac_ini=-1;
   int numIdx = ifs->coordIndex.getNum();
   int face = 0;
   for (i=0; i<numIdx; i++)
   {
	   //Si esta es la faceta, salimos del bucle
	   if (face == num_facet)
	   {
		   fac_ini = i;
		   break;
	   }

	   //Saltamos toda la faceta, saltando hasta encontrar un -1
       while (ifs->coordIndex[i] > -1)
	       i++;

	   //Incrementamos el contador de faceta actual
	   face++;
   }

   //fac_ini contiene el indice de comienzo de la faceta
   if (fac_ini < 0)
	   return -1;

   //Contamos el numero de vertices de esta faceta
   int fac_len = 0;
   while (ifs->coordIndex[i] > -1)
   {
      i++;
	  fac_len++;
   }

   //Borra fac_len+1 elementos a partir de la posicion fac_ini
   ifs->coordIndex.deleteValues(fac_ini, fac_len+1);

   //Devuelve la longitud de la faceta eliminada
   return fac_len;

}// int ifs_remove_facet(SoPath *path, int facetIndex)


///Cambia el sentido de una faceta de un SoIndexedFaceSet 
int ifs_flip_facet(const SoPath *path, int num_facet)
{
   //Extraemos el IndexedFaceSet de la cola del path
   SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)path->getTail();

   //Comprobamos que apunta a un indexedFaceset
   if (!ifs->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId() ) )
	   return -1;

   //Buscamos la faceta numero facetIndex dentro del IndexedFaceSet
   int i;
   int fac_ini=-1;
   int numIdx = ifs->coordIndex.getNum();
   int face = 0;
   for (i=0; i<numIdx; i++)
   {
	   //Si esta es la faceta, salimos del bucle
	   if (face == num_facet)
	   {
		   fac_ini = i;
		   break;
	   }

	   //Saltamos toda la faceta, saltando hasta encontrar un -1
       while (ifs->coordIndex[i] > -1)
	       i++;

	   //Incrementamos el contador de faceta actual
	   face++;
   }

   //fac_ini contiene el indice de comienzo de la faceta
   if (fac_ini < 0)
	   return -1;

   //Contamos el numero de vertices de esta faceta
   int fac_len = 0;
   while ( (ifs->coordIndex[i] > -1) && (i < numIdx) )
   {
      i++;
	  fac_len++;
   }

   //Cambiamos el sentido de la faceta, intercambiando el orden de los indices
   for (i=0; i<fac_len/2; i++)
   {
	   int tmp = ifs->coordIndex[fac_ini+i];
	   ifs->coordIndex.set1Value(fac_ini+i, ifs->coordIndex[fac_ini+fac_len-i-1]);
	   ifs->coordIndex.set1Value(fac_ini+fac_len-i-1, tmp);
   }

   //Devuelve la longitud de la faceta
   return fac_len;

}// int ifs_flip_facet(SoPath *path, int facetIndex)


///Intentar parchear un hueco de un solo triangulo de un SoIndexedFaceSet, a partir de las dos facetas que lo rodean
int ifs_patch_hole2(const SoPath *path, int f1, int f2)
{
   //Extraemos el IndexedFaceSet de la cola del path
   SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)path->getTail();

   //Comprobamos que apunta a un indexedFaceset
   if (!ifs->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId() ) )
	   return -1;

   //Comprobamos que las facetas son diferentes
   if (f1==f2)
	   return -1;

   //Buscamos las dos facetas dentro del IndexedFaceSet
   int i,j;
   int idx1=0, idx2=0;
   int numIdx = ifs->coordIndex.getNum();
   int face = 0;
   for (i=0; i<numIdx; i++)
   {
	   //Buscamos la faceta f1
	   if (face == f1)
		   idx1 = i;

	   //Buscamos la faceta f2
	   if (face == f2)
		   idx2 = i;

	   //Saltamos toda la faceta, saltando hasta encontrar un -1
       while (ifs->coordIndex[i] > -1 && (i<numIdx))
	       i++;

	   //Incrementamos el contador de faceta actual
	   face++;
   }

   //Buscamos un vertice comun entre la faceta f1 y f2
   int v1=-1;
   int v2=0, v3=0;
   for (i=idx1; ifs->coordIndex[i] > -1 && (v1<0); i++)
   {
	   for (j=idx2; ifs->coordIndex[j]>-1 ; j++)
		   if (ifs->coordIndex[i] == ifs->coordIndex[j])
		   {
			   //Salvamos el numero de vertice y las coordenadas
			   v1 = ifs->coordIndex[i];

			   //escojemos el vertice posterior a v1 sobre la faceta f1
			   v2 = ifs->coordIndex[i+1];

			   //Con cuidado de si v1 es el ultimo vertice de la faceta
			   if (v2<0)
				   v2 = ifs->coordIndex[idx1];

			   //escojemos el vertice anterior a v1 sobre la faceta f2
			   //con cuidado si v1 es el primer vertice de la faceta f2
			   if (j==idx2)
			   {
				   //Avanzamos hasta el final de la faceta f2
				   while(ifs->coordIndex[j] >=0)
					   j++;
			   }

			   //escojemos el vertice anterior a v1 sobre la faceta f2
			   v3 = ifs->coordIndex[j-1];

			   break;
		   }
   }

   //Comprobamos que se ha encontrado el vertice en comun
   if (v1 < 0 )
	   return -1;

   //Añade una nueva faceta al final del indexedFaceSet
   ifs->coordIndex.set1Value(numIdx++, v1);
   ifs->coordIndex.set1Value(numIdx++, v2);
   ifs->coordIndex.set1Value(numIdx++, v3);
   ifs->coordIndex.set1Value(numIdx++, -1);

   //Devuelve el indice de la nueva faceta
   return numIdx-4;

}// int ifs_patch_hole


///Intentar parchear un hueco de un solo triangulo de un SoIndexedFaceSet, a partir de las tres facetas que lo rodean
int ifs_patch_hole(const SoPath *path, int f1, int f2, int f3)
{
   //Extraemos el IndexedFaceSet de la cola del path
   SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)path->getTail();

   //Comprobamos que apunta a un indexedFaceset
   if (!ifs->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId() ) )
	   return -1;

   //Comprobamos si hay facetas repetidas, e aplicamos el algoritmo para 2 facetas
   if (f1==f2)
	   return ifs_patch_hole2(path, f1, f3);
   else if (f1==f3 || f2==f3)
	   return ifs_patch_hole2(path, f1, f2);

   //Buscamos las facetas dentro del IndexedFaceSet
   int i,j;
   int idx1=0, idx2=0, idx3=0;
   int numIdx = ifs->coordIndex.getNum();
   int face = 0;
   for (i=0; (i<numIdx) && !(idx1 && idx2 && idx3); i++)
   {
	   //Buscamos la faceta f1
	   if (face == f1)
		   idx1 = i;

	   //Buscamos la faceta f2
	   if (face == f2)
		   idx2 = i;

	   //Buscamos la faceta f3
	   if (face == f3)
		   idx3 = i;

	   //Saltamos toda la faceta, saltando hasta encontrar un -1
       while (ifs->coordIndex[i] > -1 && (i<numIdx))
	       i++;

	   //Incrementamos el contador de faceta actual
	   face++;
   }

   //Buscamos un vertice comun entre la faceta f1 y f2
   int v1_2=-1;
   int idx1_2=-1;
   for (i=idx1; ifs->coordIndex[i] > -1; i++)
	   for (j=idx2; ifs->coordIndex[j] > -1; j++)
		   if (ifs->coordIndex[i] == ifs->coordIndex[j])
		   {
			   v1_2 = ifs->coordIndex[i];
			   idx1_2 = i;
			   break;
		   }

   //Buscamos un vertice comun entre la faceta f2 y f3
   int v2_3=-1;
   for (i=idx2; ifs->coordIndex[i] > -1; i++)
	   for (j=idx3; ifs->coordIndex[j] > -1; j++)
		   if (ifs->coordIndex[i] == ifs->coordIndex[j])
		   {
			   v2_3 = ifs->coordIndex[i];
			   break;
		   }

   //Buscamos un vertice comun entre la faceta f3 y f1
   int v3_1=-1;
   int idx3_1=-1;
   for (i=idx3; ifs->coordIndex[i] > -1; i++)
	   for (j=idx1; ifs->coordIndex[j] > -1; j++)
		   if (ifs->coordIndex[i] == ifs->coordIndex[j])
		   {
			   v3_1 = ifs->coordIndex[i];
			   idx3_1 = j;
			   break;
		   }


   //Comprobamos que hemos encontrado 3 vertices
   if (v1_2 < 0 || v2_3 < 0 || v3_1 < 0)
	   return -1;

   //Comprobamos el orden correcto de los vertice
   if (idx1_2 < idx3_1)
	   std::swap(v1_2, v3_1);

   //Añade una nueva faceta al final del indexedFaceSet
   i = ifs->coordIndex.getNum();
   ifs->coordIndex.set1Value(i++, v1_2);
   ifs->coordIndex.set1Value(i++, v2_3);
   ifs->coordIndex.set1Value(i++, v3_1);
   ifs->coordIndex.set1Value(i++, -1);

   //Devuelve el indice de la nueva faceta
   return i-4;

}// int ifs_patch_hole




///Calcula las normales por faceta y vertice de un SoIndexedFaceSet 
void ifs_normals(const SoPath *path, SoMFVec3f &normals_face, SoMFVec3f &normals_vertex)
{
   int i;

   //Extraemos el IndexedFaceSet de la cola del path
   SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)path->getTail();

   //Buscamos el ultimo nodo de coordenadas que afecte a este nodo
   SoMFVec3f coords;
   SoNode *nodeCoord = buscaCoordenadas (path, coords);

   //Comprobamos que hemos encontrado ambos elementos
   if (!nodeCoord || !ifs->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId()) )
   {
	   normals_face.setNum(0);
	   normals_vertex.setNum(0);
	   return;
   }

   //Inicializamos los resultados
   int num_vertex = coords.getNum();
   int *cardinalidad = new int[num_vertex];
   normals_vertex.setNum(num_vertex);
   for (i=0; i< num_vertex; i++)
   {
	   cardinalidad[i]=0;
	   normals_vertex.set1Value(i, 0, 0, 0);
   }

   //Recorremos la lista de facetas calculando sus normales
   int numIdx = ifs->coordIndex.getNum();
   int face = 0;
   for (i=0; i<numIdx; i++)
   {
	   //Leemos 3 vertices de esta faceta
       SbVec3f t1(coords[ifs->coordIndex[i++]]);
       SbVec3f t2(coords[ifs->coordIndex[i++]]);
       SbVec3f t3(coords[ifs->coordIndex[i++]]);

	   //Calculamos el vector normal de la faceta
	   SbVec3f normal_f = (t2-t1).cross(t3-t1);
	   normal_f.normalize();
	   normals_face.set1Value(face, normal_f);

	   //Saltamos resto de la faceta, saltando hasta encontrar un -1
       while (ifs->coordIndex[i] > -1)
	       i++;

	   //Incrementamos el contador de faceta actual
	   face++;

   }//for (i=0; i<numIdx; i++)

   //Ahora volvemos a recorrer la lista de facetas, acumulando los vectores normales
   //de cada faceta sobre sus vertices
   face=0;
   for (i=0; i<numIdx; i++)
   {

	   //Leemos la normal de esta faceta
	   SbVec3f normal_f = normals_face[face];

	   //Acumulamos esta normal en todos los vertices de la faceta
       while (ifs->coordIndex[i] > -1)
	   {
		   int idx = ifs->coordIndex[i];

		   //Incrementamos la cardinalidad del vertice
		   cardinalidad[idx]++;

		   //Sumamos la normal de esta faceta a la del vertice
		   SbVec3f normal_v = normals_vertex[idx];
		   normal_v += normal_f;
		   normals_vertex.set1Value(idx, normal_v);

		   //Pasamos al siguiente indice
	       i++;
	   }

	   //saltamos a siguiente faceta
	   face++;

   }//for (i=0; i<numIdx; i++)

   //Ahora dividimos la normal acumulada por la cardinalidad de cada vertice
   for (i=0; i< num_vertex; i++)
   {
	   if (cardinalidad[i])
	   {
         SbVec3f normal_v = normals_vertex[i];
		 normal_v /= float(cardinalidad[i]);
		 normal_v.normalize();
	     normals_vertex.set1Value(i, normal_v);
	   }
   }

   //Limpiamos memoria
   delete cardinalidad;

}

///Calcula el recubrimiento con radio r de una malla de triangulos
SoSeparator *ifs_recubrimiento(const SoPath *path, const float r)
{
	int i;

    //Extraemos el IndexedFaceSet de la cola del path
    SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)path->getTail();

    //Buscamos el ultimo nodo de coordenadas que afecte a este nodo
    SoMFVec3f coords;
    SoNode *nodeCoord = buscaCoordenadas (path, coords);

    //Comprobamos que hemos encontrado ambos elementos
    if (!nodeCoord || !ifs->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId()) )
	   return NULL;

	//Contamos el número de vertices y de facetas
	int numVertex  = coords.getNum();
	int numIdx = ifs->coordIndex.getNum();
	int numFaces = numIdx / 4 ;

	//Creamos una malla para el recubrimiento
	SoSeparator *rec_sep = new SoSeparator();
	SoCoordinate3 *rec_ver = new SoCoordinate3();
	SoIndexedFaceSet *rec_fac = new SoIndexedFaceSet();
	rec_sep->addChild(rec_ver);
	rec_sep->addChild(rec_fac);

    //Calculamos las normales por cara y vértice de la malla original
    SoMFVec3f normals_face;
	SoMFVec3f normals_vertex;
	ifs_normals(path, normals_face, normals_vertex);

	//Multiplicamos las normales de los vertices por la longitud del radio
	SbVec3f *norm = normals_vertex.startEditing();
	for (i=0; i < numVertex; i++)
	{
		(*norm) *= r;
		norm++;
	}
    normals_vertex.finishEditing();

	//Multiplicamos las normales de las facetas por la longitud del radio
	norm = normals_face.startEditing();
	for (i=0; i < numFaces; i++)
	{
		(*norm) *= r;
		norm++;
	}
    normals_face.finishEditing();

	//Hacemos una pasada sobre la lista de vertices, añadiendo un nuevo vertice 
	//desplazado según la normal del vertice. 
	for (i=0; i < numVertex; i++)
	{
		rec_ver->point.set1Value(i, coords[i]+normals_vertex[i]);
	}


	//Recorremos la malla realizando la siguientes labores:
	// 1. Creando informacion sobre las vecindades, como un map<semiarista, faceta>.
	// 2. Creando los vertices e indices de una faceta paralela a la actual .
	std::map<int, int>mapAristas;
    #define arista(a,b) (a*numVertex + b)
    for (int face=0; face<numFaces; face++)
    {
	   int vert0 = numVertex+3*face;
	   int idx0  = 4*face;

	   //Leemos los indices de los 3 vertices de esta faceta
       int v1 = ifs->coordIndex[idx0];
       int v2 = ifs->coordIndex[idx0+1];
       int v3 = ifs->coordIndex[idx0+2];

	   //Comprobamos que es un triangulo válido
	   assert(v1>=0 && v2 >= 0 && v3 >= 0);

	   //Almacenamos información de vecindad, dando a cada semiarista un numero
	   //y almacenando en un map la faceta que contiene la semiarista.
	   mapAristas[arista(v1,v2)] = face;
	   mapAristas[arista(v2,v3)] = face;
	   mapAristas[arista(v3,v1)] = face;

	   //Creamos una faceta paralela a la actual, sumando a los vertices de esta
	   //faceta, el vector normal de esta faceta
	   rec_ver->point.set1Value(vert0,   coords[v1]+normals_face[face]);
	   rec_ver->point.set1Value(vert0+1, coords[v2]+normals_face[face]);
	   rec_ver->point.set1Value(vert0+2, coords[v3]+normals_face[face]);
	   rec_fac->coordIndex.set1Value(idx0  ,vert0);
	   rec_fac->coordIndex.set1Value(idx0+1,vert0+1);
	   rec_fac->coordIndex.set1Value(idx0+2,vert0+2);
	   rec_fac->coordIndex.set1Value(idx0+3,-1);

    }//for (int face=0; face<numFaces; face++)

	//Hacemos una segunda pasada a las facetas, añadiendo un triangulo por cada semiarista
	//para rellenar el hueco entre facetas del recubrimiento
	i=rec_fac->coordIndex.getNum();
    for (int face=0; face<numFaces; face++)
    {
	   int vert0 = numVertex+3*face;
	   int idx0  = 4*face;

	   //Leemos los indices de los 3 vertices de esta faceta
       int v1 = ifs->coordIndex[idx0];
       int v2 = ifs->coordIndex[idx0+1];
       int v3 = ifs->coordIndex[idx0+2];

	   //Para cada semiarista, creamos un triangulo que contiene la semiarista 
	   //y un vertice del triangulo que contiene a la semiarista opuesta
	   int vecino, vecino_vert0, vecino_idx0, vecino_v;

	   //Semiarista v1,v2

	   //Identificamos al vecino
	   vecino = mapAristas[arista(v2,v1)];
	   vecino_vert0 = numVertex+3*vecino;
	   vecino_idx0  = 4*vecino;

	   //Buscamos el vertice v1 en el vecino  
	   if (ifs->coordIndex[vecino_idx0] == v1)
	   {
		   vecino_v = vecino_vert0;
	   }
	   else if (ifs->coordIndex[vecino_idx0+1] == v1)
	   {
		   vecino_v = vecino_vert0+1;
	   }
	   else if (ifs->coordIndex[vecino_idx0+2] == v1)
	   {
		   vecino_v = vecino_vert0+2;
	   }
	   else
	   {
		   fprintf(stderr, "No encuentro vertice %d en faceta %d\n", v1, vecino);
		   vecino_v = -1;
	   }

	   //Insertamos dos triangulos
	   if (vecino_v >=0)
	   {
		   //Triangulo para unir la semiarista con triangulo vecino
           rec_fac->coordIndex.set1Value(i++ ,vert0);
           rec_fac->coordIndex.set1Value(i++ ,vecino_v);
           rec_fac->coordIndex.set1Value(i++ ,vert0+1);
           rec_fac->coordIndex.set1Value(i++ ,-1);

		   //Triangulo para tapar vertice v1
           rec_fac->coordIndex.set1Value(i++ ,vecino_v);
           rec_fac->coordIndex.set1Value(i++ ,vert0);
           rec_fac->coordIndex.set1Value(i++ ,v1);
           rec_fac->coordIndex.set1Value(i++ ,-1);
	   }


	   //Semiarista v2,v3

	   //Identificamos al vecino
	   vecino = mapAristas[arista(v3,v2)];
	   vecino_vert0 = numVertex+3*vecino;
	   vecino_idx0  = 4*vecino;

	   //Buscamos el vertice v2 en el vecino  
	   if (ifs->coordIndex[vecino_idx0] == v2)
	   {
		   vecino_v = vecino_vert0;
	   }
	   else if (ifs->coordIndex[vecino_idx0+1] == v2)
	   {
		   vecino_v = vecino_vert0+1;
	   }
	   else if (ifs->coordIndex[vecino_idx0+2] == v2)
	   {
		   vecino_v = vecino_vert0+2;
	   }
	   else
	   {
		   fprintf(stderr, "No encuentro vertice %d en faceta %d\n", v2, vecino);
		   vecino_v = -1;
	   }

	   //Insertamos dos nuevos triangulos
	   if (vecino_v >=0)
	   {
		   //Triangulo para unir la semiarista con triangulo vecino
           rec_fac->coordIndex.set1Value(i++ ,vert0+1);
           rec_fac->coordIndex.set1Value(i++ ,vecino_v);
           rec_fac->coordIndex.set1Value(i++,vert0+2);
           rec_fac->coordIndex.set1Value(i++,-1);

		   //Triangulo para tapar vertice v2
           rec_fac->coordIndex.set1Value(i++ ,vecino_v);
           rec_fac->coordIndex.set1Value(i++ ,vert0+1);
           rec_fac->coordIndex.set1Value(i++ ,v2);
           rec_fac->coordIndex.set1Value(i++ ,-1);
	   }

	   //Semiarista v3,v1

	   //Identificamos al vecino
	   vecino = mapAristas[arista(v1,v3)];
	   vecino_vert0 = numVertex+3*vecino;
	   vecino_idx0  = 4*vecino;

	   //Buscamos el vertice v3 en el vecino  
	   if (ifs->coordIndex[vecino_idx0] == v3)
	   {
		   vecino_v = vecino_vert0;
	   }
	   else if (ifs->coordIndex[vecino_idx0+1] == v3)
	   {
		   vecino_v = vecino_vert0+1;
	   }
	   else if (ifs->coordIndex[vecino_idx0+2] == v3)
	   {
		   vecino_v = vecino_vert0+2;
	   }
	   else
	   {
		   fprintf(stderr, "No encuentro vertice %d en faceta %d\n", v3, vecino);
		   vecino_v = -1;
	   }

	   //Insertamos dos nuevos triangulos
	   if (vecino_v >=0)
	   {
		   //Triangulo para unir la semiarista con triangulo vecino
           rec_fac->coordIndex.set1Value(i++ ,vert0+2);
           rec_fac->coordIndex.set1Value(i++ ,vecino_v);
           rec_fac->coordIndex.set1Value(i++ ,vert0);
           rec_fac->coordIndex.set1Value(i++ ,-1);

		   //Triangulo para tapar vertice v3
           rec_fac->coordIndex.set1Value(i++ ,vecino_v);
           rec_fac->coordIndex.set1Value(i++ ,vert0+2);
           rec_fac->coordIndex.set1Value(i++ ,v3);
           rec_fac->coordIndex.set1Value(i++ ,-1);
	   }

    }//for (int face=0; face<numFaces; face++)


	//Devolvemos el separador que contiene al recubrimiento
	return rec_sep;
    #undef arista
}


///Calcula el recubrimiento con radio r de una malla de triangulos
SoSeparator *ifs_recubrimiento2(const SoPath *path, const float r)
{
	int i;

    //Extraemos el IndexedFaceSet de la cola del path
    SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)path->getTail();

    //Buscamos el ultimo nodo de coordenadas que afecte a este nodo
    SoMFVec3f coords;
    SoNode *nodeCoord = buscaCoordenadas (path, coords);

    //Comprobamos que hemos encontrado ambos elementos
    if (!nodeCoord || !ifs->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId()) )
	   return NULL;

	//Contamos el número de vertices y de facetas
	int numVertex  = coords.getNum();
	//int numIdx = ifs->coordIndex.getNum();
	//int numFaces = numIdx / 4 ;

	//Creamos una malla para el recubrimiento
	SoSeparator *rec_sep = new SoSeparator();
	SoCoordinate3 *rec_ver = new SoCoordinate3();
	SoIndexedFaceSet *rec_fac = new SoIndexedFaceSet();

	rec_sep->addChild(rec_ver);
	rec_sep->addChild(rec_fac);

    //Calculamos las normales por cara y vértice de la malla original
    SoMFVec3f normals_face;
	SoMFVec3f normals_vertex;
	ifs_normals(path, normals_face, normals_vertex);

	//Multiplicamos las normales de los vertices por la longitud del radio
	SbVec3f *norm = normals_vertex.startEditing();
	for (i=0; i < numVertex; i++)
	{
		(*norm) *= r;
		norm++;
	}
    normals_vertex.finishEditing();

	//Hacemos una pasada sobre la lista de vertices, añadiendo un nuevo vertice 
	//desplazado según la normal del vertice. 
	for (i=0; i < numVertex; i++)
	{
		rec_ver->point.set1Value(i, coords[i]+normals_vertex[i]);
	}

	//Copiamos los campos del indexeFaceset original
    rec_fac->coordIndex.copyFrom(ifs->coordIndex);
    rec_fac->materialIndex.copyFrom(ifs->materialIndex);
    rec_fac->normalIndex.copyFrom(ifs->normalIndex);
    rec_fac->textureCoordIndex.copyFrom(ifs->textureCoordIndex);

	//Devolvemos el separador que contiene al recubrimiento
	return rec_sep;
}


//Volcado de un VRMLIndexedFaceSet a fichero SMF
int VRMLIndexedFaceSet_to_SMF (SoVRMLIndexedFaceSet *node, FILE *out)
{
   int i;
   fprintf(out, "#SMF file created by Coindesigner.\n");

   //Volcado de los vertices
   SoVRMLCoordinate *coord = (SoVRMLCoordinate*)node->coord.getValue();
   int numVert = coord->point.getNum();
   for (i=0; i<numVert; i++)
   {
       float x, y, z;
       coord->point.getValues(i)->getValue(x,y,z);
       fprintf(out, "v %g %g %g\n", x, y, z);
   }

   //Volcado de las facetas
   int numIdx = node->coordIndex.getNum();
   for (i=0; i<numIdx; i++)
   {
       fprintf(out, "f");
       int idx = node->coordIndex[i];
       while (idx > -1)
       {
           fprintf(out, " %d", idx+1);
	   i++;
           idx = node->coordIndex[i];
       }
       fprintf(out, "\n");
   }

   return 0;
}// int VRMLIndexedFaceSet_to_SMF (SoVRMLIndexedFaceSet *node, FILE *out)


//Volcado de un VRMLIndexedFaceSet a fichero OFF
int VRMLIndexedFaceSet_to_OFF (SoVRMLIndexedFaceSet *node, FILE *out)
{
   int i;

   SoVRMLCoordinate *coord = (SoVRMLCoordinate*)node->coord.getValue();
   int numVert = coord->point.getNum();

   //Recuento de las facetas
   int numIdx = node->coordIndex.getNum();
   int numFac = 0;
   for (i=0; i<numIdx; i++)
   {
       while (node->coordIndex[i] > -1)
       {
	   i++;
       }
       numFac++;
   }

   //Escritura de la cabecera OFF
   fprintf(out, "OFF %d %d 0\n", numVert, numFac);

   //Volcado de los vertices
   for (i=0; i<numVert; i++)
   {
       float x, y, z;
       coord->point.getValues(i)->getValue(x,y,z);
       fprintf(out, "%g %g %g\n", x, y, z);
   }

   //Volcado de las facetas
   for (i=0; i<numIdx; i++)
   {
       //Contamos los vertices de esta faceta
       int j=i;
       while (node->coordIndex[j] > -1)
       {
	   j++;
       }

       //Escribimos el numero de vertices de la faceta
       fprintf(out, "%d", j-i);

       //Escribimos los indices de la faceta
       int idx = node->coordIndex[i];
       while (idx > -1)
       {
           fprintf(out, " %d", idx);
	   i++;
           idx = node->coordIndex[i];
       }
       fprintf(out, "\n");
   }

   return 0;

}//int VRMLIndexedFaceSet_to_OFF (SoVRMLIndexedFaceSet *node, FILE *out)

//Volcado de un SoVRMLIndexedFaceSet a fichero STL (StereoLitography)
int VRMLIndexedFaceSet_to_STL (SoVRMLIndexedFaceSet *ifs, FILE *out, bool with_normals)
{
   int i;

   //Buscamos el ultimo nodo de coordenadas que afecte a este nodo
   SoVRMLCoordinate *coord = (SoVRMLCoordinate*)ifs->coord.getValue();


   if (!coord)
	   return -1;

   //Escribimos el comienzo del objeto y el nombre si lo tiene
   if (ifs->getName() )
   {
      fprintf(out, "solid %s\n", ifs->getName().getString() );
   }
   else
   {
      fprintf(out, "solid\n");
   }

   //Volcado de las facetas
   int numIdx = ifs->coordIndex.getNum();
   for (i=0; i<numIdx; i++)
   {
       //principio de faceta
       if (with_normals)
       {
           //Extraemos 3 vertices de la faceta
           SbVec3f p0 = coord->point[ifs->coordIndex[i]];
           SbVec3f p1 = coord->point[ifs->coordIndex[i+1]];
           SbVec3f p2 = coord->point[ifs->coordIndex[i+2]];

           //Calculamos el plano de la faceta y su normal
           SbPlane plane(p0,p1,p2);
           SbVec3f normal = plane.getNormal();

          //Escribimos el principio de faceta con la normal
           fprintf(out, "  facet normal %g %g %g\n", normal[0], normal[1], normal[2] );
       }
       else
       {
           fprintf(out, "  facet\n");
       }



       //Principio de loop
       fprintf(out, "    outer loop\n");

       int idx = ifs->coordIndex[i];
       while (idx > -1)
       {
           float x, y, z;
           coord->point.getValues(idx)->getValue(x,y,z);
           fprintf(out, "      vertex %g %g %g\n", x, y, z);
           i++;
           idx = ifs->coordIndex[i];
       }

       //Fin de loop
       fprintf(out, "    endloop\n");

       //fin de faceta
       fprintf(out, "  endfacet\n");
   }

   //Escribimos el fin del objeto
   fprintf(out, "endsolid\n");

   return 0;

}// int VRMLIndexedFaceSet_to_STL (SoPath *path, FILE *out)


//Volcado de un SoIndexedFaceSet a fichero SMF
int IndexedFaceSet_to_SMF (SoPath *path, FILE *out, bool with_normals)
{
	int i;

   //Extraemos el IndexedFaceSet de la cola del path
   SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)path->getTail();

   //Buscamos el ultimo nodo de coordenadas que afecte a este nodo
   SoMFVec3f coords;
   SoNode *nodeCoord = buscaCoordenadas (path, coords);

   if (!nodeCoord)
	   return -1;

   //Volcado de los vertices
   int numVert = coords.getNum();
   for (i=0; i<numVert; i++)
   {
       float x, y, z;
       coords.getValues(i)->getValue(x,y,z);
       fprintf(out, "v %g %g %g\n", x, y, z);
   }

   //Volcado de las normales
   if (with_normals)
   {
		//Calculamos las normales por cara y vértice de la malla original
		SoMFVec3f normals_face;
		SoMFVec3f normals_vertex;
		ifs_normals(path, normals_face, normals_vertex);

		//Escribimos todas las normales por vertice
		for (i=0; i<numVert; i++)
		{
			float x, y, z;
			normals_vertex.getValues(i)->getValue(x,y,z);
			fprintf(out, "vn %g %g %g\n", x, y, z);
		}
   }

   //Volcado de las facetas
   int numIdx = ifs->coordIndex.getNum();
   for (i=0; i<numIdx; i++)
   {
       fprintf(out, "f");
       int idx = ifs->coordIndex[i];
       while (idx > -1)
	   {
		   if (with_normals)
			  fprintf(out, " %d//%d", idx+1,idx+1);
		   else
			  fprintf(out, " %d", idx+1);

		   i++;
		   idx = ifs->coordIndex[i];
       }
       fprintf(out, "\n");
   }

   return 0;

}// int IndexedFaceSet_to_SMF (SoPath *path, FILE *out)


//Volcado de un SoIndexedFaceSet a string SMF
int IndexedFaceSet_to_SMF (SoPath *path, std::string &out, bool with_normals)
{
	int i;
	char line[1024];

   //Extraemos el IndexedFaceSet de la cola del path
   SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)path->getTail();

   //Buscamos el ultimo nodo de coordenadas que afecte a este nodo
   SoMFVec3f coords;
   SoNode *nodeCoord = buscaCoordenadas (path, coords);

   if (!nodeCoord)
	   return -1;

   //Volcado de los vertices
   int numVert = coords.getNum();
   for (i=0; i<numVert; i++)
   {
       float x, y, z;
       coords.getValues(i)->getValue(x,y,z);
       sprintf(line, "v %g %g %g\n", x, y, z);
	   out.append(line);
   }

   //Volcado de las normales
   if (with_normals)
   {
		//Calculamos las normales por cara y vértice de la malla original
		SoMFVec3f normals_face;
		SoMFVec3f normals_vertex;
		ifs_normals(path, normals_face, normals_vertex);

		//Escribimos todas las normales por vertice
		for (i=0; i<numVert; i++)
		{
			float x, y, z;
			normals_vertex.getValues(i)->getValue(x,y,z);
			sprintf(line, "vn %g %g %g\n", x, y, z);
			out.append(line);
		}
   }

   //Volcado de las facetas
   int numIdx = ifs->coordIndex.getNum();
   for (i=0; i<numIdx; i++)
   {
	   out.append("f");
       int idx = ifs->coordIndex[i];
       while (idx > -1)
	   {
		   if (with_normals)
			  sprintf(line, " %d//%d", idx+1,idx+1);
		   else
			  sprintf(line, " %d", idx+1);

           out.append(line);
		   i++;
		   idx = ifs->coordIndex[i];
       }
	   out.append("\n");
   }

   return 0;

}// int IndexedFaceSet_to_SMF (SoPath *path, std::string &out, bool with_normals)


//Volcado de un IndexedFaceSet a fichero OFF
int IndexedFaceSet_to_OFF (SoPath *path, FILE *out)
{

	int i;

   //Extraemos el IndexedFaceSet de la cola del path
   SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)path->getTail();

   //Buscamos el ultimo nodo de coordenadas que afecte a este nodo
   SoMFVec3f coords;
   SoNode *nodeCoord = buscaCoordenadas (path, coords);

   if (!nodeCoord)
	   return -1;

   int numVert = coords.getNum();

   //Recuento de las facetas
   int numIdx = ifs->coordIndex.getNum();
   int numFac = 0;
   for (i=0; i<numIdx; i++)
   {
       while (ifs->coordIndex[i] > -1)
       {
	   i++;
       }
       numFac++;
   }

   //Escritura de la cabecera OFF
   fprintf(out, "OFF %d %d 0\n", numVert, numFac);

   //Volcado de los vertices
   for (i=0; i<numVert; i++)
   {
       float x, y, z;
       coords.getValues(i)->getValue(x,y,z);
       fprintf(out, "%g %g %g\n", x, y, z);
   }

   //Volcado de las facetas
   for (i=0; i<numIdx; i++)
   {
       //Contamos los vertices de esta faceta
       int j=i;
       while (ifs->coordIndex[j] > -1)
       {
	   j++;
       }

       //Escribimos el numero de vertices de la faceta
       fprintf(out, "%d", j-i);

       //Escribimos los indices de la faceta
       int idx = ifs->coordIndex[i];
       while (idx > -1)
       {
           fprintf(out, " %d", idx);
	   i++;
           idx = ifs->coordIndex[i];
       }
       fprintf(out, "\n");
   }

   return 0;

}// int IndexedFaceSet_to_OFF (SoPath *path, FILE *out)


//Volcado de un SoIndexedFaceSet a fichero STL (StereoLitography)
int IndexedFaceSet_to_STL (SoPath *path, FILE *out, bool with_normals)
{
	int i;

   //Extraemos el IndexedFaceSet de la cola del path
   SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)path->getTail();

   //Buscamos el ultimo nodo de coordenadas que afecte a este nodo
   SoMFVec3f coords;
   SoNode *nodeCoord = buscaCoordenadas (path, coords);

   if (!nodeCoord)
	   return -1;

   //Escribimos el comienzo del objeto y el nombre si lo tiene
   if (ifs->getName() )
   {
      fprintf(out, "solid %s\n", ifs->getName().getString() );
   }
   else
   {
      fprintf(out, "solid\n");
   }

   //Volcado de las facetas
   int numIdx = ifs->coordIndex.getNum();
   for (i=0; i<numIdx; i++)
   {
       //principio de faceta
       if (with_normals)
       {
           //Extraemos 3 vertices de la faceta
           SbVec3f p0 = coords[ifs->coordIndex[i]];
           SbVec3f p1 = coords[ifs->coordIndex[i+1]];
           SbVec3f p2 = coords[ifs->coordIndex[i+2]];

           //Calculamos el plano de la faceta y su normal
           SbPlane plane(p0,p1,p2);
           SbVec3f normal = plane.getNormal();

          //Escribimos el principio de faceta con la normal
           fprintf(out, "  facet normal %g %g %g\n", normal[0], normal[1], normal[2] );
       }
       else
       {
           fprintf(out, "  facet\n");
       }



       //Principio de loop
       fprintf(out, "    outer loop\n");

       int idx = ifs->coordIndex[i];
       while (idx > -1)
       {
           float x, y, z;
           coords.getValues(idx)->getValue(x,y,z);
           fprintf(out, "      vertex %g %g %g\n", x, y, z);
           i++;
           idx = ifs->coordIndex[i];
       }

       //Fin de loop
       fprintf(out, "    endloop\n");

       //fin de faceta
       fprintf(out, "  endfacet\n");
   }

   //Escribimos el fin del objeto
   fprintf(out, "endsolid\n");

   return 0;

}// int IndexedFaceSet_to_STL (SoPath *pth, FILE *out)


//! Volcado de un Coordinate3 a fichero XYZ
int SoMFVec3f_to_XYZ (const SoMFVec3f &coords, FILE *out)
{
   int i;
   int numVert = coords.getNum();

   //Volcado de los vertices
   for (i=0; i<numVert; i++)
   {
       float x, y, z;
       coords.getValues(i)->getValue(x,y,z);
       fprintf(out, "%g %g %g\n", x, y, z);
   }

   return 0;

}// int SoMFVec3f_to_XYZ (const SoMFVec3f &coords, FILE *out)



/*! Busqueda del ultimo nodo SoCoordinate3 de un path */
SoNode *buscaCoordenadas (const SoPath *path, SoMFVec3f &coords)
{
  //Extraemos el ultimo nodo del path
  SoNode *node = path->getTail();

  //Comprobamos si se ha indicado un campo vertexProperty en el nodo
  if (node->getTypeId().isDerivedFrom(SoVertexShape::getClassTypeId())) 
  {
     SoVertexProperty *vp = (SoVertexProperty *) 
                       ((SoVertexShape *)node)->vertexProperty.getValue();
     if (vp != NULL)
     {
       coords = vp->vertex;
       return vp;
     }
  }
  else
  //Comprobamos si es un nodo derivado SoVRMLVertexShape
  if (node->getTypeId().isDerivedFrom(SoVRMLVertexShape::getClassTypeId())) 
  {
      SoNode *vc = ((SoVRMLVertexShape*)node)->coord.getValue();
      if (vc != NULL)
      {
        coords = ((SoVRMLCoordinate *)vc)->point;
        return vc;
      }
  }
  else
  //Comprobamos si es un nodo derivado SoVRMLShape
  if (node->getTypeId().isDerivedFrom(SoVRMLShape::getClassTypeId())) 
  {
      SoNode *vg = ((SoVRMLShape*)node)->geometry.getValue();
      if (vg->getTypeId().isDerivedFrom(SoVRMLVertexShape::getClassTypeId())) 
      {
          SoNode *vc = ((SoVRMLVertexShape*)vg)->coord.getValue();
          if (vc != NULL)
          {
            coords = ((SoVRMLCoordinate *)vc)->point;
            return vc;
          }
      }
      else
      {
          return NULL;
      }
  }


  //Realizamos una busqueda en el path
  SoSearchAction sa;
  sa.setType(SoCoordinate3::getClassTypeId() );
  sa.setInterest(SoSearchAction::LAST);
  sa.apply((SoPath *)path);

  //Miramos si hubo algun resultado o no
  if (sa.getPath() == NULL)
    return NULL;
  
  //Devolvemos las coordenadas y el nodo
  coords = ((SoCoordinate3 *)sa.getPath()->getTail())->point;
  return sa.getPath()->getTail();

}//SoNode *buscaCoordenadas (SoPath *path, SoMFVec3f &coords)


//! Calcula la mímima BBox no orientada con los ejes
SbXfBox3f measure_XfBBox(SoPath *path)
{
  //Creamos un SoGetBoundingBoxAction con un viewport por defecto 
  SoGetBoundingBoxAction *bboxaction = new SoGetBoundingBoxAction(SbViewportRegion());

  //Aplicamos la accion al path
  bboxaction->apply(path);

  //Leemos la XfBoundingBox
  SbXfBox3f xfbox = bboxaction->getXfBoundingBox();

  //La escribimos por stderr, para unas prisas
  //xfbox.print(stderr);

  //Liberamos memoria
  delete bboxaction;

  //devolvemos la caja 
  return xfbox;
} //SbXfBox3f measure_XfBBox(SoPath *path)



//! Calcula el centro de masas de un vector de puntos */
SbVec3f centroid (const SoMFVec3f &coords)
{
	//Creamos el centro de masas de retorno
	SbVec3f cdm(0,0,0);

	//Numero de puntos de entrada
	int size= coords.getNum();

	//Verificamos que no es un vector vacio
	if (size > 0)
	{
	    //Acumulamos todos los puntos de entrada
	    for (int i=0; i < size; i++)
               cdm += coords[i];

	    //Dividimos entre el numero de puntos
	    cdm /= float(size);
	}

	//Devolvemos el resultado
	return cdm;
}// SbVec3f centroid (const SoMFVec3f &coords)

//! Modifica el origen de un vector de puntos 
void center_new (SoMFVec3f &coords, const SbVec3f center)
{
    //Numero de puntos de entrada
    int size= coords.getNum();

    //Restamos el centro a cada punto del vector
    for (int i=0; i < size; i++)
    {
       SbVec3f p = coords[i] - center;
       coords.set1Value (i, p);
    }
}// void center_new (SoMFVec3f &coords, const SbVec3f center)


//! Triangulado de un IndexedFaceSet. Devuelve el numero de triangulos
int IndexedFaceSet_triangulate (SoMFInt32 &coordIndex)
{

   //salida triangulada
   SoMFInt32 tri;

   //Recorremos las facetas de entrada
   int numIdx = coordIndex.getNum();
   int k = 0;
   int numTri = 0;
   for (int i=0; i<numIdx; i++)
   {
       //Contamos los vertices de esta faceta
       int j=i;
       while (coordIndex[j] > -1)
       {
	   j++;
       }

       //Calculamos el numero de vertices mediante una resta.
       int nv = j-i;

       //Vertice comun a todos los triangulos
       int v0 = coordIndex[i];

       //Recorremos el resto de vertices de la faceta
       for (j=2; j<nv; j++)
       {
	  //Metemos el vertice comun
	  tri.set1Value(k++, v0);
	  //Metemos otros dos vertices
	  tri.set1Value(k++, coordIndex[i+j-1]);
	  tri.set1Value(k++, coordIndex[i+j]);
          //Cerramos el triangulo
          tri.set1Value(k++, -1);
	  //Incrementamos la cuenta de triangulos
	  numTri++;
       }
       
       //Incrementamos el contador para saltar esta cara
       i = i + nv;

   }// for (int i=0; i<numIdx; i++)

   //Copiamos el nuevo faceSet sobre el antiguo
   coordIndex.copyFrom (tri);

   //Devolvemos el numero de triangulos
   return numTri;

} //int IndexedFaceSet_triangulate (SoMFInt32 &coordIndex)

//! Cambia la orientación de todas las facetas de un IndexedFaceSet
void IndexedFaceSet_change_orientation (SoMFInt32 &coordIndex)
{

   //salida reorientada
   SoMFInt32 resul;

   //Recorremos las facetas de entrada
   int numIdx = coordIndex.getNum();
   int k = 0;
   for (int i=0; i<numIdx; i++)
   {
       //Contamos los vertices de esta faceta
       int j=i;
       while (coordIndex[j] > -1)
       {
	   j++;
       }

       //Calculamos el numero de vertices mediante una resta.
       int nv = j-i;

       //Recorremos los vertices de la faceta
       for (j=0; j<nv; j++)
       {
	  //Copiamos el indice en orden inverso
	  resul.set1Value(k++, coordIndex[i+nv-j-1]);
       }
       
       //Cerramos la faceta
       resul.set1Value(k++, -1);
       
       //Incrementamos el contador para saltar esta cara
       i = i + nv;

   }// for (int i=0; i<numIdx; i++)

   //Copiamos el nuevo coordIndex sobre el antiguo
   coordIndex.copyFrom (resul);

} //void IndexedFaceSet_change_orientation (SoMFInt32 &coordIndex)

//Conversion de un IndexedTriangleStripSet a IndexedFaceSet
SoIndexedFaceSet * IndexedTriangleStripSet_to_IndexedFaceSet (const SoIndexedTriangleStripSet *itss)
{
   SoIndexedFaceSet *ifs;
   int j;

   //Inicializamos las variables de salida
   ifs = new SoIndexedFaceSet();
   j = 0;
   
   //Volcado de las facetas
   int idx=0;
   int numIdx = itss->coordIndex.getNum();
   while (idx<numIdx)
   {
       int idx1 = itss->coordIndex[idx++];
       int idx2 = itss->coordIndex[idx++];
       int idx3 = itss->coordIndex[idx++];
       int numTri = 1;
       while ((idx3 > -1) && (idx<numIdx))
       {
	   //Introducimos un nuevo triangulo segun las reglas
	   //de los strips de openGL
	   ifs->coordIndex.set1Value(j++, idx1);
	   if (numTri & 1)
	   {
	      ifs->coordIndex.set1Value(j++, idx2);
	      ifs->coordIndex.set1Value(j++, idx3);
	   }
	   else
	   {
	      ifs->coordIndex.set1Value(j++, idx3);
	      ifs->coordIndex.set1Value(j++, idx2);
	   }
	   ifs->coordIndex.set1Value(j++, -1);

	   //Saltamos al siguiente indice
	   idx1 = idx2;
	   idx2 = idx3;
           idx3 = itss->coordIndex[idx++];
	   numTri++;
       }// while ((idx3 > -1) && (idx<numIdx))
   } // while (idx<numIdx)

   //Copiamos el vertexProperty, por si acaso
   ifs->vertexProperty = itss->vertexProperty.getValue() ;

   return ifs;

}// IndexedTriangleStripSet_to_IndexedFaceSet 

/* TODO
//Calcula el cierre convexo de una nube de puntos. Devuelve numero de facetas en cierre
int convex_hull (const SoMFVec3f &coords, SoMFVec3f &ch_coords, SoMFInt32 &ch_coordIndex)
{
    unsigned int f,v,nv;

    //Leemos el numero de puntos y coordenadas de los mismos
    int vcount = coords.getNum();
    const float *verts = coords.getValues(0)->getValue();

    //Creamos un objeto QHull
    QhullLibrary qh(0);
    QhullDesc qd( (QhullFlag)(QF_DEFAULT), vcount, verts, sizeof(SbVec3f));
    QhullResult result;
    QhullError err = qh.CreateConvexHull( qd, result );
    if ( err != QE_OK )
    {
       fprintf(stderr, "Error al crear cierre convexo\n");
       return -1;
    }

    //Copiamos las coordenadas de los vertices del cierre convexo
    nv = result.mNumOutputVertices;
    for(v=0; v<nv; v++)
    {
       ch_coords.set1Value(v, result.mOutputVertices[3*v], 
                              result.mOutputVertices[3*v+1], 
                              result.mOutputVertices[3*v+2] );
    }

    //Cambiamos el tamaño de ch_coordIndex
    ch_coordIndex.setNum(result.mNumIndices);

    //Copiamos las coordenadas de los indices, terminando cada faceta con -1
    int idx1=0;
    int idx2=0;
    for (f=0; f < result.mNumFaces; f++)
    {
        //el primer indice indica la longitud de la faceta
	nv = result.mIndices[idx1++];

	//Copiamos los indices
        for(v=0; v<nv; v++)
        {
           ch_coordIndex.set1Value(idx2++, result.mIndices[idx1++]);
        }

	//Introducimos el fin de faceta
        ch_coordIndex.set1Value(idx2++,-1);

    }//for

    //Cambiamos la orientacion de las facetas, para que tengan la normal hacia fuera
    IndexedFaceSet_change_orientation (ch_coordIndex);
 
    //Salvamos el numero de facetas para el resultado
    nv = result.mNumFaces;

    //Liberamos memoria
    qh.ReleaseResult(result);

    return nv;

}// int convex_hull (const SoMFVec3f &coords, SoMFVec3f &ch_coords, SoMFInt32 &ch_coordIndex)
*/


//! Importacion de un complejo simplicial en formato .neutral (netgen)
SoSeparator *import_netgen_neutral (FILE *in)
{
  //Numero de vertices, tetraedros y facetas externas
  int nv, nt, nf;

  //Creamos una estructura para devolver el resultado
  SoSeparator *sep= new SoSeparator();
  SoCoordinate3 *coord = new SoCoordinate3();
  SoIndexedFaceSet *tets = new SoIndexedFaceSet();
  SoIndexedFaceSet *ifs = new SoIndexedFaceSet();
  sep->addChild(coord);
  sep->addChild(tets);
  sep->addChild(ifs);
  coord->setName("coord");
  tets->setName("tetras");
  ifs->setName("surfs");

  //Leemos el numero de vertices del complejo
  if (fscanf(in, " %d", &nv) != 1)
    return NULL;

  //Leemos las coordenadas de los vertices
  //fprintf(stderr, "Leyendo %d vertices\n", nv);
  for (int i=0; i<nv ; i++)
  {
    float x, y, z;
    //Leemos 3 coordenadas en coma flotante
    if (fscanf(in, " %f %f %f", &x, &y, &z) != 3)
      return NULL;
    //La almacenamos en el nodo Coordinate3
    coord->point.set1Value(i, x, y, z);
  }

  //Leemos el numero de tetraedros del complejo
  if (fscanf(in, " %d", &nt) != 1)
    return NULL;

  //fprintf(stderr, "Leyendo %d tetraedros\n", nt);
  int idx=0;
  for (int i=0; i<nt ; i++)
  {
    //Leemos 5 indices enteros
    int n, a, b, c, d;
    if (fscanf(in, " %d %d %d %d %d", &n, &a, &b, &c, &d) != 5)
      return NULL;

    //Creamos cuatro facetas 
    //Tenemos en cuenta que los indices en formato .neutral son base 1 
    tets->coordIndex.set1Value(idx++, c-1);
    tets->coordIndex.set1Value(idx++, b-1);
    tets->coordIndex.set1Value(idx++, a-1);
    tets->coordIndex.set1Value(idx++, -1);
    tets->coordIndex.set1Value(idx++, b-1);
    tets->coordIndex.set1Value(idx++, c-1);
    tets->coordIndex.set1Value(idx++, d-1);
    tets->coordIndex.set1Value(idx++, -1);
    tets->coordIndex.set1Value(idx++, a-1);
    tets->coordIndex.set1Value(idx++, d-1);
    tets->coordIndex.set1Value(idx++, c-1);
    tets->coordIndex.set1Value(idx++, -1);
    tets->coordIndex.set1Value(idx++, a-1);
    tets->coordIndex.set1Value(idx++, b-1);
    tets->coordIndex.set1Value(idx++, d-1);
    tets->coordIndex.set1Value(idx++, -1);
  }

  //Leemos el numero de facetas exteriores del complejo
  if (fscanf(in, " %d", &nf) != 1)
    return NULL;

  //fprintf(stderr, "Leyendo %d facetas externas\n", nf);
  idx=0;
  for (int i=0; i<nf ; i++)
  {
    //Leemos 4 indices enteros
    int n, a, b, c;
    if (fscanf(in, " %d %d %d %d", &n, &a, &b, &c) != 4)
      return NULL;

    //Creamos una faceta con los 3 indices leidos.
    //Tenemos en cuenta que los indices en formato .neutral son base 1 
    ifs->coordIndex.set1Value(idx++, a-1);
    ifs->coordIndex.set1Value(idx++, b-1);
    ifs->coordIndex.set1Value(idx++, c-1);
    ifs->coordIndex.set1Value(idx++, -1);
  }

  return sep;
}

//! Importacion de un complejo simplicial en formato .nodes .faces (tetgen)
SoSeparator *import_tetgen (const char *nodeFilename)
{
	//Numero de vertices, tetraedros y facetas externas
	int nv, nt, nf;
	char dummy_buffer[1024];

	//Creamos una cadena con el nombre del fichero sin extension
	std::string rootFilename(nodeFilename);
	size_t extPos = rootFilename.rfind(".node");
	if (extPos > 0)
		rootFilename.erase(extPos);

	//Abrimos el fichero .node
	FILE *nodeFile = fopen(nodeFilename, "r");
	if (!nodeFile)
	{
		perror(nodeFilename);
		return NULL;
	}

	//Creamos una estructura para devolver el resultado
	SoSeparator *sep= new SoSeparator();
	sep->setName(rootFilename.c_str());
	SoCoordinate3 *coord = new SoCoordinate3();
	coord->setName(nodeFilename);
	sep->addChild(coord);

	//Leemos el numero de vertices del complejo
	if (fscanf(nodeFile, " %d", &nv) != 1)
		return NULL;
	fgets(dummy_buffer, 1024, nodeFile);

	//Leemos las coordenadas de los vertices
	//fprintf(stderr, "Leyendo %d vertices\n", nv);
	for (int i=0; i<nv ; i++)
	{
		int n;
		float x, y, z;
		//Leemos el indice del vertice y las coordenadas X, Y, Z
		if (fscanf(nodeFile, " %d %f %f %f", &n, &x, &y, &z) != 4)
			return NULL;
		fgets(dummy_buffer, 1024, nodeFile);

		//std::cerr << i << "/" << _num_vertices << ":" << x << " " << y << " " << z << std::endl;

		//La almacenamos en el nodo Coordinate3
		//coord->point.set1Value(i, x, y, z);
		coord->point.set1Value(n, x, y, z);
	}

	//Cerramos el fichero
	fclose(nodeFile);

	//Abrimos el fichero .ele
	std::string eleFilename(rootFilename);
	eleFilename.append(".ele");
	FILE *eleFile = fopen(eleFilename.c_str(), "r");
	if (!eleFile)
	{
		//Imprimimos un aviso y continuamos
		perror(eleFilename.c_str() );
	}
	else
	{
		//Añadimos un indexedFaceSet para las facetas internas
		SoIndexedFaceSet *ifs = new SoIndexedFaceSet();
		ifs->setName(eleFilename.c_str());
		sep->addChild(ifs);

		//Ultimo indice introducido en el ifs
		int ifs_idx=0;

		//Ahora leemos los triangulos que forman la superfice de la malla
		if (fscanf(eleFile, " %d", &nt) != 1)
			return NULL;
		fgets(dummy_buffer, 1024, eleFile);

		//fprintf(stderr, "Leyendo %d tetraedros internos\n", nt);
		for (int i=0; i<nt ; i++)
		{
			//Leemos 4 indices enteros
			int n, a, b, c, d;
			if (fscanf(eleFile, " %d %d %d %d %d", &n, &a, &b, &c, &d) != 5)
				return NULL;
			fgets(dummy_buffer, 1024, eleFile);

			//Creamos 4 facetas  con los indices leidos.
			//Teniendo en cuenta que a tetgen le gusta el sentido horario
			ifs->coordIndex.set1Value(ifs_idx++, c);
			ifs->coordIndex.set1Value(ifs_idx++, b);
			ifs->coordIndex.set1Value(ifs_idx++, a);
			ifs->coordIndex.set1Value(ifs_idx++, -1);
			ifs->coordIndex.set1Value(ifs_idx++, a);
			ifs->coordIndex.set1Value(ifs_idx++, b);
			ifs->coordIndex.set1Value(ifs_idx++, d);
			ifs->coordIndex.set1Value(ifs_idx++, -1);
			ifs->coordIndex.set1Value(ifs_idx++, d);
			ifs->coordIndex.set1Value(ifs_idx++, c);
			ifs->coordIndex.set1Value(ifs_idx++, a);
			ifs->coordIndex.set1Value(ifs_idx++, -1);
			ifs->coordIndex.set1Value(ifs_idx++, b);
			ifs->coordIndex.set1Value(ifs_idx++, c);
			ifs->coordIndex.set1Value(ifs_idx++, d);
			ifs->coordIndex.set1Value(ifs_idx++, -1);
		}

		//Cerramos el fichero
		fclose(eleFile);
	}

	//Abrimos el fichero .face
	std::string faceFilename(rootFilename);
	faceFilename.append(".face");
	FILE *faceFile = fopen(faceFilename.c_str(), "r");
	if (!faceFile)
	{
		//Imprimimos un aviso
		perror(faceFilename.c_str() );

		//Tratamos el fichero .node como una nube de puntos
		sep->addChild(new SoPointSet() );
	}
	else
	{
		//Añadimos un indexedFaceSet para las facetas externas
		SoIndexedFaceSet *ifs = new SoIndexedFaceSet();
		ifs->setName(faceFilename.c_str());
		sep->addChild(ifs);

		//Ultimo indice introducido en el ifs
		int ifs_idx=0;

		//Ahora leemos los triangulos que forman la superfice de la malla
		if (fscanf(faceFile, " %d", &nf) != 1)
			return NULL;
		fgets(dummy_buffer, 1024, faceFile);

		//fprintf(stderr, "Leyendo %d facetas externas\n", _numSurfaceFaces);
		for (int i=0; i<nf ; i++)
		{
			//Leemos 4 indices enteros
			int n, a, b, c;
			if (fscanf(faceFile, " %d %d %d %d", &n, &a, &b, &c) != 4)
				return NULL;
			fgets(dummy_buffer, 1024, faceFile);

			//Creamos una faceta con los 3 indices leidos.
			//Teniendo en cuenta que a tetgen le gusta el sentido horario
			ifs->coordIndex.set1Value(ifs_idx++, c);
			ifs->coordIndex.set1Value(ifs_idx++, b);
			ifs->coordIndex.set1Value(ifs_idx++, a);
			ifs->coordIndex.set1Value(ifs_idx++, -1);
		}

		//Cerramos el fichero
		fclose(faceFile);
	}

	//std::cerr << "Leidos " << ntetras << " tetraedros" << std::endl;

	return sep;
}

char *cds_export_string (SoNode *node)
{
    //Reservamos un buffer inicial
    size_t buffer_size = 1024;
    void *buffer = malloc(buffer_size);

    //Escribimos el contenido del nodo en el buffer
    SoOutput out;
    out.setBuffer(buffer, buffer_size, realloc);
    SoWriteAction wa(&out);
    wa.apply(node);

    //Leemos el buffer y reducimos su tamaño al minimo
    out.getBuffer(buffer, buffer_size);
    buffer = realloc(buffer, buffer_size+1);
    ((char *)buffer)[buffer_size]=0;

    return (char *)buffer;
}

bool cds_export_hppFile (SoNode *node, const char *className, const char *filename)
{
   static char strTemplate_str[] =
	   "//File created with coindesigner\n"
           "#ifndef _**classname**_H_\n"
           "#define _**classname**_H_\n"
	   "\n"
	   "#include <Inventor/SoDB.h>\n"
	   "#include <Inventor/SoInput.h>\n"
	   "#include <Inventor/nodes/SoSeparator.h>\n"
	   "\n"
	   "static const char ***classname**_str[] = {\n"
	   "**scene_definition**"
	   "};\n"
	   "\n"
	   "class **classname** : public SoSeparator\n"
	   "{\n"
	   "public:\n"
	   "  **classname**()\n"
	   "  {\n"
	   "    SoInput in;\n"
	   "    in.setStringArray(**classname**_str);\n"
	   "    SoSeparator *sep = SoDB::readAll(&in);\n"
	   "    sep->ref();\n"
	   "    while (sep->getNumChildren() > 0)\n"
	   "    {\n"
	   "       this->addChild(sep->getChild(0));\n"
	   "       sep->removeChild(0);\n"
	   "    }\n"
	   "    sep->unref();\n"
	   "  }\n"
	   "};\n"
	   "#endif\n";

   //Una cadena para almacenar el template y realizar sustituciones
   std::string strTemplate(strTemplate_str);

   //Buscamos palabras clave y las sustituimos
   int pos;
   const char *code;

   //Reemplazo de la clave "**classname**"
   code = "**classname**";
   while ((pos=strTemplate.find(code)) > 0)
   {
	   strTemplate.replace(pos, strlen(code), className);
   }


   //Reemplazo de la clave "**scene_definition**"
   code="**scene_definition**";
   pos=strTemplate.find(code);

   //Escribimos la escena en un buffer temporal y en un SbString
   char *sceneBuffer = cds_export_string (node);
   SbString s(sceneBuffer);
   free (sceneBuffer);

#ifdef QPROGRESSDIALOG_H
    int s_size = s.getLength();
    QProgressDialog progress(QProgressDialog::tr("Exporting..."), 0, 0, s_size);
	progress.setWindowModality(Qt::WindowModal);
	progress.show();
#endif

   //Buscamos un caracter de fin de linea en s
   int line_end = s.find("\n");

   //Si hemos encontrado un fin de linea en s
   while(line_end > -1)
   {

	   //Evitamos escribir lineas vacias
	   if (line_end > 0)
	   {

                   //Aislamos la linea y "escapamos" los caracteres problemáticos
                   std::string line(s.getSubString(0, line_end-1).getString());
                   for (unsigned i=0; i < line.size(); i++)
                   {
                      if (line[i] == '\"' || line[i] == '\\' )
                      {
                         line.insert(i++, "\\");
                      }
                   }

                   //Añadimos los codigos necesarios para la compilacion al principio y al final
                   line.insert(0,"    \"");
                   line.append("\\n\",\n");

		   //Insertamos la linea en el fichero
		   strTemplate.insert(pos, line );

		   //Actualizamos la posición del punto de inserccion
		   pos=strTemplate.find(code);
	   }

	   //Eliminamos la linea que hemos escrito
	   s.deleteSubString(0, line_end);

	   //Buscamos otra linea
	   line_end = s.find("\n");

#ifdef QPROGRESSDIALOG_H
    progress.setValue(s_size - s.getLength());
#endif

   }//while(line_end > -1)

#ifdef QPROGRESSDIALOG_H
    progress.setValue(s_size);
#endif

   //Añadimos codigo para cerrar la estructura y eliminamos el **scene_definition**
   strTemplate.replace(pos, strlen(code), "    NULL\n");

   //Creamos el fichero de salida
	FILE *hppFile = fopen(filename, "w");
	if (!hppFile)
	{
		perror(filename);
		return false;
	}

	//Escribimos el fichero .hpp en fichero de salida
	fputs(strTemplate.c_str(), hppFile);

	fclose(hppFile);

	return true;

}//bool cds_export_hppFile (SoNode *node, const char *className, const char *filename)


bool cds_export_cppFile (const char *className, const char *filename)
{
   static char strTemplate_str[] =
	   "//File created with coindesigner\n"
	   "#include <Inventor/SoInput.h>\n"
	   "#if defined(SOWIN_DLL) || defined(SOWIN_NOT_DLL)\n"
	   "#include <Inventor/Win/SoWin.h>\n"
	   "#include <Inventor/Win/viewers/SoWinExaminerViewer.h>\n"
	   "#else\n"
	   "#include <Inventor/Qt/SoQt.h>\n"
	   "#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>\n"
	   "#endif\n"
	   "#include \"**classname**.h\"\n"
	   "\n"
	   "int main (int argc, char** argv) {\n"
	   "\n"
	   "#if defined(SOWIN_DLL) || defined(SOWIN_NOT_DLL)\n"
	   "HWND win = SoWin::init(argv[0]);\n"
	   "#else\n"
	   "QWidget* win = SoQt::init(argv[0]);\n"
	   "#endif\n"
	   "if (win == NULL) exit(1);\n"
	   "\n"
	   "SoSeparator* root = new **classname**();\n"
	   "root->ref();\n"
	   "\n"
	   "#if defined(SOWIN_DLL) || defined(SOWIN_NOT_DLL)\n"
	   "SoWinExaminerViewer* viewer = new SoWinExaminerViewer(win);\n"
	   "#else\n"
	   "SoQtExaminerViewer* viewer = new SoQtExaminerViewer(win);\n"
	   "#endif\n"
	   "viewer->setSceneGraph(root);\n"
	   "viewer->setTitle(\"**classname**\");\n"
	   "viewer->show(); \n"
	   "\n"
	   "#if defined(SOWIN_DLL) || defined(SOWIN_NOT_DLL)\n"
	   "SoWin::show(win);\n"
	   "SoWin::mainLoop();\n"
	   "#else\n"
	   "SoQt::show(win);\n"
	   "SoQt::mainLoop();\n"
	   "#endif\n"
	   "\n"
	   "root->unref();\n"
	   "delete viewer;\n"
	   "\n"
	   "return 0;\n"
	   "}\n"
	   "\n";

   //Una cadena para almacenar el template y realizar sustituciones
   std::string strTemplate(strTemplate_str);

   //Buscamos palabras clave y las sustituimos
   int pos;
   const char *code;

   //Reemplazo de la clave "**classname**"
   code = "**classname**";
   while ((pos=strTemplate.find(code)) > 0)
   {
	   strTemplate.replace(pos, strlen(code), className);
   }


   //Creamos el fichero de salida
   FILE *cppFile = fopen(filename, "w");
   if (!cppFile)
   {
       perror(filename);
       return false;
   }

   //Escribimos el fichero .cpp en fichero de salida
   fputs(strTemplate.c_str(), cppFile);

   fclose(cppFile);

   return true;
} // bool cds_export_cppFile (const char *className, const char *filename)


//!Exporta un subarbol de escena a formato .ase (ASCII Scene Exporter)
//Esta funcion está pensada para mallas exportadas desde el vivid
bool cds_export_ase (SoPath *path, const char *filename)
{
	assert (path != NULL);
	assert (filename != NULL);

	//Busca el ultimo nodo de tipo coordinate3
	SoTexture2 *texture = (SoTexture2 *)buscaUltimoNodo(path, SoTexture2::getClassTypeId());
	SoTextureCoordinate2 *textureCoord = (SoTextureCoordinate2 *)buscaUltimoNodo(path, SoTextureCoordinate2::getClassTypeId());
	SoCoordinate3 *coord = (SoCoordinate3 *)buscaUltimoNodo(path, SoCoordinate3::getClassTypeId());
	SoIndexedFaceSet *faces = (SoIndexedFaceSet *)buscaUltimoNodo(path, SoIndexedFaceSet::getClassTypeId());

	//Comprobamos que hemos encontrado la informacion imprescindible
	if (coord==NULL || faces==NULL)
	{
#ifdef QMESSAGEBOX_H
		QMessageBox::critical( NULL, QObject::tr("Error"), QObject::tr("Can't export this to .ase file format"));
#endif
		return false;
	}

	//Cuenta vertices y facetas, triangula la malla si es necesario
	int numVertex = coord->point.getNum();
	SbVec3f center=centroid(coord->point);
	int numFaces  = IndexedFaceSet_triangulate (faces->coordIndex);
	if (numVertex == 0 || numFaces==0)
	{
#ifdef QMESSAGEBOX_H
		QMessageBox::critical( NULL, QObject::tr("Error"), QObject::tr("Can't export this to .ase file format"));
#endif
		return false;
	}

   //Creamos el fichero de salida
   FILE *aseFile = fopen(filename, "w");
   if (!aseFile)
   {
       perror(filename);
#ifdef QMESSAGEBOX_H
	   QMessageBox::critical( NULL, QObject::tr("Error"), 
		   QObject::tr("Can't create output file")+ " " + QString(filename) );
#endif
       return false;
   }

   //char offFilename[512];
   //sprintf(offFilename, "%s.off", filename);
   //FILE *offFile = fopen(offFilename, "w");

   //Salva el fichero de textura
   char textureFilename[1024]="";
   if (texture != NULL)
   {
	   //Create a jpg with the texture
	   sprintf(textureFilename, "%s.jpg", filename);

	   //Take image information
	   SbVec2s size;
	   int nc; 
	   const unsigned char *pixbuf = texture->image.getValue(size, nc);
	   int w=size[0];
	   int h=size[1];
	   //qDebug("nc=%d\n", nc);

       simage_save_image(textureFilename, pixbuf, w, h, nc,"jpg");
   }

   //Escribimos la informacion necesaria
   fprintf(aseFile, "*3DSMAX_ASCIIEXPORT\t200\n");
   fprintf(aseFile, "*COMMENT \"Created with Coindesigner\"\n");


   //Creamos un material con la textura adecuada
   fprintf(aseFile,
"*MATERIAL_LIST {\n"
"	*MATERIAL_COUNT 1\n"
"	*MATERIAL 0 {\n"
"		*MATERIAL_NAME \"Material #0\"\n"
"		*MATERIAL_CLASS \"Standard\"\n"
"		*MATERIAL_AMBIENT 0.6	0.6	0.6\n"
"		*MATERIAL_DIFFUSE 0.6	0.6	0.6\n"
"		*MATERIAL_SPECULAR 0.9	0.9	0.9\n"
"		*MATERIAL_SHINE 0.1\n"
"		*MATERIAL_SHINESTRENGTH 0.0\n"
"		*MATERIAL_TRANSPARENCY 0.0\n"
"		*MATERIAL_WIRESIZE 1.0\n"
"		*MATERIAL_SHADING Blinn\n"
"		*MATERIAL_XP_FALLOFF 0.0\n"
"		*MATERIAL_SELFILLUM 0.0\n"
"		*MATERIAL_FALLOFF In\n"
"		*MATERIAL_XP_TYPE Filter\n"
"		*MAP_DIFFUSE {\n"
"			*MAP_NAME \"Map #2\"\n"
"			*MAP_CLASS \"Bitmap\"\n"
"			*MAP_SUBNO 1\n"
"			*MAP_AMOUNT 1.0000\n"
"			*BITMAP \"%s\"\n"
"			*MAP_TYPE Screen\n"
"			*UVW_U_OFFSET 0.0\n"
"			*UVW_V_OFFSET 0.0\n"
"			*UVW_U_TILING 1.0\n"
"			*UVW_V_TILING 1.0\n"
"			*UVW_ANGLE 0.0\n"
"			*UVW_BLUR 1.0\n"
"			*UVW_BLUR_OFFSET 0.0\n"
"			*UVW_NOUSE_AMT 1.0\n"
"			*UVW_NOISE_SIZE 1.0\n"
"			*UVW_NOISE_LEVEL 1\n"
"			*UVW_NOISE_PHASE 0.0\n"
"			*BITMAP_FILTER Pyramidal\n"
"		}\n"
"	}\n"
"}\n", textureFilename );

   fprintf(aseFile, "*GEOMOBJECT {\n");
   fprintf(aseFile, "*MESH {\n");
   fprintf(aseFile, "*MESH_NUMVERTEX %d\n", numVertex);

   //fprintf(offFile, "OFF %d %d 0\n", numVertex, numFaces);


   //Lista de vertices
   fprintf(aseFile, "*MESH_VERTEX_LIST {\n");
   for (int i=0; i< numVertex; i++)
   {
	   const float *xyz = coord->point.getValues(i)->getValue();
	   float x = 0.33*(center[0]-xyz[0]);
	   float y = 0.33*(xyz[2]-center[2])+200;
	   float z = 0.33*(xyz[1]-center[1]);
	   //Intercambiamos las coordenadas Y Z para seguir convenio de 3DStudio
	   fprintf(aseFile, "*MESH_VERTEX %d %.5f %.5f %.5f\n",i,x,y,z);
	   //fprintf(offFile, "%.5f %.5f %.5f\n", x, y, z);
   }
   fprintf(aseFile, "}\n"); //MESH_VERTEX_LIST

   //Lista de facetas
   fprintf(aseFile, "*MESH_NUMFACES %d\n", numFaces);
   fprintf(aseFile, "*MESH_FACE_LIST {\n");
   int numIndex = faces->coordIndex.getNum();
   int j=0;
   for (int i=0; i< numFaces; i++)
   {
	   //Comprueba que los indices son correctos
	   const int a = faces->coordIndex[j++];
	   const int b = faces->coordIndex[j++];
	   const int c = faces->coordIndex[j++];
	   assert(a >=0 && b>=0 && c>= 0 && a != b && a != c && b != c);

	   fprintf(aseFile, "*MESH_FACE %d : A: %d B: %d C: %d"
						" AB: 1 BC: 1 CA: 1 *MESH_SMOOTHING 1 *MESH_MTLID 0\n",
						i,c,b,a);
	   //fprintf(offFile, "3 %d %d %d\n", c, b, a);

	   if( j < numIndex)
	   {
		   //Comprueba que esta cara es un triangulo
		   assert(faces->coordIndex[j]==-1);
		   j++;
	   }

   }//for
   fprintf(aseFile, "}\n"); //MESH_FACE_LIST

   //Informacion de coordenadas de textura
   if (texture != NULL && textureCoord != NULL)
   {
	   fprintf(aseFile, "*MESH_NUMTVERTEX %d\n", numVertex);
	   fprintf(aseFile, "*MESH_TVERTLIST {\n");
	   for (int i=0; i< numVertex; i++)
	   {
		   const float *uv =  textureCoord->point.getValues(i)->getValue();
		   fprintf(aseFile, "*MESH_TVERT\t%d\t%f\t%f\t0\n",i,uv[0],uv[1]);
	   }
	   fprintf(aseFile, "}\n"); //MESH_TVERTLIST


	   //Lista de facetas de textura
	   fprintf(aseFile, "*MESH_NUMTVFACES %d\n", numFaces);
	   fprintf(aseFile, "*MESH_TFACELIST {\n");
	   int numIndex = faces->coordIndex.getNum();
	   j=0;
	   for (int i=0; i< numFaces; i++)
	   {
		   //Presupone que son triangulos, y comprueba que lo son
		   const int a= faces->coordIndex[j++];
		   const int b= faces->coordIndex[j++];
		   const int c= faces->coordIndex[j++];
		   assert(a >=0 && b>=0 && c>= 0);

		   fprintf(aseFile, "*MESH_TFACE\t%d\t%d\t%d\t%d\n", i,c,b,a);

		   if( j < numIndex)
		   {
			   //Esta cara no es un triangulo
			   assert(faces->coordIndex[j]==-1);
			   j++;
		   }
	   }
	   fprintf(aseFile, "}\n"); //MESH_TFACELIST

   } // if (texture != NULL && textureCoord != NULL)

   fprintf(aseFile, "*MESH_NUMCVERTEX 0\n");
   fprintf(aseFile, "}\n"); //MESH

   fprintf(aseFile, "*PROP_MOTIONBLUR 0\n"
	                "*PROP_CASTSHADOW 1\n"
	                "*PROP_RECVSHADOW 1\n"
	                "*MATERIAL_REF 0\n"
	                "}\n"); //GEOMOBJECT

   fclose(aseFile);
   //fclose(offFile);

   return true;
} //bool cds_export_ase (SoPath *path, const char *filename)



/*! Busca el ultimo nodo de un tipo dado en un path */
SoNode *buscaUltimoNodo(SoPath *p, const SoType t)
{
	SoSearchAction sa;
	sa.setType(t);
	sa.setInterest(SoSearchAction::LAST);
	sa.apply(p);
	SoPath *path = sa.getPath();
	return path ? path->getTail() : NULL;
}//SoNode *buscaUltimoNodo(SoPath *p, SoType t)


/*! Remove all nodes of a given type */
  void strip_node(SoType type, SoNode * root) 
  {
    SoSearchAction sa;
    sa.setType(type);
    sa.setSearchingAll(TRUE);
    sa.setInterest(SoSearchAction::ALL);
    sa.apply(root);

    SoPathList & pl = sa.getPaths();
    for (int i = 0; i < pl.getLength(); i++) {
      SoFullPath * p = (SoFullPath*) pl[i];
        if (p->getTail()->isOfType(type)) {
          SoGroup * g = (SoGroup*) p->getNodeFromTail(1);
          g->removeChild(p->getIndexFromTail(0));
        }
      } 
    sa.reset();  
  }
  
#ifdef QIMAGE_H
  ///Convert a SoSFImage object into a QImage
  QImage * SoSFImage_to_QImage(const SoSFImage *sfimage)
  {
	//Check for NULL pointers
  	if (sfimage == NULL)
		return NULL;

	//Take image information
	SbVec2s size;
	int nc; 
	const unsigned char *pixbuf = sfimage->getValue(size, nc);
	int w=size[0];
	int h=size[1];
	//qDebug("nc=%d\n", nc);

	//Check the format of image.
	QImage::Format qformat = QImage::Format_Invalid;
	switch (nc)
	{
		case 4:  qformat = QImage::Format_ARGB32; break;
		case 3:  qformat = QImage::Format_RGB32; break;
		case 2:  qformat = QImage::Format_ARGB32; break;
		case 1:  qformat = QImage::Format_RGB32; break;
		default: qformat = QImage::Format_Invalid;
	}

	//Check the format of image. Only RGB and RGBA are supported
	if (qformat == QImage::Format_Invalid)
		return NULL;

    QImage *qimage = new QImage(w, h, qformat);
	QRgb rgb=qRgb(0,0,0);

	int idx=0;
	for (int j=h-1; j>=0; j--)
		for (int i=0; i< w; i++)
		{
			//Create a pixel acording with nc
			if (nc==4)
				rgb = qRgba(pixbuf[idx], pixbuf[idx+1], pixbuf[idx+2], pixbuf[idx+3]);
			else if (nc==3)
				rgb = qRgb(pixbuf[idx], pixbuf[idx+1], pixbuf[idx+2]);
			else if (nc==2)
				rgb = qRgba(pixbuf[idx], pixbuf[idx], pixbuf[idx], pixbuf[idx+1]);
			else if (nc==1)
				rgb = qRgb(pixbuf[idx], pixbuf[idx], pixbuf[idx]);

			//Put qut pixel in the QImage
			qimage->setPixel(i,j,rgb);

			//Next pixel in image buffer
			idx += nc;
		}//for

  	return qimage;
  }// QImage * SoSFImage_to_QImage(const SoSFImage *sfimage)

#endif
