
#include <mfield_editor.h>
#include <QLabel>
#include <QTableWidget>
#include <QColorDialog>
#include <QMenu>
#include <QMessageBox>
#include <Inventor/fields/SoFields.h>
#include <QPushButton>
#include <QDialogButtonBox>

MFieldEditor::MFieldEditor(SoMField *field, QWidget *p, Qt::WindowFlags f) : QDialog(p,f)
{
	QString S;

	Ui.setupUi(this);

	//Indicamos que no estamos listos para actualizar
	current_mfield = NULL;

    //Leemos el tipo de este campo
    SoType tipo=field->getTypeId();
    const char*nombre_tipo = tipo.getName();  

    //Damos a la tabla el numero de filas adecuado
    //Dejamos una fila libre para añadir nuevos valores
    int numFilas=field->getNum();
    Ui.table->setRowCount(numFilas+1);
    Ui.sizeLabel->setText(S.setNum(numFilas) + " " + tr("elements"));

	//Numeramos las filas a partir de cero, y añadimos una fila para nuevos elementos
	QStringList vh, hh;
    for (int i=0; i<numFilas; i++)
		vh << S.setNum(i);
	vh << tr("new");
	Ui.table->setVerticalHeaderLabels(vh);

    //Tratamiento de campos SoMFColor
    if (!strcmp(nombre_tipo, "MFColor"))
    {
        //Columnas para R, G, B
        Ui.table->setColumnCount(3);
		hh << "R" << "G" << "B";
        Ui.table->setHorizontalHeaderLabels(hh);

        //Rellenamos la tabla
        SbColor color;
        const SbColor *values = ((SoMFColor *)field)->getValues(0);
        for (int i=0; i<numFilas; i++)
        {
           color= values[i].getValue();
           Ui.table->setItem(i,0, new QTableWidgetItem(S.setNum(color[0],'g',2)));   
           Ui.table->setItem(i,1, new QTableWidgetItem(S.setNum(color[1],'g',2)));   
           Ui.table->setItem(i,2, new QTableWidgetItem(S.setNum(color[2],'g',2)));   
        }      
    }

    //Tratamiento de campos SoMFVec2f
    else if (!strcmp(nombre_tipo, "MFVec2f"))
    {
        //Cabecera de las columnas
        Ui.table->setColumnCount(2);
		hh << "X" << "Y";
        Ui.table->setHorizontalHeaderLabels(hh);

        //Rellenamos la tabla
        const SbVec2f *values = ((SoMFVec2f *)field)->getValues(0);
		float x, y;
        for (int i=0; i<numFilas; i++)
        {
           values[i].getValue(x,y);       
           Ui.table->setItem(i,0, new QTableWidgetItem(S.setNum(x,'g',5)));   
           Ui.table->setItem(i,1, new QTableWidgetItem(S.setNum(y,'g',5)));   
        }      
    }

    //Tratamiento de campos SoMFVec3f
    else if (!strcmp(nombre_tipo, "MFVec3f"))
    {
        //Cabecera de las columnas
        Ui.table->setColumnCount(3);
		hh << "X" << "Y" << "Z";
        Ui.table->setHorizontalHeaderLabels(hh);

        //Rellenamos la tabla
        const SbVec3f *values = ((SoMFVec3f *)field)->getValues(0);
		float x, y, z;
        for (int i=0; i<numFilas; i++)
        {
           values[i].getValue(x,y,z);       
           Ui.table->setItem(i,0, new QTableWidgetItem(S.setNum(x,'g',5)));   
           Ui.table->setItem(i,1, new QTableWidgetItem(S.setNum(y,'g',5)));   
           Ui.table->setItem(i,2, new QTableWidgetItem(S.setNum(z,'g',5)));   

			/* Esto tambien funciona, pero tal vez es menos eficiente
			//Leemos el campo y lo partimos en valores
			SbString valueString;
			field->get1(i, valueString);
			QStringList vv = QString(valueString.getString()).split(" ");

			//Insertamos cada valor en la tabla
			for(int j=0;j<vv.size(); j++)
    			Ui.table->setItem(i,j, new QTableWidgetItem(vv[j]));   

			*/
        }
    }

    //Tratamiento de campos SoMFVec4f
    else if (!strcmp(nombre_tipo, "MFVec4f"))
    {
        //Cabecera de las columnas
        Ui.table->setColumnCount(4);
		hh << "X" << "Y" << "Z" << "T";
        Ui.table->setHorizontalHeaderLabels(hh);

        //Rellenamos la tabla
        const SbVec4f *values = ((SoMFVec4f *)field)->getValues(0);
		float x, y, z, t;
        for (int i=0; i<numFilas; i++)
        {
           values[i].getValue(x,y,z,t);       
           Ui.table->setItem(i,0, new QTableWidgetItem(S.setNum(x,'g',5)));   
           Ui.table->setItem(i,1, new QTableWidgetItem(S.setNum(y,'g',5)));   
           Ui.table->setItem(i,2, new QTableWidgetItem(S.setNum(z,'g',5)));   
           Ui.table->setItem(i,3, new QTableWidgetItem(S.setNum(t,'g',5)));   
        }      
    }

    //Tratamiento de campos SoMFFloat, SoMFUInt32, SoMFInt32
    else if (!strcmp(nombre_tipo, "MFFloat") || !strcmp(nombre_tipo, "MFUInt32") 
          || !strcmp(nombre_tipo, "MFShort") || !strcmp(nombre_tipo, "MFInt32") 
          || !strcmp(nombre_tipo, "MFDouble") || !strcmp(nombre_tipo, "MFBool") 
          || !strcmp(nombre_tipo, "MFUShort") || !strcmp(nombre_tipo, "MFName") )
    {
        //Hace falta una columna
        Ui.table->setColumnCount(1);
		hh << tr("Value");
        Ui.table->setHorizontalHeaderLabels(hh);

        //Rellenamos la tabla usando la funcion get1 de coin
        SbString valueString;
        for (int i=0; i<numFilas; i++)
        {
           field->get1(i, valueString);
           Ui.table->setItem(i,0, new QTableWidgetItem(valueString.getString()));
        }      
    }

    //Tratamiento de campos SoMFString
    else if (!strcmp(nombre_tipo, "MFString"))
    {
        //Hace falta una columna
        Ui.table->setColumnCount(1);
		hh << tr("Value");
        Ui.table->setHorizontalHeaderLabels(hh);

        //Rellenamos la tabla usando la funcion get1 de coin
        SbString valueString;
        for (int i=0; i<numFilas; i++)
        {
           field->get1(i, valueString);

		   //Eliminamos las comillas
           valueString.deleteSubString(0,0);
           int ln = valueString.getLength();
           valueString.deleteSubString(ln-1,ln-1);

           Ui.table->setItem(i,0, new QTableWidgetItem(valueString.getString()));
        }//for
    }

	else
	{
		qDebug("Falta soporte para tipo %s\n", nombre_tipo);
	}

    //Creamos los items para la ultima fila
	for (int j=0; j<Ui.table->columnCount(); j++)
	{
           Ui.table->setItem(Ui.table->rowCount()-1,j, new QTableWidgetItem());
	}
	
	//Conectamos el menu contextual de la tabla
	connect(Ui.table, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_contextMenuFieldEditor(QPoint)));

	//Conectamos la accion en caso de cambio
	connect(Ui.table, SIGNAL(cellChanged(int, int)), this, SLOT(on_cellChanged(int, int)));

	//Salvamos una copia del puntero
	current_mfield = field;
}

