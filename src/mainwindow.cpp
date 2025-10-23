/*
    This file is part of coindesigner. (https://github.com/jmespadero/coindesigner)

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

#include "mainwindow.h"
#include "cds_viewers.h"
#include "cds_util.h"
#include "3dsLoader.h"
#include <src_editor.h>
#include <mfield_editor.h>
#include <cppexport_options.h>
#include <ivfix_options.h>
#include <settingsDialog.h>
#include <qslim_options.h>
#include <tetgen_options.h>
#include "assimpImport.h"
#include "assimpExport.h"
#include <iostream>
#include <fstream>


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
#include <QPrinter>
#include <QPrintDialog>
#include <QGraphicsScene>
#include <QPainter>
#include <QScrollBar>
#include <QHeaderView>
#include <QtDebug>
#include <QMimeData>

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
#include <Inventor/SoPrimitiveVertex.h>
//#include "Quarter/QuarterWidget.h"


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


/// Callback que vuelca errores de lectura de Inventor a un QMessageBox
static void readError_CB(const class SoError *error, void *)
{
	assert(error && error->getTypeId() == SoReadError::getClassTypeId() );

	QString S; S.asprintf("%s", error->getDebugString().getString() );

	//anade mensaje a la consola de mensajes
	global_mw->addMessage(S);
	QMessageBox::critical(NULL, "Error", S);
}

/// Callback que vuelca errores de depuracion de COIN a la consola de mensajes
static void debugError_CB(const class SoError *error, void *)
{
	QString S; S.asprintf("%s", error->getDebugString().getString() );
	//anade mensaje a la consola de mensajes
	global_mw->addMessage(error->getDebugString().getString());
}


/// Sensor que actualiza el GUI a intervalos constantes, por si ha cambiado el valor del nodo.
SoTimerSensor *refreshGUI_Sensor = NULL;

/// Callback que se activa para monitorizar un nodo
static void refreshGUI_CB(void *data, SoSensor *)
{
	//Mostramos el nodo en la tabla de edicion de campos
	assert(global_mw != NULL);
	if (data != NULL)
		global_mw->updateFieldEditor((SoNode *)data);
}
#include <iostream>
///Constructor de la clase MainWindow
MainWindow::MainWindow (QWidget *p, Qt::WindowFlags f) : QMainWindow(p, f)
{
    ///Inicializamos UI y restauramos ultima geometria
    Ui.setupUi(this);
    if (settings->contains("geometry") )
        restoreGeometry(settings->value("geometry").toByteArray());

    //Restauramos las opciones actuales
    if (settings->contains("Enable_Antialias") )
		Ui.actionEnable_Antialias->setChecked(settings->value("Enable_Antialias").toBool());
    if (settings->contains("HQ_transparency") )
		Ui.actionHQ_transparency->setChecked(settings->value("HQ_transparency").toBool());
    if (settings->contains("Feedback_Visibility") )
		Ui.actionFeedback_Visibility->setChecked(settings->value("Feedback_Visibility").toBool());
		
	//Conectamos el menu contextual de sceneGraph y fieldTable
	//connect(Ui.sceneGraph, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_sceneGraph_customContextMenuRequested(QPoint)));
	//connect(Ui.fieldTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_fieldTable_customContextMenuRequested(QPoint)));

    //No nos gusta la cabecera del sceneGraph, por que hace que el menu de contexto
    //salga fuera de su lugar
    Ui.sceneGraph->headerItem()->setHidden(true);

    //Acceso a los ficheros recientes
    for (int i = 0; i < 5; ++i) 
    {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(actionLoad_RecentFile_triggered()));
        Ui.menuRecent_Files->addAction(recentFileActs[i]);
    }
    updateRecentFileActions();

	//Conexiones de las acciones de conversion para nodos SoTransform a manip
	connect(Ui.actionSoCenterballManip, SIGNAL(triggered()), this, SLOT(on_Convert_Manip_triggered()));
	connect(Ui.actionSoHandleBoxManip, SIGNAL(triggered()), this, SLOT(on_Convert_Manip_triggered()));
	connect(Ui.actionSoJackManip, SIGNAL(triggered()), this, SLOT(on_Convert_Manip_triggered()));
	connect(Ui.actionSoTabBoxManip, SIGNAL(triggered()), this, SLOT(on_Convert_Manip_triggered()));
	connect(Ui.actionSoTrackballManip, SIGNAL(triggered()), this, SLOT(on_Convert_Manip_triggered()));
	connect(Ui.actionSoTransformBoxManip, SIGNAL(triggered()), this, SLOT(on_Convert_Manip_triggered()));
	connect(Ui.actionSoTransformerManip, SIGNAL(triggered()), this, SLOT(on_Convert_Manip_triggered()));

	//Anadimos las opciones para ocultar/mostrar los docks
	QAction *firstAction = Ui.menuTools->actions().first();
	Ui.menuTools->insertAction(firstAction, Ui.fieldTable_dockWidget->toggleViewAction());
	Ui.menuTools->insertAction(firstAction, Ui.nodePalette_dockWidget->toggleViewAction());
	Ui.menuTools->insertAction(firstAction, Ui.messages_dockWidget->toggleViewAction());
	Ui.messages_dockWidget->toggleViewAction()->setShortcut (tr("Ctrl+M"));


    //Inicializacion de COIN y SoQt
    SoDB::init();
    SoQt::init(this);

    //Soporte para SIM Voleon
#ifdef USE_VOLEON
    SoVolumeRendering::init();
    //Introducimos la opcion de SIM Voleon en la paleta de nodos
    Ui.paletteComboBox->addItem(QString("SIM Voleon"));
#endif

    //Asignamos un callback para errores de lectura de Coin3D
    SoReadError::setHandlerCallback(readError_CB, NULL);
    SoDebugError::setHandlerCallback(debugError_CB, NULL);

    //Creacion del campo SoBase_name, que actua como un campo comun para todos los nodos de la escena
    SoBase_name = new SoSFName ();

    //Creamos una nueva escena vacia
    root = new SoSeparator();
    root->ref ();
    root->setName("root");
    on_actionNew_Scene_triggered();

    //Inicializacion del buffer para cut/copy/paste/link
    node_buffer = NULL;
    node_buffer_link = NULL;

	bgColor_viewer.setValue(0,0,0);
	antialias_level = 0;

    nombreEscena = "scene001.iv";

    //Inicializamos el contenido de la paleta de nodos
    on_paletteComboBox_activated(Ui.paletteComboBox->currentIndex() );

	//Creacion del nodeSensor que mantiene el GUI actualizado.
	refreshGUI_Sensor = new SoTimerSensor (refreshGUI_CB, root);
	refreshGUI_Sensor->setInterval(0.5f);

    //Set filterMask
    fileMasks = tr("OpenInventor")+" (*.iv *.wrl);;";
#ifdef USE_ASSIMP
    std::vector<std::pair<std::string,std::vector<std::string> > > formats(assimpImportedFormats());
    for (unsigned int i = 0; i < formats.size(); ++i) {
        fileMasks += tr(formats.at(i).first.c_str())+" (";
        for (unsigned int j = 0; j < formats.at(i).second.size(); ++j) {
            if (j == 0) {
                fileMasks += "*.";
            } else {
                fileMasks += " *.";
            }
            fileMasks += formats.at(i).second.at(j).c_str();
        }
        fileMasks += ");;";
    }
#else
    fileMasks += tr("3D Studio Max 3DS")+"(*.3ds);;";
    fileMasks += tr("Wavefront Object")+"(*.obj);;";
#if USE_DIME
    fileMasks += tr("AutoCAD DXF")+"(*.dxf);;";
#endif
    fileMasks += tr("Stereolithography")+"(*.stl);;";
    fileMasks += tr("Object File Format")+"(*.off);;";
#endif
#ifdef USE_VOLEON
    fileMasks += tr("Volume Data")+"(*.mha *.vol);;";
#endif
    fileMasks += tr("Qslim")+"(*.smf);;";
    fileMasks += tr("Sphere tree")+"(*.sph);;";
    fileMasks += tr("XYZ point cloud")+"(*.xyz);;";
    fileMasks += tr("All Files")+"(*)";
}// MainWindow::MainWindow (QWidget *p, Qt::WindowFlags f) : QMainWindow(p, f)

//Acciones al cerrar la ventana principal
void MainWindow::closeEvent(QCloseEvent *event)
{
    //Si la escena ha sido modificada, pedimos confirmacion
    if (escena_modificada)
    {
        //Solicitamos confirmacion antes de salir de la aplicacion
        switch( QMessageBox::information( this, "Coindesigner",
            tr("Current scene has been modified.")+"\n"+
            tr("Do you want to save the changes to this scene?"),
            tr("Save"), tr("Discard"), tr("Cancel"),
            0, 2 )
            ) 
        {
        case 0: on_actionSave_Scene_triggered(); break;
        case 1: break;
        case 2:    event->ignore(); return;
        }
    }

	if (refreshGUI_Sensor != NULL)
    	refreshGUI_Sensor->unschedule();

    //Salvamos la geometria actual
    settings->setValue("geometry", saveGeometry());

    //Salvamos las opciones actuales
    settings->setValue("Enable_Antialias", Ui.actionEnable_Antialias->isChecked());
    settings->setValue("HQ_transparency", Ui.actionHQ_transparency->isChecked());
    settings->setValue("Feedback_Visibility", Ui.actionFeedback_Visibility->isChecked());

    //Cerramos la ventana y terminamos el bucle de eventos
    QMainWindow::closeEvent(event);
    SoQt::exitMainLoop();

}// void MainWindow::closeEvent(QCloseEvent *event)


void MainWindow::on_actionNew_Scene_triggered()
{
    //Borramos todos los nodos de la escena 
    if (!root)
    {
        root = new SoSeparator();
        root->ref ();
    	root->setName("root");
    }
    else
    {
        root->removeAllChildren();
    }

    //Borramos todo el arbol de nodos QT y creamos uno con un solo nodo
    Ui.sceneGraph->clear();

	//Indicamos que refresque el nodo root
    if(refreshGUI_Sensor != NULL)
    	refreshGUI_Sensor->setData(root);

    //Creamos el item para nodo root y reasignamos su texto
    QTreeWidgetItem *qroot = newNodeItem(root);
    qroot->setText(0, QString("root"));

    //Colgamos el item qroot en el sceneGraph
    Ui.sceneGraph->insertTopLevelItem(0, qroot);

    //Actualizamos el GUI
    Ui.sceneGraph->setCurrentItem(qroot);

    //Indicamos que la escena no ha sido modificada
    escena_modificada = false;

}//void MainWindow::on_actionNew_Scene_triggered()



bool MainWindow::load_Scene(QString filename)
{
    //Si no se ha pasado nombre de fichero, solicitamos uno
    if (filename == "")
    {
        QString selectedFilter(tr("All Files")+"(*)");
        filename = QFileDialog::getOpenFileName(this, tr("Load Scene"),
            nombreEscena, fileMasks, &selectedFilter);
        
        //Miramos si se pulso el boton cancelar
        if (filename=="")
            return false;
    }

    //Cargamos el fichero
    SoSeparator *scene = cargarFichero3D(filename);

    //Si hubo error, mostramos una ventana de error
    if (!scene)
    {
        QString S;
        S=filename+tr(": Unknown file format");
        QMessageBox::critical( this, tr("Error"), S);
        return false;
    }

    //Referenciamos el nodo scene
    scene->ref();

    //Salvamos el nombre de la escena
    nombreEscena = filename;

    //Actualiza la barra de titulo de la ventana
    setWindowTitle(QFileInfo(filename).fileName() + " - Coindesigner");

    //Configuramos Ui y recentFiles para reflejar el nombre
    setRecentFile(filename);

    //Destruimos la escena actual y creamos una nueva
    on_actionNew_Scene_triggered();

    //Colgamos el nodo del grafo de escena
    QTreeWidgetItem *qroot=Ui.sceneGraph->currentItem();

    //Colgamos el contenido de la geometria uno por uno
    for (int i=0; i<scene->getNumChildren(); i++)
    {
		newSceneGraph(scene->getChild(i), qroot, root);
    }

    //Expandimos todos los items
    Ui.sceneGraph->expandAll();

	//Liberamos memoria
    scene->unref();

    //Indicamos que la escena no ha sido modificada
    escena_modificada = false;

    return true; 
}// int MainWindow::on_actionLoad_Scene_triggered(QString filename)


void MainWindow::save_Scene(QString filename)
{
    //Si no se ha pasado nombre de fichero, solicitamos uno
    if (filename == "")
	{
        filename= QFileDialog::getSaveFileName(this, tr("Save File"), nombreEscena,
                            tr("OpenInventor Files (*.iv *.wrl);;All Files (*)") );

        //Miramos si se pulso el boton cancelar
        if (filename == "")
            return;
    }

    nombreEscena = filename;

    //Creamos un SoWriteAction para escribir la escena
    SoWriteAction writeAction;
    writeAction.getOutput()->openFile(qPrintable(filename) );

    /* 
    //OPCIONES ELEGIDAS EN NUESTRO FORMULARIO PARA SALVAR
    //Miramos si se activo el flag binario
    writeAction.getOutput()->setBinary(fd.binary->isChecked());

    //Miramos si se activo la compresion
    if (fd.compress->isChecked())
    {
        //Modo comprimido necesita al modo binario
        writeAction.getOutput()->setBinary(true);
        writeAction.getOutput()->setCompression(SbName("GZIP"), 0.9f);
    }
    */

    //Si la escena tiene extension .wrl, usamos la cabecera adecuada
    if (filename.endsWith(".wrl", Qt::CaseInsensitive))
        writeAction.getOutput()->setHeaderString("#VRML V1.0 ascii");

    writeAction.apply(root);
    writeAction.getOutput()->closeFile();

    //Actualiza la barra de titulo de la ventana
    setWindowTitle(QFileInfo(filename).fileName() + " - Coindesigner");

	//Salvamos la escena en el listado de archivos recientes
	this->setRecentFile(filename);

    //Indicamos que la escena ha sido salvada
    escena_modificada = false;
}//void MainWindow::on_actionSave_Scene_triggered(QString filename)


bool MainWindow::import_File(QString filename)
{
    //Si no se ha pasado nombre de fichero, solicitamos uno
    if (filename == "")
    {
        QString selectedFilter(tr("All Files")+"(*)");
        filename = QFileDialog::getOpenFileName(this, tr("Load Scene"),
            nombreEscena, fileMasks,&selectedFilter);
        
        //Miramos si se pulso el boton cancelar
        if (filename=="")
            return false;
    }

    //Cargamos el fichero
    SoSeparator *scene = cargarFichero3D(filename);

    //Si hubo error, mostramos una ventana de error
    if (!scene)
    {
        QString S;
        S=filename+tr(": File format unknow");
        QMessageBox::critical( this, tr("Error"), S);
        return false;
    }

	//Salvamos la escena en el listado de archivos recientes
	this->setRecentFile(filename);

    //Asignamos el nombre del fichero al separator scene
    SoBase_name->setValue(QFileInfo(filename).baseName().toLatin1());
    scene->setName(SoBase_name->getValue());

    //Buscamos el item donde colgar la escena
    QTreeWidgetItem *item_padre = Ui.sceneGraph->currentItem();
    while (!mapQTCOIN[item_padre]->getTypeId().isDerivedFrom(SoGroup::getClassTypeId())) 
    {
        item_padre=item_padre->parent();
    }
 
    //Buscamos el el nodo donde colgar el buffer
    SoGroup* nodo_padre=(SoGroup*)mapQTCOIN[item_padre];
    
    //Insertamos el contenido de la escena
    newSceneGraph(scene, item_padre, nodo_padre);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

    return true; 
} //bool MainWindow::on_actionImport_File_triggered(QString filename)


///Export to c++ action
void MainWindow::on_actionExport_C_triggered()
{
    cppexport_options cppexportDialog(root);
    cppexportDialog.exec();

}// void MainWindow::on_actionExport_C_triggered()

