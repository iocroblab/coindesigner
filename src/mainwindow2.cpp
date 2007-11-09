#include "mainwindow.h"
#include "cds_viewers.h"
#include "cds_util.h"
#include "cds_globals.h"
#include "3dsLoader.h"
#include "ui_src_view.h"

#include <qsettings.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qresource.h>
#include <qfiledialog.h>
#include <QInputDialog>
#include <QPushButton>
#include <QCloseEvent>
#include <QMenu>
#include <QContextMenuEvent>

extern QSettings *settings;

//OpenInventor includes
#include <Inventor/SoDB.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/nodes/SoNodes.h>
#include <Inventor/nodes/SoBumpMap.h>
#include <Inventor/VRMLnodes/SoVRMLNodes.h>
#include <Inventor/fields/SoFields.h>
#include <Inventor/manips/SoClipPlaneManip.h>
#include <Inventor/manips/SoTrackballManip.h>
#include <Inventor/manips/SoHandleBoxManip.h>
#include <Inventor/manips/SoJackManip.h>
#include <Inventor/manips/SoTabBoxManip.h>
#include <Inventor/manips/SoCenterballManip.h>
#include <Inventor/manips/SoTransformBoxManip.h>
#include <Inventor/manips/SoTransformerManip.h>
#include <Inventor/manips/SoSpotLightManip.h>
#include <Inventor/manips/SoPointLightManip.h>
#include <Inventor/manips/SoDirectionalLightManip.h>
#include <Inventor/actions/SoActions.h>
#include <Inventor/actions/SoToVRML2Action.h>
#include <Inventor/draggers/SoHandleBoxDragger.h>
#include <Inventor/details/SoDetails.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/elements/SoPointSizeElement.h>
#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/errors/SoReadError.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>


#ifdef USE_DIME
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/Model.h>
#include <dime/State.h>
#include <dime/convert/convert.h>
#endif

#ifdef USE_VOLEON
  #include <VolumeViz/nodes/SoTransferFunction.h>
  #include <VolumeViz/nodes/SoVolumeData.h>
  #include <VolumeViz/nodes/SoVolumeRender.h>
  #include <VolumeViz/nodes/SoVolumeRendering.h>
#endif

#ifdef macintosh
#include "console.h"
#include "unix.h"
#include "SIOUX.h"
#endif // macintosh


void MainWindow::contextMenuEvent(QContextMenuEvent *event)
 {
    //Miramos si hemos pinchado sobre un item del sceneGraph
    QPoint pos = Ui.sceneGraph->mapFromGlobal(event->globalPos());
    QTreeWidgetItem *item = Ui.sceneGraph->itemAt(pos);
    if (item)
    {
         printf("%s %p\n", qPrintable(item->text(0)), item);

         //Creacion del menu comun a todos los nodos
         QMenu menu(this);
         menu.addAction(Ui.actionCut);
         menu.addAction(Ui.actionCopy);
         menu.addAction(Ui.actionPaste);
         menu.addAction(Ui.actionDelete);
         menu.addAction(Ui.actionMove_Up);
         menu.addAction(Ui.actionMove_Down);
         menu.addSeparator();

         SoNode *nodo = mapQTCOIN[item];
         SoType  tipo = nodo->getTypeId();

         if (tipo.isDerivedFrom(SoGroup::getClassTypeId()) && nodo != root) 
         {
             menu.addAction(Ui.actionPromote_Children);
         }

		 //DEBUG
		 //QMenu transfMenu(tr("Transform into"), this);
		 //menu.addMenu(&transfMenu);

         menu.exec(event->globalPos());
     }
 }

void MainWindow::on_actionPromote_Children_activated()
{
    QTreeWidgetItem *item=Ui.sceneGraph->currentItem();
    SoGroup *nodo = (SoGroup *) mapQTCOIN[item]; 

    //El root no puede promocionar por encima de si mismo
    if(nodo == root)
      return;

    //El item que contiene al nodo group
    QTreeWidgetItem *item_padre=item->parent();
    SoGroup *nodo_padre = (SoGroup *) mapQTCOIN[item_padre]; 

    //Consultamos la posicion del grupo dentro de su padre
    int pos = nodo_padre->findChild(nodo);

    //Numero de hijos a mover
    int num_hijos = nodo->getNumChildren();

    //Movemos los nodos de coin detras del nodo grupo
    for (int i=0; i<num_hijos; i++)
    {
        nodo_padre->insertChild(nodo->getChild(i), pos+i+1);
    }

    //Ahora repetimos lo mismo pero con el arbol de items
    pos = item_padre->indexOfChild(item);

    for (int i=0; i<num_hijos; i++)
    {
        //Quitamos el primer hijo del item y lo metemos al padre
        QTreeWidgetItem *item_hijo = item->takeChild(0);
        item_padre->insertChild(pos+i+1, item_hijo);
    }
    
    //Eliminamos el nodo grupo y el item grupo
    nodo_padre->removeChild(nodo);
    mapQTCOIN.erase(item);
    delete(item);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
    
}// void MainWindow::on_actionPromote_Children_activated()

