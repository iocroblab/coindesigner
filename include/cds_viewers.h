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

#ifndef __CDS_VIEW_H__
#define __CDS_VIEW_H__

#include "ui_cds_editor.h"

//SoQt includes
#include <QWidget>
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/Qt/viewers/SoQtFlyViewer.h>
#include <Inventor/Qt/viewers/SoQtPlaneViewer.h>
#include <Inventor/nodes/SoNodes.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/events/SoEvents.h>


void pick_cb (void *ud, SoEventCallback * n);

/*! @brief Template class for creating noQuitxxxxxViewer objects 
    Template class for creating noQuitxxxxxViewer objects that override the 
    default behaviour exitMainLoop() when user press "Q" key. Instead, it will
    send a close signal to a widget.
*/
template <class SOTYPEVIEWER>
class CdsNoQuitTemplate : public SOTYPEVIEWER
{
	QWidget *closeWidget;
public :
	/*! @brief Constructor 
	    @param parent: Parent widget (container) for the SoExaminerViewer
	    @param _closeWidget: Widget which will receive the close() signal (if any)
	    @param root: Default sceneGraph for this object
	*/
	CdsNoQuitTemplate(QWidget *parent=NULL, QWidget *_closeWidget=NULL, SoNode *root=NULL) :
	   SOTYPEVIEWER(parent, NULL, true )
	   {
		   //Salvamos el padre
		   if (_closeWidget)
				closeWidget = _closeWidget;
		   else
				closeWidget = parent;

		   if (root)
			   SOTYPEVIEWER::setSceneGraph(root);
	   }

	   ///Destructor. Automatically unrefs the sceneGraph
	   ~CdsNoQuitTemplate()
	   {
		   //Unref the scene
		   SOTYPEVIEWER::setSceneGraph(NULL);
	   }

	   ///Local event processer. Override default behaviour for Q key
	   SbBool processSoEvent (const SoEvent *const event)
	   {
		   //En Unix la tecla Q mata el bucle de eventos
		   if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::Q))
		   {
			   //Unref the scene
			   SOTYPEVIEWER::setSceneGraph(NULL);

			   //printf("Eliminando widget %p\n", closeWidget);
			   if (closeWidget)
				   closeWidget->close();

			   return true;
		   }

		   return SOTYPEVIEWER::processSoEvent(event);

	   }//SbBool processSoEvent (const SoEvent *const event)

};//class CdsNoQuitTemplate


///Base class for every visual editor with a UI created with Qt Designer
class cds_editor : public QMainWindow
{
	Q_OBJECT
	
public:

	Ui::cds_editor Ui;

	//Comportamiento del visualizador ante el picado
	QAction *pickAction;

	///Separador intermedio
	SoSeparator *myRoot;

	///Separador para las marcas
	SoSeparator *mark_sep;

	///Constructor
	cds_editor (QWidget *p=0, Qt::WindowFlags f=0) : QMainWindow(p, f)
	{
		//Inicializamos el UI
		Ui.setupUi(this);

		//Inicializa la accion de picado por defecto
		pickAction = Ui.actionNone;
		this->cambiarPicado(Ui.actionInfo);

		//Conecta las acciones del menu
        connect(Ui.actionNone, SIGNAL(triggered()), this, SLOT(on_actionChanged()));
        connect(Ui.actionInfo, SIGNAL(triggered()), this, SLOT(on_actionChanged()));
        connect(Ui.actionRemove_face, SIGNAL(triggered()), this, SLOT(on_actionChanged()));

		myRoot = NULL;
		mark_sep = NULL;

	} //Constructor

public slots:
	void on_actionChanged()
	{
		cambiarPicado(qobject_cast<QAction *>(sender()));
	}

	///Cambia el efecto de la accion de picado
    void cambiarPicado(QAction *id)
	{
		//Cambiamos el menu
		pickAction->setChecked(false);
		id->setChecked(true);
		pickAction = id;

		//Miramos si debemos desactivar las marcas
		if (myRoot!=NULL && mark_sep!=NULL && pickAction != Ui.actionInfo)
		{
			//Eliminamos la marca de la escena
	    	while (myRoot->findChild(mark_sep) > -1) 
				myRoot->removeChild(mark_sep);
		}
	}

};//class cds_editor : public QMainWindow



