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

#include "cds_config.h"
#include "IfFixer.h"
#include "IfReporter.h"

extern SoSeparator *root;
SoNode *ivfix_result = NULL;

//Aplica el algoritmo ivfix al nodo ivfix_result, dejando el resultado
//en el mismo nodo. El procedimiento llamante será el encargado de 
//haber dejado en ivfix_result un valor valido.
void ivfix_options::buttonOk_clicked()
{

    //Creamos un fixer para la escena y lo configuramos según 
    //los valores del dialogo.
    IfFixer fixer;
    fixer.setReportLevel(IfFixer::NONE, stderr);
    fixer.setStripFlag( strips_chkbox->isChecked() );
    fixer.setVertexPropertyFlag( vertexProperty_chkbox->isChecked() );
    fixer.setNormalFlag( normals_chkbox->isChecked() );
    fixer.setTextureCoordFlag( textureCoord_chkbox->isChecked() );
    
    //Aplicamos los algoritmos de ivfix y dejamos el resultado en ivfix_result
    ivfix_result = fixer.fix(ivfix_result);
    
}

