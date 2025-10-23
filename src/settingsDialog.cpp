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

#include <settingsDialog.h>
#include <cds_globals.h>
#include <QTableWidget>
#include <QColorDialog>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVariant>
#include <QDir>
#include <QDebug>
#include <QProcess>

settingsDialog::settingsDialog(QWidget *p, Qt::WindowFlags f) : QDialog(p,f)
{
	QString S;
	Ui.setupUi(this);
	updateTable();
}

void settingsDialog::updateTable()
{
    //List of settings that can not be editted here
    QStringList hiddenSettings;
    hiddenSettings  << "geometry" << "recentFileList" <<
                    "Enable_Antialias" << "HQ_transparency" << "Feedback_Visibility";

	//Rellenamos el valor de la tabla
	int row=0;
	foreach(QString key, settings->allKeys())
	{
        //Avoid to show the hidden settings
        if (hiddenSettings.contains(key))
            continue;

		QVariant value = settings->value(key);

        //Damos a la tabla el numero de filas adecuado
        Ui.table->setRowCount(row+1);
        Ui.table->setVerticalHeaderItem(row, new QTableWidgetItem(key));
        Ui.table->setItem(row,0, new QTableWidgetItem(value.toString()));
        row++;

	}//	foreach(QString key, settings->allKeys())

    //Aseguramos que las columnas tienen ancho suficiente
    Ui.table->resizeColumnsToContents();
}

///Callback que se ejecuta cada vez que modificamos la tabla
void settingsDialog::on_table_cellChanged(int row, int column)
{
    //Evitamos actualizar si el cambio no lo ha hecho el usuario
    if (Ui.table->item(row,column) != Ui.table->currentItem())
		return;
}


void settingsDialog::on_buttonBox_clicked(QAbstractButton * button)
{
	if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
	{
		setToDefault(true);
		updateTable();
	}//if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
}

void settingsDialog::accept()
{

	//Recorremos toda la tabla y aplicamos cambios
    for (int row=0; row< Ui.table->rowCount(); row++)
	{
		QString key = Ui.table->verticalHeaderItem(row)->text();
		QString value = Ui.table->item(row,0)->text();
		settings->setValue(key, value);
	}

	QDialog::accept();
}


void settingsDialog::on_table_customContextMenuRequested(QPoint pos)
{
	//Miramos si se ha pulsado sobre algun item valido
    QTableWidgetItem *item = Ui.table->itemAt(pos);
	if (!item)
		return;

    int row = Ui.table->row(item);

	//Leemos la clave
	QString key = Ui.table->verticalHeaderItem(row)->text();
	QString value = item->text();

	//Preparamos un menu flotante
	QMenu popm(this);
	QAction actApp(tr("Select application"), this);
	QAction actDir(tr("Select directory"), this);
	QAction actRef("http://coindesigner.sf.net/reference/", this);
	QAction actTut("http://coindesigner.sf.net/tutorials/", this);
	QAction actRefDir(QString(cds_dir)+"/reference", this);
	QAction actTutDir(QString(cds_dir)+"/tutorials", this);
	QAction actFalse("false", this);
	QAction actTrue("true", this);
	#ifdef _WIN32
	QAction actNavig("explorer.exe", this);
	#else
	QAction actNavig("firefox", this);
	#endif

	//A�adimos acciones segun la variable
    if (key == "reference_dir")
	{
		popm.addAction(&actRef);
		//Miramos si existe el directorio de referencia
		QDir refDir(actRefDir.text());
		if (refDir.exists () && refDir.isReadable ())
			popm.addAction(&actRefDir);
	}

    else if (key == "tutorial_dir")
	{
		popm.addAction(&actTut);

		//Miramos si existe el directorio de tutoriales
		QDir tutDir(actTutDir.text());
		if (tutDir.exists () && tutDir.isReadable ())
			popm.addAction(&actTutDir);
	}

    else if (key == "helpViewer_app")
	{
		popm.addAction(&actNavig);
	}

	//Miramos si es el path de un ejecutable
	if (key.endsWith("_app"))
	{
		popm.addAction(&actApp);
	}

	else if (key.endsWith("_dir"))
	{
		popm.addAction(&actDir);
	}

	//Miramos si la opcion es de tipo Bool
	if (settings->value(key).type() == QVariant::Bool)
	{
		popm.addAction(&actFalse);
		popm.addAction(&actTrue);
	}

    //Si hay opciones en el menu, lo mostramos
	if (!popm.isEmpty())
	{
		//Mostramos el menu flotante y recogemos la opcion escogida
		QAction *act = popm.exec(QCursor::pos());

		//Si se ha escogido la opcion actApp
		if (act == &actApp)
		{
			QString filename = QFileDialog::getOpenFileName(this, 
				tr("Choose Filename"), value, 
				#ifdef _WIN32
				tr("Executable files")+" (*.exe *.bat);;" +
				#endif
				tr("All Files")+" (*)");

			//Asignamos el valor escogido
			if (filename != "")
				item->setText(filename);
		}

		//Si se ha escogido la opcion actDir
		else if (act == &actDir)
		{
			QString dir = QFileDialog::getExistingDirectory(this, 
				tr("Choose Directory"), cds_dir);

			//Asignamos el valor escogido
			if (!dir.isEmpty())
			{
				item->setText(dir);
			}
		}

		//Si se ha escogido alguna opcion con un URL
		else if (act == &actRef || act == &actTut || act == &actNavig || act == &actRefDir)
		{
			item->setText(act->text());
		}

		else if (act == &actFalse)
		{
			item->setText("false");
		}
		else if (act == &actTrue)
		{
			item->setText("true");
		}

	} // if (!popm.isEmpty())
	
    //Aseguramos que las columnas tienen ancho suficiente
    Ui.table->resizeColumnsToContents();

}//void settingsDialog::on_table_customContextMenuRequested(QPoint pos)


