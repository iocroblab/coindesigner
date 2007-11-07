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

#ifndef TETGEN_OPTIONS_H
#define TETGEN_OPTIONS_H

#include <qvariant.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qmetaobject.h>
#include <qapplication.h>
#include <private/qucomextra_p.h>


#include "cds_config.h"
#include <qprocess.h>
#include <qmessagebox.h>
#include <qvalidator.h>
#ifdef _WIN32
#include <windows.h>
#endif

class floatSpinBox : public QSpinBox
        {
          Q_OBJECT

        public:
			floatSpinBox(QWidget * parent = 0, const char * name = 0): QSpinBox(parent, name) 
			{
				setValidator(0);
			}

            QString mapValueToText( int value )
            {
				// 0.0 to 10.0
				QString ret;
				ret.sprintf("%.1f", value/10.0);
				return ret;
            }

            int mapTextToValue( bool *ok )
            {
				float num = text().toFloat(ok);
                return (int) ( 10 * num );
            }

			float fvalue() const 
			{
				return QSpinBox::value() / 10.0f;
			}
        };

class tetgen_options : public QDialog
{
   Q_OBJECT

public:
    tetgen_options( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~tetgen_options();

    QLabel* textLabel_q;
    floatSpinBox* spinBox_q;

	QLabel* textLabel_a;
    floatSpinBox* spinBox_a;

	/*
	QLabel* textLabel1_2;
    QComboBox* comboBox_W;
    QSpinBox* spinBox_t;
    QLabel* textLabel1_2_2;
    QCheckBox* chkbox_F;
	*/

    QPushButton* buttonOk;
    QPushButton* buttonCancel;


public slots:
    virtual void buttonOk_clicked();

protected:
    QVBoxLayout* tetgen_optionsLayout;
    QVBoxLayout* layout8;
    QGridLayout* layout7;
    QHBoxLayout* layout10;
    QSpacerItem* spacer2;

protected slots:
    virtual void languageChange();

};


#endif // TETGEN_OPTIONS_H
