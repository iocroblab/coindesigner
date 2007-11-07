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
#include <qmessagebox.h>

//Copia del field que estamos editando actualmente
SoSFMatrix *current_SFMatrix = NULL;


void matrix_editor::Aplicar_cambios_a_Matrix()
{           
    QString S;
    SbString oldValue;

    //Buscamos el SoMatrix que estamos editando
    SoSFMatrix *field = current_SFMatrix;

    //Sacamos una copia del field actual, por si hay errores
    field->get(oldValue);

   //Rellenamos la matriz
   SbMatrix m;
   bool ok;

   for (int i=0; i<4; i++)
     for (int j=0; j<4; j++)
     {
        //Leemos el valor de esta celda
        S = table3->text(i,j);
        m[i][j] = S.toFloat(&ok);

        //Comprobamos que no hubo error
        if (!ok)
        {
            QMessageBox::warning( this, tr("Aviso"), S,
            QMessageBox::Cancel, QMessageBox::NoButton, QMessageBox::NoButton);

           //Restauramos el valor inicial de la matriz
           field->set(oldValue.getString() );
           cargarDatos(field);
           return;
        }

     }      

     //Almacenamos los valores en el field
     field->setValue(m);

 } //void matrix_editor::Aplicar_cambios_a_Matrix()


//Rellenamos la tabla con el contenido del campo field
void matrix_editor::cargarDatos(SoSFMatrix* field)
{
    //Salvamos una copia del puntero al campo
    current_SFMatrix = field;

   //Ahora, dependiendo del tipo le damos el numero de columnas
   //y rellenamos la tabla

   //Rellenamos la tabla
   QString S;
   SbMatrix m = field->getValue();

   for (int i=0; i<4; i++)
     for (int j=0; j<4; j++)
     {
        table3->setText(i,j,S.setNum(m[i][j],'g',6));    
     }      

  //Por ultimo, ajustamos el ancho de las celdas
  table3->setColumnWidth(0,75);
  table3->setColumnWidth(1,75);
  table3->setColumnWidth(2,75);
  table3->setColumnWidth(3,75);

} // void matrix_editor::cargarDatos(SoSFMatrix* field)


