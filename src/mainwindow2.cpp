#include "mainwindow.h"
#include "cds_viewers.h"
#include "cds_util.h"
#include "cds_globals.h"
#include "3dsLoader.h"
#include <src_editor.h>
#include <mfield_editor.h>
#include <cppexport_options.h>
#include <ivfix_options.h>
#include <qslim_options.h>
#include <tetgen_options.h>

#include <QSettings>
#include <QMessageBox>
#include <QProcess>
#include <QResource>
#include <QFileDialog>
#include <QInputDialog>
#include <QPushButton>
#include <QCloseEvent>
#include <QMenu>
#include <QContextMenuEvent>
#include <QColorDialog>

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

void MainWindow::on_fieldTable_customContextMenuRequested(QPoint pos)
{
	//Miramos si se ha pulsado sobre algun item valido
    QTableWidgetItem *item = Ui.fieldTable->itemAt(pos);
	if (!item)
		return;

   //Si estamos vigilando la escena, dejamos de hacerlo para
   //poder editar los valores de la misma.
   //TODO: refreshGUI_but->setOn(false);

    //Buscamos el nodo que estamos editando
	if (!edit_node)
		return;
    SoNode *nodo = edit_node;

    //Buscamos el SoField que estamos editando
    SoField* field= map_fieldTable[item->row()];

    //Leemos el nombre de este campo
    SbName nombre_field;
    nodo->getFieldName(field, nombre_field);
    //const char*nombre_campo = nombre_field.getString();  

    //Leemos el tipo de este campo
	SoType tipo=field->getTypeId();
	const char*nombre_tipo = tipo.getName();  

	//Miramos si hay algun ayudante para este tipo...

	//Edicion de cualquier campo tipo SoSFBool
	if (!strcmp(nombre_tipo, "SFBool") )
	{
		//Conversion del SoField que estamos editando
		SoSFBool *soSFBool= (SoSFBool *)field;

		//Preparamos un menu flotante con opciones true/false
		QMenu popm(this);
		QAction actTrue("TRUE", this);
		QAction actFalse("FALSE", this);
		popm.addAction(&actTrue);
		popm.addAction(&actFalse);

		//Mostramos el menu flotante y recogemos la opcion escogida
		QAction *idx = popm.exec(QCursor::pos());

		//Comprobamos que se ha seleccionado una opción válida.
		if (idx)
			soSFBool->setValue(idx == &actTrue);
	}

	//Edicion de cualquier campo tipo SoSFEnum
	//Edicion de cualquier campo tipo SoSFBitMask
	else if (!strcmp(nombre_tipo, "SFEnum") ||
		!strcmp(nombre_tipo, "SFBitMask") )
	{
		//Convertimos el tipo de field
		SoSFEnum *soSFEnum= (SoSFEnum *)field;

		//Preparamos un menu flotante 
		QMenu popm(this);

		SbName nombre;
		int idx;
		//Probamos todos los indices y creamos una accion por cada
		for (idx=0; idx < soSFEnum->getNumEnums(); idx++)
		{
			soSFEnum->getEnum(idx, nombre);
			QAction *acc = new QAction(nombre.getString(), this);
			popm.addAction(acc);
		}

		//Mostramos el menu flotante y recogemos la opcion escogida
		QAction *acc = popm.exec(QCursor::pos());

		//Comprobamos que se ha seleccionado una opción válida.
		if (!acc)
			return;

		//Asignamos la opcion escogida
		idx = popm.actions().indexOf(acc);
		soSFEnum->getEnum(idx, nombre);
		soSFEnum->setValue(nombre);
	}

	//Edicion de cualquier campo tipo SoSFColor
	else if (!strcmp(nombre_tipo, "SFColor"))
	{
		//Convertimos el tipo de field
		SoSFColor *color= (SoSFColor *)field;

		//Lo convertimos en valores RGB y en un QColor
		const float*rgb = color->getValue().getValue();
		QColor c( int(255*rgb[0]), int(255*rgb[1]), int(255*rgb[2]) );

		//Solicitamos un color mediante QColorDialog
		c=QColorDialog::getColor(c, this);
		if (c.isValid() )
		{           
			//Modificamos el field
			color->setValue(c.red()/255.0, c.green()/255.0, c.blue()/255.0);
		}
	}


	//Edicion de cualquier campo tipo SoSFString
	else if (!strcmp(nombre_tipo, "SFString") )
	{
		SoSFString *soSFString = (SoSFString *)field;
		//Preparamos un menu flotante
		QMenu popm(this);
		QAction actFile(tr("Insert filename"), this);
		popm.addAction(&actFile);

		//Mostramos el menu flotante y recogemos la opcion escogida
		QAction *act = popm.exec(QCursor::pos());

		//Si se ha escogido la opcion filename
		if (act == &actFile)
		{
			QString filename = QFileDialog::getOpenFileName(this, 
				tr("Choose Filename"), soSFString->getValue().getString(), 
				tr("Image files")+" (*.jpg *.gif *.png *.rgb);;" +
				tr("OpenInventor Files")+" (*.iv *.wrl);;"+
				tr("All Files")+" (*)");

			//Asignamos el valor escogido
			soSFString->setValue(filename.toAscii());
		}
	}

	//Edicion de cualquier campo tipo SoMF....
    else if (!strcmp(nombre_tipo, "MFColor") ||
        !strcmp(nombre_tipo, "MFVec2f") ||
        !strcmp(nombre_tipo, "MFVec3f") ||
        !strcmp(nombre_tipo, "MFVec4f") ||
        !strcmp(nombre_tipo, "MFFloat") ||
        !strcmp(nombre_tipo, "MFInt32") ||
        !strcmp(nombre_tipo, "MFUInt32") ||
        !strcmp(nombre_tipo, "MFShort") ||
        !strcmp(nombre_tipo, "MFString") )
    {
        //Usamos el mfield_editor
        MFieldEditor mfield_ed((SoMField *)field, this);
        mfield_ed.exec();
    }
    
 /* TODO

    //Edicion de cualquier campo tipo SoSFNode
    else if (!strcmp(nombre_tipo, "SFNode") )
    {
       //Con el boton 2, editamos este nodo
       if (button==2)
       {
            //Convertimos el tipo de field
            SoSFNode *soSFNode= (SoSFNode *)field;
            SoNode *node = soSFNode->getValue();

            if (node)
            {
                //Mostramos el nodo en la tabla de edicion de campos
                updateFieldEditor (node);
                return;
            }
       }
    }
  
    //Edicion de cualquier campo tipo SoMFNode
    else if (!strcmp(nombre_tipo, "MFNode") )
    {
       //Con el boton 2, editamos este nodo
       if (button==2)
       {
             //Buscamos la posicion contando cuentas filas por
             //encima de nosotros apuntan al mismo campo
             int pos=0;
             while(map_fieldTable[fila-pos-1] == field)
               pos++;

             //Le damos el valor adecuado en la posiciÃ³n pos
            SoNode *node = ((SoMFNode *)field)->getNode(pos);

            if (node)
            {
                //Mostramos el nodo en la tabla de edicion de campos
                updateFieldEditor (node);
                return;
            }
       }
    }
  TODO */

	/* TODO
    //Edicion de cualquier campo tipo SoSFMatrix
    else if (!strcmp(nombre_tipo, "SFMatrix") )
    {
        //Usamos el formulario auxiliar
        matrix_editor matrix_ed(this);
        matrix_ed.cargarDatos((SoSFMatrix *)field);
        matrix_ed.exec();
    }
	*/
	else
    {
      //No hay ayudante para este campo
      return;
    }

    //Actualizamos el contenido de la tabla
    updateFieldEditor(nodo);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}//void MainWindow::on_fieldTable_customContextMenuRequested(QPoint pos)


