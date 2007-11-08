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

#include "ui_cds_view.h"

//SoQt includes
#include <QWidget>
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/Qt/viewers/SoQtFlyViewer.h>
#include <Inventor/Qt/viewers/SoQtPlaneViewer.h>
#include <Inventor/nodes/SoNodes.h>


//Visor que evita terminar el bucle de eventos al pulsar Q. 
template <class SOTYPEVIEWER>
class CdsNoQuitTemplate : public SOTYPEVIEWER
{
	QWidget *closeWidget;
public :
	/*! @brief Constructor de la clase
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

	   SbBool processSoEvent (const SoEvent *const event)
	   {
		   //En Unix la tecla Q mata el bucle de eventos
		   if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::Q))
		   {
			   //printf("Eliminando widget %p\n", closeWidget);
			   if (closeWidget)
				   closeWidget->close();

			   return true;
		   }

		   return SOTYPEVIEWER::processSoEvent(event);

	   }//SbBool processSoEvent (const SoEvent *const event)

};//class CdsNoQuitTemplate


//Clase base para todos los editores, Ui diseñado en designer
class cds_editor : public QMainWindow
{
	Q_OBJECT
	Ui::cds_view Ui;
public:

	///Constructor
	cds_editor (QWidget *p=0, Qt::WindowFlags f=0) : QMainWindow(p, f)
	{
		Ui.setupUi(this);
	} //Constructor
};


///Template para todos los editores, 
template <class SOTYPEVIEWER>
class CdsEditorTemplate : public cds_editor, SOTYPEVIEWER
{
	///Separador intermedio
	SoSeparator *myRoot;

	///Separador para las marcas
	SoSeparator *mark_sep;

public:

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
	}

	~CdsEditorTemplate()
	{
		mark_sep->unref();
	}
	///Modifica la escena a mostrar en el editor
	void setSceneGraph(SoSeparator *node)
	{
		myRoot->removeAllChildren();
		myRoot->addChild(node);
	}

	///Muestra las marcas
	void showMarks()
	{
		if (myRoot->findChild(mark_sep) < 0)
			myRoot->addChild(mark_sep);
	}

	///Oculta las marcas
	void hideMarks()
	{
		myRoot->removeChild (mark_sep);
	}

	void show()
	{
		SOTYPEVIEWER::show();
		cds_editor::show();
	}

	///Bucle de eventos local al editor
	SbBool processSoEvent (const SoEvent *const event)
	{
		//En Unix la tecla Q mata el bucle de eventos
		if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::Q))
		{
			//printf("Eliminando widget %p\n", parent);
			cds_editor::close();
			return true;
		}

		return SOTYPEVIEWER::processSoEvent(event);

	}//SbBool processSoEvent (const SoEvent *const event)

} ;


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