void settingsDialog::setToDefault(bool reset)
{
   QString key, value;

   //Idioma del interface grafico
   key = "lang";
   value = settings->value(key).toString();
   if (reset || value.isEmpty())
   {
      value = QLocale::system().name();
      qDebug() << "Setting: " << key << "=" << value;
      settings->setValue(key, value);
   }

   //Navegador por defecto
   key = "helpViewer_app";
   value = settings->value(key).toString();
   if (reset || value.isEmpty())
   {
      #ifdef _WIN32
      value = "explorer.exe";
      #else
      value = "firefox";
      #endif
      qDebug() << "Setting: " << key << "=" << value;
      settings->setValue(key, value);
   }

   //Directorio de la ayuda
   key = "reference_dir";
   value = settings->value(key).toString();
   if (reset || value.isEmpty())
   {
	   QString reference_dir=QString(cds_dir)+"/reference";

	  //Buscamos el directorio de ayuda en el directorio de coindesigner
#ifdef _WIN32
	  reference_dir.replace( QChar('/'), "\\" );
#endif

	  if (QFile::exists(reference_dir) && QFileInfo(reference_dir).isDir() )
	  {
		  value = QString("file://")+reference_dir;
	  }
	  else
	  {
		  value = "http://coindesigner.sf.net/reference/";
	  }

      qDebug() << "Setting: " << key << "=" << value;
      settings->setValue(key, value);
   }

   //Directorio con los tutoriales
   key = "tutorial_dir";
   value = settings->value(key).toString();
   if (reset || value.isEmpty())
   {
      value = "http://coindesigner.sf.net/tutorials/";
      qDebug() << "Setting: " << key << "=" << value;
      settings->setValue(key, value);
   }

   //Path a QSlim. Si no existe lo buscamos
   key = "qslim_app";
   value = settings->value(key).toString();
   if (reset || value.isEmpty())
   {
	   QString qslim_app;

	   //Miramos si existe una variable de entorno adecuada
	   qslim_app = getenv("QSLIM");
   	   if (!qslim_app.isEmpty() && QFile::exists(qslim_app) && !QFileInfo(qslim_app).isDir() 
		   && QFileInfo(qslim_app).isExecutable())
	   {
			value = qslim_app;
	   }
	   else
	   {
		   //Buscamos qslim en el directorio de coindesigner
#ifdef _WIN32
		   qslim_app.asprintf("%s/qslim.exe", cds_dir);
		   qslim_app.replace( QChar('/'), "\\" );
#else
		   qslim_app.asprintf("%s/qslim", cds_dir);
#endif

		   if (QFile::exists(qslim_app) && !QFileInfo(qslim_app).isDir() && QFileInfo(qslim_app).isExecutable())
		   {
			   value = qslim_app;
		   }
		   else
		   {
#ifdef _WIN32
			   qslim_app = "QSlim.exe";
#else
			   qslim_app = "qslim";
#endif

			   //Tratamos de ejecutar QSlim desde el path
			   if (QProcess::startDetached(qslim_app, QStringList("-h"), "") ) 
			   {
				   value = qslim_app;
			   }
		   }
	   }

	   if (reset || !value.isEmpty())
	   {
		   //Salvamos el path encontrado
		   qDebug() << "Setting: " << key << "=" << value;
		   settings->setValue(key, value);
	   }

   }//if (value.isEmpty())

   //Path a Tetgen. Si no existe lo buscamos
   key = "tetgen_app";
   value = settings->value(key).toString();
   if (reset || value.isEmpty())
   {
	   QString tetgen_app;

	   //Miramos si existe una variable de entorno adecuada
	   tetgen_app = getenv("TETGEN");
#ifdef _WIN32
		   tetgen_app.replace( QChar('\\'), "\\" );
#endif
   	   if (!tetgen_app.isEmpty() && QFile::exists(tetgen_app) && !QFileInfo(tetgen_app).isDir() && QFileInfo(tetgen_app).isExecutable())
	   {
			value = tetgen_app;
	   }
	   else
	   {
		   //Buscamos tetgen en el directorio de coindesigner
#ifdef _WIN32
		   tetgen_app.asprintf("%s/tetgen.exe", cds_dir);
		   tetgen_app.replace( QChar('/'), "\\" );
#else
		   tetgen_app.asprintf("%s/tetgen", cds_dir);
#endif

		   if (QFile::exists(tetgen_app) && !QFileInfo(tetgen_app).isDir() && QFileInfo(tetgen_app).isExecutable())
		   {
			   value = tetgen_app;
		   }
		   else
		   {
#ifdef _WIN32
			   tetgen_app = "tetgen.exe";
#else
			   tetgen_app = "tetgen";
#endif
				//Trata de ejecutar tetgen desde el path
			   if (QProcess::startDetached(tetgen_app, QStringList("-h"), "") ) 
			   {
				   value = tetgen_app;
			   }
		   }
	   }

	   if (reset || !value.isEmpty())
	   {
		   //Salvamos el path encontrado
		   qDebug() << "Setting: " << key << "=" << value;
		   settings->setValue(key, value);
	   }

   }//if (value.isEmpty())

}//void settingsDialog::setToDefault(bool reset)