void MainWindow::on_sceneGraph_customContextMenuRequested(QPoint pos)
{
    //Miramos si hemos pinchado sobre un item del sceneGraph
    //QPoint pos = Ui.sceneGraph->mapFromGlobal(event->globalPos());
    QTreeWidgetItem *item = Ui.sceneGraph->itemAt(pos);
	if (!item)
		return;

	QMenu menu(this);
	QMenu menuExport(tr("Export to..."), &menu);

	//qDebug("%s %p\n", qPrintable(item->text(0)), item);
	SoNode *nodo = mapQTCOIN[item];
	SoType  tipo = nodo->getTypeId();

	//Creacion del menu comun a todos los nodos (con excepciones para root)
	if (nodo != root)
	{
		menu.addAction(Ui.actionCut);
	}
	menu.addAction(Ui.actionCopy);
	menu.addAction(Ui.actionPaste);
	if (nodo != root)
	{
		menu.addAction(Ui.actionDelete);
		menu.addAction(Ui.actionMove_Up);
		menu.addAction(Ui.actionMove_Down);
	}
	menu.addSeparator();

	if (tipo.isDerivedFrom(SoGroup::getClassTypeId()) && nodo != root) 
    {
		Ui.actionPromote_Children->setData((qulonglong)item);
		menu.addAction(Ui.actionPromote_Children);
	}

	else if (tipo.isDerivedFrom(SoTexture2::getClassTypeId()) )
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
	else if (tipo == SoRotation::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in SoTrackballManip"));
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoTrackballManip::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in SoRotation"));
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo.isDerivedFrom(SoTransformManip::getClassTypeId()) ) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in SoTransform"));
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoClipPlane::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in SoClipPlaneManip"));
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoIndexedFaceSet::getClassTypeId()) 
	{
		menu.addAction(Ui.IndexedFaceSet_to_IndexedLineSet);
		menu.addAction(Ui.SoIndexedFaceSet_triangulate);
		menu.addAction(Ui.SoIndexedFaceSet_change_orientation);

		//Opcion para simplificar la geometria con QSlim
		if (!settings->value("qslim_app").toString().isEmpty())
			menu.addAction(Ui.actionQSlim);

		//Opcion para ejecutar tetgen
		if (!settings->value("tetgen_app").toString().isEmpty())
			menu.addAction(Ui.actionTetgen);

		//Submenu con las opciones de exportacion
		menuExport.addAction(Ui.Export_to_SMF);
		menuExport.addAction(Ui.Export_to_OFF);
		menuExport.addAction(Ui.Export_to_STL);
		menu.addMenu(&menuExport);
	}
	else if (tipo == SoIndexedLineSet::getClassTypeId()) 
	{
		menu.addAction(Ui.IndexedLineSet_to_IndexedFaceSet);
	}
	else if (tipo == SoCoordinate3::getClassTypeId()) 
	{
		menu.addAction(Ui.Export_to_XYZ);
		menu.addAction(Ui.Center_on_Origin);
	}
	else if (tipo == SoVRMLIndexedFaceSet::getClassTypeId()) 
	{
		menu.addAction(Ui.SoIndexedFaceSet_triangulate);
		//Submenu con las opciones de exportacion
		menuExport.addAction(Ui.Export_to_SMF);
		menuExport.addAction(Ui.Export_to_OFF);
		menuExport.addAction(Ui.Export_to_STL);
		menu.addMenu(&menuExport);
	}
	else if (tipo == SoIndexedTriangleStripSet::getClassTypeId()) 
	{
		menu.addAction(Ui.SoIndexedTriangleStripSet_to_SoIndexedFaceSet);
	}

	//Convierte un soTransform en cualquier manip
	if (tipo == SoTransform::getClassTypeId())
	{
		//DEBUG
		//QMenu transfMenu(tr("Transform into"), this);
		//menu.addMenu(&transfMenu);
		Ui.actionSoCenterballManip->setData((qulonglong)item);
		menu.addAction(Ui.actionSoCenterballManip);
		Ui.actionSoHandleBoxManip->setData((qulonglong)item);
		menu.addAction(Ui.actionSoHandleBoxManip);
		Ui.actionSoJackManip->setData((qulonglong)item);
		menu.addAction(Ui.actionSoJackManip);
		Ui.actionSoTabBoxManip->setData((qulonglong)item);
		menu.addAction(Ui.actionSoTabBoxManip);
		Ui.actionSoTrackballManip->setData((qulonglong)item);
		menu.addAction(Ui.actionSoTrackballManip);
		Ui.actionSoTransformBoxManip->setData((qulonglong)item);
		menu.addAction(Ui.actionSoTransformBoxManip);
		Ui.actionSoTransformerManip->setData((qulonglong)item);
		menu.addAction(Ui.actionSoTransformerManip);
	}
	
	//Mostramos el menu popup
	menu.exec(Ui.sceneGraph->mapToGlobal(pos));

}//void MainWindow::on_sceneGraph_customContextMenuRequested(QPoint pos)


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


