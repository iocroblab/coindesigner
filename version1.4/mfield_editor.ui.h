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
void mfield_editor::Ui.table_valueChanged( int fila, int /*columna*/ )
{  
   QString S;

    //Si cambio algun valor de la última fila, aumentamos la tabla
    if (fila+1==Ui.table->numRows())
    {
        Ui.table->setNumRows(Ui.table->numRows()+1);
        Ui.table->verticalHeader()->setLabel(fila+1, S.setNum(fila+1) );

    }         

    //Actualizamos el label con el numero de elementos
    textLabel2->setText(S.setNum(Ui.table->numRows()-1));
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
    field->setNum(Ui.table->numRows()-1);

    //Miramos cuantas columnas tiene la tabla
    int numComp = Ui.table->numCols();

    //Tratamiento de los campos SoMFString
    if (!strcmp(nombre_tipo, "MFString") )
    {
        //Convertimos el campo a SoMFString
        SoMFString *soMFString = (SoMFString *)field;

        //Recorremos las filas de la tabla
        for(int i=0;i<Ui.table->numRows()-1;i++)
        {
           //Extraemos el contenido de las celdas
           S = Ui.table->text(i,0);

           //Convertimos en ascii y en SbString
           const char *txt=S.ascii();

           //Aplicamos el valor al campo
           soMFString->set1Value(i, SbString(txt));

       } // for(int i=0;i<Ui.table->numRows()-1;i++)
    }
    else

    //Recorremos las filas de la tabla
    for(int i=0;i<Ui.table->numRows()-1;i++)
    {
       //Sacamos una copia del field actual, por si hay errores
       field->get1(i, oldValue);
       //fprintf(stderr, "old: %s\n", oldValue.getString());

       //Extraemos el contenido de las celdas
       S = Ui.table->text(i,0);

       //Concatenamos todas las celdas
       for (int j=1; j < numComp; j++)
       {
           S.append(' ');
           const char *txt2 = Ui.table->text(i,j).ascii();
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
//El resto de campos se dejan para la funcion Ui.table_valueChanged()
void mfield_editor::Ui.table_clicked(int fila, int columna)
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
         Ui.table->setText(fila,0,S.setNum(c.red()/255.0,'g',2));   
         Ui.table->setText(fila,1,S.setNum(c.green()/255.0,'g',2)); 
         Ui.table->setText(fila,2,S.setNum(c.blue()/255.0,'g',2));    
     } 
     
         
    }
    else

    {
      //No hay ayudante para este campo
      return;
    }

    //Hacemos la revisión de la ultima fila, etc...
    Ui.table_valueChanged(fila, columna);

} // void mfield_editor::Ui.table_clicked(int fila, int columna)


