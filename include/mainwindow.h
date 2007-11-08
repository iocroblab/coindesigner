
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

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "ui_mainwindow.h"
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/fields/SoSFName.h>
#include <Inventor/Qt/SoQt.h>
#ifdef USE_VOLEON
  #include <VolumeViz/nodes/SoTransferFunction.h>
  #include <VolumeViz/nodes/SoVolumeData.h>
  #include <VolumeViz/nodes/SoVolumeRender.h>
  #include <VolumeViz/nodes/SoVolumeRendering.h>
#endif

class MainWindow : public QMainWindow
{
	Q_OBJECT
	Ui::MainWindow Ui;

private:

	///Nodo que actualmente estamos editando en el fieldTable
	SoNode *edit_node;

	///Nodo que actua de buffer para operaciones cut/copy/paste
	SoNode *node_buffer;

	///Nodo que actua de buffer para operaciones link
	SoNode *node_buffer_link;

	///Un campo falso, que usaremos para editar el SoBase::name de cada nodo
	SoSFName *SoBase_name;

	///Mapa que relaciona cada fila de la tabla fieldTable con el SoField correspondiente
	std::map<int, SoField*>map_fieldTable;

	///Flag que indica si la escena ha sido modificada
	bool escena_modificada;

	///Nombre de la escena
	QString nombreEscena;

	//Mapa que relaciona cada QTreeWidgetItem con un SoNode
	std::map<QTreeWidgetItem*,SoNode*>mapQTCOIN;

	///Acciones del menu para ficheros recientes
	QAction *recentFileActs[5];

public:

	///The one and only.... root node!!!
	SoSeparator *root;

	///Constructor de la clase MainWindow
	MainWindow (QWidget *p=0, Qt::WindowFlags f=0);

    ///Carga una escena 
	bool load_Scene (QString filename);

	///Salva la escena actual
	void save_Scene (QString filename);

	///Añade un archivo a la escena actual
	bool import_File(QString filename);

public slots:

	//Slots para menu Scene
	void on_actionNew_Scene_activated();
	void on_actionLoad_Scene_activated() {load_Scene("");};
	void on_actionSave_Scene_activated() {save_Scene("");};
	void on_actionLoad_RecentFile();
	void on_actionImport_File_activated() {import_File("");};
	void on_actionExport_VRML2_activated();
	void on_actionView_Source_activated();
	void on_actionQuit_activated();

	//Slots para menu Edit
	void on_actionCut_activated();
	void on_actionCopy_activated();
	void on_actionPaste_activated();
	void on_actionDelete_activated();
	void on_actionLink_activated();
	void on_actionMove_Up_activated();
	void on_actionMove_Down_activated();
	void on_actionPreferences_activated();

	//Slots para menu View
	void on_actionExaminerViewer_activated();
	void on_actionFlyViewer_activated();
	void on_actionPlaneViewer_activated();
	void on_actionRenderArea_activated();
	///Configura la variable de entorno COIN_SHOW_FPS_COUNTER
	void on_actionShow_FPS_toggled(bool on);

	//Slots para menu Tools
	void on_actionNode_Palette_toggled(bool on);
	void on_actionField_Editor_toggled(bool on);
	void on_actionExaminerViewer_Editor_activated();
	void on_actionPlaneViewer_Editor_activated();

	//Slots para el menu Help
	void on_actionCoin3D_Reference_activated();
	void actionShow_Node_Reference_activated();
	void on_actionTutorial_1_activated();
	void on_actionTutorial_2_activated();
	void on_actionMirror_demo_activated();
	void on_actionAbout_activated();

	//Slots para el area de trabajo
	void on_paletteComboBox_activated(int idx);
	void on_nodePalette_itemDoubleClicked(QTreeWidgetItem *item, int);
	void on_sceneGraph_currentItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *prev=0);
	void on_sceneGraph_itemClicked(QTreeWidgetItem *item, int column);
	void on_fieldTable_cellChanged(int row, int column);

	void closeEvent(QCloseEvent *event);

private:

	///Autogenera la paleta de componentes mediante exploracion de coin3D
	void generarListaComponentes(SoType t, bool plano, QTreeWidgetItem *padre=0);

	///Crea y configura un QTreeWidgetItem para representar un SoNode
	QTreeWidgetItem *newNodeItem(SoNode *node);

	///Crea subgrafo de escena QT a partir de un subgrafo de Coin3D
	void newSceneGraph(QTreeWidgetItem *padre, SoNode *node);

	///Busca un icono adecuado a esta clase y lo asigna al item
	bool setNodeIcon(QTreeWidgetItem * item);

	///Abre un url en un visor de HTML externo
	void open_html_viewer(const QString &url);

	///Construye un SoPath a partir de un item del sceneGraph
	SoPath *getPathFromItem(QTreeWidgetItem *item);

	///Carga una escena en cualquier formato conocido
	SoSeparator *cargarFichero3D(QString file);

    ///Aplica a escena los cambios introducidos en la celda del fieldTable
	void on_fieldTable_userChanged(int row, int column);

	///Funciones para soporte de recent files
	void setRecentFile(const QString &fileName);
	void updateRecentFileActions();

}; //class MainWindow

#endif

