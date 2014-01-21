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
#include "ui_cppexport_options.h"

class cppexport_options : public QDialog
{
	Q_OBJECT
	Ui::cppexport_options Ui;
	SoNode *node;
public:

	///Constructor
	cppexport_options (SoNode *root, QWidget *p=0, Qt::WindowFlags f=0);

 private slots:
	void accept();
	void on_fileDialogButton_clicked();
	void on_classnameLineEdit_textChanged(const QString &classname );
	void on_groupBox_clicked( bool );

}; //class cppexport_options

