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
#include "cds_util.h"

void cppexport_options::on_accept()
{
  //TODO: apuntar a root
  SoSeparator *root;

  if (!cds_export_hppFile((SoNode*)root, qPrintable(Ui.classnameLineEdit->text()), 
  										 qPrintable(Ui.filenameLineEdit->text()) ))
  {
	  //En caso de error escribimos un aviso
	  QString S;
	  S=tr("Error al escribir el fichero")+ " " + Ui.filenameLineEdit->text();
	  QMessageBox::warning( this, tr("Aviso"), S,
		  QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
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
		  S=tr("Error al escribir el fichero")+ " " + cppFilename;
		  QMessageBox::warning( this, tr("Aviso"), S,
			  QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		  return;
	  } // if

  }//if (Ui.radioButton_app->isChecked())

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

void cppexport_options::on_groupBox_clicked( int )
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

