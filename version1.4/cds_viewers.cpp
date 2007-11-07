
#include "cds_viewers.h"
#include "cds_util.h"
#include "form1.h"

#include <Inventor/details/SoDetails.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/nodes/SoNodes.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/events/SoEvents.h>

#include <qstatusbar.h>
#include <qlistview.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qcolordialog.h>

//Variables globales externas
extern SoSeparator *root;
extern SoSeparator *mark_sep;
extern SoCoordinate3 *mark_coord3;

extern Form1 *form1;
extern std::map<QListViewItem*,SoNode*>mapQTCOIN;
extern SbColor bgColor_viewer;

/*! Callback del evento raypick, que muestra informaciÛn sobre
    el punto apuntado por el ratÛn.*/
static void pick_cb (void *ud, SoEventCallback * n)
{
  const SoMouseButtonEvent * mbe = (SoMouseButtonEvent *)n->getEvent();
  int i;

  //Numero de facetas seleccionadas para parchear
  static int numFacetasPatch = 0;

 //Identificamos la ventana, el visor y la barra de status
 CdsViewer *cdsviewer = (CdsViewer *)ud;
 SoRenderArea *viewer = cdsviewer->viewer;
 QStatusBar *statusBar = cdsviewer->statusBar();

  //Miramos si queremos alguna opciÛn de picado
 if (cdsviewer->accionPicado == CdsViewer::NoneId)
	 return;

 //Comprobamos que se ha pulsado el boton izquierdo del ratÛn
 if ( mbe->getState() == SoButtonEvent::DOWN && mbe->getButton() == SoMouseButtonEvent::BUTTON1 ) 
  {

    //Aseguramos que mark_sep no est· colgado de root, para evitar picar sobre la marca
    while (root->findChild(mark_sep) > -1) 
       root->removeChild(mark_sep);

    SoRayPickAction rp(viewer->getViewportRegion());
    rp.setPoint(mbe->getPosition());
    rp.apply(viewer->getSceneManager()->getSceneGraph());

	//Miramos si hemos pinchado sobre algun elemento
    SoPickedPoint * point = rp.getPickedPoint();
    if (point == NULL) 
	{
		statusBar->message(CdsViewer::tr("Nada interesante..."));

		//Reseteamos el contador de facetas
		numFacetasPatch=0;

		return;
    }
    n->setHandled();

    //Extraemos las coordenadas del objeto apuntado
    SbVec3f v = point->getPoint();

    //Actualizamos la posiciÛn de la marca
    mark_coord3->point.setValue(v);

    //Aseguramos que mark_sep est· colgado de root
	if (cdsviewer->accionPicado == CdsViewer::InfoId && root->findChild(mark_sep) < 0) 
       root->addChild(mark_sep);

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

    //Mostramos el path, salvo el √∫ltimo nodo
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
		M += QString(" <> ") + CdsViewer::tr("Nombre=") + QString(nombre_nodo);

    fprintf(stdout, "%s; Name=%s\n", nombre_tipo, nombre_nodo);

    //Detalle del objeto apuntado
    const SoDetail *pickDetail = point->getDetail();
    if (pickDetail != NULL)
    {

      //Si hemos pinchado sobre una faceta, mostramos su informaci√≥n
      if (pickDetail->getTypeId() == SoFaceDetail::getClassTypeId()) 
      {
        SoFaceDetail *facDetail = (SoFaceDetail *) pickDetail;
        //Mostramos el indice de la faceta
        fprintf(stdout,"Face index=%d; Vertex list=",facDetail->getFaceIndex());
		M.append(S.sprintf(" <> Face index=%d <> Vertex =",facDetail->getFaceIndex()));

        //Mostramos informaci√≥n sobre todos sus vertices
        for (i=0; i < facDetail->getNumPoints(); i++)
        {
          const SoPointDetail *pointDetail = facDetail->getPoint(i);
          fprintf(stdout, " %d", pointDetail->getCoordinateIndex());
		  M.append(S.sprintf(" %d", pointDetail->getCoordinateIndex()));
        }//for

        fprintf(stdout, "\n");

        SoMFVec3f coords;
        SoNode *nodeCoord = buscaCoordenadas (path, coords);

        if (nodeCoord)
        {
          const char *nombre_tipo = nodeCoord->getTypeId().getName();  
          const char *nombre_nodo = nodeCoord->getName().getString();
          fprintf(stdout, "%s coordinate node; Name=%s\n", nombre_tipo, 
			  nombre_nodo);

          //Mostramos informaci√≥n sobre todos sus vertices
          for (i=0; i < facDetail->getNumPoints(); i++)
          {
            int idx = facDetail->getPoint(i)->getCoordinateIndex();
            fprintf(stdout, "%d = ", idx);
            coords[idx].print(stdout);
            fprintf(stdout, "\n");
            
          }//for

        }// if (nodeCoord)


	    //Si se ha solicitado eliminar la faceta
        if (cdsviewer->accionPicado == CdsViewer::DeleteId)
		   ifs_remove_facet(path, facDetail->getFaceIndex() );

		//Si se ha solicitado dar la vuelta a la faceta
        else if (cdsviewer->accionPicado == CdsViewer::FlipId)
		   ifs_flip_facet(path, facDetail->getFaceIndex() );

		else if (cdsviewer->accionPicado == CdsViewer::PatchId)
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

		/*
		//TEST:Hinchamos la faceta
		SoMFVec3f normals_face;
		SoMFVec3f normals_vertex;
		ifs_normals(path, normals_face, normals_vertex);
		//A cada vertice de la faceta, le sumamos la normal de la faceta, o del vertice
                for (i=0; i < facDetail->getNumPoints(); i++)
                {
                    int idx = facDetail->getPoint(i)->getCoordinateIndex();
                    SbVec3f v = coords[idx];
                    //v += normals_face[facDetail->getFaceIndex()];
                    v += normals_vertex[idx];
                    ((SoCoordinate3 *)nodeCoord)->point.set1Value(idx, v);
                }//for
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
    if (cdsviewer->accionPicado == CdsViewer::InfoId)
	   statusBar->message(M);

    //Liberamos el path
    path->unref();

  }//if (mbe->getButton() == SoMouseButtonEvent::BUTTON2 &&

} // static void pick_cb (void *ud, SoEventCallback * n)



CdsExaminerViewer::CdsExaminerViewer(CdsViewer * pParent) : 
#ifdef _WIN32
                      SoWinExaminerViewer(pParent->centralWidget()->winId(), NULL, true ) 
#else
                      SoQtExaminerViewer(pParent->centralWidget(), NULL, true )
#endif
 {
	 //Salvamos referencia al widget contenedor
	 parent = pParent;

	 //Color de fondo
	 this->setBackgroundColor(bgColor_viewer);

	 //Escena por defecto
	 this->setSceneGraph (root);

 } //Constructor

SbBool CdsExaminerViewer::processSoEvent (const SoEvent *const event)
  {
     //En Unix la tecla Q mata el bucle de eventos
     if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::Q))
     {
	     //printf("Eliminando widget %p\n", parent);
	     parent->close(false);
         return true;
     }

	 if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::NUMBER_0))
     {
		 parent->cambiarPicado(CdsViewer::NoneId);
		 return true;
     }

	 if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::NUMBER_1))
     {
	     parent->cambiarPicado(CdsViewer::InfoId);
		 return true;
     }

	 if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::NUMBER_2))
     {
	     parent->cambiarPicado(CdsViewer::DeleteId);
		 return true;
     }

	 if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::NUMBER_3))
     {
	     parent->cambiarPicado(CdsViewer::FlipId);
		 return true;
     }

	 if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::NUMBER_4))
     {
		 parent->cambiarPicado(CdsViewer::PatchId);
		 return true;
     }

     return SoExaminerViewer::processSoEvent(event);
  }


CdsFlyViewer::CdsFlyViewer(CdsViewer * pParent = 0) : 
#ifdef _WIN32
                      SoWinFlyViewer(pParent->centralWidget()->winId(), NULL, true ) 
#else
                      SoQtFlyViewer(pParent->centralWidget(), NULL, true )
#endif
 {
	 //Salvamos referencia al widget contenedor
	 parent = pParent;

	 //Color de fondo
	 this->setBackgroundColor(bgColor_viewer);

	 //Escena por defecto
	 this->setSceneGraph (root);

 } //Constructor

SbBool CdsFlyViewer::processSoEvent (const SoEvent *const event)
  {
     //En Unix la tecla Q mata el bucle de eventos
     if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::Q))
     {
	     //printf("Eliminando widget %p\n", parent);
	     parent->close(false);
         return true;
     }

     return SoFlyViewer::processSoEvent(event);
  }


CdsPlaneViewer::CdsPlaneViewer(CdsViewer * pParent = 0) : 
#ifdef _WIN32
                      SoWinPlaneViewer(pParent->centralWidget()->winId(), NULL, true ) 
#else
                      SoQtPlaneViewer(pParent->centralWidget(), NULL, true )
#endif
 {
	 //Salvamos referencia al widget contenedor
	 parent = pParent;

	 //Color de fondo
	 this->setBackgroundColor(bgColor_viewer);

	 //Escena por defecto
	 this->setSceneGraph (root);

 } //Constructor

SbBool CdsPlaneViewer::processSoEvent (const SoEvent *const event)
  {
     //En Unix la tecla Q mata el bucle de eventos
     if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::Q))
     {
	     //printf("Eliminando widget %p\n", parent);
	     parent->close(false);
         return true;
     }

     return SoPlaneViewer::processSoEvent(event);
  }

static void pick_cb (void *ud, SoEventCallback * n);


//Constructor
CdsViewer::CdsViewer (SoType tipoViewer)
	{
		//Creamos un widget contenedor para el viewer
       QWidget *qWindow = new QWidget(this, "Viewer");
       this->setCentralWidget( qWindow );

	   //Identificamos el tipo de viewer
	   if (tipoViewer == CdsExaminerViewer::getClassTypeId())
	   {
		  viewer = new CdsExaminerViewer (this);
          this->setCaption( "Coindesigner - ExaminerViewer" );
	   }
	   else if (tipoViewer == CdsFlyViewer::getClassTypeId())
	   {
		  viewer = new CdsFlyViewer (this);
          this->setCaption( "Coindesigner - FlyViewer" );
	   }
	   else if (tipoViewer == CdsPlaneViewer::getClassTypeId())
	   {
		  viewer = new CdsPlaneViewer (this);
          this->setCaption( "Coindesigner - ExaminerViewer" );
	   }
	   else if (tipoViewer == SoRenderArea::getClassTypeId())
	   {
		  //RenderArea no tiene una clase propia en coindesigner
          #ifdef _WIN32
          viewer = new SoWinRenderArea (qWindow->winId());
          #else
          viewer = new SoQtRenderArea (qWindow);
          #endif

		  //Configuramos color de fondo y escena
          viewer->setBackgroundColor(bgColor_viewer);
          viewer->setSceneGraph (root);

		  this->setCaption( "Coindesigner - RenderArea" );
	   }

	   //Conectamos la seÒal de cierre del widget con nuestra seÒal de cierre
	   //connect(qWindow, SIGNAL(destroyed(QObject *)), this, SLOT(close()));

	   //AÒadimos soporte para pick
	   if (tipoViewer != SoRenderArea::getClassTypeId())
	   {
		  SoEventCallback * ecb = new SoEventCallback;
		  ecb->addEventCallback(SoMouseButtonEvent::getClassTypeId(), pick_cb, this);
		  ((SoSeparator *)viewer->getSceneManager()->getSceneGraph())->addChild(ecb);

	     //Configuramos el submenu para seleccionar la accion de picado
	     pickMenu.setCheckable(true);
	     pickMenu.insertItem(tr("Ninguna"), this, SLOT(cambiarPicado(int)), 0, NoneId);
	     pickMenu.insertItem(tr("Ver info"), this, SLOT(cambiarPicado(int)), 0, InfoId);
	     pickMenu.insertItem(tr("Borrar faceta"), this, SLOT(cambiarPicado(int)), 0, DeleteId);
	     pickMenu.insertItem(tr("Cambiar orientaciÛn faceta"), this, SLOT(cambiarPicado(int)), 0, FlipId);
		 pickMenu.insertItem(tr("Parchear hueco"), this, SLOT(cambiarPicado(int)), 0, PatchId);

	     //Seleccionamos por defecto la opciÛn de picado
	     this->accionPicado = InfoId;
	     pickMenu.setItemChecked(this->accionPicado, true);

         //AÒadimos el submenu al menu principal
	     menuBar()->insertItem(tr("Acciones picado"), &pickMenu);
	   }

	   //Configuramos el submenu para seleccionar opciones de visualizaciÛn
	   viewMenu.setCheckable(true);
	   viewMenu.insertItem(tr("Antialias OFF"), this, SLOT(cambiarAntialias(int)), 0, AAlias0);
	   viewMenu.insertItem(tr("Antialias x3"), this, SLOT(cambiarAntialias(int)), 0, AAlias3);
	   viewMenu.insertItem(tr("Antialias x5"), this, SLOT(cambiarAntialias(int)), 0, AAlias5);
	   viewMenu.insertItem(QPixmap::fromMimeSource("colorize.png"),
	                       tr("Color de fondo"), this, SLOT(cambiarColor()), 0, BGColorId);

	   //Seleccionamos por defecto la opciÛn de Antialias
	   this->AAliasLevel = AAlias0;
	   viewMenu.setItemChecked(this->AAliasLevel, true);

	   //OpciÛn del sistema de coordenadas para el examinerViewer
	   if (tipoViewer.isDerivedFrom(SoExaminerViewer::getClassTypeId()))
	   {
		 viewMenu.insertItem(tr("Sistema de Coordenadas"), this, SLOT(cambiarFeedbackVisibility(int)), 0, FeedbackId);
	   }

       //AÒadimos el submenu al menu principal
	   menuBar()->insertItem(tr("Ver"), &viewMenu);

	   //Cambiamos el tamaÒo por defecto de la ventana
	   this->resize(600, 450);

	   //Nos mostramos
	   this->show();
}


//Cambia el color de fondo del visualizador
void CdsViewer::cambiarColor()
{
   SbColor col = viewer->getBackgroundColor();
   QColor c( int(255*col[0]), int(255*col[1]), int(255*col[2]) );

   //Solicitamos un color mediante QColorDialog
   c=QColorDialog::getColor(c);
   if (c.isValid() )
   {           
       //Modificamos el field
       col.setValue(c.red()/255.0, c.green()/255.0, c.blue()/255.0);
	   viewer->setBackgroundColor(col);
   }

}

//Cambia el visualizador del sistema de coordenadas
void CdsViewer::cambiarFeedbackVisibility(int id)
{

	if (viewer->getTypeId().isDerivedFrom(SoExaminerViewer::getClassTypeId()))
	{
		//Leemos el estado actual del sistema de coordenadas
		SoExaminerViewer *examViewer = (SoExaminerViewer *)viewer;
		bool fv = examViewer->isFeedbackVisible();

		//Invertimos su valor
		examViewer->setFeedbackVisibility(!fv);

		//Actualizamos el menu
	    viewMenu.setItemChecked(id, !fv);
	}

}


//Cambia el efecto antialias del visualizador
void CdsViewer::cambiarAntialias(int level)
{
	//Cambiamos el menu
	viewMenu.setItemChecked(AAliasLevel, false);
	viewMenu.setItemChecked(level, true);

	AAliasLevel = level;

	switch (AAliasLevel)
	{
	case AAlias0: { viewer->setAntialiasing(false, 0); break;}
	case AAlias3: { viewer->setAntialiasing(true, 3); break;}
	case AAlias5: { viewer->setAntialiasing(true, 5); break;}
	}
}


//Cambia el efecto de la accion de picado
void CdsViewer::cambiarPicado(int id)
{
	//Cambiamos el menu
	pickMenu.setItemChecked(accionPicado, false);
	pickMenu.setItemChecked(id, true);

	//Salvamos la escogida
	accionPicado = id;

	if (accionPicado == InfoId)
	{
		this->statusBar()->show();
	}
	else
	{
		//Escondemos la barra de status
		this->statusBar()->hide();

		//Eliminamos la marca de la escena
	    while (root->findChild(mark_sep) > -1) 
			root->removeChild(mark_sep);
	}

}
