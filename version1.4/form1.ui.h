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
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include "cds_config.h" 
#include "mfield_editor.h"
#include "matrix_editor.h"
#include "ivfix_options.h"
#include "qslim_options.h"
#include "tetgen_options.h"
#include "cppexport_options.h"
#include "cds_config_form.h"
#include "3dsLoader.h"
#include "cds_util.h"
#include "cds_viewers.h"
#include "cdsFileDialog.h"


#include <map>
#include <vector>
#include <iostream>



//Qt includes
#include <qstring.h>
#include <qwidget.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qaccel.h>
#include <qdir.h>
#include <qcolordialog.h>
#include <qinputdialog.h>
#include <qtextcodec.h>
#include <qprocess.h>
#include <qthread.h>
#include <qstatusbar.h>
#include <qnetwork.h>
#include <qurlinfo.h>
#include <qurloperator.h>


//The one and only .... root node!!!
SoSeparator *root=NULL;

//El nodo que estamos editando actualmente
SoNode *edit_node=NULL;

//Un puntero a la lista de componentes actual
ivPadre_t *listComp=NULL;

//Un flag para indicar si hemos modificado la escena desde la 
//ltima vez que salvamos
bool escena_modificada = false;

//Mapa que relaciona una fila de la tabla fieldTable con
//el SoField correspondiente
std::map<int, SoField*>map_fieldTable;

// Buffer de subescenas. Usado en las operaciones de cortar, copiar 
// y pegar nodos de openInventor
SoNode *node_buffer=NULL;

//Directorio actual
QString currentDir = QDir::currentDirPath();

//El indice de la opcion SIMVoleon en el comboBox1
#ifdef USE_VOLEON
int comboBox1_simVoleon_idx = -1;
#endif

//Un puntero al formulario principal, necesario para llamadas desde exterior
Form1 *form1=NULL;

// Sensor que actualiza el GUI a intervalos constantes, por si ha
// cambiado el valor del nodo.
SoTimerSensor *refreshGUI_Sensor = NULL;

/// Callback que actualiza el GUI 
static void refreshGUI_CB(void *data, SoSensor *sensor);

//Color de fondo de los viewers
SbColor bgColor_viewer (0,0,0); 

//Variable que indica si debemos mostrar información al picar en
//la escena o no.
bool mostrar_pick_info = true;

void Form1::inicializar()
{

	//Salvo una referencia a este formulario
	form1 = this;

	//inicializo las listas para que se ordenen de manera correcta 
	//sobre todo es importante la que representa el arbol de Open Inventor
	sceneTree->viewport()->setFocusPolicy(QWidget::ClickFocus);
	sceneTree->setSorting(-1);
	sceneTree->setSorting(-1,false);

	//Creación del nodeSensor que mantiene el GUI actualizado.
	refreshGUI_Sensor = new SoTimerSensor (refreshGUI_CB, root);
	refreshGUI_Sensor->setInterval(1.0);

	//Indicamos que la escena no ha sido modificada
	escena_modificada = false;

	//Creamos un nuevo SoExaminerViewer
	//nuevoCdsExaminerViewer();

}// void Form1::inicializar()



//Composicion del ListView2 a partir de una tabla de componentes
void Form1::CargarListaComponentes(ivPadre_t *listaComponentes)
{
	unsigned numComp,j;
	QTreeWidgetItem *item;

	//Una lista de QTreeWidgetItems, para encontrar los padres
	std::vector<QTreeWidgetItem*> vecComp;

	//Dejamos listComp apuntando a listaComponentes
	listComp = listaComponentes;

	//Apuntamos al primer componente de la tabla
	ivPadre_t *comp=listComp;
	numComp=0;

	//Vaciamos la lista actual
	nodeTree->clear();

	//Recorremos la tabla de componentes, creando nuevos items
	//hasta llegar a un componente nulo
	while (comp->clase)
	{
		//fprintf(stderr, "%03d %s\n", numComp, comp->clase);

		//El nodo SoNode tiene tratamiento especial, ya que
		//no aparece como un nodo del arbol y no tiene padre
		if (!strcmp(comp->clase , "Node"))
		{
			//Insertamos un elemento para mantener sincronizadas
			//las tablas vecComp y listComp
			vecComp.push_back(NULL);   
		}
		else
		{
			//Buscamos el padre de esta clase en vecComp
			QTreeWidgetItem *padre=NULL;
			for (j=0; j<vecComp.size(); j++)
			{
				if (!strcmp (comp->padre, listComp[j].clase))
				{
					//Si lo encontramos, tomamos nota del item del padre y salimos.
					padre=vecComp[j];
					break;
				}
			}

			//Comprobamos que realmente se encontro el padre
			if (j >= vecComp.size())
			{
				QString S;
				S.sprintf("clase %s : No se encuentra la clase padre %s",
					comp->clase, comp->padre);
				QMessageBox::critical( this, tr("Error cr&iacute;tico"), S,
					QMessageBox::Abort, QMessageBox::NoButton, QMessageBox::NoButton);
				assert (j < vecComp.size());
			}

			//Si el padre es NULL, quiere decir que este nodo
			//deriva directamente de SoNode, por lo que lo 
			//colgamos del nodeTree directamente.
			if (!padre)
			{
				item = new QTreeWidgetItem(nodeTree, comp->clase);
			}
			else
			{
				//Añadimos el nuevo nodo colgando del nodo padre
				item = new QTreeWidgetItem(padre, comp->clase);

				//Indicamos al padre que se muestre abierto
				//padre->setOpen(true);
			}

			//Guardamos una copia extra en vecComp
			vecComp.push_back(item);   
		}//if-else 

		//Apuntamos al componente siguiente
		comp++;
		numComp++;
	}//while

} // void Form1::CargarListaComponentes()



///Callback activada al cambiar el estado del boton getInfo
void Form1::getInfo_toggled(bool on)
{
	mostrar_pick_info = on;
}



void Form1::exportarCPP_activated()
{

    //Mostramos el dialogo de cppexport
    cppExportDialog cppExportDialog(this);

    cppExportDialog.exec();
}



//Importa un fichero raw en un nodo SoVolumeData
int Form1::ImportarVolumenRaw(QString filename)
{
#ifdef USE_VOLEON
    //Si no se ha pasado nombre de fichero, solicitamos uno
    if (filename == "")
       filename = QFileDialog::getOpenFileName(currentDir.ascii(),
             tr("Todos los ficheros (*)"),
                 this, tr("Importar geometr&iacute;a") );
    
    if (filename == "")
        return 0;

	//Cargamos el fichero en memoria
	QFile fich(filename);
	fich.open(IO_ReadOnly);
	QByteArray buff = fich.readAll();
	if ( buff.isNull() )
       return -1;

    //Creamos un nuevo nodo SoVolumeData
	SoVolumeData *vol = new SoVolumeData();
  
    //Si hubo error,  salimos inmediatamente
	if (!vol)
       return -1;

	//Rellenamos el contenido del nodo
	SbVec3s dims(128, 128, 128);
	vol->setVolumeData(dims, buff.data() );


    //Asignamos el nombre del fichero al separator
    SoBase_name->setValue(QFileInfo(filename).baseName().ascii());
    vol->setName(SoBase_name->getValue());

    //Colgamos el nodo vol del grafo de escena
    QTreeWidgetItem *padre=sceneTree->currentItem();

    //Miramos si este nodo deriva de SoGroup, directa o indirectamente
    SoNode *nodo = mapQTCOIN[padre];
    if (!nodo->getTypeId().isDerivedFrom(SoGroup::getClassTypeId())) 
    {
            padre=padre->parent();
    }
 
    //donde corresponda, y luego insertar en los mapas y en el QlistView
    SoGroup* Anclaje=(SoGroup*)mapQTCOIN[padre];
    Anclaje->addChild(vol);  
    InsertarElemento(padre, vol);

    //Actualizamos la tabla  de campos
    actualizar_fieldTable (vol);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

#endif
    return 0;

} //int Form1::ImportarVolumenRaw(QString filename)


