
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

/*! Callback del evento raypick, que muestra información sobre el punto apuntado por el ratón.*/
static void pick_cb (void *ud, SoEventCallback * n)
{

	{ ............ }

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

	   //Conectamos la señal de cierre del widget con nuestra señal de cierre
	   //connect(qWindow, SIGNAL(destroyed(QObject *)), this, SLOT(close()));

	   //Añadimos soporte para pick
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
	     pickMenu.insertItem(tr("Cambiar orientación faceta"), this, SLOT(cambiarPicado(int)), 0, FlipId);
		 pickMenu.insertItem(tr("Parchear hueco"), this, SLOT(cambiarPicado(int)), 0, PatchId);

	     //Seleccionamos por defecto la opción de picado
	     this->accionPicado = InfoId;
	     pickMenu.setItemChecked(this->accionPicado, true);

         //Añadimos el submenu al menu principal
	     menuBar()->insertItem(tr("Acciones picado"), &pickMenu);
	   }

	   //Configuramos el submenu para seleccionar opciones de visualización
	   viewMenu.setCheckable(true);
	   viewMenu.insertItem(tr("Antialias OFF"), this, SLOT(cambiarAntialias(int)), 0, AAlias0);
	   viewMenu.insertItem(tr("Antialias x3"), this, SLOT(cambiarAntialias(int)), 0, AAlias3);
	   viewMenu.insertItem(tr("Antialias x5"), this, SLOT(cambiarAntialias(int)), 0, AAlias5);
	   viewMenu.insertItem(QPixmap::fromMimeSource("colorize.png"),
	                       tr("Color de fondo"), this, SLOT(cambiarColor()), 0, BGColorId);

	   //Seleccionamos por defecto la opción de Antialias
	   this->AAliasLevel = AAlias0;
	   viewMenu.setItemChecked(this->AAliasLevel, true);

	   //Opción del sistema de coordenadas para el examinerViewer
	   if (tipoViewer.isDerivedFrom(SoExaminerViewer::getClassTypeId()))
	   {
		 viewMenu.insertItem(tr("Sistema de Coordenadas"), this, SLOT(cambiarFeedbackVisibility(int)), 0, FeedbackId);
	   }

       //Añadimos el submenu al menu principal
	   menuBar()->insertItem(tr("Ver"), &viewMenu);

	   //Cambiamos el tamaño por defecto de la ventana
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
