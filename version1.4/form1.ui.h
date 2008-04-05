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
MainWindow *form1=NULL;

// Sensor que actualiza el GUI a intervalos constantes, por si ha
// cambiado el valor del nodo.
SoTimerSensor *refreshGUI_Sensor = NULL;

/// Callback que actualiza el GUI 
static void refreshGUI_CB(void *data, SoSensor *sensor);



void MainWindow::inicializar()
{

	//Salvo una referencia a este formulario
	form1 = this;

	//Creacion del nodeSensor que mantiene el GUI actualizado.
	refreshGUI_Sensor = new SoTimerSensor (refreshGUI_CB, root);
	refreshGUI_Sensor->setInterval(1.0);

}// void MainWindow::inicializar()



void MainWindow::SoMatrixTransform_to_SoTransform()
{
   QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 
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
   updateFieldEditor (trans);

   //Indicamos que la escena ha sido modificada
   escena_modificada = true;
}



void MainWindow::SoCoordinate3_to_qhull()
{
    //Identificamos el item actual
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem();
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
    updateFieldEditor (ifs);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}// void MainWindow::SoCoordinate3_to_qhull()



void MainWindow::cargar_fichero_locale(const char *fichero)
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

} // void MainWindow::cargar_fichero_locale(const char *fichero)


//Esta función se encarga de mirar si tenemos programado algún ayudante
//que nos permita editar fácilmente el campo. Los ayudantes son 
//dialogos del tipo QFileDialog, QColorDialog o PopupMenus
//El resto de campos se dejan para la funcion fieldTable_valueChanged()
void MainWindow::fieldTable_clicked(int fila, int , int button)
{
    
   //Si estamos vigilando la escena, dejamos de hacerlo para
   //poder editar los valores de la misma.
   refreshGUI_but->setOn(false);

    //Buscamos el nodo que estamos editando
    SoNode *nodo=mapQTCOIN[Ui.sceneGraph->currentItem()];
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
                updateFieldEditor (node);
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
                updateFieldEditor (node);
                return;
            }
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
    updateFieldEditor(nodo);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

} // void MainWindow::fieldTable_clicked(int fila, int columna, int button)




//Aplica qslim a un nodo de la escena
extern SoNode *qslim_result;
int MainWindow::qslim_activated ()
{
    if (!settings.readEntry("/coindesigner/qslim_app"))
    {
       QString S;
       S = tr("No se ha encontrado la herramienta QSLIM");
       QMessageBox::warning( this, tr("Error"), S);
       return -1;
    }

    //Buscamos el nodo actual
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 

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
    QTreeWidgetItem *padre=Ui.sceneGraph->currentItem()->parent();
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
    updateFieldEditor (qslim_result);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

    return 0;

} // int MainWindow::qslim_activated ()


//Aplica tetgen a un nodo de la escena
extern SoNode *tetgen_result;
int MainWindow::tetgen_activated ()
{
    if (!settings.readEntry("/coindesigner/tetgen_app"))
    {
       QString S;
       S = tr("No se ha encontrado la herramienta TETGEN");
       QMessageBox::warning( this, tr("Error"), S);
       return -1;
    }

    //Buscamos el nodo actual
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 

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
    QTreeWidgetItem *padre=Ui.sceneGraph->currentItem()->parent();
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
    updateFieldEditor (tetgen_result);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

    return 0;

} // int MainWindow::tetgen_activated ()




int MainWindow::recubrimiento2()
{
	//Consultamos la cantidad que queremos hinchar
	bool ok;
    double distance = QInputDialog::getDouble(
            tr("Recubrimiento"), tr("Distancia del recubrimiento:"), 1, -1000, 1000, 2, &ok, this );
    if (!ok)
		return -1;

    //Buscamos el nodo actual
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 

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
    QTreeWidgetItem *padre=Ui.sceneGraph->currentItem()->parent();
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
    updateFieldEditor (recubr_result);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

       return 0;
}//int MainWindow::recubrimiento2()


//!Callback para encender/apagar el refresco del GUI
void MainWindow::refreshGUI_but_toggled( bool on)
{
    if (on)
        refreshGUI_Sensor->schedule();
    else
        refreshGUI_Sensor->unschedule();
}


void MainWindow::showmenu()
{

    if (tipo == SoMatrixTransform::getClassTypeId()) 
    {
      pm->insertItem(QPixmap::fromMimeSource("rebuild.png"),tr("Convertir en SoTransform"),  this, SLOT(SoMatrixTransform_to_SoTransform()) );
    }
    else 

    if (tipo.isDerivedFrom(SoIndexedFaceSet::getClassTypeId())) 
    {
      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Recubrimiento"),this,SLOT(recubrimiento2()));
    }
    else 
    
    if (tipo.isDerivedFrom(SoCoordinate3::getClassTypeId())) 
    {
      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Calcular cierre convexo"),  this, SLOT(SoCoordinate3_to_qhull()) );
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
      if (settings.readEntry("/coindesigner/qslim_app"))
	      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Simplificar con QSLIM"),this,SLOT(qslim_activated ()));
      if (settings.readEntry("/coindesigner/tetgen_app"))
	      pm->insertItem(QPixmap::fromMimeSource("package_utilities.png"),tr("Tetrahedralizar con TETGEN"),this,SLOT(tetgen_activated ()));
    }
    else 
       
   
 //Mostramos el menu contextual
 pm->exec( QCursor::pos() );
           
} //void MainWindow::showmenu()