///Situa todos los hijos de un nodo SoGroup en el mismo nivel que el SoGroup
void Form1::Promocionar_hijos()
{
    //Tengo que sacar el item superior hasta que saque su hermano mayor
    //El metodo mas rapido es sacar el padre e ir bajando almacenando un
    //elemento y el anterior,
    //cuando encuentre mi item los cambio el superior por el inferior
    QTreeWidgetItem *item=sceneTree->currentItem();
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
    //Mantenemos un puntero al "ultimo de la fila"
    QTreeWidgetItem *detras_de=item;
    for (int i=0; i<num_hijos; i++)
    {
        QTreeWidgetItem *item_hijo=item->firstChild();
	    //Lo pasamos del nodo group a su padre
	    item->takeItem(item_hijo);
        item_padre->insertItem(item_hijo);

	    //Lo ordenamos correctamente, e indicamos que el proximo 
	    //item se debe colocar detras de este item
        item_hijo->moveItem(detras_de);
	    detras_de = item_hijo;
    }
    
    //Eliminamos el nodo grupo y el item grupo
    item_padre->takeItem(item);
    nodo_padre->removeChild(nodo);
    //nodo->removeAllChildren();

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
    
} // void Form1::Promocionar_hijos()

///Incrusta un fichero de textura para evitar dependencia de fichero externo
void Form1::incrustarTexture2(SoNode *node)
{
  //Miramos si nos han pasado algun nodo o debemos usar el item actual 
  if (node == NULL)
      node = mapQTCOIN[sceneTree->currentItem()];

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
  
  //Mostramos el nodo en la tabla de edicion de campos
  actualizar_fieldTable (node);

  //Indicamos que la escena ha sido modificada
  escena_modificada = true;

}//void Form1::incrustarTexture2(SoNode *node)