///Convert a light or clipPlane into its correspondent manip 
void MainWindow::on_Convert_Manip_activated()
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
    else if (node->getTypeId() == SoRotation::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoTrackballManip();
    }
	//Los SoTrackballManip se convierten en SoRotation
    else if (node->getTypeId() == SoTrackballManip::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoRotation();
    }  
	//El resto de SoTransformManip se convierten en SoTransform
	else if (node->getTypeId().isDerivedFrom(SoTransformManip::getClassTypeId())) 
    {
        newNode=(SoNode*)new SoTransform();
    }
	//Los tranform se convierten en un manip dependiendo de la accion llamante
    else if (node->getTypeId() == SoTransform::getClassTypeId()) 
    {
		if (action == Ui.actionSoCenterballManip )
			newNode=(SoNode*)new SoCenterballManip();
		else if (action == Ui.actionSoHandleBoxManip )
			newNode=(SoNode*)new SoHandleBoxManip();
		else if (action == Ui.actionSoJackManip )
			newNode=(SoNode*)new SoJackManip();
		else if (action == Ui.actionSoTabBoxManip )
			newNode=(SoNode*)new SoTabBoxManip();
		else if (action == Ui.actionSoTrackballManip )
			newNode=(SoNode*)new SoTrackballManip();
		else if (action == Ui.actionSoTransformBoxManip )
			newNode=(SoNode*)new SoTransformBoxManip();
		else if (action == Ui.actionSoTransformerManip )
			newNode=(SoNode*)new SoTransformerManip();
		else
		{
			//Me han pasado un nodo no soportado
			QString S;
			S.sprintf("Wrong action. Blame developper!!");
			QMessageBox::warning( this, tr("Warning"), S);

			return;
		}

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

	newNode->setName(node->getName());

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

}//void MainWindow::on_actionConvert_Manip_activated()



