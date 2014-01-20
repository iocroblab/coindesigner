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

#include <QProcess>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <Inventor/SoInput.h>
#include <string.h>

#include "qslim_options.h"
#include "cds_globals.h"
#include "cds_util.h"
extern QSettings *settings;

///Constructor
qslim_options::qslim_options(SoPath *path, QWidget *p, Qt::WindowFlags f) : QDialog(p, f)
{
	Ui.setupUi(this);

	input = path;
	output = NULL;
}

void qslim_options::accept()
{
	//Aplicamos los algoritmos de qslim y dejamos el resultado en qslim_result
	QString qslim_app = settings->value("qslim_app").toString();
	if (qslim_app.isEmpty())
	{
		QMessageBox::warning (this, tr ("Error"), tr("Path to QSlim not defined in settings"));
		output = NULL;
		return;
	}

	//Creacion de los argumentos
	QStringList args;
	args << "-O";
	args << QString::number (Ui.comboBox_O->currentIndex ());
	args << "-W";
	args << QString::number (Ui.comboBox_W->currentIndex ());
	args << "-t";
	args << QString::number (Ui.spinBox_t->value ());
	args << "-M";
	args << "iv";
	if (Ui.chkbox_F->isChecked ())
		args << "-F";
	args << "-q";

	//qDebug() << qslim_app << " " << args.join (" ");

	//Iniciamos QSlim
	QProcess myProcess(this);
	myProcess.start(qslim_app, args);

	//Esperamos hasta que comience el proceso y verificamos que lo hizo bien
	if (!myProcess.waitForStarted())
	{
		// error handling
		QString S;
		S = tr ("Error executing: ");
		S += qslim_app + args.join (" ");
		QMessageBox::warning (this, tr ("Error"), S);
		QDialog::accept();
		return;
	}

	//Convertimos el nodo a formato SMF
    std::string SMFString;

   //Escribimos el contenido en el fichero SMF
    if (input->getTail()->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId())) 
    {
    	IndexedFaceSet_to_SMF (input, SMFString);
    }
    else if (input->getTail()->getTypeId().isDerivedFrom(SoVRMLIndexedFaceSet::getClassTypeId())) 
    {
		//TODO VRMLIndexedFaceSet_to_SMF(input, SMFString);
    }
    
	//qDebug() << SMFString.c_str();

	//Introducimos la cadena en entrada estandar
	myProcess.write(SMFString.c_str());
	myProcess.closeWriteChannel();

	//Esperamos hasta que haya finalizado
	if (!myProcess.waitForFinished (-1))
	{
		// error handling
		QString S;
		S = tr ("QSlim returned an error");
		S += myProcess.readAllStandardError (); 
		QMessageBox::warning (this, tr ("Error"), S);
		QDialog::accept();
		return;
	}

    //Leemos la salida estandar del proceso mediante el parser de openInventor
	SoInput in;
	QByteArray stdoutup = myProcess.readAllStandardOutput ();
    in.setBuffer(stdoutup.data(), stdoutup.size());
	output = SoDB::readAll(&in);
    //qDebug("Qslim returned node %p", output);

	assert(output);
	output->setName("qslim_output");

	QDialog::accept();
} // void qslim_options::accept()

