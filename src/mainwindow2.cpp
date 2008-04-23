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

extern QSettings *settings;

//OpenInventor includes
#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoNodes.h>


#include <QPrinter>
#include <QPrintDialog>
#include <QGraphicsScene>
#include <QPainter>
#include <QScrollBar>

///Imprime el grafo de escena en un documento
void MainWindow::on_actionPrintSceneGraph_activated()
{

//Creamos un nuevo dialogo

	QTreeWidget scn;
	scn.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//scn.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QStringList headers;
	headers << tr("Node Class") << tr("Node Name") << tr("RefCount");
	scn.setHeaderLabels(headers);
	//Clone Ui.sceneGraph contents
	QTreeWidgetItem *qroot = Ui.sceneGraph->topLevelItem(0)->clone(); 
	scn.addTopLevelItem(qroot);
	scn.expandAll();

	//Rellena el nombre de los items
	
	//Usamos dos listas de items auxiliares
	QList<QTreeWidgetItem *>list1;
	QList<QTreeWidgetItem *>list2;
	list1 << qroot;
	list2 << Ui.sceneGraph->topLevelItem(0);
	int rootRefCount = root->getRefCount()-1;

    while (!list1.isEmpty())
	{
		//Leemos el primer elemento de cada lista, y lo sacamos de la lista
		QTreeWidgetItem *it1 = list1.takeFirst();
		QTreeWidgetItem *it2 = list2.takeFirst();
		//Rellenamos el nombre del nodo
		SoNode *node = mapQTCOIN[it2];
		assert(node != 0);
		it1->setText(1,node->getName().getString() );
		int relativeRefCount = node->getRefCount() - rootRefCount;
		if (relativeRefCount != 1)
			it1->setText(2,QString::number(relativeRefCount));

		//Añadimos a la lista los hijos de cada item
		for (int i=0; i< it1->childCount(); i++)
		{
			list1.append(it1->child(i));
			list2.append(it2->child(i));
		}

	}//while

	for (int i=0; i< scn.columnCount(); i++)
		scn.resizeColumnToContents(i);
	


	QPrinter printer;
	printer.setOutputFileName("print.pdf"); 
	//Solicita una impresora para inprimir
	QPrintDialog *dialog = new QPrintDialog(&printer, this);
	dialog->setWindowTitle(tr("Print Scene Graph"));
	if (dialog->exec() != QDialog::Accepted)
		return;


	//Medimos la altura del arbol mediante incrementos hasta hacer desaparecer la barra vertical
    QScrollBar *vs = scn.verticalScrollBar();
    int heightTree = 20;
	do
	{
		scn.hide();
    	heightTree += 10;
		scn.resize(400,heightTree);
		qDebug("%d",heightTree); 

		scn.show();
	} while(vs->isVisible());
	//scn.hide();

    int heightPage = printer.pageRect().height()-10;
	int numPages = 1+heightTree/heightPage;
	qDebug("tree=%d page=%d numPages=%d", heightTree, heightPage, numPages);

	QPainter painter;
	painter.begin(&printer);

	//Ultimo elemento de cada pagina
	QTreeWidgetItem *topItem = qroot;
	scn.scrollToTop();

	//Renderiza las paginas una a una
	int page;
	for (int page = 1; page < numPages; ++page) 
	{
		qDebug("pagina %d", page);

		//Creamos un nuevo pixmap donde renderizar el sceneGraph
 		QPixmap pixmap(printer.pageRect().size());
		scn.resize(printer.pageRect().width(), heightPage);
		pixmap.fill();

		scn.scrollToItem(topItem,QAbstractItemView::PositionAtTop);

		scn.render(&pixmap, QPoint(0,0), QRegion(0, 0, pixmap.width(), heightPage));
		pixmap.save(QString("print_%1.png").arg(page));

		//Manda el pixmap a la impresora
		painter.drawPixmap(0,0, pixmap);

		//Calcula el proximo item que sera cabecera
		topItem = scn.itemAt(10, heightPage-10); 

		printer.newPage();
	}

	//La ultima página
		qDebug("pagina %d y final", page);
		//Creamos un nuevo pixmap donde renderizar el sceneGraph
 		QPixmap pixmap(printer.pageRect().size());
		scn.resize(printer.pageRect().size());
		pixmap.fill();

		scn.scrollToItem(topItem,QAbstractItemView::PositionAtTop);

		scn.render(&pixmap, QPoint(0,0), QRegion(0, 0, pixmap.width(), heightPage));
		pixmap.save(QString("print_%1.png").arg(page));

		//Manda el pixmap a la impresora
		painter.drawPixmap(0,0, pixmap);

	//Finaliza y envia el documento a la impresora
	painter.end();

}