void MainWindow::on_IndexedFaceSet_to_IndexedLineSet_activated()
{    
	QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 
	SoIndexedFaceSet *oldNode= (SoIndexedFaceSet*)mapQTCOIN[item];

	SoIndexedLineSet *newNode=new SoIndexedLineSet;
	newNode->vertexProperty.copyFrom(oldNode->vertexProperty);
	newNode->coordIndex.copyFrom(oldNode->coordIndex);
	newNode->materialIndex.copyFrom(oldNode->materialIndex);
	newNode->normalIndex.copyFrom(oldNode->normalIndex);
	newNode->textureCoordIndex.copyFrom(oldNode->textureCoordIndex);

	mapQTCOIN[item]=(SoNode*)newNode;
	QTreeWidgetItem *padre=item->parent();
	SoGroup *nodo_padre=(SoGroup*)mapQTCOIN[padre];
	nodo_padre->replaceChild(oldNode,newNode);

	//Configuramos el icono y el texto del item
	setNodeIcon(item);

	//Actualizamos la tabla  de campos
	updateFieldEditor (newNode);

	//Indicamos que la escena ha sido modificada
	escena_modificada = true;

} // void MainWindow::on_IndexedFaceSet_to_IndexedLineSet_activated()

void MainWindow::on_IndexedLineSet_to_IndexedFaceSet_activated()
{    
	QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 
	SoIndexedLineSet *oldNode= (SoIndexedLineSet*)mapQTCOIN[item];

	SoIndexedFaceSet *newNode = new SoIndexedFaceSet;
	newNode->vertexProperty.copyFrom(oldNode->vertexProperty);
	newNode->coordIndex.copyFrom(oldNode->coordIndex);
	newNode->materialIndex.copyFrom(oldNode->materialIndex);
	newNode->normalIndex.copyFrom(oldNode->normalIndex);
	newNode->textureCoordIndex.copyFrom(oldNode->textureCoordIndex);

	mapQTCOIN[item]=(SoNode*)newNode;
	QTreeWidgetItem *padre=item->parent();
	SoGroup *nodo_padre=(SoGroup*)mapQTCOIN[padre];
	nodo_padre->replaceChild(oldNode,newNode);

	//Configuramos el icono y el texto del item
	setNodeIcon(item);

	//Actualizamos la tabla  de campos
	updateFieldEditor (newNode);

	//Indicamos que la escena ha sido modificada
	escena_modificada = true;

} // void MainWindow::on_IndexedLineSet_to_IndexedFaceSet_activated()

