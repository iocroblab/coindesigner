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

	//Leemos la impresora destino
	QPrinter printer;
	//printer.setOutputFileName("print.pdf"); 
	QPrintDialog *dialog = new QPrintDialog(&printer, this);
	dialog->setWindowTitle(tr("Print Scene Graph"));
	if (dialog->exec() != QDialog::Accepted)
		return;

	//Creamos un nuevo arbol, con mas informacion que Ui.sceneGraph
	QTreeWidget scn;
	scn.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scn.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QStringList headers;
	headers << tr("Node Class") << tr("Node Name") << tr("RefCount");
	scn.setHeaderLabels(headers);
	//Clone Ui.sceneGraph contents
	QTreeWidgetItem *qroot = Ui.sceneGraph->topLevelItem(0)->clone(); 
	scn.addTopLevelItem(qroot);
	scn.expandAll();

	//Rellena el nombre de los items
	
	//Usamos dos listas de items auxiliares
	int numItems=0;
	int rootRefCount = root->getRefCount()-1;

	/*
	QList<QTreeWidgetItem *>list1;
	QList<QTreeWidgetItem *>list2;
	list1 << qroot;
	list2 << Ui.sceneGraph->topLevelItem(0);

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

		//Incrementamos el contador de items
		numItems++;

	}//while
	*/

	//Leemos el primer elemento de cada lista, y lo sacamos de la lista
	QList<QTreeWidgetItem *>scnItemList;
	QTreeWidgetItem *it1 = qroot;
	QTreeWidgetItem *it2 = Ui.sceneGraph->topLevelItem(0);

	do 
	{
		//Rellenamos el nombre del nodo
		SoNode *node = mapQTCOIN[it2];
		assert(node != 0);
		it1->setText(1,node->getName().getString() );
		int relativeRefCount = node->getRefCount() - rootRefCount;
		if (relativeRefCount != 1)
			it1->setText(2,QString::number(relativeRefCount));

		//TEST
		it1->setText(2,QString::number(numItems));

		//Almacenamos los item en orden en una lista
		scnItemList.push_back(it1);
		numItems++;

		it1 = scn.itemBelow(it1);
		it2 = Ui.sceneGraph->itemBelow(it2);
	}
	while (it1 != NULL);

	for (int i=0; i< scn.columnCount(); i++)
		scn.resizeColumnToContents(i);

    int heightPage = printer.pageRect().height();
	int itemsPerPage = heightPage/17; //This is empiric 20 pixels per item
	int numPages = 1+numItems/itemsPerPage;
	qDebug("heightPage=%d numItems=%d itemsPerPage=%d numPages=%d", 
				heightPage, numItems, itemsPerPage, numPages);

	QPainter painter;
	painter.begin(&printer);

	//Primer elemento de cada pagina
	QTreeWidgetItem *topItem = qroot;
	scn.resize(printer.pageRect().width(), heightPage);
	scn.show();

	//Renderiza las paginas salvo la ultima
	int page;
	for (page = 0; page < numPages-1; ++page) 
	{
		//Calcula el proximo item que sera cabecera de página
		topItem = scnItemList.at(page*itemsPerPage);
		scn.scrollToItem(topItem,QAbstractItemView::PositionAtTop);

		//Creamos un nuevo pixmap donde renderizar el sceneGraph
 		QPixmap pixmap(printer.pageRect().size());
		pixmap.fill();

		//Renderiza una página
		scn.render(&pixmap, QPoint(0,0), QRegion(0, 0, pixmap.width(), heightPage));
		pixmap.save(QString("print_%1.png").arg(page));

		//Manda el pixmap a la impresora
		painter.drawPixmap(0,0, pixmap);
		printer.newPage();
	}

	//La ultima página
	qDebug("pagina %d y final", page);

	//Calcula el proximo item que sera cabecera
	topItem = scnItemList.at(page*itemsPerPage);
	scn.scrollToItem(topItem,QAbstractItemView::PositionAtTop);

	//Creamos un nuevo pixmap donde renderizar el sceneGraph
	QPixmap pixmap(printer.pageRect().size());
	scn.resize(printer.pageRect().size());
	pixmap.fill();

	scn.render(&pixmap, QPoint(0,0), QRegion(0, 0, pixmap.width(), heightPage));
	pixmap.save(QString("print_%1.png").arg(page));

	//Manda el pixmap a la impresora
	painter.drawPixmap(0,0, pixmap);

	//Finaliza y envia el documento a la impresora
	painter.end();

}

