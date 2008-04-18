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

#include "ui_src_view.h"
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
     QTextCharFormat defUseFormat;
     QTextCharFormat quotationFormat;

 public:
	 Highlighter(QTextDocument *parent = 0);

 protected:
     void highlightBlock(const QString &text);

};

class SrcEditor : public QDialog
{
	Q_OBJECT
	Ui::src_view Ui;

	Highlighter hl;

public:
	SoSeparator *result;

	///Constructor
	SrcEditor (SoSeparator *scene, bool readOnly=false, QWidget *p=0, Qt::WindowFlags f=0);

private slots:
	void on_enableHL_stateChanged(int state);
	void on_buttonBox_clicked(QAbstractButton * button);
	void on_textEdit_cursorPositionChanged();

	void accept();

}; //class SrcEditor