//Incrusta todas las texturas de la escena
void Form1::incrustar_texturas_activated ()
{
	if (QMessageBox::question( this, tr("Warning"), 
		tr("Incrustar las texturas puede incrementar considerablemente el tama�o "
		"de la escena.\n�Est� seguro de desear incrustar todas las texturas?"),
		QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
		return;

	//Buscamos todos los nodos de tipo SoTexture2
    SoSearchAction sa;
    sa.setType(SoTexture2::getClassTypeId());
    sa.setSearchingAll(TRUE);
    sa.setInterest(SoSearchAction::ALL);
    sa.apply(root);

	//Aplicamos touch a cada nodo
    for (int i=0; i < sa.getPaths().getLength(); i++) 
	{
      SoFullPath * fp = (SoFullPath *)sa.getPaths()[i];
	  incrustarTexture2(fp->getTail());
    }

	//Buscamos todos los nodos de tipo SoBumpMap
	sa.setType(SoBumpMap::getClassTypeId());
    sa.setSearchingAll(TRUE);
    sa.setInterest(SoSearchAction::ALL);
    sa.apply(root);

	//Aplicamos touch a cada nodo
    for (int i=0; i < sa.getPaths().getLength(); i++) 
	{
      SoFullPath * fp = (SoFullPath *)sa.getPaths()[i];
	  incrustarTexture2(fp->getTail());
    }

	//Refrescamos el editor de campos
	actualizar_fieldTable(mapQTCOIN[sceneTree->currentItem()]);

}//int Form1::incrustar_texturas_activated ()


//Esta funcion intercambia de un nodo a su manipulador m�s directo y viceversa
void Form1::convertManip()
{
    
    QTreeWidgetItem * item=sceneTree->currentItem(); 
    SoNode *nodo = mapQTCOIN[item];
    SoNode *newNode = NULL;

    if (nodo->getTypeId() == SoSpotLightManip::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoSpotLight();
    }  
    else
       
    if (nodo->getTypeId() == SoSpotLight::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoSpotLightManip();
    }  
    else

    if (nodo->getTypeId() == SoPointLightManip::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoPointLight();
    }  
    else
       
    if (nodo->getTypeId() == SoPointLight::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoPointLightManip();
    }  
    else
      
    if (nodo->getTypeId() == SoDirectionalLightManip::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoDirectionalLight();
    }  
    else
       
    if (nodo->getTypeId() == SoDirectionalLight::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoDirectionalLightManip();
    }  

    else
    if (nodo->getTypeId() == SoClipPlaneManip::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoClipPlane();
    }  

    else
    if (nodo->getTypeId() == SoClipPlane::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoClipPlaneManip();

		/* Intento de resolver un bug de coin3D. No funciona
		//Calcula el path del nodo
		SoPath *path =getPathFromItem(item);
		((SoClipPlaneManip*)newNode)->replaceNode(path);
		path->unref();

		//Configuramos el icono y el texto del item
		mapQTCOIN[item] = newNode;
		setNodeIcon(item);
		//Actualizamos la tabla  de campos
		actualizar_fieldTable (newNode);
		//Indicamos que la escena ha sido modificada
		escena_modificada = true
		return;
		//*/
    }  
	else
	{
		//Me han pasado un nodo no soportado
		QString S;
		S.sprintf("No puedo convertir %s en manip", nodo->getTypeId().getName().getString());
		QMessageBox::warning( this, tr("Warning"), S);

		return;
	}

	//Copiamos los fields del nodo original  que se llamen igual en newNode
	//Extraemos su lista de campos
	SoFieldList  fields;
	nodo->getFields(fields);
	int num_fields=fields.getLength();
	//Recorremos todos los fields del nodo antiguo
	for (int f=0; f < num_fields; f++)
	{
		//Leemos el nombre de este field
		SoField *field = fields[f];
		SbName nombre_field;
		nodo->getFieldName(field, nombre_field);
		//Miramos si el nuevo nodo tiene un field llamado igual
		SoField *dst_field = newNode->getField(nombre_field);
		if(dst_field && dst_field->isOfType(field->getTypeId()) )
		{
			//Copiamos el valor del campo
			dst_field->copyFrom(*field);
		}
	}//for (int f=0; f < num_fields; f++)

	//Reemplazamos el nodo en el grafo de escena
    QTreeWidgetItem *padre=item->parent();
    SoGroup *nodo_padre=(SoGroup*)mapQTCOIN[padre];
    nodo_padre->replaceChild(nodo, newNode);

    //Configuramos el icono y el texto del item
    mapQTCOIN[item] = newNode;
    setNodeIcon(item);

    //Actualizamos la tabla  de campos
    actualizar_fieldTable (newNode);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}

void Form1::SoTransform_to_SoCenterballManip()
{
   //Esta funcion tranforma el Transform en SoCenterballManip
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoCenterballManip * Manip=new SoCenterballManip;
   SoTransform * Trans=(SoTransform*)mapQTCOIN[item];

   //Ya tengo los dos elementos, paso a copiar los campos
   Manip->rotation.setValue(Trans->rotation.getValue());
   Manip->translation.setValue(Trans->translation.getValue());
   Manip->scaleFactor.setValue(Trans->scaleFactor.getValue());
   Manip->scaleOrientation.setValue(Trans->scaleOrientation.getValue());
   Manip->center.setValue(Trans->center.getValue()); 
   mapQTCOIN[item]=(SoNode*)Manip;

   //Ahora me falta reemplazar en el Arbol de Coin
   QTreeWidgetItem *padre=item->parent();
   SoGroup * nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(Trans,Manip);
   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (Manip);

   //Indicamos que la escena ha sido modificada
   escena_modificada = true;
}



void Form1::SoTransform_to_SoJackManip()
{
   //tranforma el Transform en SoJackManip
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoJackManip * Manip=new SoJackManip;
   SoTransform * Trans=(SoTransform*)mapQTCOIN[item];

   //Ya tengo los dos elementos, paso a copiar los campos
   Manip->rotation.setValue(Trans->rotation.getValue());
   Manip->translation.setValue(Trans->translation.getValue());
   Manip->scaleFactor.setValue(Trans->scaleFactor.getValue());
   Manip->scaleOrientation.setValue(Trans->scaleOrientation.getValue());
   Manip->center.setValue(Trans->center.getValue()); 
   mapQTCOIN[item]=(SoNode*)Manip;

   //Ahora me falta reemplazar en el Arbol de Coin
   QTreeWidgetItem *padre=item->parent();
   SoGroup * nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(Trans,Manip);
   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (Manip);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}


   //transforma el Transform en SoTrackballManip
void Form1::SoTransform_to_SoHandleBoxManip()
{
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoHandleBoxManip * Manip=new SoHandleBoxManip;
   SoTransform * Trans=(SoTransform*)mapQTCOIN[item];
   //Ya tengo los dos elementos, paso a copiar los campos
   Manip->rotation.setValue(Trans->rotation.getValue());
   Manip->translation.setValue(Trans->translation.getValue());
   Manip->scaleFactor.setValue(Trans->scaleFactor.getValue());
   Manip->scaleOrientation.setValue(Trans->scaleOrientation.getValue());
   Manip->center.setValue(Trans->center.getValue()); 
   mapQTCOIN[item]=(SoNode*)Manip;
   //Ahora me falta reemplazar en el Arbol de Coin
   QTreeWidgetItem *padre=item->parent();
   SoGroup * nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(Trans,Manip);
   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (Manip);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}


void Form1::SoTransform_to_SoTrackballManip()
{
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoTrackballManip * Manip=new SoTrackballManip;
   SoTransform * Trans=(SoTransform*)mapQTCOIN[item];
   //Ya tengo los dos elementos, paso a copiar los campos
   Manip->rotation.setValue(Trans->rotation.getValue());
   Manip->translation.setValue(Trans->translation.getValue());
   Manip->scaleFactor.setValue(Trans->scaleFactor.getValue());
   Manip->scaleOrientation.setValue(Trans->scaleOrientation.getValue());
   Manip->center.setValue(Trans->center.getValue()); 
   mapQTCOIN[item]=(SoNode*)Manip;
   //Ahora me falta reemplazar en el Arbol de Coin
   QTreeWidgetItem *padre=item->parent();
   SoGroup * nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(Trans,Manip);
   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (Manip);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}


void Form1::SoTransform_to_SoTransformerManip()
{
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoTransformerManip * Manip=new SoTransformerManip;
   SoTransform * Trans=(SoTransform*)mapQTCOIN[item];
   //Ya tengo los dos elementos, paso a copiar los campos
   Manip->rotation.setValue(Trans->rotation.getValue());
   Manip->translation.setValue(Trans->translation.getValue());
   Manip->scaleFactor.setValue(Trans->scaleFactor.getValue());
   Manip->scaleOrientation.setValue(Trans->scaleOrientation.getValue());
   Manip->center.setValue(Trans->center.getValue()); 
   mapQTCOIN[item]=(SoNode*)Manip;
   //Ahora me falta reemplazar en el Arbol de Coin
   QTreeWidgetItem *padre=item->parent();
   SoGroup * nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(Trans,Manip);
   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (Manip);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}


void Form1::SoTransform_to_SoTabBoxManip()
{
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoTabBoxManip * Manip=new SoTabBoxManip;
   SoTransform * Trans=(SoTransform*)mapQTCOIN[item];
   //Ya tengo los dos elementos, paso a copiar los campos
   Manip->rotation.setValue(Trans->rotation.getValue());
   Manip->translation.setValue(Trans->translation.getValue());
   Manip->scaleFactor.setValue(Trans->scaleFactor.getValue());
   Manip->scaleOrientation.setValue(Trans->scaleOrientation.getValue());
   Manip->center.setValue(Trans->center.getValue()); 
   mapQTCOIN[item]=(SoNode*)Manip;
   //Ahora me falta reemplazar en el Arbol de Coin
   QTreeWidgetItem *padre=item->parent();
   SoGroup * nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(Trans,Manip);
   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (Manip);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}

void Form1::SoTransform_to_SoTransformBoxManip()
{
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoTransformBoxManip * Manip=new SoTransformBoxManip;
   SoTransform * Trans=(SoTransform*)mapQTCOIN[item];
   //Ya tengo los dos elementos, paso a copiar los campos
   Manip->rotation.setValue(Trans->rotation.getValue());
   Manip->translation.setValue(Trans->translation.getValue());
   Manip->scaleFactor.setValue(Trans->scaleFactor.getValue());
   Manip->scaleOrientation.setValue(Trans->scaleOrientation.getValue());
   Manip->center.setValue(Trans->center.getValue()); 
   mapQTCOIN[item]=(SoNode*)Manip;
   //Ahora me falta reemplazar en el Arbol de Coin
   QTreeWidgetItem *padre=item->parent();
   SoGroup * nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(Trans,Manip);
   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (Manip);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}

void Form1::SoMatrixTransform_to_SoTransform()
{
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoTransform* trans=new SoTransform;
   SoMatrixTransform * matTrans=(SoMatrixTransform*)mapQTCOIN[item];

   //Ya tengo los dos elementos, paso a copiar los campos
   trans->setMatrix(matTrans->matrix.getValue());
   mapQTCOIN[item]=(SoNode*)trans;

   //Ahora me falta reemplazar en el Arbol de Coin
   QTreeWidgetItem *padre=item->parent();
   SoGroup * nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(matTrans,trans);

   mapQTCOIN[item]=(SoNode*)trans;
   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (trans);

   //Indicamos que la escena ha sido modificada
   escena_modificada = true;
}



void Form1::SoRotation_to_SoTrackballManip()
{
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoTrackballManip *Manip=new SoTrackballManip;
   SoRotation *rot=(SoRotation*)mapQTCOIN[item];
   //Ya tengo los dos elementos, paso a copiar los campos
   Manip->rotation.setValue(rot->rotation.getValue());
   //Reemplazo en el mapQTCOIN por el manip
   mapQTCOIN[item]=(SoNode*)Manip;
   //Ahora me falta reemplazar en el Arbol de Coin
   QTreeWidgetItem *padre=item->parent();
   SoGroup *nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(rot,Manip);
   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (Manip);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

} // void Form1::SoRotation_to_SoTrackballManip()


void Form1::SoManip_to_SoTransform()
{
    
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoTransform * Trans=new SoTransform;
   SoTransform *old= (SoTransform*)mapQTCOIN[item];
   Trans->rotation.setValue(old->rotation.getValue());
   Trans->translation.setValue(old->translation.getValue());
   Trans->scaleFactor.setValue(old->scaleFactor.getValue());
   Trans->scaleOrientation.setValue(old->scaleOrientation.getValue());
   Trans->center.setValue(old->center.getValue()); 
   mapQTCOIN[item]=(SoNode*)Trans;
   QTreeWidgetItem *padre=item->parent();
   SoGroup * nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(old,Trans);
   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (Trans);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}// void Form1::SoManip_to_SoTransform()


void Form1::SoTrackballManip_to_SoRotation()
{
    
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoRotation *rot=new SoRotation;
   SoTrackballManip *old= (SoTrackballManip*)mapQTCOIN[item];
   rot->rotation.setValue(old->rotation.getValue());
   mapQTCOIN[item]=(SoNode*)rot;
   QTreeWidgetItem *padre=item->parent();
   SoGroup * nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(old,rot);
   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (rot);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

} //void Form1::SoTrackballManip_to_SoRotation()


void Form1::SoIndexedFaceSet_to_SoIndexedLineSet()
{
    
   QTreeWidgetItem * item=sceneTree->currentItem(); 
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
   actualizar_fieldTable (newNode);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

} // void Form1::SoIndexedFaceSet_to_SoIndexedLineSet()

void Form1::SoIndexedLineSet_to_SoIndexedFaceSet()
{
    
   QTreeWidgetItem * item=sceneTree->currentItem(); 
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
   actualizar_fieldTable (newNode);

   //Indicamos que la escena ha sido modificada
   escena_modificada = true;

} // void Form1::SoIndexedFaceSet_to_SoIndexedLineSet()


///Exporta un SoIndexedFaceSet o SoVRMLIndexedFaceSet a un fichero SMF
void Form1::SoIndexedFaceSet_to_SMF()
{
	//Nombre del fichero donde escribir
    QString nombre_fich;

    nombre_fich = QFileDialog::getSaveFileName("",
             tr("Ficheros SMF (*.smf *.obj);;Todos los ficheros (*)"),
             this,
             tr("Exportar a fichero SMF/OBJ"));

    //Miramos si se pulso el boton cancelar
    if (nombre_fich=="")
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

	QTreeWidgetItem * item=sceneTree->currentItem(); 
	SoNode *nodo = mapQTCOIN[item];

    //Abrimos el fichero de salida
    FILE *file = fopen(nombre_fich.ascii(), "w");

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

}// void Form1::SoIndexedFaceSet_to_SMF()

///Exporta un SoIndexedFaceSet o SoVRMLIndexedFaceSet a un fichero OFF
void Form1::SoIndexedFaceSet_to_OFF()
{
    QTreeWidgetItem * item=sceneTree->currentItem(); 

    //Construimos un path  segun la información del arbol de QT
	SoNode *nodo = mapQTCOIN[item];

    //Solicitamos un nombre de fichero
    QString nombre_fich;
    nombre_fich = QFileDialog::getSaveFileName("",
             tr("Ficheros OFF (*.off);;Todos los ficheros (*)"),
             this,
             tr("Exportar a fichero OFF"));

    //Miramos si se pulso el boton cancelar
    if (nombre_fich=="")
        return;

    //Abrimos el fichero de salida
    FILE *file = fopen(nombre_fich.ascii(), "w");

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

}// void Form1::SoIndexedFaceSet_to_OFF()


///Exporta un SoIndexedFaceSet o SoVRMLIndexedFaceSet a un fichero STL
void Form1::SoIndexedFaceSet_to_STL()
{
    QTreeWidgetItem * item=sceneTree->currentItem(); 

    //Construimos un path  segun la información del arbol de QT
	SoNode *nodo = mapQTCOIN[item];

    //Solicitamos un nombre de fichero
    QString nombre_fich;
    nombre_fich = QFileDialog::getSaveFileName("",
             tr("Ficheros STL (*.stl);;Todos los ficheros (*)"),
             this,
             tr("Exportar a fichero STL"));

    //Miramos si se pulso el boton cancelar
    if (nombre_fich=="")
        return;

    //Abrimos el fichero de salida
    FILE *file = fopen(nombre_fich.ascii(), "w");

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

}// void Form1::SoIndexedFaceSet_to_STL()


///Triangula las facetas de más de tres lados de un SoIndexedFaceSet o SoVRMLIndexedFaceSet
void Form1::SoIndexedFaceSet_triangulate()
{
    QTreeWidgetItem * item=sceneTree->currentItem(); 
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

}// void Form1::SoIndexedFaceSet_triangulate()


void Form1::SoIndexedFaceSet_change_orientation()
{
    QTreeWidgetItem * item=sceneTree->currentItem(); 
    SoIndexedFaceSet *ifs = (SoIndexedFaceSet *)mapQTCOIN[item];
   
   IndexedFaceSet_change_orientation (ifs->coordIndex);
   IndexedFaceSet_change_orientation (ifs->materialIndex);
   IndexedFaceSet_change_orientation (ifs->normalIndex);
   IndexedFaceSet_change_orientation (ifs->textureCoordIndex);

}// void Form1::SoIndexedFaceSet_change_orientation()


void Form1::SoCoordinate3_center_new()
{
    //Identificamos el item actual
    QTreeWidgetItem * item=sceneTree->currentItem();
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
    actualizar_fieldTable (mapQTCOIN[item]);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}// void Form1::SoCoordinate3_center_new()


void Form1::SoCoordinate3_to_XYZ()
{
    //Solicitamos un nombre de fichero
    QString nombre_XYZ;
    nombre_XYZ = QFileDialog::getSaveFileName("",
             tr("Ficheros XYZ (*.xyz);;Todos los ficheros (*)"),
             this,
             tr("Exportar a fichero XYZ"));

    //Miramos si se pulso el boton cancelar
    if (nombre_XYZ=="")
        return;

    //Abrimos el fichero de salida
    FILE *file = fopen(nombre_XYZ.ascii(), "w");

    //Identificamos el item actual
    QTreeWidgetItem * item=sceneTree->currentItem();

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

}// void Form1::SoCoordinate3_to_XYZ()


void Form1::SoCoordinate3_to_qhull()
{
    //Identificamos el item actual
    QTreeWidgetItem * item=sceneTree->currentItem();
    SoCoordinate3 *nodo = (SoCoordinate3 *)mapQTCOIN[item];

    //Creamos un coordinate3 y un indexedFaceSet para el cierre
    SoVertexProperty  *coord = new SoVertexProperty();
    SoIndexedFaceSet *ifs = new SoIndexedFaceSet();
    ifs->vertexProperty = coord;

    //Creamos el cierre convexo
    int result = convex_hull (nodo->point, coord->vertex, ifs->coordIndex );

    if (result < 0)
    {
           QString S;
           S.sprintf("Error al crear cierre convexo");
           QMessageBox::warning( this, tr("Warning"), S);
	   return;
    }

    //Cambiamos el nombre del nuevo nodo
    SoBase_name->setValue("convex_hull");
    ifs->setName(SoBase_name->getValue());

    //Buscamos el padre del nodo Coordinate3 que contiene la nube de puntos
    QTreeWidgetItem *padre=item->parent();
    while (!mapQTCOIN[padre]->getTypeId().isDerivedFrom(SoGroup::getClassTypeId())) 
    {
		padre=padre->parent();
    }
 
    //Colgamos el cierre convexo y actualizamos los mapas de QT
    SoGroup* Anclaje=(SoGroup*)mapQTCOIN[padre];
    Anclaje->addChild(ifs);  
    InsertarElemento(padre, ifs);

    //Actualizamos la tabla  de campos
    actualizar_fieldTable (ifs);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}// void Form1::SoCoordinate3_to_qhull()


void Form1::SoIndexedTriangleStripSet_to_SoIndexedFaceSet()
{
    
   QTreeWidgetItem * item=sceneTree->currentItem(); 
   SoIndexedTriangleStripSet *oldNode= (SoIndexedTriangleStripSet*)mapQTCOIN[item];

   SoIndexedFaceSet *newNode= IndexedTriangleStripSet_to_IndexedFaceSet (oldNode);

   mapQTCOIN[item]=(SoNode*)newNode;
   QTreeWidgetItem *padre=item->parent();
   SoGroup *nodo_padre=(SoGroup*)mapQTCOIN[padre];
   nodo_padre->replaceChild(oldNode,newNode);

   //Configuramos el icono y el texto del item
   setNodeIcon(item);

   //Actualizamos la tabla  de campos
   actualizar_fieldTable (newNode);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

} // void Form1::SoIndexedTriangleStripSet_to_SoIndexedFaceSet()




void Form1::showmenu()
{
    //Creamos un menu generico
    QPopupMenu* pm = new QPopupMenu( this );
    pm->insertItem(QPixmap::fromMimeSource("editcut.png"), tr("&Cortar"),  this, SLOT(menu_edit_cut_action()), CTRL+Key_X );
    pm->insertItem(QPixmap::fromMimeSource("editcopy.png"), tr("&Copiar"),  this, SLOT(menu_edit_copy_action()), CTRL+Key_C);
    pm->insertItem(QPixmap::fromMimeSource("editpaste.png"), tr("&Pegar"),  this, SLOT(menu_edit_paste_action()), CTRL+Key_V );
    pm->insertItem(QPixmap::fromMimeSource("editdelete.png"), tr("&Eliminar"),  this, SLOT(menu_edit_delete_action()), Key_Delete);
    pm->insertItem(QPixmap::fromMimeSource("up.png"), tr("&Subir"),  this, SLOT(Subir_nodo_action()), CTRL+Key_Up );
    pm->insertItem(QPixmap::fromMimeSource("down.png"), tr("&Bajar"),  this, SLOT(Bajar_nodo_action()), CTRL+Key_Down );

    //Identificamos el item actual
    QTreeWidgetItem * item=sceneTree->currentItem();
    SoNode *nodo = mapQTCOIN[item];
    SoType  tipo = nodo->getTypeId();

    //Añadimos opciones dependiendo del tipo de nodo
    
    if (tipo.isDerivedFrom(SoGroup::getClassTypeId())) 
    {
      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Promocionar hijos"),  this, SLOT(Promocionar_hijos()));
    }

    if (tipo.isDerivedFrom(SoTransform::getClassTypeId())) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoCenterballManip"),  this, SLOT(SoTransform_to_SoCenterballManip()));
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoHandleBoxManip"), this, SLOT(SoTransform_to_SoHandleBoxManip()));
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoJackManip"), this, SLOT(SoTransform_to_SoJackManip()));
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoTabBoxManip"), this, SLOT(SoTransform_to_SoTabBoxManip()));
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoTrackballManip"),  this, SLOT(SoTransform_to_SoTrackballManip()));
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoTransformBoxManip"), this, SLOT(SoTransform_to_SoTransformBoxManip()));
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoTransformerManip"), this, SLOT(SoTransform_to_SoTransformerManip()));
      
       //Manipuladores
       if (tipo.isDerivedFrom(SoTransformManip::getClassTypeId())) 
       {
         pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoTransform"), this, 
			         SLOT(SoManip_to_SoTransform()) );

         if (tipo == SoTrackballManip::getClassTypeId()) 
         {
           pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoRotation"), this, 
			           SLOT(SoTrackballManip_to_SoRotation()));
         }
       }

    }
    else 

    if (tipo == SoMatrixTransform::getClassTypeId()) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoTransform"),  this, SLOT(SoMatrixTransform_to_SoTransform()) );
    }
    else 

    if (tipo == SoRotation::getClassTypeId()) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoTrackballManip"),  this, SLOT(SoRotation_to_SoTrackballManip()) );
    }

    if (tipo == SoDirectionalLight::getClassTypeId()) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"), tr("Convertir en SoDirectionalLightManip"),  this, SLOT(convertManip()) );
    }
    else 

    if (tipo == SoSpotLight::getClassTypeId()) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"), tr("Convertir en SoSpotLightManip"),  this, SLOT(convertManip()) );
    }
    else 

    if (tipo == SoPointLight::getClassTypeId()) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"), tr("Convertir en SoPointLightManip"),  this, SLOT(convertManip()) );
    }
    else

    if (tipo == SoDirectionalLightManip::getClassTypeId()) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"), tr("Convertir en SoDirectionalLight"),  this, SLOT(convertManip()) );
    }
    else 

    if (tipo == SoSpotLightManip::getClassTypeId()) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"), tr("Convertir en SoSpotLight"),  this, SLOT(convertManip()) );
    }
    else 

    if (tipo == SoPointLightManip::getClassTypeId()) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"), tr("Convertir en SoPointLight"),  this, SLOT(convertManip()) );
    }
    else 

    if (tipo == SoClipPlaneManip::getClassTypeId()) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"), tr("Convertir en SoClipPlane"),  this, SLOT(convertManip()) );
    }
    else 

    if (tipo == SoClipPlane::getClassTypeId()) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"), tr("Convertir en SoClipPlaneManip"),  this, SLOT(convertManip()) );
    }
    else 

    if (tipo.isDerivedFrom(SoIndexedFaceSet::getClassTypeId())) 
    {
      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Triangular facetas"),this,SLOT(SoIndexedFaceSet_triangulate()) );
      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Cambiar orientacion facetas"),this,SLOT(SoIndexedFaceSet_change_orientation()) );
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoIndexedLineSet"),this,SLOT(SoIndexedFaceSet_to_SoIndexedLineSet()) );
      pm->insertItem(QPixmap::fromMimeSource("filesaveas.png"), tr("Exportar a fichero SMF/OBJ"),this,SLOT(SoIndexedFaceSet_to_SMF()));
      pm->insertItem(QPixmap::fromMimeSource("filesaveas.png"),tr("Exportar a fichero OFF"),this,SLOT(SoIndexedFaceSet_to_OFF()));
      pm->insertItem(QPixmap::fromMimeSource("filesaveas.png"),tr("Exportar a fichero STL"),this,SLOT(SoIndexedFaceSet_to_STL()));
      if (settings.readEntry("/coindesigner/qslim_app"))
	      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Simplificar con QSLIM"),this,SLOT(qslim_activated ()));
      if (settings.readEntry("/coindesigner/tetgen_app"))
	      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Tetrahedralizar con TETGEN"),this,SLOT(tetgen_activated ()));
      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Recubrimiento"),this,SLOT(recubrimiento2()));
    }
    else 
    
    if (tipo.isDerivedFrom(SoIndexedLineSet::getClassTypeId())) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoIndexedFaceSet"),  this, SLOT(SoIndexedLineSet_to_SoIndexedFaceSet()) );
    }
    else 
    
    if (tipo.isDerivedFrom(SoCoordinate3::getClassTypeId())) 
    {
      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Centrar en origen"),  this, SLOT(SoCoordinate3_center_new()) );
      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Calcular cierre convexo"),  this, SLOT(SoCoordinate3_to_qhull()) );
      pm->insertItem(QPixmap::fromMimeSource("filesaveas.png"), tr("Exportar a fichero XYZ"),  this, SLOT(SoCoordinate3_to_XYZ()) );
    }
    else 
    
    if (tipo.isDerivedFrom(SoVertexProperty::getClassTypeId())) 
    {
      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Centrar en origen"),  this, SLOT(SoCoordinate3_center_new()) );
      pm->insertItem(QPixmap::fromMimeSource("filesaveas.png"), tr("Exportar a fichero XYZ"),  this, SLOT(SoCoordinate3_to_XYZ()) );
    }
    else 
    
    if (tipo.isDerivedFrom(SoVRMLIndexedFaceSet::getClassTypeId())) 
    {
      pm->insertItem(tr("Triangular facetas"),    this,SLOT(SoIndexedFaceSet_triangulate()) );
      pm->insertItem(QPixmap::fromMimeSource("filesaveas.png"),tr("Exportar a fichero SMF/OBJ"),this,SLOT(SoIndexedFaceSet_to_SMF()));
      pm->insertItem(QPixmap::fromMimeSource("filesaveas.png"),tr("Exportar a fichero OFF"),this,SLOT(SoIndexedFaceSet_to_OFF()));
      pm->insertItem(QPixmap::fromMimeSource("filesaveas.png"),tr("Exportar a fichero STL"),this,SLOT(SoIndexedFaceSet_to_STL()));
      if (settings.readEntry("/coindesigner/qslim_app"))
	      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Simplificar con QSLIM"),this,SLOT(qslim_activated ()));
      if (settings.readEntry("/coindesigner/tetgen_app"))
	      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Tetrahedralizar con TETGEN"),this,SLOT(tetgen_activated ()));
    }
    else 
    
    if (tipo.isDerivedFrom(SoIndexedTriangleStripSet::getClassTypeId())) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoIndexedFaceSet"),  this, SLOT(SoIndexedTriangleStripSet_to_SoIndexedFaceSet()) );
    }
   
    if (tipo.isDerivedFrom(SoTexture2::getClassTypeId()) )
    {
       //Miramos si este nodo tiene algun nombre de fichero asociado
       SoTexture2 *tex= (SoTexture2 *)nodo;
       if (tex->filename.getValue().getLength() > 0)
          pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Incrustar fichero de imagen"),  this, SLOT(incrustarTexture2()) );
    }
    else
    if (tipo.isDerivedFrom(SoBumpMap::getClassTypeId()) )
    {
       //Miramos si este nodo tiene algun nombre de fichero asociado
       SoBumpMap *tex= (SoBumpMap *)nodo;
       if (tex->filename.getValue().getLength() > 0)
          pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Incrustar fichero de imagen"),  this, SLOT(incrustarTexture2()) );
    }
   
 //Mostramos el menu contextual
 pm->exec( QCursor::pos() );
           
} //void Form1::showmenu()

