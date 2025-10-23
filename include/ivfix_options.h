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

#include <Inventor/nodes/SoNode.h>
#include "ui_ivfix_options.h"

class ivfix_options : public QDialog
{
	Q_OBJECT
	Ui::ivfix_options Ui;
	SoNode *input;
public:

	//Resultado de ivfix
	SoNode *output;

	///Constructor
	ivfix_options (SoNode *root, QWidget* p = nullptr, Qt::WindowFlags f = {});

 private slots:
	void accept();

}; //class ivfix_options

