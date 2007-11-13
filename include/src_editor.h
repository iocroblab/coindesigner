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

#include <QMessageBox>

#include "ui_src_view.h"
#include <cds_util.h>
#include <QDialogButtonBox>
#include <Inventor/nodes/SoSeparator.h>
#include <QSyntaxHighlighter>

class Highlighter : public QSyntaxHighlighter
 {
     Q_OBJECT

 private:
     struct HighlightingRule
     {
         QRegExp pattern;
         QTextCharFormat format;
     };
     QVector<HighlightingRule> highlightingRules;

	 QTextCharFormat keywordFormat;
     QTextCharFormat singleLineCommentFormat;
	 QTextCharFormat quotationFormat;

 public:
	 Highlighter(QTextDocument *parent = 0) : QSyntaxHighlighter(parent)
	 {
		 HighlightingRule rule;

		 //Palabras reservadas
		 SoTypeList tl;
		 keywordFormat.setForeground(Qt::darkBlue);
		 keywordFormat.setFontWeight(QFont::Bold);

		 //Recorre todos los tipos derivados de SoNode
		 SoType::getAllDerivedFrom(SoNode::getClassTypeId(), tl);
		 for (int j=0; j < tl.getLength(); j++) 
		 {
			 QString pattern ="\\b"+QString(tl[j].getName().getString())+"\\b";
			 rule.pattern = QRegExp(pattern);
			 rule.format = keywordFormat;
			 highlightingRules.append(rule);
		 }

		 //Comentarios con #
		 singleLineCommentFormat.setForeground(Qt::red);
		 rule.pattern = QRegExp("#[^\n]*");
		 rule.format = singleLineCommentFormat;
		 highlightingRules.append(rule);

		 //Cadenas de texto
		 quotationFormat.setForeground(Qt::darkGreen);
		 rule.pattern = QRegExp("\".*\"");
		 rule.format = quotationFormat;
		 highlightingRules.append(rule);

	 }

 protected:
     void highlightBlock(const QString &text)
	 {
		 foreach (HighlightingRule rule, highlightingRules) 
		 {
			 QRegExp expression(rule.pattern);
			 int index = text.indexOf(expression);
			 while (index >= 0) 
			 {
				 int length = expression.matchedLength();
				 setFormat(index, length, rule.format);
				 index = text.indexOf(expression, index + length);
			 }
		 }
		 setCurrentBlockState(0);

	 }

};

class SrcEditor : public QDialog
{
	Q_OBJECT
	Ui::src_view Ui;

	Highlighter hl;

public:
	SoSeparator *result;

	///Constructor
	SrcEditor (SoSeparator *scene, bool readOnly=false, QWidget *p=0, Qt::WindowFlags f=0) : QDialog(p, f)
	{
		Ui.setupUi(this);
		if (readOnly)
		{
			//Marcamos el editText como readonly y solo dejamos el boton cerrar
			Ui.textEdit->setReadOnly(true);
			Ui.buttonBox->setStandardButtons(QDialogButtonBox::Close);
		}
		else
		{
			//Añadimos el boton Test
			Ui.buttonBox->addButton(tr("Test"), QDialogButtonBox::ApplyRole);
		}

		hl.setDocument(Ui.textEdit->document());

		result=NULL;

		//Rellenamos el textEdit con la escena
		Ui.textEdit->setText(cds_export_string(scene));
	}

	private slots:
		void on_buttonBox_clicked(QAbstractButton * button)
		{
			if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
			{
				//Copia el src a un buffer de memoria
				QString src = Ui.textEdit->toPlainText();
				char *buf = new char[src.size()];
				memcpy(buf, src.toAscii(), src.size() );

				//Leemos la escena desde el buffer de memoria
				SoInput input;
				input.setBuffer(buf, src.size() ) ;
				result = SoDB::readAll(&input);

				//Liberamos la memoria de la escena 
				if(result)
				{
					result->ref();
					result->unref();
					result = NULL;

					QMessageBox::information(this, tr("Test"), tr("The scene is valid")); 
				}

				delete buf;
				qDebug("Apply=%p\n", result);

			}//if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)

		}

		void accept()
		{
			//Copia el src a un buffer de memoria
			QString src = Ui.textEdit->toPlainText();
			char *buf = new char[src.size()];
			memcpy(buf, src.toAscii(), src.size() );

			//Leemos la escena desde el buffer de memoria
			SoInput input;
			input.setBuffer(buf, src.size() ) ;
			result = SoDB::readAll(&input);
			delete buf;

			//Solo aceptamos el dialogo si se compiló correctamente
			if (result)
				QDialog::accept();
		}

}; //class SrcEditor