void Form1::Form1_destroyed( QObject * )
{
    __chivato__;
    Form1::menu_escena_salir();
}
                          

void Form1::cargar_fichero_locale(const char *fichero)
{
    //Leemos el locale y extraemos la parte del lenguaje
    QString loc = QTextCodec::locale();
    QString lang = loc.section('.',0,0);

    //fprintf(stderr, "loc=%s\n", loc.ascii() );
    //fprintf(stderr, "lang=%s\n", lang.ascii() );

    //Construimos el nombre del fichero con el lenguaje y la extensión
    QString Path;
    Path.sprintf("%s/%s_%s.html", cds_dir, fichero, lang.ascii());

    //Comprobamos si existe el fichero, 
    if (!QFile::exists(Path))
    {
       // Si no existe, probamos otro idioma similar (ejs: es_AR -> es)
       Path.sprintf("%s/%s_%s.html", cds_dir, fichero, 
                                     lang.section('_',0,0).ascii());

       //Si no existe intentamos con el ingles.
       if (!QFile::exists(Path))
       {
          fprintf(stderr, "%s file not found. Trying english version.\n",
                          Path.ascii() );
          Path.sprintf("%s/%s_en.html", cds_dir, fichero);
       }
    }

    //Creamos el path al fichero de tutorial
    //QString path = settings.readEntry("/coindesigner/tutorial_dir")+filename+".html";
    this->open_html_viewer(Path);

} // void Form1::cargar_fichero_locale(const char *fichero)

 
void Form1::nada()
{
}


