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

#include "ivfix_options.h"
#include "cds_util.h"
#include "ivfix/IfFixer.h"
#include "ivfix/IfReporter.h"

ivfix_options::ivfix_options (SoNode *root, QWidget *p, Qt::WindowFlags f) : QDialog(p, f)
{
	assert (root != NULL);
	input = root;
	output = NULL;
	Ui.setupUi(this);
}

//Aplica el algoritmo ivfix al nodo ivfix_result, dejando el resultado
//en el mismo nodo. El procedimiento llamante será el encargado de 
//haber dejado en ivfix_result un valor valido.
void ivfix_options::accept()
{
    //Creamos un fixer para la escena y lo configuramos según 
    //los valores del dialogo.
    IfFixer fixer;
    fixer.setReportLevel(IfFixer::NONE, stderr);
    fixer.setStripFlag( Ui.strips_chkbox->isChecked() );
    fixer.setVertexPropertyFlag( Ui.vertexProperty_chkbox->isChecked() );
    fixer.setNormalFlag( Ui.normals_chkbox->isChecked() );
    fixer.setTextureCoordFlag( Ui.textureCoord_chkbox->isChecked() );
    
    //Aplicamos los algoritmos de ivfix y dejamos el resultado en ivfix_result
    output = fixer.fix(input);

	QDialog::accept();
}