///Exporta un SoIndexedFaceSet o SoVRMLIndexedFaceSet a un fichero SMF
void MainWindow::on_Export_to_SMF_activated()
{
	//Nombre del fichero donde escribir
    QString filename = QFileDialog::getSaveFileName(this, tr("Export File"), "",
		tr("OBJ/SMF Files")+" (*.obj *.smf);;"+tr("All Files")+" (*)");

    //Miramos si se pulso el boton cancelar
    if (filename=="")
        return;

	/* 
	//PLAN B. NUESTRO DIALOGO 
	cdsFileDialog fd(this, tr("Exportar a fichero SMF/OBJ"), cdsFileDialog::saveNormal);
    fd.setMode( QFileDialog::AnyFile );
	//fd->setViewMode( QFileDialog::Detail );
	fd.setFilter( tr("Ficheros SMF (*.smf *.obj)"));
	fd.addFilter( tr("Todos los ficheros (*)") );
	fd.setSelectedFilter(0);

	//Ejecutamos el dialogo y miramos si se pulso el boton aceptar
	if (fd.exec() != QDialog::Accepted )
		return;

	//Leemos el fichero seleccionado
    nombre_fich = fd.selectedFile();
	*/

	QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 
	SoNode *nodo = mapQTCOIN[item];

    //Abrimos el fichero de salida
#ifdef _WIN32
    FILE *file;
	fopen_s(&file, SbName(filename.toAscii()).getString(), "w");
#else
    FILE *file = fopen(SbName(filename.toAscii()).getString(), "w");
#endif

    //Escribimos el contenido en el fichero SMF
    if (nodo->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId())) 
    {
		SoPath *path =getPathFromItem(item);
		//PLAN B. IndexedFaceSet_to_SMF(path, file, fd.normal->isChecked());
		IndexedFaceSet_to_SMF(path, file, true);
		path->unref();
    }
    else if (nodo->getTypeId().isDerivedFrom(SoVRMLIndexedFaceSet::getClassTypeId())) 
    {
      VRMLIndexedFaceSet_to_SMF((SoVRMLIndexedFaceSet *)nodo, file);
    }

    //Cerramos el fichero
    fclose(file);

}// void MainWindow::on_Export_to_SMF_activated()

///Exporta un SoIndexedFaceSet o SoVRMLIndexedFaceSet a un fichero OFF
void MainWindow::on_Export_to_OFF_activated()
{
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 

    //Construimos un path  segun la informacion del arbol de QT
	SoNode *nodo = mapQTCOIN[item];

	//Nombre del fichero donde escribir
    QString filename = QFileDialog::getSaveFileName(this, tr("Export File"), "",
		tr("OFF Files")+" (*.off);;"+tr("All Files")+" (*)");

    //Miramos si se pulso el boton cancelar
    if (filename=="")
        return;

    //Abrimos el fichero de salida
#ifdef _WIN32
    FILE *file;
	fopen_s(&file, SbName(filename.toAscii()).getString(), "w");
#else
    FILE *file = fopen(SbName(filename.toAscii()).getString(), "w");
#endif

    //Escribimos el contenido en el fichero OFF
    if (nodo->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId())) 
    {
		SoPath *path =getPathFromItem(item);
		IndexedFaceSet_to_OFF(path, file);
		path->unref();
    }
    else if (nodo->getTypeId().isDerivedFrom(SoVRMLIndexedFaceSet::getClassTypeId())) 
    {
      VRMLIndexedFaceSet_to_OFF((SoVRMLIndexedFaceSet *)nodo, file);
    }

    //Cerramos el fichero
    fclose(file);

}// void MainWindow::on_Export_to_OFF_activated()


