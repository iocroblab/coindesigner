/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#include <qfiledialog.h>
#include <qvalidator.h>
#include <qmessagebox.h>
#include "cds_util.h"

void cppExportDialog::init()
{
    //Creamos un validador para el nombre de la clase
    QRegExp rx( "[_a-zA-Z0-9]{1,64}" );
    QValidator* validator = new QRegExpValidator( rx, this );
    classnameLineEdit->setValidator( validator );

   //Actualizamos el howto
	buttonGroup1_clicked(0);
}

void cppExportDialog::fileDialogButton_clicked()
{
    QString hppFilename(filenameLineEdit->text() );

    //Solicitamos un nombre de fichero
    hppFilename = QFileDialog::getSaveFileName(hppFilename,
             tr("Ficheros de cabecera C++ (*.h *.hpp);;Todos los ficheros (*)"),
             this,
             tr("Exportar escena como C++"));

    //Miramos si se pulso el boton cancelar
    if (hppFilename=="")
        return;

    filenameLineEdit->setText(hppFilename);
}

void cppExportDialog::classnameLineEdit_textChanged( const QString &classname )
{
    //Extrae el directorio actual
    QFileInfo hppFilename(filenameLineEdit->text());
    QString hppDirname = QDir::convertSeparators(hppFilename.dirPath(true)+"/");
    
    //Cambiamos el nombre del fichero de salida
    filenameLineEdit->setText(hppDirname+classname+".h");
 
    //Activamos el boton OK
   buttonOk->setEnabled(true);
   
   //Actualizamos el howto
   buttonGroup1_clicked(0);

}


void cppExportDialog::buttonOk_clicked()
{
  extern SoSeparator *root;

  if (!cds_export_hppFile((SoNode*)root, classnameLineEdit->text(), filenameLineEdit->text() ))
  {
	  //En caso de error escribimos un aviso
	  QString S;
	  S=tr("Error al escribir el fichero")+ " " + filenameLineEdit->text();
	  QMessageBox::warning( this, tr("Aviso"), S,
		  QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	  return;
  } // if

  
  //Si es necesario, creamos un .ccp para lanzar visualizar la nueva clase
  if (radioButton_app->isChecked())
  {
	  QString cppFilename = filenameLineEdit->text();
	  int extPos = cppFilename.findRev(".h",-1, false);
	  cppFilename.replace(extPos, 999, QString(".cpp"));
	  if (!cds_export_cppFile(classnameLineEdit->text(), cppFilename ))
	  {
		  //En caso de error escribimos un aviso
		  QString S;
		  S=tr("Error al escribir el fichero")+ " " + cppFilename;
		  QMessageBox::warning( this, tr("Aviso"), S,
			  QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		  return;
	  } // if

  }//if (radioButton_app->isChecked())

}


void cppExportDialog::buttonGroup1_clicked( int )
{

  QString msj;
  if (classnameLineEdit->text().isEmpty())
  {
    msj += tr("Introduzca un nombre para la clase");
  }
  else if (radioButton_app->isChecked())
  {
    msj += QString("soqt-config --build ")+classnameLineEdit->text()+".exe ";
    msj += classnameLineEdit->text()+".cpp\n";
  }
  else if (radioButton_class->isChecked())
  {
    msj += QString("#include \"")+classnameLineEdit->text()+".h\"\n";
    msj += "....\n";
    msj += QString("root->addChild(new ")+classnameLineEdit->text()+"() );\n";
  }
 
  howtoEdit->setText(msj);
  
}
