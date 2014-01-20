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

#ifdef _WIN32
#include <windows.h>
#endif

#include "cppexport_options.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QValidator>
#include "cds_util.h"

cppexport_options::cppexport_options (SoNode *root, QWidget *p, Qt::WindowFlags f) : QDialog(p, f)
{
	Ui.setupUi(this);

	//Creamos un validador para el nombre de la clase
	QRegExp rx( "[_a-zA-Z0-9]{1,64}" );
	QValidator* validator = new QRegExpValidator( rx, this );
	Ui.classnameLineEdit->setValidator( validator );

	//Configuramos el campo de ayuda
	connect(Ui.radioButton_app, SIGNAL(clicked(bool)), this, SLOT(on_groupBox_clicked(bool)));
	connect(Ui.radioButton_class, SIGNAL(clicked(bool)), this, SLOT(on_groupBox_clicked(bool)));
	on_groupBox_clicked(0);

	//Salvamos puntero al nodo
	node = root;
}

void cppexport_options::accept()
{
	if (Ui.classnameLineEdit->text().isEmpty())
	{
		QMessageBox::warning( this, tr("Warning"), tr("Input a name for your class"));
		return;
	}

	if (!cds_export_hppFile(node, qPrintable(Ui.classnameLineEdit->text()), 
		qPrintable(Ui.filenameLineEdit->text()) ))
	{
		//En caso de error escribimos un aviso
		QString S;
		S=tr("Error while creating file")+ " " + Ui.filenameLineEdit->text();
		QMessageBox::warning( this, tr("Warning"), S);
		return;
	} // if

	//Si es necesario, creamos un .ccp para lanzar visualizar la nueva clase
	if (Ui.radioButton_app->isChecked())
	{
		QString cppFilename = Ui.filenameLineEdit->text();
		int extPos = cppFilename.lastIndexOf(".h",-1, Qt::CaseInsensitive);
		cppFilename.replace(extPos, 999, QString(".cpp"));
		if (!cds_export_cppFile(qPrintable(Ui.classnameLineEdit->text()), qPrintable(cppFilename) ))
		{
			//En caso de error escribimos un aviso
			QString S;
			S=tr("Error while creating file")+ " " + cppFilename;
			QMessageBox::warning( this, tr("Warning"), S);
			return;
		} // if
	}//if (Ui.radioButton_app->isChecked())

	QDialog::accept();
}

void cppexport_options::on_fileDialogButton_clicked()
{
   	QString hppFilename(Ui.filenameLineEdit->text() );

   	//Solicitamos un nombre de fichero
	hppFilename= QFileDialog::getSaveFileName(this, tr("Save as..."), hppFilename,
             		tr("C++ header files")+"(*.h *.hpp);;"+tr("All files")+"(*)");

   	//Miramos si se pulso el boton cancelar
   	if (hppFilename=="")
       	return;

   	Ui.filenameLineEdit->setText(hppFilename);
}

void cppexport_options::on_classnameLineEdit_textChanged( const QString &classname )
{
    //Extrae el directorio actual
    QFileInfo hppFilename(Ui.filenameLineEdit->text());
    QString hppDirname = QDir::convertSeparators(hppFilename.absolutePath()+"/");
    
    //Cambiamos el nombre del fichero de salida
    Ui.filenameLineEdit->setText(hppDirname+classname+".h");
 
    //Activamos el boton OK
   //buttonOk->setEnabled(true);
   
   //Actualizamos el howto
   on_groupBox_clicked(0);

}

void cppexport_options::on_groupBox_clicked( bool )
{

  QString msj;
  if (Ui.classnameLineEdit->text().isEmpty())
  {
    msj += tr("Input a name for your class");
  }
  else if (Ui.radioButton_app->isChecked())
  {
    msj += QString("soqt-config --build ")+Ui.classnameLineEdit->text()+".exe ";
    msj += Ui.classnameLineEdit->text()+".cpp\n";
  }
  else if (Ui.radioButton_class->isChecked())
  {
    msj += QString("#include \"")+Ui.classnameLineEdit->text()+".h\"\n";
    msj += "....\n";
    msj += QString("root->addChild(new ")+Ui.classnameLineEdit->text()+"() );\n";
  }
 
  Ui.howtoEdit->setText(msj);
}

