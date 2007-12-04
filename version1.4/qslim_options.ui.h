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

#include "cds_config_form.h"
#include <qprocess.h>
#include <qmessagebox.h>
#ifdef _WIN32
#include <windows.h>
#endif

SoNode *qslim_result = NULL;

//Aplica el algoritmo qslim al nodo qslim_result, dejando el resultado
//en el mismo nodo. El procedimiento llamante será el encargado de 
//haber dejado en qslim_result un valor valido.
void qslim_options::buttonOk_clicked ()
{

  //Aplicamos los algoritmos de qslim y dejamos el resultado en qslim_result
  if (!settings.readEntry("/coindesigner/qslim_app"))
  {
    QString S;
    S = tr ("No se ha encontrado la herramienta QSLIM");
    QMessageBox::warning (this, tr ("Error"), S, QMessageBox::Ok,
			  QMessageBox::NoButton, QMessageBox::NoButton);
    qslim_result = NULL;
    return;
  }
  else
  {
    //Construimos un proceso con las opciones dadas.
    QProcess *proc = new QProcess (this);
    proc->addArgument (settings.readEntry("/coindesigner/qslim_app"));
    proc->addArgument ("-O");
    proc->addArgument (QString::number (comboBox_O->currentItem ()));
    proc->addArgument ("-W");
    proc->addArgument (QString::number (comboBox_W->currentItem ()));
    proc->addArgument ("-t");
    proc->addArgument (QString::number (spinBox_t->value ()));
    proc->addArgument ("-M");
    proc->addArgument ("iv");
    if (chkbox_F->isChecked ())
      proc->addArgument ("-F");
    proc->addArgument ("-o");
    proc->addArgument ("qslim_out.iv");
    proc->addArgument ("-q");
    proc->addArgument ("qslim_in.smf");

    //Lanzamos el proceso
    if (!proc->launch (""))
    {
      // error handling
      QString S;
      S = tr ("Error ejecutando: ");
      S += proc->arguments ().join (" ");
      QMessageBox::warning (this, tr ("Error"), S, QMessageBox::Ok,
			    QMessageBox::NoButton, QMessageBox::NoButton);
      return;
    }

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
  }

}
