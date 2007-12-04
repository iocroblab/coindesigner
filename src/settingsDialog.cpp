
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

	//Leemos la clave
	QString key = Ui.table->verticalHeaderItem(row)->text();
	QString value = item->text();

	//Preparamos un menu flotante
	QMenu popm(this);
	QAction actFile(tr("Insert filename"), this);
	QAction actDir(tr("Insert directory"), this);

	//Miramos si es el path de un ejecutable
	if (key.endsWith("_app"))
	{
		popm.addAction(&actFile);
	}

	else if (key.endsWith("_dir"))
	{
		popm.addAction(&actDir);
	}

    //Si hay opciones en el menu, lo mostramos
	if (!popm.isEmpty())
	{
		//Mostramos el menu flotante y recogemos la opcion escogida
		QAction *act = popm.exec(QCursor::pos());

		//Si se ha escogido la opcion filename
		if (act == &actFile)
		{
			QString filename = QFileDialog::getOpenFileName(this, 
				tr("Choose Filename"), value, 
				#ifdef _WIN32
				tr("Executable files")+" (*.exe *.bat);;" +
				#endif
				tr("All Files")+" (*)");

			//Asignamos el valor escogido
			if (filename != "")
				item->setText(filename);
		}

		//Si se ha escogido la opcion filename
		else if (act == &actDir)
		{
			QString dir = QFileDialog::getExistingDirectory(this, 
				tr("Choose Directory"), value);

			//Asignamos el valor escogido
			if (dir != "")
				item->setText(dir);
		}
	}
	
    //Aseguramos que las columnas tienen ancho suficiente
    Ui.table->resizeColumnsToContents();

}//void settingsDialog::on_table_customContextMenuRequested(QPoint pos)


void settingsDialog::resetToDefault()
{
}//void settingsDialog::resetToDefault()
