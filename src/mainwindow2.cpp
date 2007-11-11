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
			 Ui.actionPromote_Children->setData((qulonglong)item);
			 menu.addAction(Ui.actionPromote_Children);
		 }

		 if (tipo.isDerivedFrom(SoTexture2::getClassTypeId()) )
		 {
			 //Miramos si este nodo tiene algun nombre de fichero asociado
			 SoTexture2 *tex= (SoTexture2 *)nodo;
			 if (tex->filename.getValue().getLength() > 0)
			 {
				 Ui.actionEmbedTexture->setData((qulonglong)item);
				 menu.addAction(Ui.actionEmbedTexture);
			 }
		 }

		 else if (tipo.isDerivedFrom(SoBumpMap::getClassTypeId()) )
		 {
			 //Miramos si este nodo tiene algun nombre de fichero asociado
			 SoBumpMap *tex= (SoBumpMap *)nodo;
			 if (tex->filename.getValue().getLength() > 0)
			 {
				 Ui.actionEmbedTexture->setData((qulonglong)item);
				 menu.addAction(Ui.actionEmbedTexture);
			 }
		 }

		 else if (tipo == SoDirectionalLight::getClassTypeId()) 
		 {
			 Ui.Convert_Manip->setData((qulonglong)item);
			 Ui.Convert_Manip->setText(tr("Convert in SoDirectionalLightManip"));
			 menu.addAction(Ui.Convert_Manip);
		 }
		 else if (tipo == SoSpotLight::getClassTypeId()) 
		 {
			 Ui.Convert_Manip->setData((qulonglong)item);
			 Ui.Convert_Manip->setText(tr("Convert in SoSpotLightManip"));
			 menu.addAction(Ui.Convert_Manip);
		 }
		 else if (tipo == SoPointLight::getClassTypeId()) 
		 {
			 Ui.Convert_Manip->setData((qulonglong)item);
			 Ui.Convert_Manip->setText(tr("Convert in SoPointLightManip"));
			 menu.addAction(Ui.Convert_Manip);
		 }
		 else if (tipo == SoDirectionalLightManip::getClassTypeId()) 
		 {
			 Ui.Convert_Manip->setData((qulonglong)item);
			 Ui.Convert_Manip->setText(tr("Convert in SoDirectionalLight"));
			 menu.addAction(Ui.Convert_Manip);
		 }
		 else if (tipo == SoSpotLightManip::getClassTypeId()) 
		 {
			 Ui.Convert_Manip->setData((qulonglong)item);
			 Ui.Convert_Manip->setText(tr("Convert in SoSpotLight"));
			 menu.addAction(Ui.Convert_Manip);
		 }
		 else if (tipo == SoPointLightManip::getClassTypeId()) 
		 {
			 Ui.Convert_Manip->setData((qulonglong)item);
			 Ui.Convert_Manip->setText(tr("Convert in SoPointLight"));
			 menu.addAction(Ui.Convert_Manip);
		 }
		 else if (tipo == SoClipPlaneManip::getClassTypeId()) 
		 {
			 Ui.Convert_Manip->setData((qulonglong)item);
			 Ui.Convert_Manip->setText(tr("Convert in SoClipPlane"));
			 menu.addAction(Ui.Convert_Manip);
		 }
		 else if (tipo == SoClipPlane::getClassTypeId()) 
		 {
			 Ui.Convert_Manip->setData((qulonglong)item);
			 Ui.Convert_Manip->setText(tr("Convert in SoClipPlaneManip"));
			 menu.addAction(Ui.Convert_Manip);
		 }


		 //DEBUG
		 //QMenu transfMenu(tr("Transform into"), this);
		 //menu.addMenu(&transfMenu);

		 //Mostramos el menu popup
		 menu.exec(event->globalPos());
     }
 }

