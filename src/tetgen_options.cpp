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
#include <QFile>
#include <QDir>
#include <Inventor/SoInput.h>
#include <string.h>

#include "tetgen_options.h"
#include "cds_globals.h"
#include "cds_util.h"
extern QSettings *settings;

///Constructor
tetgen_options::tetgen_options(SoPath *path, QWidget *p, Qt::WindowFlags f) : QDialog(p, f)
{
	Ui.setupUi(this);

	input = path;
	output = NULL;

	//Conecta todos los controles y actualiza los argumentos por primera vez
	connect(Ui.spinBox_q, SIGNAL(valueChanged(double)),this,SLOT(argsChanged()));
	connect(Ui.spinBox_a, SIGNAL(valueChanged(double)),this,SLOT(argsChanged()));
	argsChanged();
}

///Recalcula los argumentos a pasar a tetgen
void tetgen_options::argsChanged()
{
	tetgen_args.sprintf("-pOq%.2fa%.2f", Ui.spinBox_q->value(), Ui.spinBox_a->value() );
	Ui.tetgen_args->setText(tetgen_args);
}

void tetgen_options::accept()
{
	//Aplicamos los algoritmos de tetgen y dejamos el resultado en tetgen_result
	QString tetgen_app = settings->value("tetgen_app").toString();
	if (tetgen_app.isEmpty())
	{
		QMessageBox::warning (this, tr ("Error"), tr("Path to tetgen not defined in settings"));
		output = NULL;
		return;
	}

	//Creacion de los argumentos
	QString offFilename = QDir::tempPath()+ "/tempfile.off";
	QStringList args;
	argsChanged();
	args << tetgen_args << offFilename;

	qDebug() << tetgen_app << " " << args.join (" ");

	//Escribimos el nodo en el fichero OFF
	FILE *offFile = fopen(qPrintable(offFilename), "w");

	if (offFile == NULL)
	{
		QMessageBox::warning (this, tr ("Error"), tr("Could not create file ")+QString(offFilename));
		output = NULL;
		return;
	}

    if (input->getTail()->getTypeId().isDerivedFrom(SoIndexedFaceSet::getClassTypeId())) 
    {
    	IndexedFaceSet_to_OFF (input, offFile);
    }
    else if (input->getTail()->getTypeId().isDerivedFrom(SoVRMLIndexedFaceSet::getClassTypeId())) 
    {
		VRMLIndexedFaceSet_to_OFF((SoVRMLIndexedFaceSet *)input->getTail(), offFile);
    }

	//Iniciamos tetgen
	QProcess myProcess(this);
	myProcess.start(tetgen_app, args);

	//Esperamos hasta que comience el proceso y verificamos que lo hizo bien
	if (!myProcess.waitForStarted())
	{
		// error handling
		QString S;
		S = tr ("Error executing: ");
		S += tetgen_app + QString(" ") + args.join (" ");
		QMessageBox::warning (this, tr ("Error"), S);
		QDialog::accept();
		return;
	}

	//Esperamos hasta que haya finalizado
	if (!myProcess.waitForFinished (-1))
	{
		// error handling
		QString S;
		S = tr("tetgen returned an error")+"\n";
		S += myProcess.readAllStandardError (); 
		QMessageBox::warning (this, tr ("Error"), S);
		QDialog::accept();
		return;
	}

	QString nodeFilename = offFilename + ".1.node";
	//TODO: continuar la funcion de llamada a tetgen
/*
    //Leemos la salida estandar del proceso mediante el parser de openInventor
	SoInput in;
	QByteArray stdoutup = myProcess.readAllStandardOutput ();
    in.setBuffer(stdoutup.data(), stdoutup.size());
	output = SoDB::readAll(&in);
    //qDebug("tetgen returned node %p", output);
*/
	assert(output);
	output->setName("tetgen_output");

	QDialog::accept();
} // void tetgen_options::accept()

