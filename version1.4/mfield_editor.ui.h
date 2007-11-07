/*
    This file is part of coindesigner.

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

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include "cds_config.h"
#include <qcolordialog.h>
#include <qmessagebox.h>

//Copia del field que estamos editando actualmente
SoMField *current_mfield = NULL;


//Si ha cambiado un valor de la tabla, miramos si es necesario
//hacerla crecer.
void mfield_editor::table3_valueChanged( int fila, int /*columna*/ )
{  
   QString S;

    //Si cambio algun valor de la última fila, aumentamos la tabla
    if (fila+1==table3->numRows())
    {
        table3->setNumRows(table3->numRows()+1);
        table3->verticalHeader()->setLabel(fila+1, S.setNum(fila+1) );

    }         

    //Actualizamos el label con el numero de elementos
    textLabel2->setText(S.setNum(table3->numRows()-1));
}

void mfield_editor::Aplicar_cambios_a_MField()
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
    field->setNum(table3->numRows()-1);

    //Miramos cuantas columnas tiene la tabla
    int numComp = table3->numCols();

    //Tratamiento de los campos SoMFString
    if (!strcmp(nombre_tipo, "MFString") )
    {
        //Convertimos el campo a SoMFString
        SoMFString *soMFString = (SoMFString *)field;

        //Recorremos las filas de la tabla
        for(int i=0;i<table3->numRows()-1;i++)
        {
           //Extraemos el contenido de las celdas
           S = table3->text(i,0);

           //Convertimos en ascii y en SbString
           const char *txt=S.ascii();

           //Aplicamos el valor al campo
           soMFString->set1Value(i, SbString(txt));

       } // for(int i=0;i<table3->numRows()-1;i++)
    }
    else

    //Recorremos las filas de la tabla
    for(int i=0;i<table3->numRows()-1;i++)
    {
       //Sacamos una copia del field actual, por si hay errores
       field->get1(i, oldValue);
       //fprintf(stderr, "old: %s\n", oldValue.getString());

       //Extraemos el contenido de las celdas
       S = table3->text(i,0);

       //Concatenamos todas las celdas
       for (int j=1; j < numComp; j++)
       {
           S.append(' ');
           const char *txt2 = table3->text(i,j).ascii();
           S.append(txt2);
       }

       //Convertimos en ascii
       const char *txt=S.ascii();

       //Leemos la cadena mediante el parser de openInventor
       //fprintf(stderr, "Introduciendo: %s\n", txt);
       if (!field->set1(i, txt))
       {
          //Si hubo un error en la lectura, restablecemos el valor
          field->set1(i, oldValue.getString() );
          fprintf (stderr, tr("Error leyendo campo %s en fila %d\n"), 
                           nombre_tipo, i);
       }

   }//for

 } //void mfield_editor::Almacenar_datos_salir()


//Esta función se encarga de mirar si tenemos programado algún ayudante
//que nos permita editar fácilmente el campo. Los ayudantes son 
//dialogos del tipo QFileDialog, QColorDialog o PopupMenus
//El resto de campos se dejan para la funcion table3_valueChanged()
void mfield_editor::table3_clicked(int fila, int columna)
{
   QString S;

   //Buscamos el SoMField que estamos editando
   SoMField *field = current_mfield;

    //Leemos el tipo de este campo
    SoType tipo=field->getTypeId();
    const char*nombre_tipo = tipo.getName();  

    //Miramos si hay algun ayudante para editar este tipo...

    //Tratamiento de campos SoMFColor
    if (!strcmp(nombre_tipo, "MFColor"))
    {
        //Convertimos el tipo de field
        SoMFColor* color=(SoMFColor *)field;
        QColor c;
       //Lo convertimos en valores RGB y en un QColor
       if ((*color).getNum()!=0)
        {    
       const float*rgb = (*color)[fila].getValue();
       QColor c( int(255*rgb[0]), int(255*rgb[1]), int(255*rgb[2]) );
       }
       //Solicitamos un color mediante QColorDialog
       c=QColorDialog::getColor(c);
       if (c.isValid() )
       {           
          //Modificamos el field
          //Quitar esta línea para evitar autoaplicar los cambios
          //color->set1Value(fila,c.red()/255.0,c.green()/255.0,c.blue()/255.0);

          //Actualizamos el contenido de la tabla
         table3->setText(fila,0,S.setNum(c.red()/255.0,'g',2));   
         table3->setText(fila,1,S.setNum(c.green()/255.0,'g',2)); 
         table3->setText(fila,2,S.setNum(c.blue()/255.0,'g',2));    
     } 
     
         
    }
    else

    {
      //No hay ayudante para este campo
      return;
    }

    //Hacemos la revisión de la ultima fila, etc...
    table3_valueChanged(fila, columna);

} // void mfield_editor::table3_clicked(int fila, int columna)

