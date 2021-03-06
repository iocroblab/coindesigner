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

#include "ui_qslim_options.h"
#include <Inventor/nodes/SoSeparator.h>

class qslim_options : public QDialog
{
	Q_OBJECT
	Ui::qslim_options Ui;
public:

	///Nodo de entrada
	SoPath *input;

	///Nodo resultado
	SoSeparator *output;

	///Constructor
	qslim_options (SoPath *path, QWidget *p=0, Qt::WindowFlags f=0);

 private slots:

	//Aplica el algoritmo qslim al nodo input, dejando el resultado
	//en el nodo output.
	void accept();

}; //class qslim_options

