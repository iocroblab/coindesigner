/*
    This file is part of coindesigner. (https://github.com/jmespadero/coindesigner)

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


#include "src_editor.h"
#include <QMessageBox>
#include <QCheckBox>
#include <cds_util.h>
#include <QDialogButtonBox>


Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
	HighlightingRule rule;

	//Palabras reservadas
	SoTypeList tl;
	keywordFormat.setForeground(Qt::darkBlue);
	keywordFormat.setFontWeight(QFont::Bold);

	//Recorre todos los tipos derivados de SoNode, creando palabras reservadas
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

	//DEF y USE
	defUseFormat.setForeground(Qt::red);
	rule.pattern = QRegExp("\\bDEF\\s+[\\w+]*\\b");
	rule.format = defUseFormat;
	highlightingRules.append(rule);
	rule.pattern = QRegExp("\\bUSE\\s+[\\w+]*\\b");
	highlightingRules.append(rule);
	
	//Cadenas de texto
	quotationFormat.setForeground(Qt::darkGreen);
	rule.pattern = QRegExp("\".*\"");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

}

void Highlighter::highlightBlock(const QString &text)
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

SrcEditor::SrcEditor (SoSeparator *scene, bool readOnly, QWidget *p, Qt::WindowFlags f) : QDialog(p, f)
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
		//A�adimos el boton Test
		Ui.buttonBox->addButton(tr("Test"), QDialogButtonBox::ApplyRole);
	}

	//Rellenamos el textEdit con la escena
	char *src = cds_export_string(scene);
	Ui.textEdit->setText(src);

	//Inicializamos highlight dependiendo del tama�o del fuente
	Ui.enableHL->setChecked(strlen(src) < 50000);

	//hl.setDocument(Ui.textEdit->document());

	free (src);
	result=NULL;
} // SrcEditor::SrcEditor (SoSeparator *scene, bool readOnly, QWidget *p, Qt::WindowFlags f) : QDialog(p, f)


void SrcEditor::on_enableHL_stateChanged(int state)
{
	//qDebug("%s %d\n", __FUNCTION__, state);
	if (state == Qt::Checked)
		hl.setDocument(Ui.textEdit->document());
	else
		hl.setDocument(NULL);
}

void SrcEditor::on_buttonBox_clicked(QAbstractButton * button)
{
	if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
	{
		//Copia el src a un buffer de memoria
		QString src = Ui.textEdit->toPlainText();
		char *buf = new char[src.size()];
		memcpy(buf, src.toLatin1(), src.size() );

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

		delete[] buf;

	}//if (Ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)

}// void SrcEditor::on_buttonBox_clicked(QAbstractButton * button)

void SrcEditor::accept()
{
	//Copia el src a un buffer de memoria
	QString src = Ui.textEdit->toPlainText();
	char *buf = new char[src.size()];
	memcpy(buf, src.toLatin1(), src.size() );

	//Leemos la escena desde el buffer de memoria
	SoInput input;
	input.setBuffer(buf, src.size() ) ;
	result = SoDB::readAll(&input);
	delete[] buf;

	//Solo aceptamos el dialogo si se compil� correctamente
	if (result)
		QDialog::accept();
}// void SrcEditor::accept()


///Changes the current line label
void SrcEditor::on_textEdit_cursorPositionChanged()
{
	Ui.lineLabel->setText(tr("Line:")+QString::number(1+Ui.textEdit->textCursor().blockNumber()));
}// void SrcEditor::on_textEdit_cursorPositionChanged()

