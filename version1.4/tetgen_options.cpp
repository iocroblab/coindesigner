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


#include <tetgen_options.h>
#include <cds_util.h>
#include "cds_config_form.h"

/*
 *  Constructs a tetgen_options as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
tetgen_options::tetgen_options( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
	if ( !name )
		setName( "tetgen_options" );
	setSizeGripEnabled( false );
	setModal( TRUE );
	tetgen_optionsLayout = new QVBoxLayout( this, 11, 6, "tetgen_optionsLayout"); 

	layout8 = new QVBoxLayout( 0, 0, 6, "layout8"); 

	layout7 = new QGridLayout( 0, 1, 1, 0, 6, "layout7"); 

	//Opcion -q
	textLabel_q = new QLabel( this, "textLabel_q" );
	layout7->addWidget( textLabel_q, 0, 0 );

	spinBox_q = new floatSpinBox(this, "spinBox_q" );
	spinBox_q->setButtonSymbols( QSpinBox::UpDownArrows );
	spinBox_q->setMaxValue( 500 );
	spinBox_q->setMinValue( 1 );
	spinBox_q->setLineStep( 1 );
	spinBox_q->setValue( 20 );
	layout7->addWidget( spinBox_q, 0, 1 );

	//Opcion -a
	textLabel_a = new QLabel( this, "textLabel_a" );
	layout7->addWidget( textLabel_a, 1, 0 );

	spinBox_a = new floatSpinBox(this, "spinBox_a" );
	spinBox_a->setButtonSymbols( QSpinBox::UpDownArrows );
	spinBox_a->setMaxValue( 9999999 );
	spinBox_a->setMinValue( 1 );
	spinBox_a->setLineStep( 1 );
	spinBox_a->setValue( spinBox_a->maxValue() );
	layout7->addWidget( spinBox_a, 1, 1 );

	layout8->addLayout( layout7 );

	tetgen_optionsLayout->addLayout( layout8 );

	layout10 = new QHBoxLayout( 0, 0, 6, "layout10"); 

	buttonOk = new QPushButton( this, "buttonOk" );
	buttonOk->setAutoDefault( TRUE );
	buttonOk->setDefault( TRUE );
	layout10->addWidget( buttonOk );
	spacer2 = new QSpacerItem( 60, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	layout10->addItem( spacer2 );

	buttonCancel = new QPushButton( this, "buttonCancel" );
	buttonCancel->setAutoDefault( TRUE );
	layout10->addWidget( buttonCancel );
	tetgen_optionsLayout->addLayout( layout10 );
	languageChange();
	resize( QSize(386, 193).expandedTo(minimumSizeHint()) );
	clearWState( WState_Polished );

	// signals and slots connections
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( buttonOk_clicked() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
tetgen_options::~tetgen_options()
{
    // no need to delete child widgets, Qt does it all for us
}


/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void tetgen_options::languageChange()
{
    setCaption( tr( "Opciones de tetgen" ) );
    textLabel_q->setText( tr( "Minimum radius-edge ratio (-q)" ) );
    textLabel_a->setText( tr( "Maximum tetrahedron volume constraint (-a)" ) );

 //   textLabel1_2->setText( tr( "Quadric weighting policy" ) );
 //   comboBox_W->clear();
 //   comboBox_W->insertItem( tr( "uniform" ) );
 //   comboBox_W->insertItem( tr( "area" ) );
 //   comboBox_W->insertItem( tr( "angle" ) );
 //   comboBox_W->setCurrentItem( 1 );
 //   textLabel1_2_2->setText( tr( "Target number of faces" ) );
 //   chkbox_F->setText( tr( "Use face contraction instead of edge contraction" ) );
    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setText( tr( "&Cancelar" ) );
    buttonCancel->setAccel( QKeySequence( tr( "Alt+C" ) ) );
}


SoNode *tetgen_result = NULL;

//Aplica el algoritmo tetgen al nodo tetgen_result, dejando el resultado
//en el mismo nodo. El procedimiento llamante será el encargado de 
//haber dejado en tetgen_result un valor valido.
void tetgen_options::buttonOk_clicked ()
{

  //Aplicamos los algoritmos de tetgen y dejamos el resultado en tetgen_result


  if (!settings.readEntry("/coindesigner/tetgen_app"))
  {
    QString S;
    S = tr ("No se ha encontrado la herramienta TETGEN");
    QMessageBox::warning (this, tr ("Error"), S, QMessageBox::Ok,
			  QMessageBox::NoButton, QMessageBox::NoButton);
    tetgen_result = NULL;
    return;
  }
  else
  {
    //Construimos un proceso con las opciones dadas.
    QProcess *proc = new QProcess (this);
    proc->addArgument (settings.readEntry("/coindesigner/tetgen_app"));
	QString tetgen_args;
	tetgen_args.sprintf("-pOq%.1fa%.1f", spinBox_q->fvalue(), spinBox_a->fvalue() );
    proc->addArgument (tetgen_args);
/*
    proc->addArgument ("-O");
    proc->addArgument (QString::number (spinBox_q->currentItem ()));
    proc->addArgument ("-W");
    proc->addArgument (QString::number (comboBox_W->currentItem ()));
    proc->addArgument ("-t");
    proc->addArgument (QString::number (spinBox_t->value ()));
    proc->addArgument ("-M");
    proc->addArgument ("iv");
    if (chkbox_F->isChecked ())
      proc->addArgument ("-F");
    proc->addArgument ("-o");
    proc->addArgument ("tetgen_out.iv");
    proc->addArgument ("-q");
*/
    proc->addArgument ("tetgen_in.off");

    //Lanzamos el proceso
    if (!proc->launch (""))
    {
      // error handling
      QString S;
      S = tr ("Error ejecutando: ");
      S += proc->arguments ().join (" ");
      QMessageBox::warning (this, tr ("Error"), S, QMessageBox::Ok,
			    QMessageBox::NoButton, QMessageBox::NoButton);

	  //Salimos
	  QFile::remove("tetgen_out.iv");
	  delete proc;
      return;
    }

    while (proc->isRunning ())
    {
#ifdef _WIN32
      Sleep (1000);
#else
      sleep (1);
#endif
    }


    //Leemos el resultado en tetgen_result
    tetgen_result = import_tetgen("tetgen_in.1.node");

    //Comprobamos que se la leido algo correcto
    if (!tetgen_result)
    {
      QString S;
      S = tr ("la salida de tetgen no es valida");
      S += proc->arguments ().join (" ");
      QMessageBox::warning (this, tr ("Error"), S, QMessageBox::Ok,
			    QMessageBox::NoButton, QMessageBox::NoButton);
    }

    //Borramos el fichero de entrada
    QFile::remove("tetgen_out.iv");
    delete proc;
  }

}
