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
#include <QtDebug>

extern QSettings *settings;

//OpenInventor includes
#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoNodes.h>


#include <QPrinter>
#include <QPrintDialog>
#include <QGraphicsScene>
#include <QPainter>
#include <QScrollBar>
#include <QHeaderView>

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
	printer.setFullPage(false);

	//Creamos un nuevo arbol, con mas informacion que Ui.sceneGraph
	QTreeWidget scn;
	scn.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scn.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QStringList headers;
	headers << tr("Node Class") << tr("Node Name") << tr("RefCount");
	scn.setHeaderLabels(headers);
	scn.setAlternatingRowColors(true);
	//Clone Ui.sceneGraph contents
	QTreeWidgetItem *qroot = Ui.sceneGraph->topLevelItem(0)->clone(); 
	scn.addTopLevelItem(qroot);
	scn.expandAll();

	//Rellena el nombre de los items
	
	//Usamos dos listas de items auxiliares
	int numItems=0;

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
		int relativeRefCount = node->getRefCount();
		if (relativeRefCount != 1)
			it1->setText(2,QString::number(relativeRefCount));

		//TEST
		//it1->setText(2,QString::number(numItems));

		//Almacenamos los item en orden en una lista
		scnItemList.push_back(it1);
		numItems++;

		//Y pasamos al siguiente par de items
		it1 = scn.itemBelow(it1);
		it2 = Ui.sceneGraph->itemBelow(it2);
	}
	while (it1 != NULL);

	for (int i=0; i< scn.columnCount(); i++)
		scn.resizeColumnToContents(i);

	int headerHeight = scn.header()->height();
    int heightPage = printer.pageRect().height();
	int itemsPerPage = (heightPage-headerHeight) / (scn.visualItemRect(qroot).height()+1); 
	//qDebug("heightPage=%d headerHeight=%d numItems=%d itemsPerPage=%d ", heightPage, headerHeight, numItems, itemsPerPage);

	QPainter painter;
	painter.begin(&printer);

	//Primer elemento de cada pagina
	QTreeWidgetItem *topItem = NULL;
	QTreeWidgetItem *lastItem = NULL;
	scn.resize(printer.pageRect().size());

	//Renderiza las paginas salvo la ultima
	int page=0;
	do
	{
		//Calcula el proximo item que sera cabecera de página
		topItem = scnItemList.at(page*itemsPerPage);

		//Calcula el proximo item que sera fin de pagina
		int lastItemIdx = page*itemsPerPage+itemsPerPage-1;
		if (lastItemIdx >= numItems)
			lastItemIdx = numItems - 1;
		lastItem = scnItemList.at(lastItemIdx);

		//qDebug("page=%d topItem=%d lastItem=%d", page, page*itemsPerPage, lastItemIdx); 

		//Calculamos la posicion del primer y ultimo item de la pagina
		scn.scrollToItem(topItem,QAbstractItemView::PositionAtTop);

		QRect topRect = scn.visualItemRect(topItem);
		QRect lastRect = scn.visualItemRect(lastItem);
		heightPage = lastRect.bottom() - topRect.top();
		//qDebug("topRect.top=%d lastRect.bottom=%d heightPage=%d",topRect.top(), lastRect.bottom(), heightPage); 

 		QPixmap pixmap(printer.pageRect().size());
		pixmap.fill();

		//Renderiza la cabecera al principio de la pagina
		scn.header()->render(&pixmap);

		//Renderiza los items de la pagina
		scn.viewport()->render(&pixmap, QPoint(0,headerHeight-topRect.top()), QRegion(0, topRect.top(), pixmap.width(), heightPage));

		//Manda el pixmap a la impresora
		//pixmap.save(QString("print_%1.png").arg(page));
		painter.drawPixmap(printer.pageRect(), pixmap);

		//Miramos si hace falta alguna otra página
		if (lastItemIdx < numItems - 1)
			printer.newPage();
		else
			break;

		page++;

	} while (true);

	//Finaliza y envia el documento a la impresora
	painter.end();

	//Una pausa para depurar / ver el arbol
	//scn.show();QDialog dlg; dlg.exec();
}

