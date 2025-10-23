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

#include <cds_viewers.h>
#include <Inventor/details/SoDetails.h>
#include "mainwindow.h"
#include "cds_util.h"

/*! Callback del evento raypick, que muestra informacion sobre el punto apuntado por el raton.*/
void pick_cb (void *ud, SoEventCallback * n)
{
  //Pasa el evento al editor llamante
  assert(ud != NULL);
  CdsExaminerEditor *cdsviewer = (CdsExaminerEditor *)ud;
  cdsviewer->pickCallback(n);
}

  
/*! Callback del evento raypick, que muestra informacion sobre el punto apuntado por el raton.*/
template <class SOTYPEVIEWER>
void CdsEditorTemplate<SOTYPEVIEWER>::pickCallback (SoEventCallback * n)
{
  const SoMouseButtonEvent * mbe = (SoMouseButtonEvent *)n->getEvent();
  int i;

  //Numero de facetas seleccionadas para parchear
  //static int numFacetasPatch = 0;

 //Identificamos la ventana, el visor y la barra de status
 QStatusBar *statusBar = this->statusBar();

 //qDebug("%p->%s(%p) -> state=%d button=%d", this, __FUNCTION__, n, mbe->getState(),  mbe->getButton() ) ;

  //Miramos si queremos alguna opcion de picado
 if (this->pickAction == Ui.actionNone)
	 return;

 bool infoAction = this->pickAction == Ui.actionInfo;

 //Comprobamos que se ha pulsado el boton izquierdo del raton
 if (mbe->getState() == SoButtonEvent::DOWN && mbe->getButton() == SoMouseButtonEvent::BUTTON1 ) 
 {

    //Aseguramos que mark_sep no esta colgado de myRoot, para evitar picar sobre la marca
    while (myRoot->findChild(mark_sep) > -1) 
       myRoot->removeChild(mark_sep);

    SoRayPickAction rp(this->getViewportRegion());
    rp.setPoint(mbe->getPosition());
    rp.apply(this->getSceneManager()->getSceneGraph());

	//Miramos si hemos pinchado sobre algun elemento
    SoPickedPoint * point = rp.getPickedPoint();
    if (point == NULL) 
	{
		statusBar->showMessage(QObject::tr("Nothing there."));

		//Reseteamos el contador de facetas
        //numFacetasPatch=0;

		return;
    }
    n->setHandled();

    //Extraemos las coordenadas del objeto apuntado
    SbVec3f v = point->getPoint();

    //Actualizamos la posicion de la marca
    mark_coord->point.setValue(v);

    //Aseguramos que mark_sep esta colgado de myRoot
 	if (this->pickAction == Ui.actionInfo && myRoot->findChild(mark_sep) < 0) 
       myRoot->addChild(mark_sep);

	QString M, S;

    //Imprimimos las coordenadas del punto 
	S.asprintf("<b>Pick XYZ=</b> %f %f %f", v[0], v[1], v[2]);

    //Imprimimos las coordenadas del punto en la barra del estado y en consola de mensajes
	M.asprintf("XYZ= %f %f %f", v[0], v[1], v[2]);

	if (infoAction)
		global_mw->addMessage(S);

    //Path del objeto apuntado
    SoPath *path = point->getPath()->copy(3,0);
    path->ref();

    //SoNode del objeto apuntado
    SoNode *nodo = path->getTail();

	//Buscamos el nodo seleccionado en el arbol de mainwindow
	global_mw->getItemFromPath(path);

    //Mostramos el path, salvo el ultimo nodo
	S.asprintf("<b>Node path=</b>");
    for (i=0; i < path->getLength()-1; i++)
    {
      const char *nombre_tipo = path->getNode(i)->getTypeId().getName(); 
	  S.append(QString(nombre_tipo) + QChar('[') + QString::number(path->getIndex(i+1)) + QString("]->"));
    }

    //Averiguamos el tipo del nodo
    SoType tipo = nodo->getTypeId();
    const char *nombre_tipo = tipo.getName();  

	//Mostramos el tipo en barra de status y el path completo en ventana mensajes
	S.append(QString(nombre_tipo));

	M += QString(" <> ") + QString(nombre_tipo);

	//Averiguamos el nombre del nodo
	const char *nombre_nodo = nodo->getName().getString();

	//Mostramos el nombre en barra de status
	if (nombre_nodo && strlen(nombre_nodo) > 0)
	{
		M += QString(" <> ") + QObject::tr("Name=") + QString(nombre_nodo);
		S.append(QString("; ") +QObject::tr("<b>Node Name=</b>") + QString(nombre_nodo));
	}

	if (infoAction)
		global_mw->addMessage(S);

    //Detalle del objeto apuntado
    const SoDetail *pickDetail = point->getDetail();
    if (pickDetail != NULL)
    {

      //Si hemos pinchado sobre una faceta, mostramos su informacion
      if (pickDetail->getTypeId() == SoFaceDetail::getClassTypeId()) 
      {
        SoFaceDetail *facDetail = (SoFaceDetail *) pickDetail;

        //Mostramos el indice de la faceta y la lista de vertices
	S.asprintf("Face index=%d; Vertex list=",facDetail->getFaceIndex());

        //Mostramos informacion sobre todos sus vertices
        for (i=0; i < facDetail->getNumPoints(); i++)
        {
          const SoPointDetail *pointDetail = facDetail->getPoint(i);
		  S.append( QString(" ") + QString::number(pointDetail->getCoordinateIndex()) );
        }//for

	M.append(QString(" <> ") + S);

	if (infoAction)
		global_mw->addMessage(S);

        SoMFVec3f coords;
        SoNode *nodeCoord = buscaCoordenadas (path, coords);

        if (nodeCoord)
        {
          const char *nombre_tipo = nodeCoord->getTypeId().getName();  
          const char *nombre_nodo = nodeCoord->getName().getString();

		  if (infoAction)
		  {
			  S.asprintf("%s coordinates node; Coordinate node name=%s", nombre_tipo, nombre_nodo);
			  global_mw->addMessage(S);

			  //Mostramos informacion sobre todos sus vertices
			  for (i=0; i < facDetail->getNumPoints(); i++)
			  {
				  int idx = facDetail->getPoint(i)->getCoordinateIndex();
				  S.asprintf("Vertex %d = %10f %10f %10f", idx, coords[idx][0],coords[idx][1],coords[idx][2]);
				  global_mw->addMessage(S);
			  }//for
		  }

        }// if (nodeCoord)


	    //Si se ha solicitado eliminar la faceta
        if (this->pickAction == Ui.actionRemove_face)
		   ifs_remove_facet(path, facDetail->getFaceIndex() );

		//Si se ha solicitado dar la vuelta a la faceta
        else if (this->pickAction == Ui.actionFlip_face)
		   ifs_flip_facet(path, facDetail->getFaceIndex() );

/* TODO: Activar esto
		else if (cdsviewer->pickAction == CdsViewer::PatchId)
		{
			//Facetas seleccionadas para el patch
			static int facetasPatch[3];

			//Salvamos la facetas seleccionada
			facetasPatch[numFacetasPatch++] = facDetail->getFaceIndex();

			//Mostramos las facetas seleccionadas
			M.asprintf("Parcheando facetas:");
			for (int i=0; i<numFacetasPatch; i++)
				M.append(S.asprintf(" %d", facetasPatch[i]));
			statusBar->message(M);
			statusBar->show();

			//Si tenemos 3, intentamos cerrar un hueco
			if (numFacetasPatch == 3)
			{
				numFacetasPatch=0;
				ifs_patch_hole(path, facetasPatch[0], facetasPatch[1], facetasPatch[2]);
			}


		}
//*/
	  }
      else

      //Si hemos pinchado sobre una linea, mostramos su informacion
      if (pickDetail->getTypeId() == SoLineDetail::getClassTypeId()) 
      {
        SoLineDetail *lineDetail = (SoLineDetail *) pickDetail;

        //Mostramos el indice de la faceta y la lista de vertices
	S.asprintf("Line index=%d; Vertex= [",lineDetail->getLineIndex());

        //Mostramos informacion sobre todos sus vertices
        const SoPointDetail *pointDetail0 = lineDetail->getPoint0();
        const SoPointDetail *pointDetail1 = lineDetail->getPoint1();
	S.append(QString::number(pointDetail0->getCoordinateIndex())+QString(", "));
	S.append(QString::number(pointDetail1->getCoordinateIndex())+QString("]") );

	M.append(QString(" <> ") + S);

	if (infoAction)
		global_mw->addMessage(S);

        SoMFVec3f coords;
        SoNode *nodeCoord = buscaCoordenadas (path, coords);

        if (nodeCoord)
        {
          const char *nombre_tipo = nodeCoord->getTypeId().getName();  
          const char *nombre_nodo = nodeCoord->getName().getString();

		  if (infoAction)
		  {
			  S.asprintf("%s coordinates node; Coordinate node name=%s", nombre_tipo, nombre_nodo);
			  global_mw->addMessage(S);

			  //Mostramos informacion sobre todos sus vertices
			  int idx = pointDetail0->getCoordinateIndex();
			  S.asprintf("Vertex %d = %10f %10f %10f", idx, coords[idx][0],coords[idx][1],coords[idx][2]);
			  idx = pointDetail1->getCoordinateIndex();
			  S.asprintf("Vertex %d = %10f %10f %10f", idx, coords[idx][0],coords[idx][1],coords[idx][2]);
			  global_mw->addMessage(S);
		  }

        }// if (nodeCoord)



      }
      else

      //Si hemos pinchado sobre un punto
      if (pickDetail->getTypeId() == SoPointDetail::getClassTypeId()) 
      {
        SoPointDetail *pointDetail = (SoPointDetail *) pickDetail;

        //Mostramos el indice del punto
        int idx = pointDetail->getCoordinateIndex();

		//Preparamos un mensaje para la consola de mensajes
		if (infoAction)
			S.asprintf("Point index=%d", idx);

		//Mostramos informacion en barra de status
		M.append(S.asprintf(" <> Point index=%d", idx));
        SoMFVec3f coords;
        SoNode *nodeCoord = buscaCoordenadas (path, coords);
        if (nodeCoord)
        {
          const char *nombre_tipo = nodeCoord->getTypeId().getName();  
          const char *nombre_nodo = nodeCoord->getName().getString();

		  //Mostramos informacion en barra de status
		  M.append(S.asprintf(" of %s node name=%s", nombre_tipo, nombre_nodo));

		  //Mejoramos el mensaje para la consola de mensajes
		  S.asprintf("%s node; Name=%s; Index=%d; XYZ = %10f %10f %10f",
			  nombre_tipo, nombre_nodo, idx, coords[idx][0],coords[idx][1],coords[idx][2]);
        }// if (nodeCoord)

		if (infoAction)
			global_mw->addMessage(S);

      }
      else

      //Si hemos pinchado sobre un texto
      if (pickDetail->getTypeId() == SoTextDetail::getClassTypeId()) 
      {
        SoTextDetail *textDetail = (SoTextDetail *) pickDetail;

        //Mostramos el indice del texto
		S.asprintf("Char index=[%d][%d]",textDetail->getStringIndex(),
                                        textDetail->getCharacterIndex());
		if (infoAction)
			global_mw->addMessage(S);

	    //Mostramos informacion en barra de status
		M.append(S.asprintf(" <> Char index=[%d][%d]",textDetail->getStringIndex(),
                                               textDetail->getCharacterIndex()));
      }
      else

      //Si hemos pinchado sobre un cubo
      if (pickDetail->getTypeId() == SoCubeDetail::getClassTypeId()) 
      {
        const char *parts[] = {"FRONT", "BACK", "LEFT", "RIGHT", "TOP", "BOTTOM"};
        SoCubeDetail *cubeDetail = (SoCubeDetail *) pickDetail;

        //Mostramos la cara pinchada
        int part = cubeDetail->getPart();
        S.asprintf("Face index = %d (%s)", part, parts[part]);
		if (infoAction)
			global_mw->addMessage(S);

		//Mostramos informacion en barra de status
		M.append(S.asprintf(" <> Face index = %d (%s)", part, parts[part]));
      }
      else

      {
         //qDebug(CdsViewer::tr("Tipo de nodo desconocido\n"));
      }

    }//if (pickDetail != NULL)

	//Mostramos el mensaje en la ventana del viewer
 	if (this->pickAction == Ui.actionInfo)
	   statusBar->showMessage(M);

    //Liberamos el path
    path->unref();

  }//if (mbe->getButton() == SoMouseButtonEvent::BUTTON2 &&

} //void pickAction (SoEventCallback * n)

