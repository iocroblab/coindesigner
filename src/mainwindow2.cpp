/*
    This file is part of coindesigner. (http://coindesigner.sf.net)

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
#include "cds_globals.h"
#include "3dsLoader.h"
#include <src_editor.h>
#include <mfield_editor.h>
#include <cppexport_options.h>

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

extern QSettings *settings;

//OpenInventor includes
#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoNodes.h>

#ifdef USE_VOLEON
  #include <VolumeViz/nodes/SoTransferFunction.h>
  #include <VolumeViz/nodes/SoVolumeData.h>
  #include <VolumeViz/nodes/SoVolumeRender.h>
  #include <VolumeViz/nodes/SoVolumeRendering.h>
#endif



#ifdef USE_VOLEON
SoSeparator *MainWindow::read_mha_volume(const QString &filename)
{
	SbVec3s dimension(0,0,0);
	char *data = NULL;
	QString dataFilename = "";

	//Abrimos el fichero
	addMessage("Abriendo fichero " + filename);
	QFile mhaFile(filename);
	if (mhaFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
		return NULL;

	yyGeometry= new SoSeparator();

	//Añadimos un VolumeData+TransferFunction+VolumeRender
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

	//Ahora configuramos los nodos
	yyGeometry->setName(qPrintable(filename));
	voldata->setName(qPrintable(dataFilename));
	voldata->setVolumeData(dimension, data);
	transfunc->reMap(30, 80);

	//Devolvemos la escena leida
	return yyGeometry;

}
#endif