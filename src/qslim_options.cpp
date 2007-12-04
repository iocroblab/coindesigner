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
#include <QMessagebox>
#ifdef _WIN32
#include <windows.h>
#endif

#include "qslim_options.h"
#include "cds_globals.h"
extern QSettings *settings;

///Constructor
qslim_options::qslim_options(SoNode *node, QWidget *p, Qt::WindowFlags f) : QDialog(p, f)
{
	Ui.setupUi(this);

	input = node;
	output = NULL;

	//Resto de configuracion aquí, por ejemplo
	//Ui.chkbox_F->setChecked(true);	
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
	args << "-o";
	args << "qslim_out.iv";
	args << "-q";
	args << "qslim_in.smf";


	int result = QProcess::execute(qslim_app, args);

	if (result)
	{
	}
	else
	{
		// error handling
		QString S;
		S = tr ("Error executing: ");
		S += qslim_app + args.join (" ");
		QMessageBox::warning (this, tr ("Error"), S);
		return;
	}
/*
	while (proc->isRunning ())
	{
#ifdef _WIN32
		Sleep (1000);
#else
		sleep (1);
#endif
	}

	//Leemos el resultado en qslim_result
	SoInput input;
	if (!input.openFile ("qslim_out.iv"))
	{
		QString S;
		S = tr ("No encuentro qslim_out.iv ");
		S += proc->arguments ().join (" ");
		QMessageBox::warning (this, tr ("Error"), S, QMessageBox::Ok,
			QMessageBox::NoButton, QMessageBox::NoButton);
		delete proc;
		return;
	}

	//Miramos si es un fichero nativo de openInventor
	if (input.isValidFile ())
	{
		//Lo leemos sobre qslim_result
		qslim_result = SoDB::readAll (&input);
	}
	else
	{
		QString S;
		S = tr ("qslim_out.iv no es valido");
		S += proc->arguments ().join (" ");
		QMessageBox::warning (this, tr ("Error"), S, QMessageBox::Ok,
			QMessageBox::NoButton, QMessageBox::NoButton);
	}

	//Borramos el fichero de entrada
	QFile::remove("qslim_out.iv");
	delete proc;
*/
}
