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

/*! Callback del evento raypick, que muestra información sobre el punto apuntado por el ratón.*/
void pick_cb (void *ud, SoEventCallback * n)
{
  //Pasa el evento al editor llamante
  assert(ud != NULL);
  CdsExaminerEditor *cdsviewer = (CdsExaminerEditor *)ud;
  cdsviewer->pickCallback(n);
}

  
/*! Callback del evento raypick, que muestra información sobre el punto apuntado por el ratón.*/
template <class SOTYPEVIEWER>
void CdsEditorTemplate<SOTYPEVIEWER>::pickCallback (SoEventCallback * n)
{
  const SoMouseButtonEvent * mbe = (SoMouseButtonEvent *)n->getEvent();
  int i;

  //Numero de facetas seleccionadas para parchear
  static int numFacetasPatch = 0;

 //Identificamos la ventana, el visor y la barra de status
 QStatusBar *statusBar = this->statusBar();

 //qDebug("%p->%s(%p) -> state=%d button=%d", this, __FUNCTION__, n, mbe->getState(),  mbe->getButton() ) ;

  //Miramos si queremos alguna opción de picado
 if (this->pickAction == Ui.actionNone)
	 return;

 bool infoAction = this->pickAction == Ui.actionInfo;

 //Comprobamos que se ha pulsado el boton izquierdo del ratón
 if (mbe->getState() == SoButtonEvent::DOWN && mbe->getButton() == SoMouseButtonEvent::BUTTON1 ) 
 {

    //Aseguramos que mark_sep no está colgado de myRoot, para evitar picar sobre la marca
    while (myRoot->findChild(mark_sep) > -1) 
       myRoot->removeChild(mark_sep);

    SoRayPickAction rp(this->getViewportRegion());
    rp.setPoint(mbe->getPosition());
    rp.apply(this->getSceneManager()->getSceneGraph());

	//Miramos si hemos pinchado sobre algun elemento
    SoPickedPoint * point = rp.getPickedPoint();
    if (point == NULL) 
	{
		statusBar->showMessage(tr("Nothing there."));

		//Reseteamos el contador de facetas
		numFacetasPatch=0;

		return;
    }
    n->setHandled();

    //Extraemos las coordenadas del objeto apuntado
    SbVec3f v = point->getPoint();

    //Actualizamos la posición de la marca
    mark_coord->point.setValue(v);

    //Aseguramos que mark_sep está colgado de myRoot
 	if (this->pickAction == Ui.actionInfo && myRoot->findChild(mark_sep) < 0) 
       myRoot->addChild(mark_sep);

	QString M, S;

    //Imprimimos las coordenadas del punto 
	S.sprintf("<b>Pick XYZ=</b> %f %f %f", v[0], v[1], v[2]);

    //Imprimimos las coordenadas del punto en la barra del estado y en consola de mensajes
	M.sprintf("XYZ= %f %f %f", v[0], v[1], v[2]);

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
	S.sprintf("<b>Node path=</b>");
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
		M += QString(" <> ") + tr("Name=") + QString(nombre_nodo);
		S.append(QString("; ") +tr("<b>Node Name=</b>") + QString(nombre_nodo));
	}

	if (infoAction)
		global_mw->addMessage(S);

    //Detalle del objeto apuntado
    const SoDetail *pickDetail = point->getDetail();
    if (pickDetail != NULL)
    {

      //Si hemos pinchado sobre una faceta, mostramos su informaciÃ³n
      if (pickDetail->getTypeId() == SoFaceDetail::getClassTypeId()) 
      {
        SoFaceDetail *facDetail = (SoFaceDetail *) pickDetail;

        //Mostramos el indice de la faceta y la lista de vertices
		S.sprintf("Face index=%d; Vertex list=",facDetail->getFaceIndex());

        //Mostramos informaciÃ³n sobre todos sus vertices
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
			  S.sprintf("%s coordinates node; Coordinate node name=%s", nombre_tipo, nombre_nodo);
			  global_mw->addMessage(S);

			  //Mostramos informacion sobre todos sus vertices
			  for (i=0; i < facDetail->getNumPoints(); i++)
			  {
				  int idx = facDetail->getPoint(i)->getCoordinateIndex();
				  S.sprintf("Vertex %d = %10f %10f %10f", idx, coords[idx][0],coords[idx][1],coords[idx][2]);
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
			M.sprintf("Parcheando facetas:");
			for (int i=0; i<numFacetasPatch; i++)
				M.append(S.sprintf(" %d", facetasPatch[i]));
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

      //Si hemos pinchado sobre un punto
      if (pickDetail->getTypeId() == SoPointDetail::getClassTypeId()) 
      {
        SoPointDetail *pointDetail = (SoPointDetail *) pickDetail;

        //Mostramos el indice del punto
        int idx = pointDetail->getCoordinateIndex();

		//Preparamos un mensaje para la consola de mensajes
		if (infoAction)
			S.sprintf("Point index=%d", idx);

		//Mostramos informacion en barra de status
		M.append(S.sprintf(" <> Point index=%d", idx));
        SoMFVec3f coords;
        SoNode *nodeCoord = buscaCoordenadas (path, coords);
        if (nodeCoord)
        {
          const char *nombre_tipo = nodeCoord->getTypeId().getName();  
          const char *nombre_nodo = nodeCoord->getName().getString();

		  //Mostramos informacion en barra de status
		  M.append(S.sprintf(" of %s node name=%s", nombre_tipo, nombre_nodo));

		  //Mejoramos el mensaje para la consola de mensajes
		  S.sprintf("%s node; Name=%s; Index=%d; XYZ = %10f %10f %10f",
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
		S.sprintf("Char index=[%d][%d]",textDetail->getStringIndex(),
                                        textDetail->getCharacterIndex());
		if (infoAction)
			global_mw->addMessage(S);

	    //Mostramos informacion en barra de status
		M.append(S.sprintf(" <> Char index=[%d][%d]",textDetail->getStringIndex(),
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
        S.sprintf("Face index = %d (%s)", part, parts[part]);
		if (infoAction)
			global_mw->addMessage(S);

		//Mostramos informacion en barra de status
		M.append(S.sprintf(" <> Face index = %d (%s)", part, parts[part]));
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

