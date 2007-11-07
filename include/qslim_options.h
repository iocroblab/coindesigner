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

#include <qprocess.h>
#include <qmessagebox.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "ui_qslim_options4.h"

class qslim_options : public QDialog
{
	Q_OBJECT
	Ui::qslim_options Ui;
public:

	///Constructor
	qslim_options (QWidget *p=0, Qt::WindowFlags f=0) : QDialog(p, f)
	{
		Ui.setupUi(this);

		//Resto de configuracion aquí, por ejemplo
		//Ui.chkbox_F->setChecked(true);	
	}

 private slots:
	//Aplica el algoritmo qslim al nodo qslim_result, dejando el resultado
	//en el mismo nodo. El procedimiento llamante será el encargado de 
	//haber dejado en qslim_result un valor valido.
	void on_buttonOk_clicked()
	{

		//Ejemplo de lectura de valores así
		int idx=Ui.comboBox_O->currentIndex();

		QString S;
		S.sprintf("comboBox_O->currentIndex = %d\n", idx);
		S+= Ui.comboBox_O->currentText();

		QMessageBox::warning (this, tr ("Error"), S, QMessageBox::Ok,
			QMessageBox::NoButton, QMessageBox::NoButton);
		return;
	}

}; //class qslim_options