///Carga una escena incrustada como resource
void MainWindow::load_Scene_Demo(const QString &filename)
{
    QResource demoRsrc(filename);
    assert(demoRsrc.isValid());

    long long size =  demoRsrc.size();
    //qDebug ("size=%lld \tisCompressed=%d\n", size, demoRsrc.isCompressed());

    char *buf =  new char[size];

    if(demoRsrc.compressionAlgorithm() != QResource::NoCompression)
    {
        memcpy(buf, qUncompress(demoRsrc.data(), size).data(), size);
    }
    else
    {
        memcpy(buf, demoRsrc.data(), size);
    }

    //Leemos la escena desde el buffer de memoria
    SoInput input;
    input.setBuffer(buf, size) ;
    SoSeparator *scene = SoDB::readAll(&input);
	assert(scene);
	scene->ref();

    //Actualiza la barra de titulo de la ventana
    setWindowTitle(QFileInfo(filename).fileName() + " - Coindesigner");

    //Destruimos la escena actual y creamos una nueva
    on_actionNew_Scene_triggered();

    //Colgamos el nodo del grafo de escena
    QTreeWidgetItem *qroot = Ui.sceneGraph->currentItem();
    for (int i=0; i<scene->getNumChildren(); i++)
    {
        newSceneGraph(scene->getChild(i), qroot, root);
    }

	scene->unref();
    escena_modificada = false;

    delete[] buf;

}//void MainWindow::load_Scene_Demo(const QString &filename)



///Exporta la escena en formato VRML2
void MainWindow::on_actionExport_VRML2_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
            nombreEscena, 
            tr("VRML Files")+"(*.wrl);;"+tr("All Files")+" (*)");

    //Miramos si se pulso el boton cancelar
    if (filename=="")
        return;

    SoToVRML2Action wrlAction;
    wrlAction.apply(root);

    SoVRMLGroup *wrl2 = wrlAction.getVRML2SceneGraph();
    wrl2->ref();

    SoWriteAction action;

    action.getOutput()->openFile(SbName(filename.toLatin1()).getString());
    action.getOutput()->setHeaderString("#VRML V2.0 utf8");
    action.apply(wrl2);
    action.getOutput()->closeFile();

    wrl2->unref();

}// void MainWindow::on_actionExport_VRML2_triggered()


void MainWindow::on_actionExport_ASE_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
            nombreEscena, 
            tr("ASE Files")+"(*.ase);;"+tr("All Files")+" (*)");

    //Miramos si se pulso el boton cancelar
    if (filename=="")
        return;

    SoPath *path = new SoPath(root);
    path->ref();
    cds_export_ase(path, filename.toLatin1());
	path->unref();

}//void MainWindow::on_actionExport_ASE_triggered()


struct GeomData {
    std::vector<SbVec3f> vertices;
    std::vector<SbVec3i32> triangles;
    unsigned int addVertex(const float *point) {
        SbVec3f vertex(point[0],point[1],point[2]);
        std::map<SbVec3f,unsigned int>::const_iterator it = verticesMap.find(vertex);
        if (it != verticesMap.end()) {
            return it->second;
        } else {
            unsigned int index = vertices.size();
            vertices.push_back(vertex);
            verticesMap.insert(std::pair<SbVec3f,unsigned int>(vertex,index));
            return index;
        }
    }
private:
    struct cmpVertex {
        bool operator()(const SbVec3f& a, const SbVec3f& b) const {
            return (a[0] < b[0]) ||
                   (a[1] < b[1] && a[0] == b[0]) ||
                   (a[2] < b[2] && a[1] == b[1] && a[0] == b[0]);
        }
    };
    std::map<SbVec3f,unsigned int,cmpVertex> verticesMap;
};

void trianglePCD(void *data, SoCallbackAction *action,
                const SoPrimitiveVertex *vertex1,
                const SoPrimitiveVertex *vertex2,
                const SoPrimitiveVertex *vertex3) {
    GeomData *geomData = (GeomData*)data;
    SbVec3f point;
    const SbVec3f vertex[] = {vertex1->getPoint(),
                              vertex2->getPoint(),
                              vertex3->getPoint()};
    const SbMatrix  matrix = action->getModelMatrix();
    unsigned int index[3];
    for (int i = 0; i < 3; ++i) {
        matrix.multVecMatrix(vertex[i],point);

        index[i] = geomData->addVertex(point.getValue());
    }

    geomData->triangles.push_back(SbVec3i32(index[0],index[1],index[2]));
}


void MainWindow::on_actionExport_PCD_triggered() {
    QString filename(QFileDialog::getSaveFileName(this, tr("Save File"),
            nombreEscena,
            tr("PCD Files")+"(*.pcd);;"+tr("All Files")+" (*)"));
    if (filename.isEmpty()) return;

    SoCallbackAction triAction;
    GeomData geomData;
    triAction.addTriangleCallback(SoShape::getClassTypeId(),
                                  trianglePCD,(void*)&geomData);
    triAction.apply(root);
    
    //converting to std::string
    std::ofstream fs;
    fs.open(filename.toStdString().c_str());
    
    // Now we are saving the pcd data to visualize it    
    std::cout << "Creating the box cloud!" << std::endl;
    
    fs << "# .PCD v.7 - Point Cloud Data file format" << std::endl;
    fs << "VERSION .7" << std::endl;
    fs << "FIELDS x y z" << std::endl;
    fs << "SIZE 4 4 4" << std::endl;
    fs << "TYPE F F F" << std::endl;
    fs << "COUNT 1 1 1" << std::endl;
    fs << "WIDTH "<< geomData.vertices.size() << std::endl;
    fs << "HEIGHT 1" << std::endl;
    fs << "VIEWPOINT 0 0 0 1 0 0 0" << std::endl;
    fs << "POINTS "<< geomData.vertices.size() << std::endl;
    fs << "DATA ascii" << std::endl;

    float x,y,z;
    for (std::vector<SbVec3f>::const_iterator it(geomData.vertices.begin());
         it != geomData.vertices.end(); ++it) {
       it->getValue(x,y,z);
       fs << x << " " <<  y << " " << z << std::endl;
    }
    fs.close();
    
    std::cout << "Done!" << std::endl;
}//void MainWindow::on_actionExport_PCD_triggered()


void MainWindow::on_actionExport_As_triggered() {
    std::vector<std::pair<std::string,std::string> > formats(assimpExportedFormats());
    QString filter;
    for (std::vector<std::pair<std::string,std::string> >::const_iterator it(formats.begin());
            it != formats.end(); ++it) {
        filter.append(tr(std::string(it->first+std::string(" Files")).c_str())).
                append(" (*.").append(it->second.c_str()).append(");;");
    }
    filter.append(tr("All Files")+" (*)");
    QString selectedFilter(tr("All Files")+"(*)");
    QString filename(QFileDialog::getSaveFileName(this,tr("Save File"),
                                                  nombreEscena,filter,&selectedFilter));
    if (filename.isEmpty()) return;

    QString selectedExtension(selectedFilter.left(selectedFilter.length()-1).
                              mid(selectedFilter.lastIndexOf("*")+2));
    if (!selectedExtension.isEmpty()) {
        filename = filename.left(filename.lastIndexOf(".")+1).append(selectedExtension);
    }
    std::string error;
    if (exportScene(filename.toStdString(),root,&error)) {
        addMessage(tr("Scene was successfully exported."));
    } else {
        addMessage(tr(error.c_str()));
    }
}//void MainWindow::on_actionExport_As_triggered()


///Imprime el grafo de escena en un documento
void MainWindow::on_actionPrintSceneGraph_triggered()
{
	//Leemos la impresora destino
	QPrinter printer;
	//printer.setOutputFileName("print.pdf"); 
	QPrintDialog *dialog = new QPrintDialog(&printer, this);
	dialog->setWindowTitle(tr("Print Scene Graph"));
	if (dialog->exec() != QDialog::Accepted)
		return;
	printer.setFullPage(false);

	//Creamos un nuevo arbol, con mas informacion que Ui.sceneGraph
	QTreeWidget scn;
	scn.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scn.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QStringList headers;
	headers << tr("Node Class") << tr("Node Name") << tr("RefCount");
	scn.setHeaderLabels(headers);
	scn.setAlternatingRowColors(true);
	//Clone Ui.sceneGraph contents
	QTreeWidgetItem *qroot = Ui.sceneGraph->topLevelItem(0)->clone(); 
	scn.addTopLevelItem(qroot);
	scn.expandAll();

	//Rellena el nombre de los items
	
	//Usamos dos listas de items auxiliares
	int numItems=0;

	//Leemos el primer elemento de cada lista, y lo sacamos de la lista
	QList<QTreeWidgetItem *>scnItemList;
	QTreeWidgetItem *it1 = qroot;
	QTreeWidgetItem *it2 = Ui.sceneGraph->topLevelItem(0);

	do 
	{
		//Rellenamos el nombre del nodo
		SoNode *node = mapQTCOIN[it2];
		assert(node != 0);
		it1->setText(1,node->getName().getString() );
		int relativeRefCount = node->getRefCount();
		if (relativeRefCount != 1)
			it1->setText(2,QString::number(relativeRefCount));

		//TEST
		//it1->setText(2,QString::number(numItems));

		//Almacenamos los item en orden en una lista
		scnItemList.push_back(it1);
		numItems++;

		//Y pasamos al siguiente par de items
		it1 = scn.itemBelow(it1);
		it2 = Ui.sceneGraph->itemBelow(it2);
	}
	while (it1 != NULL);

	for (int i=0; i< scn.columnCount(); i++)
		scn.resizeColumnToContents(i);

    // compute the printable area in device pixels (matches QPixmap/QImage pixels)
    const QRect paintRect = printer.pageLayout().paintRectPixels(printer.resolution());

    int headerHeight = scn.header()->height();
    int heightPage   = paintRect.height();
    int itemsPerPage = (heightPage - headerHeight) / (scn.visualItemRect(qroot).height() + 1);

    // first item of each page
    QTreeWidgetItem* topItem  = nullptr;
    QTreeWidgetItem* lastItem = nullptr;

    // make the tree widget match the printable area
    scn.resize(paintRect.size());

    // render pages in blocks of itemsPerPage items
    QPainter painter(&printer);
    int page = 0;

    do {
        // page head/tail items
        topItem = scnItemList.at(page * itemsPerPage);

        int lastItemIdx = page * itemsPerPage + itemsPerPage - 1;
        if (lastItemIdx >= numItems) lastItemIdx = numItems - 1;
        lastItem = scnItemList.at(lastItemIdx);

        // position first/last rects of the page
        scn.scrollToItem(topItem, QAbstractItemView::PositionAtTop);

        const QRect topRect  = scn.visualItemRect(topItem);
        const QRect lastRect = scn.visualItemRect(lastItem);
        heightPage           = lastRect.bottom() - topRect.top();

        // draw into a raster surface sized exactly to the printable area
        QPixmap pixmap(paintRect.size());
        pixmap.fill();

        // header at top
        scn.header()->render(&pixmap);

        // items for the page
        scn.viewport()->render(
            &pixmap,
            QPoint(0, headerHeight - topRect.top()),
            QRegion(0, topRect.top(), pixmap.width(), heightPage)
        );

        // send to printer: target is the printable area
        painter.drawPixmap(paintRect, pixmap);

        // next page?
        if (lastItemIdx < numItems - 1)
            printer.newPage();
        else
            break;

        ++page;

    } while (true);

	//Finaliza y envia el documento a la impresora
	painter.end();

	//Una pausa para depurar / ver el arbol
	//scn.show();QDialog dlg; dlg.exec();
}// void MainWindow::on_actionPrintSceneGraph_triggered()


///Muestra un dialogo donde visualizar el codigo fuente de la escena
void MainWindow::on_actionEdit_Source_triggered()
{
    SrcEditor srcEditor(root);
    srcEditor.exec();

	//Comprobamos que se ha compilado correctamente la escena
    if (srcEditor.result == 0)
        return;

    //Referenciamos el nodo scene
    srcEditor.result->ref();

    //Destruimos la escena actual y creamos una nueva
    on_actionNew_Scene_triggered();

    //Colgamos el nodo del grafo de escena
    QTreeWidgetItem *qroot=Ui.sceneGraph->currentItem();

    //Colgamos el contenido de la geometria uno por uno
    for (int i=0; i<srcEditor.result->getNumChildren(); i++)
    {
        newSceneGraph(srcEditor.result->getChild(i), qroot, root);
    }

    //Expandimos todos los items
    Ui.sceneGraph->expandAll();

    //Liberamos la memoria de la escena
    srcEditor.result->unref();

    //Indicamos que la escena no ha sido modificada
    escena_modificada = true;

}//void MainWindow::on_actionView_Source_triggered()


void MainWindow::on_actionQuit_triggered()
{
    //Cerramos la ventana principal, lo que produce salir de la aplicacion
    close();
}

///////////////////////
//Slots para menu Edit
///////////////////////

void MainWindow::on_actionCut_triggered()
{
   //Identificamos el nodo seleccionado
    QTreeWidgetItem *item = Ui.sceneGraph->currentItem(); 
    SoNode *node=mapQTCOIN[item];

    //No permitimos borrar el nodo raiz.
    if (node == root)
    {
		global_mw->addMessage(tr("Are you crazy? I'm the root node."));
       return;
    }

    //Liberamos el contenido del buffer actual
    if (node_buffer)
    {
        node_buffer->unref();
        node_buffer_link = NULL;
    }

    //Apuntamos al nodo en el buffer 
    node_buffer=node;

    //Aumentamos el numero de referencias para evitar que coin lo borre de memoria
    node_buffer->ref();

    //Simulamos que se ha solicitado eliminar el nodo del grafo de escena
    on_actionDelete_triggered();

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}//void MainWindow::on_actionCut_triggered()


///Copia un nodo al buffer para cut/copy/paste/link
void MainWindow::on_actionCopy_triggered()
{
    //Buscamos el item seleccionado
    QTreeWidgetItem * item = Ui.sceneGraph->currentItem();
    if (!item)
       return;

    //Liberamos el buffer actual
    if(node_buffer)
    {
        node_buffer->unref();
        node_buffer_link = NULL;
    }

   //Creamos una copia del nodo en el buffer
   node_buffer = mapQTCOIN[item]->copy(true);
   node_buffer->ref();

   //Guardamos un puntero directo al nodo copiado, para operaciones link
   node_buffer_link = mapQTCOIN[item];
}

void MainWindow::on_actionPaste_triggered()
{
    //Comprobamos que hay algo en el buffer
    if (!node_buffer)
        return;

    //Buscamos el item donde colgar el buffer
    QTreeWidgetItem *item_padre = Ui.sceneGraph->currentItem();
    while (!mapQTCOIN[item_padre]->getTypeId().isDerivedFrom(SoGroup::getClassTypeId())) 
    {
        item_padre=item_padre->parent();
    }
 
    //Buscamos el el nodo donde colgar el buffer
    SoGroup* nodo_padre=(SoGroup*)mapQTCOIN[item_padre];
    
    //Pegamos una copia del contenido del buffer
    newSceneGraph(node_buffer->copy(true), item_padre, nodo_padre);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}//void MainWindow::on_actionPaste_triggered()


///Borra el nodo actual de la escena
void MainWindow::on_actionDelete_triggered()
{
   //Identificamos el nodo seleccionado
    QTreeWidgetItem *item = Ui.sceneGraph->currentItem(); 
    SoNode *node=mapQTCOIN[item];

    //No permitimos borrar el nodo raiz.
    if (node == root)
    {
        global_mw->addMessage(tr("No way, I'm Connor MacLeod (see http://imdb.com/title/tt0091203/ )"));
        return;
    }

    //Si estamos borrando el nodo en buffer_link, invalidamos el buffer
    //BUG:Deberiamos certificar que node no es un ancestro de node_buffer_link,
    //puesto que entonces tambien estariamos eliminandolo
    if (node == node_buffer_link)
        node_buffer_link = NULL;

    //Identificamos al nodo padre del item a borrar
    QTreeWidgetItem *item_padre = item->parent();
    SoGroup *nodo_padre = (SoGroup *)mapQTCOIN[item_padre];

    //Borramos el node de la escena, del mapQTCOIN y del Ui.sceneGraph
    nodo_padre->removeChild(node);
    mapQTCOIN.erase(item);
    delete item;         

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}// void MainWindow::on_actionDelete_triggered()