///Template para todos los editores, 
template <class SOTYPEVIEWER>
class CdsEditorTemplate : public cds_editor, SOTYPEVIEWER
{
	///Posiciones de las marcas
	SoCoordinate3 *mark_coord;

public :
	/*! @brief Constructor de la clase
	@param root: Default sceneGraph for this object
	*/
	CdsEditorTemplate(SoNode *root=NULL) : cds_editor(), SOTYPEVIEWER(this->centralWidget(), NULL, true )
	{
		//Inicializamos nuestro root privado
		myRoot = new SoSeparator();

		//Creacion de la estructura donde colgaremos las marcas de puntos pinchados (raypick)
		mark_sep = new SoSeparator;
		mark_sep->addChild(new SoResetTransform() );
		SoBaseColor *mark_color = new SoBaseColor;
		mark_color->rgb.setValue (1.0f, 0.0f, 0.0f);
		mark_sep->addChild(mark_color);
		SoDrawStyle *mark_style = new SoDrawStyle;
		mark_style->pointSize = 10;
		mark_sep->addChild(mark_style);
		mark_coord = new SoCoordinate3;
		mark_coord->point.setValue(0,0,0);
		mark_sep->addChild(mark_coord);
		SoMarkerSet *markerSet = new SoMarkerSet;
		markerSet->markerIndex.setValue(SoMarkerSet::CIRCLE_FILLED_9_9);
		mark_sep->addChild(markerSet);
		mark_sep->ref();

		//Si nos han pasado un nodo root, lo colgamos
		if (root)
			myRoot->addChild(root);

		SOTYPEVIEWER::setSceneGraph(myRoot);

		//Conecta la señal de cerrado de la ventana con unref, para liberar memoria
		connect(this, SIGNAL(close), this, SLOT(unref));

		//Soporte para pick (picado con el raton)
		SoEventCallback * ecb = new SoEventCallback;
		ecb->addEventCallback(SoMouseButtonEvent::getClassTypeId(), pick_cb, this);
		((SoSeparator *)SOTYPEVIEWER::getSceneManager()->getSceneGraph())->addChild(ecb);

	}// CdsEditorTemplate(SoNode *root=NULL) : cds_editor(), SOTYPEVIEWER(this->centralWidget(), NULL, true )

	~CdsEditorTemplate()
	{
		//Unrefs
		mark_sep->unref();
		SOTYPEVIEWER::setSceneGraph(NULL);
	}

	///Modifica la escena a mostrar en el editor
	void setSceneGraph(SoSeparator *node)
	{
		myRoot->removeAllChildren();
		myRoot->addChild(node);
	}

	///Show visual marks
	void showMarks()
	{
		if (myRoot->findChild(mark_sep) < 0)
			myRoot->addChild(mark_sep);
	}

	///Hide visual marks
	void hideMarks()
	{
		myRoot->removeChild (mark_sep);
	}

	///Shows the window
	void show()
	{
		SOTYPEVIEWER::show();
		cds_editor::show();
	}

	///Local event processer. Override default behaviour for Q key
	SbBool processSoEvent (const SoEvent *const event)
	{
		//En Unix la tecla Q mata el bucle de eventos
		if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::Q))
		{
			//printf("Eliminando widget %p\n", parent);
			cds_editor::close();
			return true;
		}

		//Pasamos el evento a la clase padre
		return SOTYPEVIEWER::processSoEvent(event);

	}//SbBool processSoEvent (const SoEvent *const event)

	//Callback de pickAction, llamada a traves del pick_cb
    void pickCallback (SoEventCallback * n);

}; // class CdsEditorTemplate : public cds_editor, SOTYPEVIEWER;


//Instanciacion para los principales visores de SoQt
typedef CdsNoQuitTemplate<SoQtExaminerViewer> NoQuitExaminerViewer;
typedef CdsNoQuitTemplate<SoQtFlyViewer> NoQuitFlyViewer;
typedef CdsNoQuitTemplate<SoQtPlaneViewer> NoQuitPlaneViewer;
typedef CdsNoQuitTemplate<SoQtRenderArea> NoQuitRenderArea;

//Instanciacion para los principales editores 
typedef CdsEditorTemplate<SoQtExaminerViewer> CdsExaminerEditor;
typedef CdsEditorTemplate<SoQtFlyViewer> CdsFlyEditor;
typedef CdsEditorTemplate<SoQtPlaneViewer> CdsPlaneEditor;

#endif
