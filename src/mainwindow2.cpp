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