void MainWindow::on_actionLink_triggered()
{
    //Comprobamos que hay algo en el buffer
    if (!node_buffer_link)
        return;

    //Buscamos el item donde colgar el buffer
    QTreeWidgetItem *item_padre = Ui.sceneGraph->currentItem();
    while (!mapQTCOIN[item_padre]->getTypeId().isDerivedFrom(SoGroup::getClassTypeId())) 
    {
        item_padre=item_padre->parent();
    }
 
    //Buscamos el el nodo donde colgar el buffer
    SoGroup* nodo_padre=(SoGroup*)mapQTCOIN[item_padre];
    
    //Es facil crear un bucle infinito con links, asi que evitamos
    //algunos casos de usuarios inexpertos
    //BUG: Deberiamo ver que no enlazamos en ningun ancestro de buffer_link
    SoPath *path=getPathFromItem(item_padre);
    if (path->containsNode (node_buffer_link))
    {
        //En caso de error escribimos un aviso
        QMessageBox::warning( this, tr("Warning"), tr("You can not paste as link this node here."));
        return;
    }
    path->unref();

    //Insertamos el nodo apuntado por el buffer
    newSceneGraph(node_buffer_link, item_padre, nodo_padre);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}//void MainWindow::on_actionLink_triggered()


void MainWindow::on_actionMove_Up_triggered()
{
   //Identificamos el nodo seleccionado
    QTreeWidgetItem *item = Ui.sceneGraph->currentItem(); 
    SoNode *node=mapQTCOIN[item];

    //No permitimos subir el nodo raiz.
    if (node == root)
    {
		global_mw->addMessage(tr("This is the high you can reach being a root node :-("));
        return;
    }

    //Miramos si el item es el primero de su grupo
    QTreeWidgetItem *item_padre = item->parent();
    int item_pos = item_padre->indexOfChild (item); 
    if ( item_pos < 1)
        return;

    //Buscamos el item anterior al actual y lo movemos por debajo
    QTreeWidgetItem *item_prev = item_padre->child(item_pos-1);
    bool item_prev_status = item_prev->isExpanded ();
    item_padre->removeChild (item_prev);
    item_padre->insertChild(item_pos, item_prev);
    item_prev->setExpanded(item_prev_status);

    //Ahora, movemos el nodo de coin delante del nodo anterior
    SoGroup *nodo_padre = (SoGroup *)mapQTCOIN[item_padre];

    //Consultamos la posicion del nodo dentro de su padre
    int pos = nodo_padre->findChild(node);

    //Copiamos el nodo actual delante del nodo anterior
    nodo_padre->insertChild(node, pos-1);

    //El nodo original ha sido desplazado a la posicion pos+1
    nodo_padre->removeChild(pos+1);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}

void MainWindow::on_actionMove_Down_triggered()
{
   //Identificamos el nodo seleccionado
    QTreeWidgetItem *item = Ui.sceneGraph->currentItem(); 
    SoNode *node=mapQTCOIN[item];

    //No permitimos bajar el nodo raiz.
    if (node == root)
    {
        global_mw->addMessage(tr("Don't let me down!!! I'm the root node."));
        return;
    }

    //Miramos si el item es el ultimo de su grupo
    QTreeWidgetItem *item_padre = item->parent();
    int item_pos = item_padre->indexOfChild (item); 
    if ( item_pos == item_padre->childCount()-1 )
        return;

    //Buscamos el item posterior al actual y lo movemos encima
    QTreeWidgetItem *item_post = item_padre->child(item_pos+1);
    item_padre->removeChild (item_post);
    item_padre->insertChild(item_pos, item_post);

    //Ahora, movemos el nodo de coin detras del nodo anterior
    SoGroup *nodo_padre = (SoGroup *)mapQTCOIN[item_padre];

    //Consultamos la posicion del nodo dentro de su padre
    int pos = nodo_padre->findChild(node);

    //Copiamos el nodo en la posicion pos detras del nodo siguiente
    nodo_padre->insertChild(node, pos+2);

    //Eliminamos el nodo en la posicion pos
    nodo_padre->removeChild(pos);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;
}

void MainWindow::on_actionPreferences_triggered()
{
	settingsDialog settinsDlg;
	settinsDlg.exec();
}


void MainWindow::on_actionExaminerViewer_triggered()
{
    //Creacion del QDockWidget
    QDockWidget *dockWidget = new QDockWidget(this);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHeightForWidth(dockWidget->sizePolicy().hasHeightForWidth());
    dockWidget->setSizePolicy(sizePolicy);
    dockWidget->setWindowTitle(QString::fromUtf8("ExaminerViewer"));

    //Widget intermedio
    QWidget *viewWidget = new QWidget(dockWidget);
    viewWidget->setMinimumSize (150, 200);
    viewWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    dockWidget->setWidget(viewWidget);

    /*
    SIM::Coin3D::Quarter::QuarterWidget *quarterWidget;
    quarterWidget = new SIM::Coin3D::Quarter::QuarterWidget(dockWidget);
    quarterWidget->setObjectName(QString::fromUtf8("quarterWidget"));
    quarterWidget->setMouseTracking(false);
    quarterWidget->setFocusPolicy(Qt::TabFocus);
    quarterWidget->setHeadlightEnabled(true);
    quarterWidget->setInteractionModeOn(true);
    quarterWidget->setSceneGraph(root);
    quarterWidget->viewAll();
    quarterWidget->setNavigationModeFile(QUrl("coin:///scxml/navigation/examiner.xml"));
    dockWidget->setWidget(quarterWidget);
	//*/


    //Creacion del Viewer
    NoQuitExaminerViewer *viewer = new NoQuitExaminerViewer(viewWidget, dockWidget);
    viewer->setSceneGraph(root);
	configureViewer(viewer);

	//Decide si mostrar el eje de coordenadas de orientacion
	viewer->setFeedbackVisibility(Ui.actionFeedback_Visibility->isChecked());

    this->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget);

}//void MainWindow::on_actionExaminerViewer_triggered()


void MainWindow::on_actionFlyViewer_triggered()
{
    //Creacion del QDockWidget
    QDockWidget *dockWidget = new QDockWidget(this);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHeightForWidth(dockWidget->sizePolicy().hasHeightForWidth());
    dockWidget->setSizePolicy(sizePolicy);
    dockWidget->setWindowTitle(QString::fromUtf8("FlyViewer"));

    //Widget intermedio
    QWidget *viewWidget = new QWidget(dockWidget);
    viewWidget->setMinimumSize (150, 200);
    viewWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    //Creacion del Viewer
    NoQuitFlyViewer *viewer = new NoQuitFlyViewer(viewWidget, dockWidget);
    viewer->setSceneGraph(root);
	configureViewer(viewer);

    dockWidget->setWidget(viewWidget);
    this->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget);

}//void MainWindow::on_actionFlyViewer_triggered()


void MainWindow::on_actionPlaneViewer_triggered()
{
    //Creacion del QDockWidget
    QDockWidget *dockWidget = new QDockWidget(this);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHeightForWidth(dockWidget->sizePolicy().hasHeightForWidth());
    dockWidget->setSizePolicy(sizePolicy);
    dockWidget->setWindowTitle(QString::fromUtf8("PlaneViewer"));

    //Widget intermedio
    QWidget *viewWidget = new QWidget(dockWidget);
    viewWidget->setMinimumSize (150, 200);
    viewWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    //Creacion del Viewer
    NoQuitPlaneViewer *viewer = new NoQuitPlaneViewer(viewWidget, dockWidget);
    viewer->setSceneGraph(root);
	configureViewer(viewer);

    dockWidget->setWidget(viewWidget);
    this->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget);

}//void MainWindow::on_actionPlaneViewer_triggered()


void MainWindow::on_actionRenderArea_triggered()
{
    //Creacion del QDockWidget
    QDockWidget *dockWidget = new QDockWidget(this);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHeightForWidth(dockWidget->sizePolicy().hasHeightForWidth());
    dockWidget->setSizePolicy(sizePolicy);
    dockWidget->setWindowTitle(QString::fromUtf8("RenderArea"));

    //Widget intermedio
    QWidget *viewWidget = new QWidget(dockWidget);
    viewWidget->setMinimumSize (150, 200);
    viewWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    //Creacion del Viewer
    NoQuitRenderArea *viewer = new NoQuitRenderArea(viewWidget, dockWidget);
    viewer->setSceneGraph(root);
	configureViewer(viewer);

    dockWidget->setWidget(viewWidget);
    this->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget);
}//void MainWindow::on_actionRenderArea_triggered()


///Configura la variable de entorno COIN_SHOW_FPS_COUNTER
void MainWindow::on_actionShow_FPS_toggled(bool on)
{
#ifdef _WIN32
    _putenv(on?"COIN_SHOW_FPS_COUNTER=1":"COIN_SHOW_FPS_COUNTER=0");
#else
        setenv("COIN_SHOW_FPS_COUNTER", on?"1":"0", 1);

#endif
}//void MainWindow::on_actionShow_FPS_triggered()


///Configura el nivel de antialias
void MainWindow::on_actionEnable_Antialias_toggled(bool on)
{

	if (on)
		antialias_level = 5;
	else
		antialias_level = 0;
}// void MainWindow::on_actionEnable_Antialias_toggled(bool on)



//Slots para menu Tools

///Aplica los parametros de render actuales a un viewer
void MainWindow::configureViewer(SoQtRenderArea *viewer)
{
	//Set Background Color
	viewer->setBackgroundColor(bgColor_viewer);

	//Antialias Level
	if (antialias_level == 0)
		viewer->setAntialiasing(false, 0);
	else
		viewer->setAntialiasing(true, antialias_level);

	//Transparency type
	if (Ui.actionHQ_transparency->isChecked())
	{
		viewer->setAlphaChannel(true);
		viewer->setTransparencyType(SoGLRenderAction::SORTED_LAYERS_BLEND);
	}
}

///Crea un nuevo ExaminerViewer_Editor
void MainWindow::on_actionExaminerViewer_Editor_triggered()
{
    CdsExaminerEditor *viewer = new CdsExaminerEditor(root);
	configureViewer(viewer);

	//Decide si mostrar el eje de coordenadas de orientacion
	viewer->setFeedbackVisibility(Ui.actionFeedback_Visibility->isChecked());

    viewer->show();
}

///Crea un nuevo PlaneViewer_Editor
void MainWindow::on_actionPlaneViewer_Editor_triggered()
{
    CdsPlaneEditor *viewer = new CdsPlaneEditor(root);
	configureViewer(viewer);
    viewer->show();
}


///Abre la pagina principal de la referencia de Coin3D
void MainWindow::on_actionCoin3D_Reference_triggered()
{
    QString url = settings->value("reference_dir").toString()+"/hierarchy.html";
    this->open_html_viewer(url);
}

///Abre la pagina de referencia de la clase seleccionada en Ui.nodePalette
void MainWindow::on_actionShow_Node_Reference_triggered()
{

    if (Ui.nodePalette->currentItem() == NULL)
	   return;

    //Leemos el nombre de la clase y aseguramos que tiene el prefijo So
    QString className = Ui.nodePalette->currentItem()->text(0);
    if (!className.startsWith("So"))
       className.prepend("So");

    QString url = settings->value("reference_dir").toString()+"/class"+className+".html";
    this->open_html_viewer(url);
}

///Abre la pagina del tutorial1
void MainWindow::on_actionTutorial_1_triggered()
{
    QString url = settings->value("tutorial_dir").toString()+"/tut01/index.html";
    this->open_html_viewer(url);
}

///Abre la pagina del tutorial2
void MainWindow::on_actionTutorial_2_triggered()
{
    QString url = settings->value("tutorial_dir").toString()+"/tut02/index.html";
    this->open_html_viewer(url);
}

///Carga la demo helloCone.iv
void MainWindow::on_actionHello_Cone_demo_triggered()
{
	load_Scene_Demo(":/demos/helloCone.iv");
}

///Carga la demo mirror.iv
void MainWindow::on_actionMirror_demo_triggered()
{
	load_Scene_Demo(":/demos/mirror.iv");
}

///Carga la demo planetEarth.iv.gz
void MainWindow::on_actionPlanet_Earth_demo_triggered()
{
	load_Scene_Demo(":/demos/planetEarth.iv.gz");
}

///Carga la demo engine.iv
void MainWindow::on_actionEngine_demo_triggered()
{
	load_Scene_Demo(":/demos/engine.iv");
}

///Dialogo About
void MainWindow::on_actionAbout_triggered()
{
    QString msj;
    msj = QString("<p>Coindesigner %1 (%2)<p>").arg(CDS_VERSION).arg(CDS_BUILD_DESC);
    //msj .asprintf("<p>Coindesigner version %f (%s)<p>", CDS_VERSION, __DATE__ );
    msj += tr("Written by")+" Jose M. Espadero "+tr("and")+" Tomas Aguado";
    msj += "<br>         " + tr("with useful contributions from") + " Manfred Kroehnert";
    msj += "<p><a href=https://github.com/jmespadero/coindesigner>https://github.com/jmespadero/coindesigner)</a>";

    //System Info
    msj += "<hr><b>"+tr("System info:")+"</b>";
    msj += "<br>Coindesigner dir="; msj += cds_dir;
    msj += "<br>OpenInventor version="; msj += SoDB::getVersion();
    msj += "<br>SoQt version="; msj += SoQt::getVersionString();
    msj += "<br>Qt version="; msj += qVersion();

    //Version de VOLEON
#ifdef USE_VOLEON
    msj += QString("<br>SIM Voleon version=")+ SIMVOLEON_VERSION;
#endif

    //Version de DIME
#ifdef USE_DIME
    msj += "<br>SIM DIME extension";
#endif

    //Comprobamos si existe el ejecutable de QSLIM 
    if (settings->contains("qslim_app") )
    {
        msj += "<br>QSlim = " + settings->value("qslim_app").toString();
    }
    else
    {
        msj += "<br>QSlim = " + tr("not found.");
    }

    //Comprobamos si existe el ejecutable de TETGEN 
    if (settings->contains("tetgen_app"))
    {
        msj += "<br>Tetgen = " + settings->value("tetgen_app").toString();
    }
    else
    {
        msj += "<br>Tetgen = " + tr("not found.");
    }

    //Informacion de settings
    msj += "<hr><b>"+tr("Debug Info:")+"</b>";
    QStringList keys = settings->allKeys();
    for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) 
    {
        msj += "<br>" + *it + " = "+ settings->value(*it).toString();
    }

    //Mostramos el dialogo
    QMessageBox::about( this, "Coindesigner", msj );

}//void MainWindow::on_actionAbout_triggered()

///Cambia el contenido de la paleta de nodos
void MainWindow::on_paletteComboBox_activated(int idx)
{
    //Miramos la opcion escogida
    switch (idx)
    {
    case -1: break;
    case 0: generarListaComponentes(SoNode::getClassTypeId(), false); break;
    case 1: generarListaComponentes(SoNode::getClassTypeId(), true); break;
    case 2: generarListaComponentes(comp_simple, true); break;
    case 3: generarListaComponentes(comp_SGI, false); break;
    default:
#ifdef USE_VOLEON
        if (Ui.paletteComboBox->currentText() == QString("SIM Voleon"))
        {
            generarListaComponentes(comp_simVoleon, false); break;
        }
        else
#endif
        {
            //En caso de error escribimos un aviso
            QString S;
            S = Ui.paletteComboBox->currentText() + ": "+tr("Unknow option");
            QMessageBox::warning( this, tr("Warning"), S);
        }
    } // switch (idx)

    //Ordenamos los nodos del arbol
    Ui.nodePalette->sortItems(0, Qt::AscendingOrder);

}//void MainWindow::on_paletteComboBox_activated(int value)

