
#include <mfield_editor.h>
#include <QLabel>
#include <QTableWidget>
#include <QColorDialog>
#include <QMessageBox>
#include <Inventor/fields/SoFields.h>


MFieldEditor::MFieldEditor(SoMField *field, QWidget *p, Qt::WindowFlags f) : QDialog(p,f)
{
	QString S;

	Ui.setupUi(this);

	//Salvamos una copia del puntero
	current_mfield = field;

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

			/* Esto tambien funciona, pero talvez es menos eficiente
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
		   //TODO ¿hay que liberar luego los items?
        }      
    }

    //Tratamiento de campos SoMFFloat, SoMFUInt32, SoMFInt32
    else if (!strcmp(nombre_tipo, "MFFloat") || !strcmp(nombre_tipo, "MFUInt32") 
                                             || !strcmp(nombre_tipo, "MFInt32") )
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
        }      
    }

	else
	{
		qDebug("Falta soporte para tipo %s\n", nombre_tipo);
	}

}


void MFieldEditor::on_table_cellChanged(int row, int column)
{
    //Evitamos actualizar si el cambio no lo ha hecho el usuario
    if (Ui.table->item(row,column) != Ui.table->currentItem())
    {
		return;
    }
}


void MFieldEditor::on_buttonBox_clicked(QAbstractButton * button)
{
	if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
	{
	}//if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
}

void MFieldEditor::accept()
{
	QDialog::accept();
}