//Esta función se encarga de mirar si tenemos programado algún ayudante
//que nos permita editar fácilmente el campo. Los ayudantes son 
//dialogos del tipo QFileDialog, QColorDialog o PopupMenus
//El resto de campos se dejan para la funcion fieldTable_valueChanged()
void Form1::fieldTable_clicked(int fila, int , int button)
{
    
   //Si estamos vigilando la escena, dejamos de hacerlo para
   //poder editar los valores de la misma.
   refreshGUI_but->setOn(false);

    //Buscamos el nodo que estamos editando
    SoNode *nodo=mapQTCOIN[sceneTree->currentItem()];
    nodo = edit_node;
    assert(nodo);

    //Buscamos el SoField que estamos editando
    SoField* field= map_fieldTable[fila];

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
       //Conversión del SoField que estamos editando
       SoSFBool *soSFBool= (SoSFBool *)field;

       //Preparamos un menu flotante con opciones true/false
       QPopupMenu* popm = new QPopupMenu( this );
       popm->insertItem("TRUE",this, SLOT(nada()), CTRL+Key_T,1);
       popm->insertItem("FALSE",this,SLOT(nada()), CTRL+Key_F,0);

       //Mostramos el menu flotante y recogemos la opción escogida
       int idx = popm->exec(QCursor::pos());

       //Comprobamos que se ha seleccionado una opci�n v�lida.
       if (idx < 0)
           return;

       soSFBool->setValue(idx);
    }
    else
    
    //Edicion de cualquier campo tipo SoSFEnum
    //Edicion de cualquier campo tipo SoSFBitMask
    if (!strcmp(nombre_tipo, "SFEnum") ||
        !strcmp(nombre_tipo, "SFBitMask") )
    {
       //Convertimos el tipo de field
       SoSFEnum *soSFEnum= (SoSFEnum *)field;

       //Preparamos un menu flotante 
       QPopupMenu* popm = new QPopupMenu( this );

       SbName nombre;
       int idx;
       //Probamos todos los indices y creamos una opcion por cada
       for (idx=0; idx < soSFEnum->getNumEnums(); idx++)
       {
           soSFEnum->getEnum(idx, nombre);
           popm->insertItem(nombre.getString(),this, SLOT(nada()),0,idx);
       }

       //Mostramos el menu flotante y recogemos la opción escogida
       idx = popm->exec(QCursor::pos());

       //Comprobamos que se ha seleccionado una opci�n v�lida.
       if (idx < 0)
           return;

       //Asignamos la opcion escogida
       soSFEnum->getEnum(idx, nombre);
       soSFEnum->setValue(nombre);
    }
    else
    
    //Edicion de cualquier campo tipo SoSFString
    if (!strcmp(nombre_tipo, "SFString") )
    {
       //Miramos si hace falta la asistencia de un QFileDialog
       //Si pulsamos con el boton derecho, lanzamos el QFileDialog
       if (button==2)
       {
          QString s = QFileDialog::getOpenFileName(currentDir.ascii(),
                  tr("Im&aacute;genes (*.jpg *.gif *.png *.rgb);;"
                     "Ficheros openInventor (*.iv *.wrl);;"
                     "Todos los ficheros (*)"),
                  this,
                  tr("Fichero a cargar") );

          //Asignamos el valor escogido
          SoSFString *soSFString = (SoSFString *)field;
          soSFString->setValue(s.ascii());
       }
    }
    else

    //Edicion de cualquier campo tipo SoSFNode
    if (!strcmp(nombre_tipo, "SFNode") )
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
                actualizar_fieldTable (node);
                return;
            }
       }
    }
    else
  
    //Edicion de cualquier campo tipo SoMFNode
    if (!strcmp(nombre_tipo, "MFNode") )
    {
       //Con el boton 2, editamos este nodo
       if (button==2)
       {
             //Buscamos la posicion contando cuentas filas por
             //encima de nosotros apuntan al mismo campo
             int pos=0;
             while(map_fieldTable[fila-pos-1] == field)
               pos++;

             //Le damos el valor adecuado en la posición pos
            SoNode *node = ((SoMFNode *)field)->getNode(pos);

            if (node)
            {
                //Mostramos el nodo en la tabla de edicion de campos
                actualizar_fieldTable (node);
                return;
            }
       }
    }
    else
  
    //Edicion de cualquier campo tipo SoSFColor
    if (!strcmp(nombre_tipo, "SFColor"))
    {
        //Convertimos el tipo de field
       SoSFColor *color= (SoSFColor *)field;
       //Lo convertimos en valores RGB y en un QColor
       const float*rgb = color->getValue().getValue();
       QColor c( int(255*rgb[0]), int(255*rgb[1]), int(255*rgb[2]) );

       //Solicitamos un color mediante QColorDialog
       c=QColorDialog::getColor(c);
       if (c.isValid() )
       {           
           //Modificamos el field
           color->setValue(c.red()/255.0, c.green()/255.0, c.blue()/255.0);
       }
    }
    else

    //Edicion de cualquier campo tipo SoMF....
    if (!strcmp(nombre_tipo, "MFColor") ||
        !strcmp(nombre_tipo, "MFVec2f") ||
        !strcmp(nombre_tipo, "MFVec3f") ||
        !strcmp(nombre_tipo, "MFVec4f") ||
        !strcmp(nombre_tipo, "MFFloat") ||
        !strcmp(nombre_tipo, "MFInt32") ||
        !strcmp(nombre_tipo, "MFUInt32") ||
        !strcmp(nombre_tipo, "MFShort") ||
        !strcmp(nombre_tipo, "MFString") )
    {
        //Usamos el formulario auxiliar
        mfield_editor mfield_ed(this);
        mfield_ed.cargarDatos((SoMField *)field);
        mfield_ed.exec();
    }
    else

    //Edicion de cualquier campo tipo SoSFMatrix
    if (!strcmp(nombre_tipo, "SFMatrix") )
    {
        //Usamos el formulario auxiliar
        matrix_editor matrix_ed(this);
        matrix_ed.cargarDatos((SoSFMatrix *)field);
        matrix_ed.exec();
    }

    else
    {
      //No hay ayudante para este campo
      return;
    }

    //Actualizamos el contenido de la tabla
    actualizar_fieldTable(nodo);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

} // void Form1::fieldTable_clicked(int fila, int columna, int button)


