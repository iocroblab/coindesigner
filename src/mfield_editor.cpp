
#include <mfield_editor.h>
#include <QLabel>
#include <QTableWidget>
#include <QColordialog>
#include <QMessagebox>
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
        Ui.table->setVerticalHeaderLabels(hh);

        SoMFColor* soMFColor=(SoMFColor *)field;

        //Rellenamos la tabla
        SbColor color;
        for (int i=0; i<numFilas; i++)
        {
           color=(*soMFColor)[i];       
           Ui.table->item(i,0)->setText(S.setNum(color[0],'g',2));   
           Ui.table->item(i,1)->setText(S.setNum(color[1],'g',2));   
           Ui.table->item(i,2)->setText(S.setNum(color[2],'g',2));   
        }      
    }

    //Tratamiento de campos SoMFVec3f
    else if (!strcmp(nombre_tipo, "MFVec3f"))
    {
        //Columnas para R, G, B
        Ui.table->setColumnCount(3);
		hh << "X" << "Y" << "Z";
        Ui.table->setVerticalHeaderLabels(hh);

        SoMFVec3f* soMFVec3f=(SoMFVec3f *)field;

        //Rellenamos la tabla
        const SbVec3f *values = soMFVec3f->getValues(0);
		float x, y, z;
        //for (int i=0; i<numFilas; i++)
        //{
        //   values[i].getValue(x,y,z);       
        //   Ui.table->item(i,0)->setText(S.setNum(x,'g',6));   
        //   Ui.table->item(i,1)->setText(S.setNum(y,'g',6)); 
        //   Ui.table->item(i,2)->setText(S.setNum(z,'g',6));    
        //}      

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

