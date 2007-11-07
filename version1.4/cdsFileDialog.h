
#include <qfiledialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <Inventor/SoOutput.h>

//Dialogo para seleccionar un fichero, con opciones para extenderlo
class cdsFileDialog : public QFileDialog
{

Q_OBJECT

public:
	//Opciones para extender el dialogo
	enum Options {saveNormal=1, saveBinary=2, saveCompress=4};
	QCheckBox *normal;
	QCheckBox *binary;
	QCheckBox *compress;

  //Constructor
	cdsFileDialog( QWidget* parent, const QString & caption = QString::null, int option = 0 ) 
	  : QFileDialog(parent, NULL )
  {
	  //Titulo del dialogo
	  setCaption(caption);

	  //Revisamos las opciones
	  if (option & cdsFileDialog::saveNormal)
	  {
		  normal = new QCheckBox(tr("Calcular normales"), this);
		  addWidgets( NULL, (QWidget*)normal, NULL );
	  }

	  if (option & cdsFileDialog::saveBinary)
	  {
		  binary = new QCheckBox(tr("Salvar en modo binario"), this);
		  addWidgets( NULL, (QWidget*)binary, NULL );
	  }

	  if (option & cdsFileDialog::saveCompress)
	  {
		compress = new QCheckBox(tr("Salvar en modo comprimido"), this);
		addWidgets( NULL, (QWidget*)compress, NULL );

		//Miramos si existe mecanismo de compresion valido
		SoOutput output;
		compress->setEnabled(output.setCompression(SbName("GZIP"), 0.9f) );
	  }
  }
};
