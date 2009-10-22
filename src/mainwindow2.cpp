/*
    This file is part of coindesigner. (http://coindesigner.sf.net)

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

#include "mainwindow.h"
#include "cds_viewers.h"
#include "cds_util.h"
#include "cds_globals.h"
#include "3dsLoader.h"
#include <src_editor.h>
#include <mfield_editor.h>
#include <cppexport_options.h>

#include <QSettings>
#include <QMessageBox>
#include <QProcess>
#include <QResource>
#include <QFileDialog>
#include <QInputDialog>
#include <QPushButton>
#include <QCloseEvent>
#include <QMenu>
#include <QContextMenuEvent>
#include <QColorDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QGraphicsScene>
#include <QPainter>
#include <QScrollBar>
#include <QHeaderView>
#include <QtDebug>

extern QSettings *settings;

//OpenInventor includes
#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoNodes.h>