///Put all children of a group node on the same level that its parent
void MainWindow::on_actionPromote_Children_activated()
{
	QTreeWidgetItem *item=Ui.sceneGraph->currentItem();

	//Trata de leer el argumento del sender()->data
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
	{
		bool ok = false;
		QTreeWidgetItem *item2 = (QTreeWidgetItem *)action->data().toULongLong(&ok);
		if (ok && item2)
			item = item2;

		//Borramos el contenido del action->data(), por si acaso acaba en otra parte
		action->setData(0);
	}

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


///Embed external texture image files into the node
void MainWindow::on_actionEmbedTexture_activated(SoNode *node)
{
  //Miramos si nos han pasado algun nodo o debemos usar el item actual 
	if (node == NULL)
	{
		//Comienza por intentar usar el item actual
		QTreeWidgetItem *item=Ui.sceneGraph->currentItem();

		//Trata de leer el argumento del sender()->data
		QAction *action = qobject_cast<QAction *>(sender());
		if (action)
		{
			bool ok = false;
			QTreeWidgetItem *item2 = (QTreeWidgetItem *)action->data().toULongLong(&ok);
			if (ok && item2)
				item = item2;

			//Borramos el contenido del action->data(), por si acaso acaba en otra parte
			action->setData(0);
		}

		//Usa el node del item actual, o el pasado como sender()->data()
		node = mapQTCOIN[item];
	}

  SoType tipo = node->getTypeId();

  if (tipo == SoTexture2::getClassTypeId())
  {
	//Convertimos en un nodo SoTexture2
	SoTexture2 *tex= (SoTexture2 *)node;

	//Miramos si este nodo tiene algun nombre de fichero asociado
	if (tex->filename.getValue().getLength() == 0)
		return;

	//Aplicamos touch al campo image
	tex->image.touch();
   
	//Limpiamos el contenido del campo filename
	tex->filename.setValue(NULL);
  }
  else if (tipo == SoBumpMap::getClassTypeId())
  {
	//Convertimos en un nodo SoTexture2
	SoBumpMap *tex= (SoBumpMap *)node;

	//Miramos si este nodo tiene algun nombre de fichero asociado
	if (tex->filename.getValue().getLength() == 0)
		return;

	//Aplicamos touch al campo image
	tex->image.touch();
   
	//Limpiamos el contenido del campo filename
	tex->filename.setValue(NULL);
  }
  else
  {
	  __chivato__
	  QString S;
	  S.sprintf("No puedo incrustar en %s", node->getTypeId().getName().getString() );
	  QMessageBox::warning( this, tr("Error"), S);
	  return;
  }
  
  //Mostramos el nodo modificado en la tabla de edicion de campos
  updateFieldEditor(node);

  //Indicamos que la escena ha sido modificada
  escena_modificada = true;

}//void MainWindow::on_actionEmbedTexture_activated()


///Convert a node into its correspondent manip 
void MainWindow::on_Convert_Manip_activated(QTreeWidgetItem *item)
{
  //Miramos si nos han pasado algun nodo o debemos usar el item actual 
	if (item == NULL)
	{
		//Comienza por intentar usar el item actual
		item=Ui.sceneGraph->currentItem();

		//Trata de leer el argumento del sender()->data
		QAction *action = qobject_cast<QAction *>(sender());
		if (action)
		{
			bool ok = false;
			QTreeWidgetItem *item2 = (QTreeWidgetItem *)action->data().toULongLong(&ok);
			if (ok && item2)
				item = item2;

			//Borramos el contenido del action->data(), por si acaso acaba en otra parte
			action->setData(0);
		}
	}

	//Creacion del nuevo nodo sustituto del anterior
	SoNode *node = mapQTCOIN[item];
    SoNode *newNode = NULL;

    if (node->getTypeId() == SoSpotLightManip::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoSpotLight();
    }  
	else if (node->getTypeId() == SoSpotLight::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoSpotLightManip();
    }  
    else if (node->getTypeId() == SoPointLightManip::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoPointLight();
    }  
    else if (node->getTypeId() == SoPointLight::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoPointLightManip();
    }  
    else if (node->getTypeId() == SoDirectionalLightManip::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoDirectionalLight();
    }  
    else if (node->getTypeId() == SoDirectionalLight::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoDirectionalLightManip();
    }  
    else if (node->getTypeId() == SoClipPlaneManip::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoClipPlane();
    }  
    else if (node->getTypeId() == SoClipPlane::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoClipPlaneManip();

		/* Intento de resolver un bug de coin3D. No funciona.
		//Calcula el path del nodo
		SoPath *path =getPathFromItem(item);
		((SoClipPlaneManip*)newNode)->replaceNode(path);
		path->unref();

		//Configuramos el icono y el texto del item
		mapQTCOIN[item] = newNode;
		setNodeIcon(item);
		//Actualizamos la tabla  de campos
		updateFieldEditor (newNode);
		//Indicamos que la escena ha sido modificada
		escena_modificada = true
		return;
		//*/
    }  
	else
	{
		//Me han pasado un nodo no soportado
		QString S;
		S.sprintf("Can't convert %s into a manip", node->getTypeId().getName().getString());
		QMessageBox::warning( this, tr("Warning"), S);

		return;
	}

	//Copiamos los fields del nodo original que se llamen igual en newNode
	//Extraemos su lista de campos
	SoFieldList  fields;
	node->getFields(fields);
	int num_fields=fields.getLength();
	//Recorremos todos los fields del nodo antiguo
	for (int f=0; f < num_fields; f++)
	{
		//Leemos el nombre de este field
		SoField *field = fields[f];
		SbName nombre_field;
		node->getFieldName(field, nombre_field);
		//Miramos si el nuevo nodo tiene un field llamado igual
		SoField *dst_field = newNode->getField(nombre_field);
		if(dst_field && dst_field->isOfType(field->getTypeId()) )
		{
			//Copiamos el valor del campo
			dst_field->copyFrom(*field);
		}
	}//for (int f=0; f < num_fields; f++)

	//Reemplazamos el nodo en node por newNode
    QTreeWidgetItem *item_padre=item->parent();
    SoGroup *nodo_padre=(SoGroup*)mapQTCOIN[item_padre];
    nodo_padre->replaceChild(node, newNode);

    //Configuramos el icono y el texto del item
    mapQTCOIN[item] = newNode;
    setNodeIcon(item);
	item->setText(0, QString(newNode->getTypeId().getName() ));
    //TODO item->setToolTip(0, QString(t.getName()));

    //Actualizamos la tabla  de campos
    updateFieldEditor (newNode);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}//void MainWindow::on_actionConvert_Manip_activated(QTreeWidgetItem *item)

