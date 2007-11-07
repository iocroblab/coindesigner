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

//Clase base para el resto de visores
class cds_view : public QMainWindow
{
	Q_OBJECT
	Ui::cds_view Ui;
public:

	///Constructor
	cds_view (QWidget *p=0, Qt::WindowFlags f=0) : QMainWindow(p, f)
	{
		Ui.setupUi(this);
	} //Constructor

private slots:

};


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

///Ventana cds_view() que contiene un SoExaminerViewer
template <class SOTYPEVIEWER>
class CdsTemplateViewer : public cds_view, SOTYPEVIEWER
{
public :
	/*! @brief Constructor de la clase
	    @param root: Default sceneGraph for this object
	*/
	CdsTemplateViewer(SoNode *root=NULL) :
	   cds_view(), SOTYPEVIEWER(this->centralWidget(), NULL, true )
	   {
		   if (root)
			   SOTYPEVIEWER::setSceneGraph(root);
	   }

	   void show()
	   {
		   SOTYPEVIEWER::show();
		   cds_view::show();
	   }

	   SbBool processSoEvent (const SoEvent *const event)
	   {
		   //En Unix la tecla Q mata el bucle de eventos
		   if (SoKeyboardEvent::isKeyPressEvent(event, SoKeyboardEvent::Q))
		   {
			   //printf("Eliminando widget %p\n", parent);
			   cds_view::close();
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

typedef CdsTemplateViewer<SoQtExaminerViewer> CdsExaminerViewer;
typedef CdsTemplateViewer<SoQtFlyViewer> CdsFlyViewer;
typedef CdsTemplateViewer<SoQtPlaneViewer> CdsPlaneViewer;

#endif