///Orden de insertar un nuevo nodo en el grafo de escena
void MainWindow::on_nodePalette_itemDoubleClicked(QTreeWidgetItem * item, int)
{
    //Leemos el tipo de nodo seleccionado a partir del texto del item
    SoType tipo = SoType::fromName (SbName(item->text(0).toLatin1()));

    //Miramos si el tipo se pudo crear correctamente
    if (tipo == SoType::badType() )
    {
        QMessageBox::warning( this, tr("Warning"), tr("Unknow SoType:")+item->text(0));
        return;
    }

    //Si la clase no es insertable, salimos inmediatamente
    if ( !tipo.canCreateInstance() )
        return;

    //Creamos un nodo del tipo indicado 
    SoNode *newNode = (SoNode *)tipo.createInstance();
    QTreeWidgetItem *newItem = newNodeItem(newNode);

    //Buscamos el item actualmente seleccionado en el sceneGraph y 
    //el primer item que pueda actuar como contenedor para colgar newNode
    QTreeWidgetItem *item_current = Ui.sceneGraph->currentItem();
    QTreeWidgetItem *item_padre = item_current;
    while (!mapQTCOIN[item_padre]->getTypeId().isDerivedFrom(SoGroup::getClassTypeId()) )
    {
        item_padre=item_padre->parent();
    }  

    //Buscamos el nodo de coin correspondiente al item_padre
    SoGroup *nodo_padre=(SoGroup*)mapQTCOIN[item_padre];

    //Comprobamos si vamos a insertar dentro de item_current o detras de item_current
    if (item_current == item_padre)
    {
        //Insertamos el nodo y el item dentro del item actual
        item_padre->addChild(newItem);
        nodo_padre->addChild(newNode);
    }
    else
    {
        //Buscamos la posicion del nodo_current dentro de su padre
        int pos = nodo_padre->findChild(mapQTCOIN[item_current]);

        //Insertamos detras del item_current
        item_padre->insertChild(pos+1, newItem);
        nodo_padre->insertChild(newNode, pos+1);
    }

    //Seleccionamos el nodo y aseguramos que es visible
    Ui.sceneGraph->setCurrentItem(newItem);
    Ui.sceneGraph->scrollToItem (newItem);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}//void MainWindow::on_nodePalette_itemDoubleClicked(QTreeWidgetItem * item, int)

///Orden de actualizar la tabla Ui.fieldTable con un nuevo item
void MainWindow::on_sceneGraph_currentItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *)
{
	if(!item)
        return;

    //Buscamos el nodo que hay que mostrar en el editor Ui.fieldTable
    SoNode *nodo=mapQTCOIN[item];
    assert(nodo);

	updateFieldEditor(nodo);
}//void MainWindow::on_sceneGraph_currentItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *)

void MainWindow::updateFieldEditor(SoNode *nodo)
{
    //Indicamos que estamos modificando la tabla
    edit_node = NULL;

	//Si no se introduce un valor valido, se usa el item actual
	if (nodo == NULL)
		nodo = mapQTCOIN[Ui.sceneGraph->currentItem()];

    //Cambiamos el sensor para vigilar este nodo
    if(refreshGUI_Sensor != NULL)
    	refreshGUI_Sensor->setData(nodo);

    //Extraemos su lista de campos
    SoFieldList  fields;
    nodo->getFields(fields);

    //Leemos el numero de campos
    int num_fields=fields.getLength();

    //Anadimos el SoBase::name, (que no es un campo real)
    //Leemos el nombre del nodo
    SbName name = nodo->getName();

    //Almacenamos el nombre en SoBase_name
    SoBase_name->setValue(name);

    //Actualizamos el campo segun el contenido
    Ui.fieldTable->setRowCount (1);
    Ui.fieldTable->setVerticalHeaderItem(0,new QTableWidgetItem ("name"));
    Ui.fieldTable->setItem(0,0,new QTableWidgetItem (name.getString()));

    //Apuntamos al "falso" campo SoBase_name
    map_fieldTable[0] = SoBase_name;

    //Contador de filas en la tabla Ui.fieldTable
    int numRows=1;

    //Rellenamos la tabla con los campos
    QString S;
    for (int f=0; f < num_fields; f++)
    {
         SoField *field = fields[f];

        //Leemos el tipo de este campo
         SoType tipo=field->getTypeId();
         const char*nombre_tipo = tipo.getName();  

         //Leemos el nombre de este campo
         SbName nombre_field;
         nodo->getFieldName(field, nombre_field);
         const char*nombre_campo = nombre_field.getString();  

         //Escribimos el nombre del tipo y del campo
         //qDebug(".%s (%s)\n", nombre_campo, nombre_tipo);

         //Anadimos una fila con el correspondiente campo
         Ui.fieldTable->setRowCount (numRows+1);

         if(field->isOfType(SoMField::getClassTypeId()))
         {
             //Cambiamos la cabecera para indicar que es un MF...
             S.asprintf("%s[%d]", nombre_campo, ((SoMField *)field)->getNum() );
         }
         else
         {
             S=nombre_campo;
         }

		//Esto soluciona un Bug en coin3D. El campo SoTexture2::enableCompressedTexture no introduce su 
		//nombre correctamente
		if (S.isEmpty())
		{
			if (nodo->getTypeId() == SoTexture2::getClassTypeId() && f== 6)
				S="enableCompressedTexture";
		}

         //Anadimos una fila para este campo
         Ui.fieldTable->setVerticalHeaderItem(numRows, new QTableWidgetItem (S));
         Ui.fieldTable->setItem(numRows,0,new QTableWidgetItem ());

		 //Comprobamos si este campo esta conectado con otros o con un engine
		 SoFieldList slaveList;
		 if (field->isConnected() || field->getForwardConnections(slaveList) )
         {
			 //Informacion acerca de la conexion
			 //Ui.fieldTable->verticalHeaderItem(numRows)->setText(S+QString("<="));
			 Ui.fieldTable->verticalHeaderItem(numRows)->setForeground(QBrush(QColor(255,0,0)));
			 Ui.fieldTable->item(numRows,0)->setForeground(QBrush(QColor(255,0,0)));

			 S.clear();

			 //Lista de campos conectados como master de field
			 SoFieldList masterFields;
			 field->getConnections(masterFields);
			 if(masterFields.getLength() > 0)
			 {
				S+= tr("This field is conected as slave of:")+"\n";
			 	for (int i =0; i< masterFields.getLength(); i++)
			 	{
				 	//Identificamos el field master y su container
				 	SoField *mf = masterFields.get(i);
				 	SoFieldContainer *mfc = mf->getContainer();
				 	SbName mName;
					QString masterName(mfc->getName().getString());
					if (masterName.isEmpty())
						masterName.asprintf("%p", mfc);
				 	mfc->getFieldName(mf, mName);
				 	S+= QString("<-") + masterName + QString(".")+ QString(mName.getString())+"\n";
			 	}//for
			 }

			 //Lista de engines conectados como master de field
			 SoEngineOutput *mf;
			 if (field->getConnectedEngine (mf))
			 {
				 S+= tr("This field is conected as slave of:")+"\n";
				 //Identificamos el engine master 
				 SoEngine *mfc = mf->getContainer();
				 SbName mName;
				 mfc->getOutputName(mf, mName);
				 QString masterName(mfc->getName().getString());
				 if (masterName.isEmpty())
					 masterName.asprintf("%p", mfc);
				 S+= QString("<-")+QString("Engine:") + masterName + QString(".")+ QString(mName.getString())+"\n";
			 }

			 if(slaveList.getLength() > 0)
			 {
				S+= tr("This field is conected as master of:")+"\n";
			 	//Lista de campos conectados como esclavos del field
			 	for (int i =0; i< slaveList.getLength(); i++)
				{
					//Identificamos el field slave y su container
					SoField *mf = slaveList.get(i);
					SoFieldContainer *mfc = mf->getContainer();
					QString slaveName(mfc->getName().getString());
					if (slaveName.isEmpty())
						slaveName.asprintf("%p", mfc);
					SbName mName;
					mfc->getFieldName(mf, mName);
					S+= QString("->")+tr("Slave Field:") + slaveName
						+ QString(".")+ QString(mName.getString())+"\n";
				}//for
			 }

			 //Asigna el toolTip
			 Ui.fieldTable->item(numRows,0)->setToolTip(S);

         }//if (field->isConnected() || field->getForwardConnections(slaveList) )


         //Mapeamos el numero de fila con el field correspondiente
         map_fieldTable[numRows] = field;

         //Rellenamos con el valor del campo, segun el tipo

         //Actualizacion de cualquier campo tipo Basico
         bool campoRelleno=false;

         //Lista con tipos SF basicos que pueden describirse con una llamada a .get()
         const char *tipoBasicoSF[] = {"SFBool", "SFDouble", "SFFloat", "SFInt32", "SFString", "SFName",
                                       "SFShort", "SFUInt32", "SFUShort", "SFVec", "SFTime",
                                       NULL};

         //Lista con tipos MF basicos que pueden describirse con una llamada a .get1()
         const char *tipoBasicoMF[] = {"MFBool", "MFDouble", "MFFloat", "MFInt32", "MFString", "MFName",
                                       "MFShort", "MFUInt32", "MFUShort", "MFVec",
                                       NULL};

         //Recorremos los tipos SF basicos
         for (unsigned i=0; tipoBasicoSF[i]; i++)
         {
            //Miramos si es de este tipo basico
            if (strstr(nombre_tipo, tipoBasicoSF[i]) )
            {
               SbString valueString;
               field->get(valueString);
           
               //Actualizamos el campo segun el contenido
               Ui.fieldTable->item(numRows,0)->setText(valueString.getString() );

               //qDebug("Usando tipoBasico %s (%s) -> %s\n", nombre_campo, nombre_tipo, valueString.getString());
               campoRelleno = true;
               break;
            }
         }

         //Recorremos los tipos MF basicos
         for (unsigned i=0; tipoBasicoMF[i]; i++)
         {
            //Miramos si es de este tipo basico
            if (strstr(nombre_tipo, tipoBasicoMF[i]) )
            {
               SbString valueString;
               //Si contiene algun valor, mostramos el primero de ellos
               if (((SoMField *)field)->getNum()>0)
               {
                   ((SoMField *)field)->get1(0, valueString);
                   Ui.fieldTable->item(numRows,0)->setText(valueString.getString() );
               }
           
               //qDebug("Usando tipoBasico %s (%s) -> %s\n", nombre_campo, nombre_tipo, valueString.getString());
               campoRelleno = true;
               break;
            }
         } //for

         //Si ya esta tratado, pasamos al siguiente campo
         if (campoRelleno)
         {
             //Aumentamos el numero de filas
             numRows++;
             continue;
         }

         //Actualizacion de cualquier campo tipo SoSFEnum o SoSFBitMask
         if (!strcmp(nombre_tipo, "SFEnum") ||
             !strcmp(nombre_tipo, "SFBitMask") )
         {
            //Convertimos el tipo de field
            SoSFEnum *soSFEnum= (SoSFEnum *)field;

            SbName nombre;
            int idx;
            //Probamos todos los indices hasta encontrar el correspondiente
            //al valor que tenemos
            for (idx=0; idx < soSFEnum->getNumEnums(); idx++)
            {
                if (soSFEnum->getEnum(idx, nombre) == soSFEnum->getValue())
                  break;
            }
     
            //Actualizamos el campo segun el contenido
            Ui.fieldTable->item(numRows,0)->setText(nombre.getString() );
         }
         //Actualizacion de cualquier campo tipo SoSFName 
         else if (!strcmp(nombre_tipo, "SFName") )
         {
            //Convertimos el tipo de field
            SoSFName *str= (SoSFName *)field;

            //Actualizamos el campo segun el contenido
            Ui.fieldTable->item(numRows,0)->setText(str->getValue().getString() );
         }
         //Actualizacion de cualquier campo tipo SoSFColor
         else if (!strcmp(nombre_tipo, "SFColor") )
         {
            //Convertimos el tipo de field
            SoSFColor *color= (SoSFColor *)field;

            //Lo convertimos en valores RGB
            const float*rgb = color->getValue().getValue();
            S.asprintf("%.02g %.2g %.02g", rgb[0], rgb[1], rgb[2]);
           
            //Actualizamos el campo segun el contenido
            Ui.fieldTable->item(numRows,0)->setText(S);
         }
         //Actualizacion de cualquier campo tipo SoSFRotation
         else if (!strcmp(nombre_tipo, "SFRotation") )
         {
            //Convertimos el tipo de field
            SoSFRotation *rot= (SoSFRotation *)field;

            //Lo descomponemos en eje y angulo
            SbVec3f axis;
            float angle;
            rot->getValue(axis, angle);

            //Lo mostramos como campos separados. Primero el eje
            S.asprintf("%s.axis", nombre_campo);
            Ui.fieldTable->verticalHeaderItem(numRows)->setText(S);

            const float*xyz = axis.getValue();
            S.asprintf("%.3g %.3g %.3g", xyz[0], xyz[1], xyz[2]);
            Ui.fieldTable->item(numRows,0)->setText(S);

            //Ahora el angulo
            numRows++;
            Ui.fieldTable->setRowCount (numRows+1);
            Ui.fieldTable->setVerticalHeaderItem(numRows,new QTableWidgetItem ());
            Ui.fieldTable->setItem(numRows,0,new QTableWidgetItem ());

            S.asprintf("%s.angle", nombre_campo);
            Ui.fieldTable->verticalHeaderItem(numRows)->setText(S);
            S.asprintf("%.3g", angle);
            Ui.fieldTable->item(numRows,0)->setText(S.setNum(angle));

            //Mapeamos el numero de fila con el field correspondiente
            map_fieldTable[numRows] = field;
         }
         //Actualizacion de cualquier campo tipo SoSFPlane
         else if (!strcmp(nombre_tipo, "SFPlane") )
         {
            //Convertimos el tipo de field
            SoSFPlane *plane= (SoSFPlane *)field;

            //Lo descomponemos en normal y distancia
            const SbVec3f norm = plane->getValue().getNormal();
            const float dist = plane->getValue().getDistanceFromOrigin();

            //Lo mostramos como campos separados. Primero el eje
            S.asprintf("%s.norm", nombre_campo);
            Ui.fieldTable->verticalHeaderItem(numRows)->setText(S);
            const float*xyz = norm.getValue();
            S.asprintf("%.3g %.3g %.3g", xyz[0], xyz[1], xyz[2]);
            Ui.fieldTable->item(numRows,0)->setText(S);

            //Ahora el angulo
            numRows++;
            Ui.fieldTable->setRowCount (numRows+1);

            S.asprintf("%s.dist", nombre_campo);
            Ui.fieldTable->setVerticalHeaderItem(numRows,new QTableWidgetItem (S));
            S.asprintf("%.3g", dist);
            Ui.fieldTable->setItem(numRows,0,new QTableWidgetItem (S.setNum(dist)));

            //Mapeamos el numero de fila con el field correspondiente
            map_fieldTable[numRows] = field;
         }
         //Actualizacion de cualquier campo tipo SoSFImage
         else if (!strcmp(nombre_tipo, "SFImage") )
         {
            //Convertimos el tipo de field
            SoSFImage *img= (SoSFImage *)field;

            //Lo descomponemos en dimensiones y numero componentes
            SbVec2s size;
            int nc;
            img->getValue(size, nc);

            //Actualizamos el campo segun el contenido
            S.asprintf("%dx%dx%d", size[0], size[1], nc);
            Ui.fieldTable->item(numRows,0)->setText(S);
         }
         //Actualizacion de cualquier campo tipo SoSFNode
         else if (!strcmp(nombre_tipo, "SFNode") )
         {
            //Convertimos el tipo de field
            SoSFNode *soSFNode= (SoSFNode *)field;
            SoNode *node = soSFNode->getValue();
            
            //Miramos si tiene nombre asignado
            SbName name("NULL");
            if (node)
            {
               name = node->getName();
            }

            //Mostramos el nombre del nodo
            Ui.fieldTable->item(numRows,0)->setText(name.getString());
            
         }
         //Actualizacion de cualquier campo tipo SoMFNode
         else if (!strcmp(nombre_tipo, "MFNode") )
         {
            //Convertimos el tipo de field
            SoMFNode *soMFNode= (SoMFNode *)field;

            //Anadimos una fila por cada nodo
            for (int i=0; i<soMFNode->getNumNodes(); i++)
            {
              SoNode *node = soMFNode->getNode(i);
            
              //Miramos si tiene nombre asignado
              SbName name("NULL");
              if (node)
              {
                 name = node->getName();
              }

              //Cambiamos la cabecera para indicar que es un MF...
              S.asprintf("%s[%d]", nombre_campo, i);
              Ui.fieldTable->verticalHeaderItem(numRows)->setText(S);

              //Mostramos el nombre del nodo
              Ui.fieldTable->item(numRows,0)->setText(name.getString());

              //Si es necesario, hacemos crecer la tabla,
              if (i < soMFNode->getNumNodes()-1)
              {
                  numRows++;
                  Ui.fieldTable->setRowCount (numRows+1);
                  Ui.fieldTable->setVerticalHeaderItem(numRows,new QTableWidgetItem ());
                  Ui.fieldTable->setItem(numRows,0,new QTableWidgetItem ());

                  //Mapeamos el numero de fila con el field correspondiente
                  map_fieldTable[numRows] = field;
              }

            }//for
  
         }
         //Actualizacion de cualquier campo tipo SoMFColor
         else if (!strcmp(nombre_tipo, "MFColor") )
         {
            //Convertimos el tipo de field
            SoMFColor *color= (SoMFColor *)field;

            //Convertimos en valores RGB el primer valor del campo
            if(color->getNum() > 0)
            {   
                const float*rgb = (*color)[0].getValue();
                S.asprintf("%.02g %.2g %.02g", rgb[0], rgb[1], rgb[2]);

                //Actualizamos el campo segun el contenido
                Ui.fieldTable->item(numRows,0)->setText(S);
            }

		 }
         //Edicion de cualquier campo tipo SoSFMatrix
        else if (!strcmp(nombre_tipo, "SFMatrix") )
	 {
		 Ui.fieldTable->item(numRows,0)->setText(tr("Edit"));
	 }

	 else
	 {
			 QMessageBox::warning( this, tr("Error"), tr("No support for type: ")+nombre_tipo);
	 }

	 //Aumentamos el numero de filas
	 numRows++;

    } // for (int f=0; f < num_fields; f++)

	//Filas que no corresponden a campos del nodo
	if (nodo->getTypeId() == SoTexture2::getClassTypeId())
	{
		//Creamos un widget que muestra esta imagen
		SoTexture2 *texture2 = (SoTexture2 *)nodo;
		QImage *image = SoSFImage_to_QImage(&texture2->image);
		if (image != NULL)
		{
			QLabel *imageLabel = new QLabel;
			imageLabel->setBackgroundRole(QPalette::Base);
			imageLabel->setPixmap(QPixmap::fromImage(*image));

			//Anadimos una fila con una imagen para este campo
			Ui.fieldTable->setRowCount (numRows+1);
			Ui.fieldTable->setVerticalHeaderItem(numRows, new QTableWidgetItem ("img"));
			Ui.fieldTable->setCellWidget(numRows,0,imageLabel);
			Ui.fieldTable->setRowHeight(numRows, image->height());
			numRows++;

			delete image;
		}

	}// if (nodo->getTypeId() == SoTexture2::getClassTypeId())

    //Aseguramos que las columnas tienen ancho suficiente
    Ui.fieldTable->resizeColumnsToContents();

    //Preparamos el mensaje a mostrar en la barra de estatus
    SoType tipo = nodo->getTypeId();
    QString M(tipo.getName().getString());

    //Nombre del nodo
    if (strlen(name.getString()) > 0)
      M += QString(" <> ") + tr("Name=") + QString(name.getString());

    //Refcount
    int refcount = nodo->getRefCount(); 
    if (refcount != 1)
    	M += QString(" <> ") + tr("rc=") + QString::number(refcount);

    //Si es un indexedFaceset, numero de facetas
    if (tipo.isDerivedFrom(SoIndexedFaceSet::getClassTypeId())) 
    {
        SoIndexedFaceSet *n2 = (SoIndexedFaceSet *)nodo;

        //Numero de facetas
        M += QString(" <> ") + tr("Faces:") + QString::number(count_facets(n2));

    }
    else if (tipo.isDerivedFrom(SoVRMLIndexedFaceSet::getClassTypeId())) 
    {
        SoVRMLIndexedFaceSet *n2 = (SoVRMLIndexedFaceSet *)nodo;

        //Numero de facetas
        M += QString(" <> ") + tr("Faces:") + QString::number(count_facets(n2));

    }

    else if (tipo.isDerivedFrom(SoCoordinate3::getClassTypeId())) 
    {
        SoCoordinate3 *n2 = (SoCoordinate3 *)nodo;

        //Numero de puntos
        M += QString(" <> ") + tr("Points:") + QString::number(n2->point.getNum());

        //Centroide
        SbVec3f centro = centroid (n2->point);
        M += QString(" <> ") + tr("Center:") + S.asprintf("(%.3f,%.3f,%.3f)", centro[0], centro[1], centro[2]);
    }

    else if (tipo.isDerivedFrom(SoVertexProperty::getClassTypeId())) 
    {
        SoVertexProperty *n2 = (SoVertexProperty *)nodo;

        //Numero de puntos
        M += QString(" <> ") + tr("Points:") + QString::number(n2->vertex.getNum());

        //Centroide
        SbVec3f centro = centroid (n2->vertex);
        M += QString(" <> ") + tr("Center:") + S.asprintf("(%.3f,%.3f,%.3f)", centro[0], centro[1], centro[2]);
    }

    //Mostramos el mensaje M en la barra de status
    this->statusBar()->showMessage(M);

    //Indicamos que el nodo representado en el fieldTable es este nodo
    edit_node = nodo;

}//void MainWindow::updateFieldEditor(SoNode *nodo)