void Form1::exportarVRML2_activated()
{

} //void Form1::exportarVRML2_activated()


void Form1::ImportarGeometria(void)
{
  ImportarGeometria("");
}


void Form1::menuCargar_EscenaAction_activated()
{
  CargarEscena("");
}// void Form1::menuCargar_EscenaAction_activated()


//Aplica ivfix sobre una escena de coindesigner
extern SoNode *ivfix_result;
int Form1::fix_scene_activated ()
{
    //indicamos que se aplique a toda la escena
    ivfix_result = root;

    //Mostramos el dialogo de ivfix
    ivfix_options ivfix_options_dlg(this);
    ivfix_options_dlg.exec();
    
    //Miramos si hay resultados en ivfix_result
    if (ivfix_result != NULL)
    {
       //Ahora destruimos la escena actual...
       Form1::menu_escena_nueva_action();
    
       //Colgamos el nodo del grafo de escena
       QTreeWidgetItem *padre=sceneTree->firstChild();
 
       root->addChild(ivfix_result);  
       InsertarElemento(padre, ivfix_result);

       //Actualizamos la tabla  de campos
       actualizar_fieldTable (ivfix_result);

       //Indicamos que la escena ha sido modificada
       escena_modificada = true;

       return 0;
    }
    else
       return -1;

} // int Form1::fix_scene_activated ()