//Rellenamos la tabla con el contenido del campo field
void mfield_editor::cargarDatos(SoMField* field)
{
    QString S;

    //Salvamos una copia del puntero al campo
    current_mfield = field;

    //Leemos el tipo de este campo
    SoType tipo=field->getTypeId();
    const char*nombre_tipo = tipo.getName();  

    //Primero le damos el numero de filas adecuado
    //Dejamos una fila libre para añadir nuevos valores
    int numFilas=field->getNum();
    table3->setNumRows(numFilas+1);
    textLabel2->setText(S.setNum(numFilas));

   //Cambiamos todas las cabeceras, para que tengan base 0
   if (numFilas < 2000)
   {
      QHeader *verticalHeader = table3->verticalHeader();
      for (int i=0; i<numFilas+1; i++)
          verticalHeader->setLabel(i,S.setNum(i) );
   }
   else
   {
       QMessageBox::warning( this, tr("Aviso"), tr("Tabla demasiado grande. Los indices comenzaran en 1 en lugar de 0"),
             QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);

   }

   //Ahora, dependiendo del tipo le damos el numero de columnas
   //y rellenamos la tabla

    //Tratamiento de campos SoMFColor
    if (!strcmp(nombre_tipo, "MFColor"))
    {
        //Columnas para R, G, B
        table3->setNumCols(3);
        table3->horizontalHeader()->setLabel( 0,"R" );
        table3->horizontalHeader()->setLabel( 1,"G" );
        table3->horizontalHeader()->setLabel( 2,"B" );

        SoMFColor* soMFColor=(SoMFColor *)field;

        //Rellenamos la tabla
        SbColor color;
        for (int i=0; i<numFilas; i++)
        {
           color=(*soMFColor)[i];       
           table3->setText(i,0,S.setNum(color[0],'g',2));   
           table3->setText(i,1,S.setNum(color[1],'g',2)); 
           table3->setText(i,2,S.setNum(color[2],'g',2));    
        }      
    }
    else

    //Tratamiento de campos SoMFVec2f
    if (!strcmp(nombre_tipo, "MFVec2f"))
    {
        //Columnas para X, Y
        table3->setNumCols(2);
        table3->horizontalHeader()->setLabel( 0,"X" );
        table3->horizontalHeader()->setLabel( 1,"Y" );

        //Convertimos el field
        SoMFVec2f* soMFVec2f=(SoMFVec2f *)field;

        //Rellenamos la tabla
        SbVec2f xy;
        for (int i=0; i<numFilas; i++)
        {
           xy=(*soMFVec2f)[i];       
           table3->setText(i,0,S.setNum(xy[0],'g',6));   
           table3->setText(i,1,S.setNum(xy[1],'g',6)); 
        }      
    }
    else 

    //Tratamiento de campos SoMFVec3f
    if (!strcmp(nombre_tipo, "MFVec3f"))
    {
        //Columnas para X, Y, Z
        table3->setNumCols(3);
        table3->horizontalHeader()->setLabel( 0,"X" );
        table3->horizontalHeader()->setLabel( 1,"Y" );
        table3->horizontalHeader()->setLabel( 2,"Z" );

        //Convertimos el field
        SoMFVec3f* soMFVec3f=(SoMFVec3f *)field;

	/*
        //Rellenamos la tabla
        SbVec3f xyz;
        for (int i=0; i<numFilas; i++)
        {
           xyz=(*soMFVec3f)[i];       
           table3->setText(i,0,S.setNum(xyz[0],'g',6));   
           table3->setText(i,1,S.setNum(xyz[1],'g',6)); 
           table3->setText(i,2,S.setNum(xyz[2],'g',6));    
        }      
	*/

        //Rellenamos la tabla
        const SbVec3f *values = soMFVec3f->getValues(0);
	float x, y, z;
        for (int i=0; i<numFilas; i++)
        {
           values[i].getValue(x,y,z);       
           table3->setText(i,0,S.setNum(x,'g',6));   
           table3->setText(i,1,S.setNum(y,'g',6)); 
           table3->setText(i,2,S.setNum(z,'g',6));    
        }      
    }
    else

    //Tratamiento de campos SoMFVec4f
    if (!strcmp(nombre_tipo, "MFVec4f"))
    {
        //Columnas para X, Y, Z, T
        table3->setNumCols(4);
        table3->horizontalHeader()->setLabel( 0,"0" );
        table3->horizontalHeader()->setLabel( 1,"1" );
        table3->horizontalHeader()->setLabel( 2,"2" );
        table3->horizontalHeader()->setLabel( 3,"3" );

        //Convertimos el field
        SoMFVec4f* soMFVec4f=(SoMFVec4f *)field;

        //Rellenamos la tabla
        SbVec4f xyzt;
        for (int i=0; i<numFilas; i++)
        {
           xyzt=(*soMFVec4f)[i];       
           table3->setText(i,0,S.setNum(xyzt[0],'g',6));   
           table3->setText(i,1,S.setNum(xyzt[1],'g',6)); 
           table3->setText(i,2,S.setNum(xyzt[2],'g',6));    
           table3->setText(i,3,S.setNum(xyzt[3],'g',6));    
        }      
    }
    else

    //Tratamiento de campos SoMFInt32
    if (!strcmp(nombre_tipo, "MFInt32") )
    {
        //Hace falta una columna
        table3->setNumCols(1);
        table3->horizontalHeader()->setLabel( 0,tr("Valor") );

        //Rellenamos la tabla
        SoMFInt32* soMFInt32=(SoMFInt32 *)field;
        const int32_t *values = soMFInt32->getValues(0);
        for (int i=0; i<numFilas; i++)
        {
           table3->setText(i,0,S.setNum(values[i]));   
        }      
    }
    else

    //Tratamiento de campos SoMFFloat, SoMFUInt32
    if (!strcmp(nombre_tipo, "MFFloat") ||
        !strcmp(nombre_tipo, "MFUInt32") )
    {
        //Hace falta una columna
        table3->setNumCols(1);
        table3->horizontalHeader()->setLabel( 0,tr("Valor") );

        //Rellenamos la tabla usando la funcion get1 de coin
        SbString valueString;
        for (int i=0; i<numFilas; i++)
        {
           field->get1(i, valueString);
           table3->setText(i,0,valueString.getString() );
        }      

    }
    else

    //Tratamiento de campos SoMFString
    if ( !strcmp(nombre_tipo, "MFString") )
    {
        //Hace falta una columna
        table3->setNumCols(1);
        table3->horizontalHeader()->setLabel( 0,tr("Valor") );

        //Rellenamos la tabla usando la funcion get1 de coin
        SbString valueString;
        for (int i=0; i<numFilas; i++)
        {
           field->get1(i, valueString);
           valueString.deleteSubString(0,0);
           int ln = valueString.getLength();
           valueString.deleteSubString(ln-1,ln-1);
           table3->setText(i,0,valueString.getString() );
        }      

    }
    else

    {
        __chivato__
       fprintf (stderr, "Falta soporte para tipo %s\n", nombre_tipo);
    }


} // void mfield_editor::cargarDatos(SoMField* field)


