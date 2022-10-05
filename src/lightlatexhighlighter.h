/***************************************************************************
 *   copyright       : (C) 2003-2022 by Pascal Brachet                     *
 *   https://www.xm1math.net/texmaker/                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   @license GPL-2.0+ <https://spdx.org/licenses/GPL-2.0+.html>           *
 ***************************************************************************/


#ifndef LIGHTLATEXHIGHLIGHTER_H
#define LIGHTLATEXHIGHLIGHTER_H


#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>
#include <QColor>
#include <QTextBlockUserData>

class QTextDocument;

class LightLatexHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    LightLatexHighlighter(QTextDocument *parent = 0);
    ~LightLatexHighlighter();
    QColor ColorStandard, ColorComment, ColorMath, ColorCommand, ColorKeyword, ColorVerbatim, ColorTodo, ColorKeywordGraphic, ColorNumberGraphic;
    QStringList KeyWords, KeyWordsGraphic, KeyWordsGraphicBis;
public slots:
void setColors(QList<QColor> colors);
void setModeGraphic(bool m);
private :
bool isWordSeparator(QChar c) const;
bool isSpace(QChar c) const;
bool isGraphic;
protected:
void highlightBlock(const QString &text);
};


#endif