//Aplica qslim a un nodo de la escena
extern SoNode *qslim_result;
int Form1::qslim_activated ()
{
    if (!settings.readEntry("/coindesigner/qslim_app"))
    {
       QString S;
       S = tr("No se ha encontrado la herramienta QSLIM");
       QMessageBox::warning( this, tr("Error"), S);
       return -1;
    }

    //Buscamos el nodo actual
    QTreeWidgetItem * item=sceneTree->currentItem(); 

    //Construimos un path  segun la información del arbol de QT
    SoNode *nodo = mapQTCOIN[item];

    //Creamos el fichero qslim_in.smf
    FILE *file = fopen("qslim_in.smf", "w");

    //Escribimos el contenido en el fichero SMF
    if (nodo->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId())) 
    {
		SoPath *path =getPathFromItem(item);
		IndexedFaceSet_to_SMF(path, file);
		path->unref();
    }
    else if (nodo->getTypeId().isDerivedFrom(SoVRMLIndexedFaceSet::getClassTypeId())) 
    {
		VRMLIndexedFaceSet_to_SMF((SoVRMLIndexedFaceSet *)nodo, file);
    }
    else
      return -1;

    //Cerramos el fichero
    fclose(file);

    //Mostramos el dialogo de qslim
    qslim_options qslim_options(this);
    qslim_options.exec();
    
	//Borramos el fichero de entrada
	QFile::remove("qslim_in.smf");

    //Miramos si hay resultados en qslim_result
    if (qslim_result == NULL)
		return -1;

    //Asignamos el nombre del fichero al separator
    SoBase_name->setValue("qslim_output");
    qslim_result->setName(SoBase_name->getValue());

    //Buscamos el primer SoGroup que contiene al nodo actual
    QTreeWidgetItem *padre=sceneTree->currentItem()->parent();
    nodo = mapQTCOIN[padre];
    while (!nodo->getTypeId().isDerivedFrom(SoGroup::getClassTypeId()) && 
           !nodo->getTypeId().isDerivedFrom(SoVRMLGroup::getClassTypeId()) ) 
    {
       padre=padre->parent();
       nodo = mapQTCOIN[padre];
    }
 
    //Colgamos el nodo qslim_result del grafo de escena
    SoGroup* Anclaje=(SoGroup*)nodo;
    Anclaje->addChild(qslim_result);  
    InsertarElemento(padre, qslim_result);

    //Actualizamos la tabla  de campos
    actualizar_fieldTable (qslim_result);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

    return 0;

} // int Form1::qslim_activated ()


