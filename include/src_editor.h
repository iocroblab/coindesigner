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

#include <QMessageBox>

#include "ui_src_view.h"
#include <cds_util.h>
#include <QDialogButtonBox>
#include <Inventor/nodes/SoSeparator.h>
#include <QTimer>

class SrcEditor : public QDialog
{
	Q_OBJECT
	Ui::src_view Ui;
	QTimer timer;
public:
	SoSeparator *result;

	///Constructor
	SrcEditor (SoSeparator *scene, QWidget *p=0, Qt::WindowFlags f=0) : QDialog(p, f)
	{
		Ui.setupUi(this);
		Ui.buttonBox->setStandardButtons(QDialogButtonBox::Close);

                result=NULL;

                //Rellenamos con la escena
                Ui.textEdit->setText(cds_export_string(scene));

     		//connect(Ui.textEdit, SIGNAL(textChanged()), this, SLOT(compile()));
     		connect(&timer, SIGNAL(timeout()), this, SLOT(compile()));
     		timer.start(1000);
	}

 private slots:
	void compile()
	{
    		//Leemos la escena desde el buffer de memoria
    		QString src = Ui.textEdit->toPlainText();
		char *buf = new char[src.size()];
                memcpy(buf, src.toAscii(), src.size() );

    		SoInput input;
    		input.setBuffer(buf, src.size() ) ;
    		result = SoDB::readAll(&input);
		if(result)
		{
			result->ref();
			result->unref();
		}


                if (result)
			Ui.buttonBox->setStandardButtons(QDialogButtonBox::Ok);
		else
			Ui.buttonBox->setStandardButtons(QDialogButtonBox::Close);

                delete buf;
		printf("OK=%p\n", result);

	}

	void accept()
	{
    		//Leemos la escena desde el buffer de memoria
    		QString src = Ui.textEdit->toPlainText();
		char *buf = new char[src.size()];
                memcpy(buf, src.toAscii(), src.size() );

    		SoInput input;
    		input.setBuffer(buf, src.size() ) ;
    		result = SoDB::readAll(&input);
                delete buf;

		qDebug("HOLA\n");

		QDialog::accept();
	}

}; //class SrcEditor

