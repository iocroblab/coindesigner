
#ifndef __CDS_VIEWERS_H__
#define __CDS_VIEWERS_H__

#include <qwidget.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>

#ifdef _WIN32
//SoWin includes
#include <Inventor/Win/SoWin.h>
#include <Inventor/Win/viewers/SoWinExaminerViewer.h>
#include <Inventor/Win/viewers/SoWinFlyViewer.h>
#include <Inventor/Win/viewers/SoWinPlaneViewer.h>
#define SoFullViewer SoWinFullViewer
#define SoExaminerViewer SoWinExaminerViewer
#define SoFlyViewer SoWinFlyViewer
#define SoPlaneViewer SoWinPlaneViewer
#define SoRenderArea SoWinRenderArea

#else
//SoQt includes
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/Qt/viewers/SoQtFlyViewer.h>
#include <Inventor/Qt/viewers/SoQtPlaneViewer.h>
#define SoFullViewer SoQtFullViewer
#define SoExaminerViewer SoQtExaminerViewer
#define SoFlyViewer SoQtFlyViewer
#define SoPlaneViewer SoQtPlaneViewer
#define SoRenderArea SoQtRenderArea

#endif


class CdsViewer : public QMainWindow
{
private:
	Q_OBJECT
	int AAliasLevel;
	QPopupMenu viewMenu;
	QPopupMenu pickMenu;

public:
    //Puntero al visualizador contenido
    SoRenderArea *viewer;

	//Constructor
	CdsViewer (SoType tipoViewer);

	//Comportamiento del visualizador ante el picado
	enum MenuIds {NoneId, InfoId, DeleteId, FlipId, AAlias0, AAlias3, AAlias5, BGColorId, FeedbackId, PatchId };
	int accionPicado;

public slots:

	///Cambia el color de fondo del visualizador
	void cambiarColor();

	///Cambia el visualizador del sistema de coordenadas
	void cambiarFeedbackVisibility(int id);

	///Cambia el efecto antialiasing
	void cambiarAntialias(int level);

	///Cambia el efecto de la accion de picado
    void cambiarPicado(int id);

}; //class CdsViewer



class CdsExaminerViewer : public SoExaminerViewer
{
public :
  CdsViewer *parent;
  CdsExaminerViewer(CdsViewer * pParent);
  SbBool processSoEvent (const SoEvent *const event);
} ;


class CdsFlyViewer : public SoFlyViewer
{
public :
  CdsViewer *parent;
  CdsFlyViewer(CdsViewer * pParent);
  SbBool processSoEvent (const SoEvent *const event);
} ;


class CdsPlaneViewer : public SoPlaneViewer
{
public :
  CdsViewer *parent;
  CdsPlaneViewer(CdsViewer * pParent);
  SbBool processSoEvent (const SoEvent *const event);
} ;

#endif