///Exporta un SoIndexedFaceSet o SoVRMLIndexedFaceSet a un fichero STL
void MainWindow::on_Export_to_STL_activated()
{
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 

    //Construimos un path  segun la informaciÃ³n del arbol de QT
	SoNode *nodo = mapQTCOIN[item];

	//Nombre del fichero donde escribir
    QString filename = QFileDialog::getSaveFileName(this, tr("Export File"), "",
		tr("STL Files")+" (*.stl);;"+tr("All Files")+" (*)");

    //Miramos si se pulso el boton cancelar
    if (filename=="")
        return;

    //Abrimos el fichero de salida
#ifdef _WIN32
    FILE *file;
	fopen_s(&file, SbName(filename.toAscii()).getString(), "w");
#else
    FILE *file = fopen(SbName(filename.toAscii()).getString(), "w");
#endif

    //Escribimos el contenido en el fichero OFF
    if (nodo->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId())) 
    {
		SoPath *path =getPathFromItem(item);
		IndexedFaceSet_to_STL(path, file, true);
		path->unref();
    }
    else if (nodo->getTypeId().isDerivedFrom(SoVRMLIndexedFaceSet::getClassTypeId())) 
    {
      VRMLIndexedFaceSet_to_STL((SoVRMLIndexedFaceSet *)nodo, file, true);
    }

    //Cerramos el fichero
    fclose(file);

}// void MainWindow::on_Export_to_STL_activated()

void MainWindow::on_Export_to_XYZ_activated()
{
	//Nombre del fichero donde escribir
    QString filename = QFileDialog::getSaveFileName(this, tr("Export File"), "",
		tr("XYZ Files")+" (*.xyz);;"+tr("All Files")+" (*)");

    //Miramos si se pulso el boton cancelar
    if (filename=="")
        return;

    //Abrimos el fichero de salida
#ifdef _WIN32
    FILE *file;
	fopen_s(&file, SbName(filename.toAscii()).getString(), "w");
#else
    FILE *file = fopen(SbName(filename.toAscii()).getString(), "w");
#endif

    //Identificamos el item actual
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem();

    //Escribimos el contenido en el fichero XYZ
    if (mapQTCOIN[item]->getTypeId().isDerivedFrom(SoCoordinate3::getClassTypeId())) 
    {
       SoCoordinate3 *nodo = (SoCoordinate3 *)mapQTCOIN[item];
       SoMFVec3f_to_XYZ(nodo->point, file);
    }
    else if (mapQTCOIN[item]->getTypeId().isDerivedFrom(SoVertexProperty::getClassTypeId())) 
    {
       SoVertexProperty *nodo = (SoVertexProperty *)mapQTCOIN[item];
       SoMFVec3f_to_XYZ(nodo->vertex, file);
    }

    //Cerramos el fichero
    fclose(file);

}// void MainWindow::on_Export_to_XYZ_activated()

void MainWindow::on_Center_on_Origin_activated()
{
    //Identificamos el item actual
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem();

    if (mapQTCOIN[item]->getTypeId().isDerivedFrom(SoCoordinate3::getClassTypeId())) 
	{
        SoCoordinate3 *nodo = (SoCoordinate3 *)mapQTCOIN[item];

        //Calculamos su centroide
        SbVec3f cdm = centroid (nodo->point);

        //Centramos los vertices sobre el centroide
        center_new(nodo->point, cdm);
    }
    else if (mapQTCOIN[item]->getTypeId().isDerivedFrom(SoVertexProperty::getClassTypeId())) 
    {
        SoVertexProperty *nodo = (SoVertexProperty *)mapQTCOIN[item];
    
        //Calculamos su centroide
        SbVec3f cdm = centroid (nodo->vertex);

        //Centramos los vertices sobre el centroide
        center_new(nodo->vertex, cdm);
    }

	//Actualizamos la tabla  de campos
	updateFieldEditor (mapQTCOIN[item]);

	//Indicamos que la escena ha sido modificada
	escena_modificada = true;

}// void MainWindow::on_Center_on_Origin_activated()

