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

 qDebug("%p->%s(%p) -> state=%d button=%d\n", this, __FUNCTION__, n, mbe->getState(),  mbe->getButton() ) ;

  //Miramos si queremos alguna opción de picado
 if (this->pickAction == Ui.actionNone)
	 return;

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

    //Imprimimos las coordenadas del punto 
    fprintf(stdout, "\nPoint XYZ= %f %f %f\n", v[0], v[1], v[2]);

    //Imprimimos las coordenadas del punto en la barra del mensaje
	QString M, S;
	M.sprintf("XYZ= %f %f %f", v[0], v[1], v[2]);

    //Path del objeto apuntado
    SoPath *path = point->getPath()->copy(2,0);
    path->ref();

    //SoNode del objeto apuntado
    SoNode *nodo = path->getTail();

/* TODO: Activar todo esto y modificar para que compile
    //Buscamos el nodo en el mapa mapQTCOIN
    std::map<QListViewItem*,SoNode*>::iterator map_it;
    for (map_it=mapQTCOIN.begin(); map_it != mapQTCOIN.end(); map_it++)
    {
       //Si este elemento corresponde al nodo
       if (map_it->second == nodo)
       {
            if (form1 != NULL)
            {
              //Seleccionamos el nodo y aseguramos que es visible
              form1->sceneTree->setCurrentItem(map_it->first);
              form1->sceneTree->ensureItemVisible(map_it->first);
            }

            //Salimos del bucle de busqueda
            break;
       } //if (map_it->second == nodo)
    }//for
*/

    //Mostramos el path, salvo el Ãºltimo nodo
    for (i=0; i < path->getLength()-1; i++)
    {
      const char *nombre_tipo = path->getNode(i)->getTypeId().getName(); 
      fprintf(stdout, "%s[%d]-> ", nombre_tipo, path->getIndex(i+1) );
    }

    //Averiguamos el tipo del nodo
    SoType tipo = nodo->getTypeId();
    const char *nombre_tipo = tipo.getName();  

	//Mostramos el tipo en barra de status
	M += QString(" <> ") + QString(nombre_tipo);

	//Averiguamos el nombre del nodo
	const char *nombre_nodo = nodo->getName().getString();

	//Mostramos el nombre en barra de status
	if (nombre_nodo && strlen(nombre_nodo) > 0)
		M += QString(" <> ") + tr("Nombre=") + QString(nombre_nodo);

    fprintf(stdout, "%s; Name=%s\n", nombre_tipo, nombre_nodo);

    //Detalle del objeto apuntado
    const SoDetail *pickDetail = point->getDetail();
    if (pickDetail != NULL)
    {

      //Si hemos pinchado sobre una faceta, mostramos su informaciÃ³n
      if (pickDetail->getTypeId() == SoFaceDetail::getClassTypeId()) 
      {
        SoFaceDetail *facDetail = (SoFaceDetail *) pickDetail;
        //Mostramos el indice de la faceta
        fprintf(stdout,"Face index=%d; Vertex list=",facDetail->getFaceIndex());
		M.append(S.sprintf(" <> Face index=%d <> Vertex =",facDetail->getFaceIndex()));

        //Mostramos informaciÃ³n sobre todos sus vertices
        for (i=0; i < facDetail->getNumPoints(); i++)
        {
          const SoPointDetail *pointDetail = facDetail->getPoint(i);
          fprintf(stdout, " %d", pointDetail->getCoordinateIndex());
		  M.append(S.sprintf(" %d", pointDetail->getCoordinateIndex()));
        }//for

        fprintf(stdout, "\n");

/* TODO: Activar esto
        SoMFVec3f coords;
        SoNode *nodeCoord = buscaCoordenadas (path, coords);

        if (nodeCoord)
        {
          const char *nombre_tipo = nodeCoord->getTypeId().getName();  
          const char *nombre_nodo = nodeCoord->getName().getString();
          fprintf(stdout, "%s coordinate node; Name=%s\n", nombre_tipo, 
			  nombre_nodo);

          //Mostramos informaciÃ³n sobre todos sus vertices
          for (i=0; i < facDetail->getNumPoints(); i++)
          {
            int idx = facDetail->getPoint(i)->getCoordinateIndex();
            fprintf(stdout, "%d = ", idx);
            coords[idx].print(stdout);
            fprintf(stdout, "\n");
            
          }//for

        }// if (nodeCoord)


	    //Si se ha solicitado eliminar la faceta
        if (cdsviewer->pickAction == CdsViewer::DeleteId)
		   ifs_remove_facet(path, facDetail->getFaceIndex() );

		//Si se ha solicitado dar la vuelta a la faceta
        else if (cdsviewer->pickAction == CdsViewer::FlipId)
		   ifs_flip_facet(path, facDetail->getFaceIndex() );

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
        fprintf(stdout,"Point index=%d\n", idx);

		//Mostramos informacion en barra de status
		M.append(S.sprintf(" <> Point index=%d", idx));
/* TODO: ACtivar esto
        SoMFVec3f coords;
        SoNode *nodeCoord = buscaCoordenadas (path, coords);
        if (nodeCoord)
        {
          const char *nombre_tipo = nodeCoord->getTypeId().getName();  
          const char *nombre_nodo = nodeCoord->getName().getString();
          fprintf(stdout, "%s node; Name=%s; Center=", nombre_tipo, nombre_nodo);

		  //Mostramos informacion en barra de status
		  M.append(S.sprintf(" of %s node name=%s", nombre_tipo, nombre_nodo));

		  centroid(coords).print(stdout);
          fprintf(stdout, "\n");

          fprintf(stdout, "Vertex %d = ", idx);
          coords[idx].print(stdout);
          fprintf(stdout, "\n");


        }// if (nodeCoord)
*/
      }
      else

      //Si hemos pinchado sobre un texto
      if (pickDetail->getTypeId() == SoTextDetail::getClassTypeId()) 
      {
        SoTextDetail *textDetail = (SoTextDetail *) pickDetail;

        //Mostramos el indice del texto
        fprintf(stdout,"Char index=[%d][%d]\n",textDetail->getStringIndex(),
                                               textDetail->getCharacterIndex());
	    //Mostramos informacion en barra de status
		M.append(S.sprintf(" <> Char index=[%d][%d]",textDetail->getStringIndex(),
                                               textDetail->getCharacterIndex()));



      }
      else

      //Si hemos pinchado sobre un cubo
      if (pickDetail->getTypeId() == SoCubeDetail::getClassTypeId()) 
      {
        char *parts[] = {"FRONT", "BACK", "LEFT", "RIGHT", "TOP", "BOTTOM"};
        SoCubeDetail *cubeDetail = (SoCubeDetail *) pickDetail;

        //Mostramos la cara pinchada
        int part = cubeDetail->getPart();
        fprintf(stdout,"Face index = %d (%s)\n", part, parts[part]);

		//Mostramos informacion en barra de status
		M.append(S.sprintf(" <> Face index = %d (%s)", part, parts[part]));

      }
      else

      {
         //fprintf(stdout, CdsViewer::tr("Tipo de nodo desconocido\n"));
      }

    }//if (pickDetail != NULL)

	//Mostramos el mensaje en la ventana del viewer
 	if (this->pickAction == Ui.actionInfo)
	   statusBar->showMessage(M);

    //Liberamos el path
    path->unref();

  }//if (mbe->getButton() == SoMouseButtonEvent::BUTTON2 &&

} //void pickAction (SoEventCallback * n)