void MainWindow::on_fieldTable_cellChanged(int row, int column)
{
    //Evitamos actualizar si el cambio no lo ha hecho el usuario
    if (edit_node && Ui.fieldTable->item(row,column) == Ui.fieldTable->currentItem())
    {
        on_fieldTable_userChanged(row, column); 
    }
}


///Aplica modificaciones del usuario sobre el editor de campos
void MainWindow::on_fieldTable_userChanged(int row, int column)
{
    //Evitamos modificaciones no realizadas por el usuario
    if (!edit_node)
        return;

    //Buscamos el nodo que estamos editando
    SoNode *nodo = edit_node;

    //Buscamos el SoField correspondiente a la fila que hemos modificado
    SoField* field = map_fieldTable[row];
    if (!field)
       return;

	//Resize column patch contributed by Manfred Kroehnert
	Ui.fieldTable->resizeColumnToContents(column);

    //Si se ha modificado un SoMField, sacamos el editor de Mfield (y perdemos el cambio)
	//salvo que sea MFNode, que si lo gestionamos aqui
	if (field->getTypeId().isDerivedFrom(SoMField::getClassTypeId()) 
	    && field->getTypeId() != SoMFNode::getClassTypeId() ) 
	{
	    //Usamos el mfield_editor
        MFieldEditor mfield_ed((SoMField *)field, this);
        mfield_ed.exec();

		//Actualizamos el Ui
		updateFieldEditor(nodo);
		return;
	}

    //Leemos el nombre de este campo
    SbName nombre_field;
    nodo->getFieldName(field, nombre_field);
    //const char*nombre_campo = nombre_field.getString();  

    //Leemos el tipo de este campo
    SoType tipo=field->getTypeId();
    const char*nombre_tipo = tipo.getName();

    //Leemos el contenido de la celda modificada
    QTableWidgetItem *item=Ui.fieldTable->item(row,column);
    SbString item_string(qPrintable(item->text()));
    const char *item_txt = item_string.getString();

    //Lista con tipos SF basicos que pueden leerse con una llamada a .set()
    const char *tipoBasicoSF[] = {"SFBool", "SFDouble", "SFFloat", "SFInt32", "SFString", "SFName",
                                  "SFShort", "SFUInt32", "SFUShort", "SFVec", "SFColor", "SFTime",
                                  NULL};

    //Tratamiento especial para el SoBase_name, que no es un campo real del nodo
    if (field == SoBase_name)
    {
        //Almacenamos el valor en el campo SoBase_name
        SoBase_name->setValue(item_string);

        //Almacenamos el valor en el nodo
        nodo->setName(SoBase_name->getValue());

		//Update the name of the node
		updateNodeItemName(Ui.sceneGraph->currentItem());

        //Salimos de la funcion inmediatamente
        escena_modificada = true;
        return;
    }
    else
    {
        //Recorremos los tipos basicos que pueden leerse con una llamada a .set()
        for (unsigned i=0; tipoBasicoSF[i]; i++)
        {
            //Miramos si es de este tipo basico
            if (strstr(nombre_tipo, tipoBasicoSF[i]) )
            {
                //Sacamos una copia del field
                SbString oldValue;
                field->get(oldValue);

                //Leemos la cadena mediante el parser de openInventor
                if (!field->set(item_txt))
                {
                    //Si hubo un error en la lectura, restablecemos el valor
                    field->set(oldValue.getString() );
                    item->setText(item_txt);

                    //Mostramos un mensaje de aviso
                    QString S;
                    S=tr("Invalid value for field:")+nombre_tipo;
                    QMessageBox::warning( this, tr("Warning"), S);
                }

                //qDebug("Escrito tipoBasico %s (%s) -> %s\n", nombre_field.getString(), nombre_tipo, txt);

                //Salimos de la funcion inmediatamente
                escena_modificada = true;
                return;

            }//if (strstr(nombre_tipo, tipoBasicoSF[i]) )
        }//for (unsigned i=0; tipoBasicoSF[i]; i++)


        //Edicion de cualquier campo SFRotation
        if (!strcmp(nombre_tipo, "SFRotation") )
        {
            //Este es un poco especial, porque hemos dividido el
            //campo en dos filas (axis y angle). Tenemos que deshacer
            //la descomposicion

            //Convertimos el tipo de field
            SoSFRotation *rot= (SoSFRotation *)field;

            //Aseguramos que fila apunta al valor de axis
            QString label= Ui.fieldTable->verticalHeaderItem(row)->text();
            if (label.contains(".angle"))
            {
                row--;
                label = Ui.fieldTable->verticalHeaderItem(row)->text();
            }

            if (!label.contains(".axis"))
            {
                __chivato__;
				qDebug("Error interno: no encuentro campo .axis\n");
            }

            //Extraemos el contenido de ambas celdas
            SbString axis_string(qPrintable(Ui.fieldTable->item(row,column)->text()));
            const char *axis_txt = axis_string.getString();
            SbString angle_string(qPrintable(Ui.fieldTable->item(row+1,column)->text()));
            const char *angle_txt = angle_string.getString();

            //Leemos las cadenas mediante el parser de openInventor
            //si ambas lecturas son correctas, asignamos el valor
            SoSFVec3f axis;
            SoSFFloat angle;
            if (axis.set(axis_txt) && angle.set(angle_txt))
            {
                rot->setValue(axis.getValue(), angle.getValue());
            }
            else
            {
                //Si hubo error de lectura lo dejamos como estaba
                Ui.fieldTable->item(row,column)->setText(axis_txt);
                Ui.fieldTable->item(row+1,column)->setText(angle_txt);

                //Mostramos un mensaje de aviso
                QString S=tr("No valid value for:")+nombre_tipo;
                QMessageBox::warning( this, tr("Warning"), S);
            }
        }
        //Edicion de cualquier campo SFPlane
        else if (!strcmp(nombre_tipo, "SFPlane") )
        {
            //Este es un poco especial, porque hemos dividido el
            //campo en dos filas (normal y distancia). Tenemos que deshacer
            //la descomposicion

            //Convertimos el tipo de field
            SoSFPlane *plane= (SoSFPlane *)field;

            //Aseguramos que fila apunta al valor de normal
            QString label= Ui.fieldTable->verticalHeaderItem(row)->text();
            if (label.contains(".dist"))
            {
                row--;
                label = Ui.fieldTable->verticalHeaderItem(row)->text();
            }

            if (!label.contains(".norm"))
            {
                qDebug("Error interno: no encuentro campo .norm\n");
            }

            //Extraemos el contenido de ambas celdas
            SbString norm_string(qPrintable(Ui.fieldTable->item(row,column)->text()));
            const char *norm_txt = norm_string.getString();
            SbString dist_string(qPrintable(Ui.fieldTable->item(row+1,column)->text()));
            const char *dist_txt = dist_string.getString();

            //Leemos las cadenas mediante el parser de openInventor
            //si ambas lecturas son correctas, asignamos el valor
            SoSFVec3f norm;
            SoSFFloat dist;
            if (norm.set(norm_txt) && dist.set(dist_txt))
            {
                SbPlane pl(norm.getValue(), dist.getValue() );
                plane->setValue(pl);
            }
            else
            {
                //Si hubo error de lectura lo dejamos como estaba
                Ui.fieldTable->item(row,column)->setText(norm_txt);
                Ui.fieldTable->item(row+1,column)->setText(dist_txt);

                //Mostramos un mensaje de aviso
                QString S=tr("No valid value for:")+nombre_tipo;
                QMessageBox::warning( this, tr("Warning"), S);
            }
        }

        //Edicion de cualquier campo SFNode
        else if (!strcmp(nombre_tipo, "SFNode") )
        {
            //Buscamos los nodos con ese nombre
            SoSearchAction searchAction;
            searchAction.setName(SbName(item_string));
            searchAction.setInterest(SoSearchAction::ALL);
            searchAction.setSearchingAll(true);
            searchAction.apply(root);
            SoPathList lista = searchAction.getPaths();

            //Miramos que haya exactamente 1 resultado
            int num = lista.getLength();
            if (num == 1)
            {
                //asignamos el nodo encontrado al final del path
                SoNode *node =  lista[0]->getTail();

                //Si es un nodo simple, tenemos su field
                if (!strcmp(nombre_tipo, "SFNode") )
                {
                    ((SoSFNode *)field)->setValue(node);
                }
                //Si es un MFNode, hay que buscar la posicion
                else if (!strcmp(nombre_tipo, "MFNode") )
                { 
                    //Buscamos la posicion contando cuentas filas por
                    //encima de nosotros apuntan al mismo campo
                    int pos=0;
                    while(map_fieldTable[row-pos-1] == field)
                        pos++;

                    //Le damos el valor adecuado en la posicion pos
                    ((SoMFNode *)field)->set1Value(pos, node);
                }
                else
                {  
                    //No hay soporte.
                    item->setText(item_txt);

                    //Mostramos un mensaje de aviso
                    QString S=tr("Unsupported type:")+nombre_tipo;
                    QMessageBox::warning( this, tr("Warning"), S);
                }
        }
        else
        {
            //Mostramos un mensaje de aviso
            QString S;
            S.asprintf(tr("There are %d nodes with the name:").toLatin1(), num);
            S+=item_txt;
            QMessageBox::warning( this, tr("Warning"), S);

            //Intentamos restaurar el valor del campo
            SbName name("NULL");
            SoNode *node = ((SoSFNode *)field)->getValue();
            if (node)
            {
                name = node->getName();
            }

            item->setText(name.getString());
        }
        }

        else
        {  
            //No hay soporte
            item->setText(item_txt);

            //Mostramos un mensaje de aviso
            QString S;
            S=nombre_tipo+tr(": Unsupported type");
            QMessageBox::warning( this, tr("Warning"), S);
        }

        //Indicamos que la escena ha sido modificada
        escena_modificada = true;
    }
}//void on_fieldTable_cellChanged(int row, int column)