//Aplica tetgen a un nodo de la escena
extern SoNode *tetgen_result;
int Form1::tetgen_activated ()
{
    if (!settings.readEntry("/coindesigner/tetgen_app"))
    {
       QString S;
       S = tr("No se ha encontrado la herramienta TETGEN");
       QMessageBox::warning( this, tr("Error"), S);
       return -1;
    }

    //Buscamos el nodo actual
    QTreeWidgetItem * item=sceneTree->currentItem(); 

    //Construimos un path  segun la información del arbol de QT
    SoNode *nodo = mapQTCOIN[item];

    //Creamos el fichero tetgen_in.off
    FILE *file = fopen("tetgen_in.off", "w");

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
    else
      return -1;

    //Cerramos el fichero
    fclose(file);

    //Mostramos el dialogo de tetgen
    tetgen_options tetgen_options(this);
    tetgen_options.exec();
    
    //Borramos el fichero de entrada
    QFile::remove("tetgen_in.off");

    //Miramos si hay resultados en tetgen_result
    if (tetgen_result == NULL)
		return -1;

    //Asignamos el nombre del fichero al separator
    SoBase_name->setValue("tetgen_output");
    tetgen_result->setName(SoBase_name->getValue());

    //Buscamos el primer SoGroup que contiene al nodo actual
    QTreeWidgetItem *padre=sceneTree->currentItem()->parent();
    nodo = mapQTCOIN[padre];
    while (!nodo->getTypeId().isDerivedFrom(SoGroup::getClassTypeId()) && 
           !nodo->getTypeId().isDerivedFrom(SoVRMLGroup::getClassTypeId()) ) 
    {
       padre=padre->parent();
       nodo = mapQTCOIN[padre];
    }
 
    //Colgamos el nodo tetgen_result del grafo de escena
    SoGroup* Anclaje=(SoGroup*)nodo;
    Anclaje->addChild(tetgen_result);  
    InsertarElemento(padre, tetgen_result);

    //Actualizamos la tabla  de campos
    actualizar_fieldTable (tetgen_result);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

    return 0;

} // int Form1::tetgen_activated ()




int Form1::recubrimiento2()
{
	//Consultamos la cantidad que queremos hinchar
	bool ok;
    double distance = QInputDialog::getDouble(
            tr("Recubrimiento"), tr("Distancia del recubrimiento:"), 1, -1000, 1000, 2, &ok, this );
    if (!ok)
		return -1;

    //Buscamos el nodo actual
    QTreeWidgetItem * item=sceneTree->currentItem(); 

    //Construimos un path  segun la información del arbol de QT
    SoNode *nodo = mapQTCOIN[item];
	SoPath *path =getPathFromItem(item);

	// Hinchado de facetas
	SoSeparator *recubr_result= ifs_recubrimiento2(path, distance);

	path->unref();

	//Miramos si hay resultados en recubr_result
    if (recubr_result == NULL)
		return -1;

    //Asignamos el nombre del fichero al separator
    SoBase_name->setValue("recubrimiento");
    recubr_result->setName(SoBase_name->getValue());

    //Buscamos el primer SoGroup que contiene al nodo actual
    QTreeWidgetItem *padre=sceneTree->currentItem()->parent();
    nodo = mapQTCOIN[padre];
    while (!nodo->getTypeId().isDerivedFrom(SoGroup::getClassTypeId()) && 
           !nodo->getTypeId().isDerivedFrom(SoVRMLGroup::getClassTypeId()) ) 
    {
       padre=padre->parent();
       nodo = mapQTCOIN[padre];
    }
 
    //Colgamos el nodo recubr_result del grafo de escena
    SoGroup* Anclaje=(SoGroup*)nodo;
    Anclaje->addChild(recubr_result);  
    InsertarElemento(padre, recubr_result);

    //Actualizamos la tabla  de campos
    actualizar_fieldTable (recubr_result);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

       return 0;
}//int Form1::recubrimiento2()


//! Callback que se activa cuando hay algun cambio en el nodo seleccionado.
static void refreshGUI_CB(void *data, SoSensor *)
{
    //Mostramos el nodo en la tabla de edición de campos
    form1->actualizar_fieldTable((SoNode *)data);
}


//!Callback para encender/apagar el refresco del GUI
void Form1::refreshGUI_but_toggled( bool on)
{
    if (on)
        refreshGUI_Sensor->schedule();
    else
        refreshGUI_Sensor->unschedule();
}


//!Callback para cambiar el color de fondo de los visores
void Form1::cambiar_bgColor_viewer()
{
   //Lo convertimos en valores RGB y en un QColor
   const float*rgb = bgColor_viewer.getValue();
   QColor c( int(255*rgb[0]), int(255*rgb[1]), int(255*rgb[2]) );

   //Solicitamos un color mediante QColorDialog
   c=QColorDialog::getColor(c);
   if (c.isValid() )
   {           
       //Modificamos el field
       bgColor_viewer.setValue(c.red()/255.0, c.green()/255.0, c.blue()/255.0);
   }
}


void Form1::menu_edit_preferences_slot()
{
    //Mostramos el dialogo de cds_config_form
    cds_config_form cds_config_form(this);
    cds_config_form.exec();
}