///Callback que se ejecuta cada vez que modificamos la tabla
void MFieldEditor::on_cellChanged(int row, int column)
{
    //Evitamos actualizar si el cambio no lo ha hecho el usuario
    if (current_mfield == NULL || Ui.table->item(row,column) != Ui.table->currentItem())
		return;

	QString S;
	int numRows = Ui.table->rowCount();

	//Si cambio algun valor de la última fila, aumentamos la tabla
	if (row+1 == numRows)
	{
		//Hacemos crecer la tabla una nueva fila
		Ui.sizeLabel->setText(S.setNum(numRows) + " " + tr("elements"));

		Ui.table->setRowCount(numRows+1);

		Ui.table->verticalHeaderItem(numRows-1)->setText(S.setNum(numRows-1));
		Ui.table->setVerticalHeaderItem(numRows, new QTableWidgetItem(tr("new")));	
	
    	//Creamos los items para la ultima fila
		for (int j=0; j<Ui.table->columnCount(); j++)
		{
			Ui.table->setItem(Ui.table->rowCount()-1,j, new QTableWidgetItem());
		}
	}         
}


void MFieldEditor::on_buttonBox_clicked(QAbstractButton * button)
{
	if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
	{
		QString S;
		SbString oldValue;

		//Buscamos el SoMField que estamos editando
		SoMField *field = current_mfield;

		//Leemos el tipo de este campo
		SoType tipo=field->getTypeId();
		const char*nombre_tipo = tipo.getName();  

		//Actualizamos el contenido del SoMField

		//Si es necesario, ampliamos el tamaño del SoMField
		field->setNum(Ui.table->rowCount()-1);

		//Miramos cuantas columnas tiene la tabla
		int numComp = Ui.table->columnCount();

		//Tratamiento de los campos SoMFString
		if (!strcmp(nombre_tipo, "MFString") )
		{
			//Convertimos el campo a SoMFString
			SoMFString *soMFString = (SoMFString *)field;

			//Recorremos las filas de la tabla
			for(int i=0;i<Ui.table->rowCount()-1;i++)
			{
				//Extraemos el contenido de las celdas
				S = Ui.table->item(i,0)->text();

				//Aplicamos el valor al campo
				soMFString->set1Value(i, SbString(qPrintable(S)));

			} // for(int i=0;i<Ui.table->rowCount()-1;i++)
		}
		else

			//Recorremos las filas de la tabla
			for(int i=0;i<Ui.table->rowCount()-1;i++)
			{
				//Sacamos una copia del field actual, por si hay errores
				field->get1(i, oldValue);
				//qDebug("old: %s\n", oldValue.getString());

				//Extraemos el contenido de la primera celda
				S = Ui.table->item(i,0)->text();

				//Concatenamos el resto de celdas de la fila
				for (int j=1; j < numComp; j++)
				{
					S.append(' ');
					S.append(Ui.table->item(i,j)->text());
				}

				//Leemos la cadena mediante el parser de openInventor
				//fprintf(stderr, "Introduciendo: %s\n", txt);
				if (!field->set1(i, qPrintable(S)))
				{
					//Si hubo un error en la lectura, restablecemos el valor anterior
					field->set1(i, oldValue.getString() );
					QMessageBox::warning( this, tr("Warning"), tr("Error on row:")+S.setNum(i));
					qDebug("Error leyendo campo en fila %d\n", i);
				}

			}//for

	}//if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
}