///Genera la paleta de componentes mediante una lista predefinida
void MainWindow::generarListaComponentes(const ivPadre_t *st, bool plano)
{

  //Vaciamos el arbol actual
  Ui.nodePalette->clear();

  while (st && st->clase)
  {
    //qDebug("%s %s\n", st->clase, st->padre);

	QString tName(st->clase);
	QString pName(st->padre);

    SoType t=SoType::fromName (st->clase);
    if (t.isBad())
    {
        QString S;
        S = tName + " type.isBad()";
        QMessageBox::critical(this, tr("Critical Error"), S, QMessageBox::Abort);
        assert (!t.isBad());
    }

    //evitamos introducir nodos no instanciables en modo plano
	if (plano && !t.canCreateInstance())
	{
		//Apuntamos al siguiente elemento de la lista y seguimos
		st++;
		continue;
	}

    QTreeWidgetItem *item = NULL;

    //Si el tipo deriva de SoNode, o estamos en modo plano
	if (pName == "Node" || plano )
	{
       	item = new QTreeWidgetItem(Ui.nodePalette, QStringList(tName));
    }
    else
    {
	   //Buscamos el item padre
	   QList<QTreeWidgetItem *> il = Ui.nodePalette->findItems(pName, Qt::MatchRecursive);
	   assert(il.size() == 1);
       item = new QTreeWidgetItem(il.first(), QStringList(tName));
    }//else

	if (item)
	{
		//Asignamos un tooltip
		QString S;
		S += QString("Type: %1\n").arg(tName);
		S += QString("Derives from: %1\n").arg(pName);
		//S += QString("Key: %1\n").arg(t.getKey());
		S += QString("CanCreateInstance: %1").arg(t.canCreateInstance()?"Yes":"No" );
		item->setToolTip(0, S);
	}

	//Apuntamos al siguiente elemento de la lista
	st++;

  } //while

}// void MainWindow::generarListaComponentes(const ivPadre_t *st, bool plano)


///Autogenera la paleta de componentes mediante exploracion de coin3D
void MainWindow::generarListaComponentes(SoType t, bool plano, QTreeWidgetItem *padre)
{
/*
    QList<QTreeWidgetItem *> items;
 for (int i = 0; i < 10; ++i)
     items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("item: %1").arg(i))));
 treeWidget->insertTopLevelItems(0, items);
*/

    if (t.isBad())
    {
        QString S;
        S.asprintf("type.isBad()");
        QMessageBox::critical(this, tr("Critical Error"), S, QMessageBox::Abort);
        assert (!t.isBad());
    }

    SoTypeList tl;
    QTreeWidgetItem *item = NULL;

    //Si el tipo es SoNode, vaciamos la lista de componentes y comenzamos a rellenarla
    if (t == SoNode::getClassTypeId())
    {
        //Vaciamos el arbol actual
        Ui.nodePalette->clear();

        //Calcula lista de tipos derivados
        SoType::getAllDerivedFrom(t, tl);

        //Recorre todos los tipos derivados
        for (int j=0; j < tl.getLength(); j++) 
        {
            //Si es hijo directo de SoNode, lo insertamos y aplicamos recursividad
            if (!tl[j].isBad() && tl[j].getParent() == t) 
            { 
                //Evitamos el SoUnknownNode, parece ser un error de coin
                QString tName(tl[j].getName().getString());
                if (tName == "UnknownNode")
                    continue;

                //Insertamos este tipo
                if (!plano || tl[j].canCreateInstance())
                    item = new QTreeWidgetItem(Ui.nodePalette, QStringList(tName));

				if (item)
				{
					//Asignamos un tooltip
					QString S;
					S += QString("Type: %1\n").arg(tl[j].getName().getString());
					S += QString("Derives from: %1\n").arg(t.getName().getString());
					//S += QString("Key: %1\n").arg(tl[j].getKey());
					S += QString("CanCreateInstance: %1").arg(tl[j].canCreateInstance()?"Yes":"No" );
    				item->setToolTip(0, S);
				}

                //qDebug("%d %s %s\n", tl[j].getKey(), tl[j].getName().getString(), t.getName().getString() );

                //y aplicamos recursividad
                generarListaComponentes(tl[j], plano, item);

            }// if (!tl[j].isBad() && tl[j].getParent() == t) 
        }//for
    }
    else
    {
        //Calcula lista de tipos derivados
        SoType::getAllDerivedFrom(t, tl);

        //Recorre todos los tipos derivados
        for (int j=0; j < tl.getLength(); j++) 
        {
            //Anade los hijos directos de este nodo
            if (!tl[j].isBad() && tl[j].getParent() == t) 
            { 
                QString tName(tl[j].getName().getString());

				item = NULL;

                //Anadimos el tipo a la lista de componentes
                if (plano)
                { 
                    //Evitamos anadir nodos que no podemos seleccionar
                    if (tl[j].canCreateInstance())
                    {
                        item = new QTreeWidgetItem(Ui.nodePalette, QStringList(tName));
                    }
                }
                else
                {
                    item = new QTreeWidgetItem(padre, QStringList(tName));
                }

				if (item)
				{
					//Asignamos un tooltip
					QString S;
					S += QString("Type: %1\n").arg(tl[j].getName().getString());
					S += QString("Derives from: %1\n").arg(t.getName().getString());
					//S += QString("Key: %1\n").arg(tl[j].getKey());
					S += QString("CanCreateInstance: %1").arg(tl[j].canCreateInstance()?"Yes":"No" );
    				item->setToolTip(0, S);
				}

               	//qDebug("%d %s %s\n", tl[j].getKey(), tl[j].getName().getString(), t.getName().getString() );

	            //Aplica recursividad a los hijos directos
    	        generarListaComponentes(tl[j], plano, item);
            }// if (!tl[j].isBad() && tl[j].getParent() == t) 
        }//for (int j=0; j < tl.getLength(); j++) 
    }//else

} // void MainWindow::generarListaComponentes(SoType t, QTreeWidgetItem *padre, bool plano)


///Crea y configura un QTreeWidgetItem para representar un SoNode
QTreeWidgetItem *MainWindow::newNodeItem(SoNode *node)
{
    //Creamos un nuevo QTreeWidgetItem
    QTreeWidgetItem *item = new QTreeWidgetItem();

    //Asignamos el mapa de nodos
    mapQTCOIN[item]=node;

    //Buscamos el icono y el tooltip adecuado para este item
    setNodeIcon(item);
	setNodeToolTip(item);

    //Asignamos el texto del item
	updateNodeItemName(item);

    return item;
}

/// Busca un tooltip adecuado a esta clase y lo asigna al item
void MainWindow::setNodeToolTip(QTreeWidgetItem *item)
{
	SoType nodeType = mapQTCOIN[item]->getTypeId();
	QString tooltip(nodeType.getName());
	item->setToolTip(0, tooltip);

}//void MainWindow::setNodeToolTip(QTreeWidgetItem *item)

/// Busca un icono adecuado a esta clase y lo asigna al item
bool MainWindow::setNodeIcon(QTreeWidgetItem *item)
{
    //Comprobamos que se ha pasado un item valido
    assert(item);
    assert(mapQTCOIN[item]);

    //Buscamos el tipo del nodo asociado a este item
    SoType t = mapQTCOIN[item]->getTypeId();

    //Buscamos el icono mas adecuado para este item
    while (t != SoNode::getClassTypeId())
    {
        //Miramos si existe una imagen con el nombre del tipo
        QString imgName = ":/icons/nodes/"+QString(t.getName()).toLower() + ".png";

        if(QResource(imgName).isValid() )
        {
            //Anadimos esta imagen como icono del elemento
            item->setIcon (0, QIcon(imgName));
            return true;
        }
        //Miramos si es un manipulador
        if(imgName.endsWith("manip.png") )
        {
            //Anadimos imagen manip.png como icono del elemento
            item->setIcon (0, QIcon(":/icons/nodes/manip.png"));
            return true;
        }

        //Buscamos el tipo padre del actual
        t = t.getParent();
    }//while

    //No ponemos ningun icono
    //TODO item->setIcon (0, NULL);
    return false;

}// setNodeIcon(QTreeWidgetItem * item)

///Asigna un nombre al item
void MainWindow::updateNodeItemName(QTreeWidgetItem* item)
{
	if (NULL == item)
		return;
	SoNode* node = mapQTCOIN[item];
	
	QString itemName = QString(node->getTypeId().getName());
	if (0 != node->getName().getLength())
	{
		itemName.append(" <");
		itemName.append(node->getName().getString());
		itemName.append(">");
	}
	item->setText(0, itemName);
}// updateNodeItemName(QTreeWidgetItem* item)


///Abre un url en un visor de HTML externo
void MainWindow::open_html_viewer(const QString &url)
{
    //DEBUG qDebug() << "URL=" << url;

	//Miramos si el url incluye un protocolo
	QString protocol("file://");
	if (url.startsWith("http:", Qt::CaseInsensitive) ||
		url.startsWith("file:", Qt::CaseInsensitive) ||
		url.startsWith("ftp:",  Qt::CaseInsensitive) )
	{
		protocol = "";
	}

    //Leemos la aplicacion para visualizar html
    QString helpApp = settings->value("helpViewer_app").toString();

    //Creamos un nuevo proceso con el visor de ayuda
    if ( !QProcess::startDetached(helpApp, QStringList(protocol+url), "") ) 
    {
        QString S;
        S = tr("Error while executing: ") + helpApp;
        QMessageBox::warning(this, tr("Error"), S);
        return;
    }

}//void MainWindow::open_html_viewer(const QString &url)


///Construye un SoPath a partir de un item del sceneGraph
SoPath *MainWindow::getPathFromItem(QTreeWidgetItem *item)
{
    //Construimos un path  segun la informacion del arbol de QT
    SoPath *path = new SoPath(mapQTCOIN[item]);
    path->ref();
    while (path->getHead() != root )
    {
        item = item->parent();
        SoPath *path2 = new SoPath(mapQTCOIN[item]);
        path2->ref();
        path2->append(path);
        path->unref();
        path = path2;
    }
    return path;
}//SoPath *MainWindow::getPathFromItem(QTreeWidgetItem *item)

///Busca el item correspondiente al nodo cola de un path
QTreeWidgetItem *MainWindow::getItemFromPath(const SoPath *path, bool setCurrentItem)
{
	//Aseguramos que el path comienza en root
	if (path->getHead() != root)
		return NULL;

	//Comenzamos identificando el item del nodo root
	QTreeWidgetItem *item = Ui.sceneGraph->topLevelItem(0);

	//Y buscamos el item a lo largo del path
    for (int i=0; i < path->getLength()-1; i++)
    {
	  item = item->child(path->getIndex(i+1));
    }

	//Miramos si debemos seleccionar este item como currentItem
	if (setCurrentItem)
	{
		//Seleccionamos el nodo y aseguramos que es visible
		Ui.sceneGraph->setCurrentItem(item);
		Ui.sceneGraph->scrollToItem (item);
	}

	return item;

}//QTreeWidgetItem *MainWindow::getItemFromPath(const SoPath *path)


///Crea subgrafo de escena QT a partir de un subgrafo de Coin3D
void MainWindow::newSceneGraph(SoNode *node, QTreeWidgetItem *item_padre, SoGroup *nodo_padre)
{
   /*<Depuracion>
    //A partir de un SoNode* , vamos a sacar su lista de campos y tipo
    {
      SoType tipo = node->getTypeId();
      const char*nombre_tipo = tipo.getName();  

      qDebug("Clase %s\n", nombre_tipo);
      SoFieldList  fields;
      node->getFields(fields);

      //Leemos el numero de campos
      int num_fields=fields.getLength();

      for (int f=0; f < num_fields; f++)
      {
         //Leemos el tipo de este campo
         SoType tipo=fields[f]->getTypeId();
         const char*nombre_tipo = tipo.getName();  
      
         //Leemos el nombre de este campo
         SbName nombre_field;
         node->getFieldName(fields[f], nombre_field);
         const char*nombre_campo = nombre_field.getString();  

         //Escribimos el nombre del tipo y del campo
         qDebug(".%s (%s)\n", nombre_campo, nombre_tipo);
      }
    } //</Depuracion> */

    //Si se ha indicado un nodo padre, anadimos el nodo hijo
    if (nodo_padre)
        nodo_padre->addChild(node);

    //Creamos un item para representar este nodo
    QTreeWidgetItem *item =newNodeItem(node);

    //Insertamos el nodo y el item dentro del item actual
    item_padre->addChild(item);

    //Miramos si este nodo deriva de SoGroup, directa o indirectamente
    if (node->getTypeId().isDerivedFrom(SoGroup::getClassTypeId())) 
    {
        SoGroup *group = (SoGroup*)node;
        //Anadimos los hijos mediante recursividad
        for (int i=0; i<group->getNumChildren(); i++)
        {
            newSceneGraph(group->getChild(i), item, NULL);  
        }    
    }//if

	//Evitamos desplegar los SoBaseKit
	if (!node->getTypeId().isDerivedFrom(SoBaseKit::getClassTypeId()))
	{
		//Miramos si este nodo contiene campos de tipo SFNode y MFNode
		SoFieldList  fields;
		node->getFields(fields);

		//Leemos el numero de campos
		int num_fields=fields.getLength();

		//Recorremos todos los campos mirando si contiene algun SFNode o MFNode
		for (int f=0; f < num_fields; f++)
		{
			//Leemos el tipo de este campo
			SoType tipo=fields[f]->getTypeId();

			//Miramos si es un SFNode
			if (tipo.isDerivedFrom(SoSFNode::getClassTypeId())) 
			{
				//Miramos si contiene un valor valido, y lo anadimos como hijo
				SoNode *f_node = ((SoSFNode *)fields[f])->getValue();
				if (f_node != NULL)
				{
					newSceneGraph(f_node, item, NULL);

					//Asignamos icono "field.png"
					for (int i=0; i< item->childCount(); i++)
						if (mapQTCOIN[item->child(i)] == f_node)
							item->child(i)->setIcon(0, QIcon(":/icons/nodes/field.png"));
				}
			}

			/*  revisar esto... nodos aparecen repetidos??
			//Miramos si es un MFNode
			else if (tipo.isDerivedFrom(SoMFNode::getClassTypeId())) 
			{
			//Convertimos el tipo de field
			SoMFNode *soMFNode= (SoMFNode *)fields[f];

			//Anadimos una fila por cada nodo
			for (int i=0; i<soMFNode->getNumNodes(); i++)
			{
			SoNode *i_node = soMFNode->getNode(i);
			if (i_node != NULL)
			{
			newSceneGraph(i_node, item, NULL);

			//Asignamos icono "field.png"
			for (int i=0; i< item->childCount(); i++)
			if (mapQTCOIN[item->child(i)] == i_node)
			item->child(i)->setIcon(0, QIcon(":/icons/nodes/field.png"));
			}
			}//for

			} //if 
			//*/
		} //for
	}//	if (!node->getTypeId().isDerivedFrom(SoBaseKit::getClassTypeId()))


	//Expandimos el nodo para que se vean los hijos
	item_padre->setExpanded(true);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}//void MainWindow::newSceneGraph(SoNode *node, QTreeWidgetItem *item_padre, SoGroup *nodo_padre)

