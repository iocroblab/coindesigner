
#include <settingsDialog.h>
#include <QTableWidget>
#include <QColorDialog>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVariant>

settingsDialog::settingsDialog(QWidget *p, Qt::WindowFlags f) : QDialog(p,f)
{
	QString S;
	Ui.setupUi(this);
	updateTable();
}

void settingsDialog::updateTable()
{
	//Rellenamos el valor de la tabla
	int row=0;
	foreach(QString key, settings->allKeys())
	{
		QVariant value = settings->value(key);
		if (value.type() == QVariant::String)
		{
			//Damos a la tabla el numero de filas adecuado
			Ui.table->setRowCount(row+1);
			Ui.table->setVerticalHeaderItem(row, new QTableWidgetItem(key));
			Ui.table->setItem(row,0, new QTableWidgetItem(value.toString()));
			row++;
		}
	}//	foreach(QString key, settings->allKeys())

    //Aseguramos que las columnas tienen ancho suficiente
    Ui.table->resizeColumnsToContents();
}

///Callback que se ejecuta cada vez que modificamos la tabla
void settingsDialog::on_table_cellChanged(int row, int column)
{
    //Evitamos actualizar si el cambio no lo ha hecho el usuario
    if (Ui.table->item(row,column) != Ui.table->currentItem())
		return;
}


void settingsDialog::on_buttonBox_clicked(QAbstractButton * button)
{
	if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
	{
		resetToDefault();
		updateTable();
	}//if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
}

void settingsDialog::accept()
{
	QDialog::accept();
}


void settingsDialog::on_table_customContextMenuRequested(QPoint pos)
{
	//Miramos si se ha pulsado sobre algun item valido
    QTableWidgetItem *item = Ui.table->itemAt(pos);
	if (!item)
		return;

    int row = Ui.table->row(item);
    //int column = Ui.table->column(item);

	//Leemos la clave
	QString key = Ui.table->verticalHeaderItem(row)->text();
	QString value = item->text();

	if (key.endsWith("_app"))
	{
		//Preparamos un menu flotante
		QMenu popm(this);
		QAction actFile(tr("Insert filename"), this);
		popm.addAction(&actFile);

		//Mostramos el menu flotante y recogemos la opcion escogida
		QAction *act = popm.exec(QCursor::pos());

		//Si se ha escogido la opcion filename
		if (act == &actFile)
		{
			QString filename = QFileDialog::getOpenFileName(this, 
				tr("Choose Filename"), value, 
				tr("Image files")+" (*.exe);;" +
				tr("OpenInventor Files")+" (*.iv *.wrl);;"+
				tr("All Files")+" (*)");

			//Asignamos el valor escogido
			item->setText(filename);
		}
	}

/*
		//Preparamos un menu flotante con opciones true/false
		QMenu popm(this);
		QAction actTrue("TRUE", this);
		QAction actFalse("FALSE", this);
		popm.addAction(&actTrue);
		popm.addAction(&actFalse);

		//Mostramos el menu flotante y recogemos la opcion escogida
		QAction *idx = popm.exec(QCursor::pos());

		//Comprobamos que se ha seleccionado una opción válida.
		if (idx)
			item->setText(idx->text());
	}

*/

}//void settingsDialog::on_table_customContextMenuRequested(QPoint pos)


void settingsDialog::resetToDefault()
{
}//void settingsDialog::resetToDefault()