void MFieldEditor::accept()
{

	//Simulamos que se ha pulsado el boton Aplicar
	Ui.buttonBox->button(QDialogButtonBox::Apply)->click();

	QDialog::accept();
}


void MFieldEditor::on_contextMenuFieldEditor(QPoint pos)
{
	//Miramos si se ha pulsado sobre algun item valido
    QTableWidgetItem *item = Ui.table->itemAt(pos);
	if (!item || current_mfield == NULL)
		return;

    int row = Ui.table->row(item);
    //int column = Ui.table->column(item);

    //Leemos el tipo de este campo
	SoType tipo=current_mfield->getTypeId();
	const char*nombre_tipo = tipo.getName();  

	//Miramos si hay algun ayudante para este tipo...

	//Edicion de cualquier campo tipo SoMFBool
	if (!strcmp(nombre_tipo, "MFBool") )
	{
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

	//Edicion de cualquier campo tipo SoMFEnum
	//Edicion de cualquier campo tipo SoMFBitMask
	else if (!strcmp(nombre_tipo, "MFEnum") ||
		!strcmp(nombre_tipo, "MFBitMask") )
	{
		//Convertimos el tipo de field
		SoMFEnum *soMFEnum= (SoMFEnum *)current_mfield;

		//Preparamos un menu flotante 
		QMenu popm(this);

		SbName nombre;
		int idx;
		//Probamos todos los indices y creamos una accion por cada
		for (idx=0; idx < soMFEnum->getNumEnums(); idx++)
		{
			soMFEnum->getEnum(idx, nombre);
			QAction *acc = new QAction(nombre.getString(), this);
			popm.addAction(acc);
		}

		//Mostramos el menu flotante y recogemos la opcion escogida
		QAction *acc = popm.exec(QCursor::pos());

		//Comprobamos que se ha seleccionado una opción válida.
		if (!acc)
			return;

		item->setText(acc->text());
	}

	//Edicion de cualquier campo tipo SoMFColor
	else if (!strcmp(nombre_tipo, "MFColor"))
	{
		//Lo convertimos en valores RGB y en un QColor
		QColor c (int(255*Ui.table->item(row,0)->text().toFloat()),
			      int(255*Ui.table->item(row,1)->text().toFloat()),
			      int(255*Ui.table->item(row,2)->text().toFloat()) );

		//Solicitamos un color mediante QColorDialog
		c=QColorDialog::getColor(c, this);
		if (c.isValid() )
		{           
			QString S;
			//Modificamos la tabla
            Ui.table->item(row,0)->setText(S.setNum(c.red()/255.0, 'g',2));
            Ui.table->item(row,1)->setText(S.setNum(c.green()/255.0, 'g',2));
            Ui.table->item(row,2)->setText(S.setNum(c.blue()/255.0, 'g',2));
		}
	}

	else
    {
      //No hay ayudante para este campo
      return;
    }

}//void MFieldEditor::on_contextMenuFieldEditor(QPoint pos)