///Carga una escena en cualquier formato conocido
SoSeparator * MainWindow::cargarFichero3D(QString filename)
{
    
    QFileInfo fileInfo(filename);
    if (!fileInfo.isFile() || !fileInfo.isReadable ())
    {
         QString S;
         S = filename + tr(": Error while opening file.");
         QMessageBox::critical( this, tr("Error"), S, QMessageBox::Abort); 
         return NULL;
    }


#ifdef USE_ASSIMP
    //Try to open the file with the assimp library
    std::vector<std::string> assimpExtensions = assimpImportedExtensions();
    for (unsigned i = 0; i < assimpExtensions.size(); ++i) {
        if (filename.endsWith(assimpExtensions.at(i).c_str(), Qt::CaseInsensitive)) {
            return importScene(filename.toStdString());
        }
    }
#endif

    //Convertimos el fichero a char *
    const char *strFilename = SbName(filename.toLatin1()).getString();

	//Miramos si tiene pinta de ser un fichero de tetgen
    if (filename.endsWith(".node", Qt::CaseInsensitive))
    {
        //Tratamos de cargarlo como un conjunto de ficheros de tetgen
        SoSeparator *sep=import_tetgen(strFilename);

        //Miramos si se cargo correctamente
        if (sep != NULL)
            return sep;
    }

    //Abrimos el fichero desde openInventor
    SoInput input;

//Soporte para DXF mediante la libreria DIME
#ifdef USE_DIME
    if (filename.endsWith(".dxf", Qt::CaseInsensitive))
    {
      
      float maxerr = 0.1f;
      int sub = -1;  
      dimeInput in;
      //
      // open file for reading
      //
      if (!in.setFile(strFilename)) 
      {
         QString S(strFilename);
         S += tr(": Error while opening file.");
         QMessageBox::critical( this, tr("Error"), S, QMessageBox::Abort); 
         return NULL;
      }
  
      //
      // try reading the file
      //
      dimeModel model;

      if (!model.read(&in)) 
      {
        QString S;
        S += strFilename + QString(": ") + tr("Error while reading DXF file on line ") + in.getFilePosition();
                              
//        S.asprintf(tr("%s: Error while reading DXF file on line %d\n"),
//                     strFilename, in.getFilePosition());
        QMessageBox::critical( this, tr("Error"), S, QMessageBox::Abort);
        return NULL;
      }

      //
      // open output file
      const char *outfile = "tmp.iv";
      FILE *out = fopen(outfile, "wb");
      if (!out) 
      {
         QString S;
         S += strFilename + tr(": Error writting temporaly file");
         //S.asprintf(tr("%s: Error writting temporaly file"), 
         //    strFilename );
         QMessageBox::critical( this, tr("Error"), S, QMessageBox::Ok, 
         QMessageBox::NoButton, QMessageBox::NoButton);  
         return NULL;
      }
  
      dxfConverter converter;
      converter.findHeaderVariables(model);
      converter.setMaxerr(maxerr);
      if (sub > 0) converter.setNumSub(sub);
    
      if (!converter.doConvert(model)) 
      {
          QString S;
          S += strFilename + tr(": Error while converting from DXF to OpenInventor");
          //S.asprintf(tr("%s: Error while converting from DXF to OpenInventor"), 
          //   strFilename );
          QMessageBox::critical( this, tr("Error"), S, QMessageBox::Ok, 
          QMessageBox::NoButton, QMessageBox::NoButton);  
    
           fclose(out);
          return NULL;
      }
  
      //Salvamos en formato VRML1 y 3D
      converter.writeVrml(out, true, false);
      fclose(out);

      //ahora tenemos el modelo ya convertido en outfile, lo cargamos 
      //tal cual con SoInput
      input.openFile (outfile);
      if (input.isValidFile ())
      {
         //Lo leemos sobre yyGeometry
         yyGeometry=SoDB::readAll (&input);
      }
      else
      {
          QString S;
          S += strFilename + tr(": Error loading temporaly file");
//          S.asprintf(tr("%s: Error loading temporaly file"), 
//          strFilename );
          QMessageBox::critical( this, tr("Error"), S, QMessageBox::Ok, 
          QMessageBox::NoButton, QMessageBox::NoButton);  
      } 

      //Borramos el fichero temporal
      QFile::remove(outfile);
   
      //Devolvemos la escena leida
      return yyGeometry; 

    }// if (filename.endsWith(".dxf", Qt::CaseInsensitive))

//Fin de soporte para DXF mediante la libreria DIME
#endif
    
    //Abrimos el fichero con un SoInput
    if (!input.openFile (strFilename))
    {
        return NULL;
    }

    //Miramos si es un fichero nativo de openInventor
    if (input.isValidFile ())
    {
       //Lo leemos sobre yyGeometry
       SoSeparator *scene=SoDB::readAll (&input);

       //Devolvemos la escena leida
       return scene;
    }

    //Miramos si es un fichero 3ds
    if (filename.endsWith(".3ds", Qt::CaseInsensitive))
    {
        if (coin_3ds_read_file(&input, yyGeometry, 2, float(25.f/180.f*M_PI), 
                           true, true, false, true, true, 10.f)  )
        {    
           //Devolvemos la escena leida
           return yyGeometry;
        }
    }

    //Probamos nuestro parser interno (OOGL, OBJ, XYZ, SMF, etc...)

    //Abrimos el fichero de entrada
#ifdef _MSC_VER
	fopen_s(&yyin, strFilename, "r");
#else
	yyin = fopen( strFilename, "r" );
#endif

	//Comprobamos que se pudo abrir correctamente
    if (!yyin) 
    {
      perror(strFilename);
      return NULL;
    }

    //Comprobamos si es un fichero .mesh
    rewind (yyin);
    yyGeometry = import_netgen_neutral(yyin);
    if (yyGeometry)
    {
       //Cerramos el fichero
       fclose(yyin);
      
       //Devolvemos la escena leida
       return yyGeometry;
    }

    //Comprobamos si es un fichero que podemos leer con cds_parser
    rewind (yyin);
    yyrestart(yyin);
    if (yyparse()==0)
    {
       //Cerramos el fichero
       fclose(yyin);
      
       //Devolvemos la escena leida
       return yyGeometry;
    }

#ifdef USE_VOLEON
    //Miramos si es un fichero de volumen en formato .vol
    if (filename.endsWith(".vol", Qt::CaseInsensitive))
    {
        //Leemos parte de la cabecera
        rewind (yyin);
        char head[25];
        if ( fread (head, 24, 1, yyin) <= 0)
	{
		fclose (yyin);
		return NULL;
	}
    
        //Comprobamos que es un fichero en formato .vol
        if (head[0]==0x0b && head[1]==0x7e && (head[23]==8 || head[23]==16))
        {

            yyGeometry= new SoSeparator();

            //Anadimos un VolumeData+TransferFunction+VolumeRender
            SoVolumeData *voldata = new SoVolumeData();
            voldata->fileName = strFilename;
            yyGeometry->addChild(voldata);

            SoTransferFunction * transfunc = new SoTransferFunction();
            transfunc->reMap(50, 255);
            yyGeometry->addChild(transfunc);

            SoVolumeRender *volumeRender = new SoVolumeRender ();
            yyGeometry->addChild(volumeRender);

            //Devolvemos la escena leida
            fclose(yyin);
            return yyGeometry;
        }// if (head[0]==0x0b && head[1]==0x7e && (head[23]==8 || head[23]==16))
    }

    //Miramos si es un fichero de volumen en formato .mha
    else if (filename.endsWith(".mha", Qt::CaseInsensitive))
	{
        fclose(yyin);
		return read_mha_volume(filename);
	}
#endif

    //Miramos si es una imagen, y la cargamos como textura
    if (filename.endsWith(".jpg", Qt::CaseInsensitive) || filename.endsWith(".png", Qt::CaseInsensitive))
    {

		yyGeometry= new SoSeparator();

		//Anadimos un SoTexture2
		SoTexture2 *texture2= new SoTexture2();
		texture2->filename = strFilename;
		yyGeometry->addChild(texture2);

		//Devolvemos la escena leida
		fclose(yyin);
		return yyGeometry;
    }

    //Lo siento, pero ha fallado todo lo anterior
    /*
    QString S;
    S.asprintf(tr("%s: No puedo cargar este formato de datos"), 
             strFilename );
    QMessageBox::critical( this, tr("Error"), S);
    */

    return NULL;
   
}//SoSeparator * MainWindow::cargarFichero3D(QString filename)

#ifdef USE_VOLEON
SoSeparator *MainWindow::read_mha_volume(const QString &filename)
{
	SbVec3s dimension(0,0,0);
	SbVec3f elementSpacing(1,1,1);
	char *data = NULL;
	QString dataFilename = "";

	//Abrimos el fichero
	addMessage("Abriendo fichero " + filename);
	QFile mhaFile(filename);
	if (mhaFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
		return NULL;

	yyGeometry= new SoSeparator();

	//Anadimos un VolumeData+TransferFunction+VolumeRender
	SoTransform *transform = new SoTransform();
	yyGeometry->addChild(transform);
	SoVolumeData *voldata = new SoVolumeData();
	yyGeometry->addChild(voldata);
	SoTransferFunction * transfunc = new SoTransferFunction();
	yyGeometry->addChild(transfunc);
	SoVolumeRender *volumeRender = new SoVolumeRender ();
	yyGeometry->addChild(volumeRender);

	//Leemos el fichero .mha y configuramos el volumen
	QString line;
	QTextStream mhaStream(&mhaFile);
	while (!mhaStream.atEnd())
	{
		//Leemos una linea del fichero
		line = mhaStream.readLine();
		//DEBUG: addMessage(line);

		//Ignoramos lineas en blanco
		if (QRegExp("\\s*").exactMatch(line))
			continue;

		//NDims = 3
		QRegExp rx1("\\s*NDims\\s*=\\s*(\\d+)", Qt::CaseInsensitive);
		if (rx1.indexIn(line)==0)
		{
			if (rx1.cap(1).toInt() != 3)
			{
				addMessage(tr("Error: only supports files with NDims = 3"));
				return NULL;
			}
			continue;
		}

		//DimSize = 699 536 115
		rx1.setPattern("\\s*DimSize\\s*=\\s*(\\d+)\\s*(\\d+)\\s*(\\d+)");
		if (rx1.indexIn(line)==0)
		{
			//Salvamos las dimensiones
			dimension.setValue(rx1.cap(1).toInt(), rx1.cap(2).toInt(),rx1.cap(3).toInt());
			continue;
		}

		//ElementSpacing = 1.0 1.0 2.0
		rx1.setPattern("\\s*ElementSpacing\\s*=\\s*([e\\d\\.]+)\\s*([e\\d\\.]+)\\s*([e\\d\\.]+)");
		if (rx1.indexIn(line)==0)
		{
			//Salvamos ElementSpacing
			elementSpacing.setValue(rx1.cap(1).toFloat(), rx1.cap(2).toFloat(),rx1.cap(3).toFloat());
			continue;
		}

		//ElementDataFile = data.raw
		rx1.setPattern("\\s*ElementDataFile\\s*=\\s*(\\S+)");
		if (rx1.indexIn(line)==0)
		{
			dataFilename = rx1.cap(1);
			addMessage(tr("Reading data from")+" " + dataFilename);

			//Abrimos el fichero de datos
			QString currentDir(QDir::currentPath());
			QDir::setCurrent(QFileInfo(mhaFile).absoluteDir().path() );
			QFile dataFile(dataFilename);
			if (dataFile.open(QIODevice::ReadOnly) == false)
			{
				addMessage(dataFilename + tr(": Error while opening file."));
				return NULL;
			}
			QDir::setCurrent(currentDir);

			//Leemos el fichero completamente a memoria
			long datasize = dimension[0] * dimension[1] * dimension[2];
			data = new char[datasize];

			if (QDataStream(&dataFile).readRawData(data, datasize) != datasize)
			{
				addMessage(dataFilename + tr(": Error while reading file."));
				delete data;
				return NULL;
			}			

			continue;
		}

		//El resto de lineas las imprimimos como un warning
		addMessage(tr("Warning: ") + line);

	}


	//Comprobamos que hemos leido los datos necesarios
	if (data == NULL || dimension[0] <= 0 || dimension[1] <= 0 || dimension[2] <= 0)
	{
		addMessage(filename + tr(": Invalid .mha file"));
		return NULL;
	}
	else
	{
		QString S;
		S.asprintf("dimension: [%d %d %d] ;; spacing: [%f %f %f]",
			dimension[0], dimension[1], dimension[2], elementSpacing[0], elementSpacing[1], elementSpacing[2]);
		addMessage(S);
	}

	//Ahora configuramos los nodos
	yyGeometry->setName(qPrintable(filename));
	voldata->setName(qPrintable(dataFilename));

    //const char *strFilename = SbName(dataFilename.toLatin1()).getString();
	//voldata->fileName = strFilename;

	transform->translation.setValue(0.5*dimension[0],0.5*dimension[1],0.5*dimension[2]);
	transform->scaleFactor.setValue(dimension[0],dimension[1],dimension[2]);

	voldata->setVolumeData(dimension, data);
	transfunc->reMap(0x40, 0x50);

	//Devolvemos la escena leida
	return yyGeometry;

}
#endif

 void MainWindow::setRecentFile(const QString &fileName)
 {
     //Lee la lista de ficheros recientes
     QStringList files = settings->value("recentFileList").toStringList();

     //Coloca fileName en la primera posicion, teniendo cuidado de no repetirlo
     files.removeAll(fileName);
     files.prepend(fileName);

     //Recorta la lista por el final si es necesario
     while (files.size() > 5)
         files.removeLast();

     //Salva la lista
     settings->setValue("recentFileList", files);

     //Actualiza el menu scene
     updateRecentFileActions();
 }

void MainWindow::updateRecentFileActions()
{
    //Lee la lista de ficheros recientes
    QStringList files = settings->value("recentFileList").toStringList();
    int numRecentFiles = qMin(files.size(), 5);

	//Configura las acciones del menu menuRecent
	for (int i = 0; i < numRecentFiles; ++i) 
	{
		//Crea una accion con el nombre del fichero
		QString filename = QFileInfo(files[i]).fileName();
		recentFileActs[i]->setText(QString("&%1 ").arg(i+1)+filename);

		//Coloca el path completo del fichero en el campo data
		recentFileActs[i]->setData(files[i]);

		//Hace visible la accion si el fichero sigue existiendo
		QFileInfo fileInfo(files[i]);
		recentFileActs[i]->setVisible(fileInfo.isFile() && fileInfo.isReadable());
	}

	//Oculta el resto de acciones
	for (int j = numRecentFiles; j < 5; ++j)
		recentFileActs[j]->setVisible(false);

}//void MainWindow::updateRecentFileActions()


///Abre una escena del menu de escenas recientes
void MainWindow::actionLoad_RecentFile_triggered(bool)
{
     QAction *action = qobject_cast<QAction *>(sender());
     if (action)
         load_Scene(action->data().toString());
}

//Incrusta todas las texturas de la escena
void MainWindow::on_actionEmbed_all_textures_triggered()
{
	if (QMessageBox::question( this, tr("Warning"), 
		tr("Embed all textures of the scene will increase the size of the scene file "
		   "because textures are stored uncompressed. This method will affect all "
		   "SoTexture2 and SoBumpMap nodes in the scene.\n"
		   "Are you sure you want to embed all textures?"),
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
      embedTexture(fp->getTail());
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
      embedTexture(fp->getTail());
    }

	//Refrescamos el editor de campos
	updateFieldEditor(mapQTCOIN[Ui.sceneGraph->currentItem()]);

}//int MainWindow::on_actionEmbed_all_textures_triggered ()

///Callback to change background color in the viewers
void MainWindow::on_actionChange_BG_color_triggered()
{
   //Lo convertimos en valores RGB y en un QColor
   const float*rgb = bgColor_viewer.getValue();
   QColor c( int(255*rgb[0]), int(255*rgb[1]), int(255*rgb[2]) );

   //Solicitamos un color mediante QColorDialog
   c=QColorDialog::getColor(c,this);
   if (c.isValid() )
   {           
       //Modificamos el field
       bgColor_viewer.setValue(c.red()/255.0, c.green()/255.0, c.blue()/255.0);
   }
}//void MainWindow::on_actionChange_BG_color_triggered()

///Callback to simplify a shape with QSlim
void MainWindow::on_actionQSlim_triggered()
{
	//Leemos el path del nodo actual y lo pasamos a qslim_options
    SoPath *path=getPathFromItem(Ui.sceneGraph->currentItem());
	qslim_options qslimDlg(path);
	qslimDlg.exec();

    SoNode *newNode = qslimDlg.output;
	if (newNode == NULL)
		return;

    //Buscamos el item actualmente seleccionado en el sceneGraph y 
    //el primer item que pueda actuar como contenedor para colgar newNode
    QTreeWidgetItem *item_current = Ui.sceneGraph->currentItem();
    QTreeWidgetItem *item_padre = item_current;
    while (!mapQTCOIN[item_padre]->getTypeId().isDerivedFrom(SoGroup::getClassTypeId()) )
    {
        item_padre=item_padre->parent();
    }  

    //Buscamos el nodo de coin correspondiente al item_padre
    SoGroup *nodo_padre=(SoGroup*)mapQTCOIN[item_padre];

/* TODO Insertar tras item_current, no al final de item_padre
    //Comprobamos si vamos a insertar dentro de item_current o detras de item_current
    if (item_current == item_padre)
    {
        //Insertamos el nodo y el item dentro del item actual
        item_padre->addChild(newItem);
        nodo_padre->addChild(newNode);
    }
    else
    {
        //Buscamos la posicion del nodo_current dentro de su padre
        int pos = nodo_padre->findChild(mapQTCOIN[item_current]);

        //Insertamos detras del item_current
        item_padre->insertChild(pos+1, newItem);
        nodo_padre->insertChild(newNode, pos+1);
    }
*/
    //Insertamos el contenido de la escena
    newSceneGraph(newNode, item_padre, nodo_padre);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}//void MainWindow::on_actionQSlim_triggered()


///Callback to tetrahedralize with tetgen
void MainWindow::on_actionTetgen_triggered()
{
	//Leemos el path del nodo actual y lo pasamos a tetgen_options
    SoPath *path=getPathFromItem(Ui.sceneGraph->currentItem());
	tetgen_options tetgenDlg(path);
	tetgenDlg.exec();

    SoNode *newNode = tetgenDlg.output;
	if (newNode == NULL)
		return;

    //Buscamos el item actualmente seleccionado en el sceneGraph y 
    //el primer item que pueda actuar como contenedor para colgar newNode
    QTreeWidgetItem *item_current = Ui.sceneGraph->currentItem();
    QTreeWidgetItem *item_padre = item_current;
    while (!mapQTCOIN[item_padre]->getTypeId().isDerivedFrom(SoGroup::getClassTypeId()) )
    {
        item_padre=item_padre->parent();
    }  

    //Buscamos el nodo de coin correspondiente al item_padre
    SoGroup *nodo_padre=(SoGroup*)mapQTCOIN[item_padre];

/* TODO Insertar tras item_current, no al final de item_padre
    //Comprobamos si vamos a insertar dentro de item_current o detras de item_current
    if (item_current == item_padre)
    {
        //Insertamos el nodo y el item dentro del item actual
        item_padre->addChild(newItem);
        nodo_padre->addChild(newNode);
    }
    else
    {
        //Buscamos la posicion del nodo_current dentro de su padre
        int pos = nodo_padre->findChild(mapQTCOIN[item_current]);

        //Insertamos detras del item_current
        item_padre->insertChild(pos+1, newItem);
        nodo_padre->insertChild(newNode, pos+1);
    }
*/
    //Insertamos el contenido de la escena
    newSceneGraph(newNode, item_padre, nodo_padre);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}//void MainWindow::on_actionTetgen_triggered()


///Anade un mensaje a la consola de mensajes
void MainWindow::addMessage(const QString &msg)
{
	Ui.messages->append(msg);
}

///Activa/desactiva la vigilancia de cambios
void MainWindow::on_actionWatch_node_toggled(bool on)
{
	if (refreshGUI_Sensor == NULL)
		return;


    if (on)
        refreshGUI_Sensor->schedule();
    else
        refreshGUI_Sensor->unschedule();
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();

	//Comprobamos que contiene enlaces a ficheros
	if (mimeData->hasUrls()) 
	{
		//Tratamos de importar los ficheros uno por uno
		QList<QUrl> urlList = mimeData->urls();
		for (int i = 0; i < urlList.size() && i < 32; ++i) 
		{
			//Extraemos el nombre del fichero 
			QString filename = urlList.at(i).toLocalFile();
			if (filename.isEmpty())
				continue;

			addMessage(tr("Import:")+filename);
			this->import_File(filename);
		}
	}

	event->acceptProposedAction();
}


void MainWindow::on_fieldTable_customContextMenuRequested(QPoint pos)
{
	//Miramos si se ha pulsado sobre algun item valido
    QTableWidgetItem *item = Ui.fieldTable->itemAt(pos);
	if (!item)
		return;

   //Evitamos vigilar la escena para poder editar los valores de la misma.
	Ui.actionWatch_node->setChecked(false);

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

		//Comprobamos que se ha seleccionado una opcion valida.
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

		//Comprobamos que se ha seleccionado una opcion valida.
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
            soSFString->setValue(filename.toLatin1());
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

             //Le damos el valor adecuado en la posicion pos
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

		//Convertimos en BumpMap
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoBumpMap");
		menu.addAction(Ui.Convert_Manip);
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

		//Convertimos en Texture2
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoTexture2");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoDirectionalLight::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoDirectionalLightManip");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoSpotLight::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoSpotLightManip");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoPointLight::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoPointLightManip");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoDirectionalLightManip::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoDirectionalLight");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoSpotLightManip::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoSpotLight");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoPointLightManip::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoPointLight");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoClipPlaneManip::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoClipPlane");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoTranslation::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoTransform");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoRotation::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoTrackballManip");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoTrackballManip::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoRotation");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo.isDerivedFrom(SoTransformManip::getClassTypeId()) ) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoTransform");
		menu.addAction(Ui.Convert_Manip);
	}
	else if (tipo == SoClipPlane::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoClipPlaneManip");
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
		menu.addAction(Ui.SoCoordinate3_to_qhull);
		menu.addAction(Ui.Center_on_Origin);
		menu.addAction(Ui.Export_to_XYZ);
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
	else if (tipo == SoMatrixTransform::getClassTypeId()) 
	{
		Ui.Convert_Manip->setData((qulonglong)item);
		Ui.Convert_Manip->setText(tr("Convert in")+" SoTransform");
		menu.addAction(Ui.Convert_Manip);
	}
	
	//Mostramos el menu popup
	menu.exec(Ui.sceneGraph->mapToGlobal(pos));

}//void MainWindow::on_sceneGraph_customContextMenuRequested(QPoint pos)


