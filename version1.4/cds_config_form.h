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

#ifndef CDS_CONFIG_FORM_H
#define CDS_CONFIG_FORM_H

#include <qvariant.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtable.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qmetaobject.h>
#include <qapplication.h>
#include <qsettings.h>
#include <private/qucomextra_p.h>
#include "cds_config.h"

#ifdef _WIN32
#include <windows.h>
#endif

///Objeto para almacenar la configuracion de coindesigner
extern QSettings settings;

///Elimina todos los settings de la aplicacion
void cds_removeSettings();

///Almacena todos los settings por defecto de la aplicacion
void cds_defaultSettings();



class cds_config_form : public QDialog
{
   Q_OBJECT

public:
    cds_config_form( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~cds_config_form();

    QTable* table;

    QPushButton* buttonOk;
    QPushButton* buttonReset;
    QPushButton* buttonCancel;


public slots:
    virtual void buttonOk_clicked();
    virtual void buttonReset_clicked();
    virtual void table_clicked(int fila, int columna);

protected:
    QVBoxLayout* cds_config_form_layout;
    QHBoxLayout* layout10;
    QSpacerItem* spacer2;

protected slots:
    virtual void languageChange();

};


#endif // CDS_CONFIG_FORM_H