void MainWindow::on_actionIvfix_activated()
{
	ivfix_options ivfixDialog(root);
	ivfixDialog.exec();
	SoNode *ivfix_result = ivfixDialog.output;

	//Miramos si hay resultados en ivfix_result
	if (ivfix_result != NULL)
	{
		//Destruimos la escena actual y creamos una nueva
		on_actionNew_Scene_activated();

		//Colgamos el nodo del grafo de escena
		QTreeWidgetItem *qroot=Ui.sceneGraph->currentItem();
		newSceneGraph(ivfix_result, qroot, root);

		//Expandimos todos los items
		Ui.sceneGraph->expandAll();

		//Actualizamos la tabla  de campos
		updateFieldEditor (root);

		//Indicamos que la escena ha sido modificada
		escena_modificada = true;
	}

}//void MainWindow::on_actionIvfix_activated()


///Triangula las facetas de mas de tres lados de un SoIndexedFaceSet o SoVRMLIndexedFaceSet
void MainWindow::on_SoIndexedFaceSet_triangulate_activated()
{
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 
    SoNode *nodo = mapQTCOIN[item];

    if (nodo->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId())) 
    {
		SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)nodo;
		IndexedFaceSet_triangulate (ifs->coordIndex);
		IndexedFaceSet_triangulate (ifs->materialIndex);
		IndexedFaceSet_triangulate (ifs->normalIndex);
		IndexedFaceSet_triangulate (ifs->textureCoordIndex);
    }
    else if (nodo->getTypeId().isDerivedFrom(SoVRMLIndexedFaceSet::getClassTypeId())) 
    {
		SoVRMLIndexedFaceSet *ifs = (SoVRMLIndexedFaceSet *)nodo;   
		IndexedFaceSet_triangulate (ifs->coordIndex);
		IndexedFaceSet_triangulate (ifs->colorIndex);
		IndexedFaceSet_triangulate (ifs->normalIndex);
		IndexedFaceSet_triangulate (ifs->texCoordIndex);
    }
    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}//void MainWindow::on_SoIndexedFaceSet_triangulate_activated()


///Cambia la orientacion de todas las facetas
void MainWindow::on_SoIndexedFaceSet_change_orientation_activated()
{
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 
	assert(mapQTCOIN[item]->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId()));
    SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)mapQTCOIN[item];
   
   IndexedFaceSet_change_orientation (ifs->coordIndex);
   IndexedFaceSet_change_orientation (ifs->materialIndex);
   IndexedFaceSet_change_orientation (ifs->normalIndex);
   IndexedFaceSet_change_orientation (ifs->textureCoordIndex);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}// void MainWindow::on_SoIndexedFaceSet_change_orientation_activated()


void MainWindow::on_SoIndexedTriangleStripSet_to_SoIndexedFaceSet_activated()
{
	QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 
	SoIndexedTriangleStripSet *oldNode= (SoIndexedTriangleStripSet*)mapQTCOIN[item];

	SoIndexedFaceSet *newNode= IndexedTriangleStripSet_to_IndexedFaceSet (oldNode);

	mapQTCOIN[item]=(SoNode*)newNode;
	QTreeWidgetItem *padre=item->parent();
	SoGroup *nodo_padre=(SoGroup*)mapQTCOIN[padre];
	nodo_padre->replaceChild(oldNode,newNode);

	//Configuramos el icono y el texto del item
	setNodeIcon(item);
	item->setText(0, QString(newNode->getTypeId().getName() ));
	//TODO item->setToolTip(0, QString(t.getName()));

	//Actualizamos la tabla  de campos
	updateFieldEditor (newNode);

	//Indicamos que la escena ha sido modificada
	escena_modificada = true;

}//void MainWindow::on_SoIndexedTriangleStripSet_to_SoIndexedFaceSet_activated()