///Put all children of a group node on the same level that its parent
void MainWindow::on_actionPromote_Children_triggered()
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
    
}// void MainWindow::on_actionPromote_Children_triggered()

///Embed external texture image files into the node
void MainWindow::on_actionEmbedTexture_triggered()
{
    SoNode *node;

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

    embedTexture(node);
}//void MainWindow::on_actionEmbedTexture_triggered()


///Embed external texture image files into the current node
void MainWindow::embedTexture(SoNode *node)
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
      S.asprintf("No puedo incrustar en %s", node->getTypeId().getName().getString() );
	  QMessageBox::warning( this, tr("Error"), S);
	  return;
  }
  
  //Mostramos el nodo modificado en la tabla de edicion de campos
  updateFieldEditor(node);

  //Indicamos que la escena ha sido modificada
  escena_modificada = true;

}//void MainWindow::embedTexture()


///Convert a light or clipPlane into its correspondent manip 
void MainWindow::on_Convert_Manip_triggered()
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
    else if (node->getTypeId() == SoTranslation::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoTransform();
    }
    else if (node->getTypeId() == SoRotation::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoTrackballManip();
    }
    else if (node->getTypeId() == SoBumpMap::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoTexture2();
    }
    else if (node->getTypeId() == SoTexture2::getClassTypeId()) 
    {
        newNode=(SoNode*)new SoBumpMap();
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
			S.asprintf("Wrong action. Blame developper!!");
			QMessageBox::warning( this, tr("Warning"), S);

			return;
		}

	}  
	//Los nodos SoMatrixTransform se convierten en SoTransform
	else if (node->getTypeId().isDerivedFrom(SoMatrixTransform::getClassTypeId())) 
    {
        newNode=(SoNode*)new SoTransform();
		//Como SoMatrixTransform y SoTransform no tienen campos comunes lo copiamos aqui
		((SoTransform*)newNode)->setMatrix(((SoMatrixTransform*)node)->matrix.getValue());

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
		S.asprintf("Can't convert %s into a manip", node->getTypeId().getName().getString());
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

}//void MainWindow::on_actionConvert_Manip_triggered()



void MainWindow::on_IndexedFaceSet_to_IndexedLineSet_triggered()
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

} // void MainWindow::on_IndexedFaceSet_to_IndexedLineSet_triggered()

void MainWindow::on_IndexedLineSet_to_IndexedFaceSet_triggered()
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

} // void MainWindow::on_IndexedLineSet_to_IndexedFaceSet_triggered()

///Exporta un SoIndexedFaceSet o SoVRMLIndexedFaceSet a un fichero SMF
void MainWindow::on_Export_to_SMF_triggered()
{
	//Nombre del fichero donde escribir
    QString filename = QFileDialog::getSaveFileName(this, tr("Export File"), "",
		tr("OBJ/SMF ")+tr("Files")+" (*.obj *.smf);;"+tr("All Files")+" (*)");

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
#ifdef _MSC_VER
    FILE *file;
    fopen_s(&file, SbName(filename.toLatin1()).getString(), "w");
#else
    FILE *file = fopen(SbName(filename.toLatin1()).getString(), "w");
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

}// void MainWindow::on_Export_to_SMF_triggered()

///Exporta un SoIndexedFaceSet o SoVRMLIndexedFaceSet a un fichero OFF
void MainWindow::on_Export_to_OFF_triggered()
{
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 

    //Construimos un path  segun la informacion del arbol de QT
	SoNode *nodo = mapQTCOIN[item];

	//Nombre del fichero donde escribir
    QString filename = QFileDialog::getSaveFileName(this, tr("Export File"), "",
		tr("OFF ")+tr("Files")+" (*.off);;"+tr("All Files")+" (*)");

    //Miramos si se pulso el boton cancelar
    if (filename=="")
        return;

    //Abrimos el fichero de salida
#ifdef _MSC_VER
    FILE *file;
    fopen_s(&file, SbName(filename.toLatin1()).getString(), "w");
#else
    FILE *file = fopen(SbName(filename.toLatin1()).getString(), "w");
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

}// void MainWindow::on_Export_to_OFF_triggered()


///Exporta un SoIndexedFaceSet o SoVRMLIndexedFaceSet a un fichero STL
void MainWindow::on_Export_to_STL_triggered()
{
    QTreeWidgetItem * item=Ui.sceneGraph->currentItem(); 

    //Construimos un path  segun la informacion del arbol de QT
	SoNode *nodo = mapQTCOIN[item];

	//Nombre del fichero donde escribir
    QString filename = QFileDialog::getSaveFileName(this, tr("Export File"), "",
		tr("STL ")+tr("Files")+" (*.stl);;"+tr("All Files")+" (*)");

    //Miramos si se pulso el boton cancelar
    if (filename=="")
        return;

    //Abrimos el fichero de salida
#ifdef _MSC_VER
    FILE *file;
    fopen_s(&file, SbName(filename.toLatin1()).getString(), "w");
#else
    FILE *file = fopen(SbName(filename.toLatin1()).getString(), "w");
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

}// void MainWindow::on_Export_to_STL_triggered()

void MainWindow::on_Export_to_XYZ_triggered()
{
	//Nombre del fichero donde escribir
    QString filename = QFileDialog::getSaveFileName(this, tr("Export File"), "",
		tr("XYZ ")+tr("Files")+" (*.xyz);;"+tr("All Files")+" (*)");

    //Miramos si se pulso el boton cancelar
    if (filename=="")
        return;

    //Abrimos el fichero de salida
#ifdef _MSC_VER
    FILE *file;
    fopen_s(&file, SbName(filename.toLatin1()).getString(), "w");
#else
    FILE *file = fopen(SbName(filename.toLatin1()).getString(), "w");
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

}// void MainWindow::on_Export_to_XYZ_triggered()

void MainWindow::on_Center_on_Origin_triggered()
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

}// void MainWindow::on_Center_on_Origin_triggered()

void MainWindow::on_actionIvfix_triggered()
{
	ivfix_options ivfixDialog(root);
	ivfixDialog.exec();
	SoSeparator *ivfix_result = (SoSeparator *)ivfixDialog.output;

	//Miramos si hay resultados en ivfix_result
	if (ivfix_result != NULL)
	{
		//Destruimos la escena actual y creamos una nueva
		on_actionNew_Scene_triggered();

		//Colgamos el nodo del grafo de escena
		QTreeWidgetItem *qroot=Ui.sceneGraph->currentItem();
		for(int i=0; i< ivfix_result->getNumChildren(); i++)
		{
			newSceneGraph(ivfix_result->getChild(i), qroot, root);
		}

		//Expandimos todos los items
		Ui.sceneGraph->expandAll();

		//Actualizamos la tabla  de campos
		updateFieldEditor (root);

		//Indicamos que la escena ha sido modificada
		escena_modificada = true;
	}

}//void MainWindow::on_actionIvfix_triggered()


///Triangula las facetas de mas de tres lados de un SoIndexedFaceSet o SoVRMLIndexedFaceSet
void MainWindow::on_SoIndexedFaceSet_triangulate_triggered()
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
}//void MainWindow::on_SoIndexedFaceSet_triangulate_triggered()


///Cambia la orientacion de todas las facetas
void MainWindow::on_SoIndexedFaceSet_change_orientation_triggered()
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

}// void MainWindow::on_SoIndexedFaceSet_change_orientation_triggered()


void MainWindow::on_SoIndexedTriangleStripSet_to_SoIndexedFaceSet_triggered()
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

}//void MainWindow::on_SoIndexedTriangleStripSet_to_SoIndexedFaceSet_triggered()

void MainWindow::on_SoCoordinate3_to_qhull_triggered()
{
    //Identificamos el item actual
    QTreeWidgetItem *item_current = Ui.sceneGraph->currentItem();
    SoCoordinate3 *nodo = (SoCoordinate3 *)mapQTCOIN[item_current];

    //Creamos un coordinate3 y un indexedFaceSet para el cierre
    SoVertexProperty  *coord = new SoVertexProperty();
    SoIndexedFaceSet *ifs = new SoIndexedFaceSet();
    ifs->vertexProperty = coord;

    //Creamos el cierre convexo
    int result = convex_hull (nodo->point, coord->vertex, ifs->coordIndex );

    if (result < 0)
    {
		QMessageBox::warning( this, tr("Warning"), tr("Error while computing convex hull"));
		return;
    }

    //Cambiamos el nombre del nuevo nodo
    SoBase_name->setValue("convex_hull");
    ifs->setName(SoBase_name->getValue());

    //Buscamos el item actualmente seleccionado en el sceneGraph y 
    //el primer item que pueda actuar como contenedor para colgar newNode
	QTreeWidgetItem *item_padre = item_current;
    while (!mapQTCOIN[item_padre]->getTypeId().isDerivedFrom(SoGroup::getClassTypeId()) )
    {
        item_padre=item_padre->parent();
    }  

    //Buscamos el nodo de coin correspondiente al item_padre
    SoGroup *nodo_padre=(SoGroup*)mapQTCOIN[item_padre];
 
	//Insertamos el contenido de la escena
    newSceneGraph(ifs, item_padre, nodo_padre);

    //Actualizamos la tabla  de campos
    updateFieldEditor (ifs);

    //Indicamos que la escena ha sido modificada
    escena_modificada = true;

}// void MainWindow::on_SoCoordinate3_to_qhull_triggered()


/*! Apply SoReorganizeAction to the scene 
	Example taken from SoReorganizeAction help page
*/
void MainWindow::on_actionSoReorganizeAction_triggered()
{
	//Reorganize the whole scene
    SoReorganizeAction reorg;
    reorg.apply(root);

	//Remove unused nodes
    //fprintf(stderr,"stripping old nodes from scene graph\n");
	strip_node(SoCoordinate3::getClassTypeId(), root);
	strip_node(SoCoordinate4::getClassTypeId(), root);
	strip_node(SoNormal::getClassTypeId(), root);
	strip_node(SoTextureCoordinate2::getClassTypeId(), root);

	//Make a copy of simplified scene
	SoSeparator *tmpNode = (SoSeparator *)root->copy(true);
	tmpNode->ref();

	//Destruimos la escena actual y creamos una nueva
	on_actionNew_Scene_triggered();

	//Colgamos el nodo del grafo de escena
	QTreeWidgetItem *qroot=Ui.sceneGraph->currentItem();
	for(int i=0; i< tmpNode->getNumChildren(); i++)
	{
		newSceneGraph(tmpNode->getChild(i), qroot, root);
	}
	tmpNode->unref();

	//Expandimos todos los items
	Ui.sceneGraph->expandAll();

	//Actualizamos la tabla  de campos
	updateFieldEditor (root);

	//Indicamos que la escena ha sido modificada
	escena_modificada = true;

}//void MainWindow::on_actionSoReorganizeAction_triggered()

