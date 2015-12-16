#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QWidget>
#include <QVarLengthArray>
 
class QPaintEvent;
class QResizeEvent;
class QSize;
//class QWidget;

class LineNumberArea;

/// a class for the debug, g-code, and canon-line text displays
class TextArea : public QPlainTextEdit {
 Q_OBJECT

public:
    /// create a TextArea
    TextArea(QWidget *parent = 0);
    /// paintEvent
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    /// return desired width of number-area
    int lineNumberAreaWidth()  {
        int digits = 1;
        int max = qMax(1, blockCount());
        while (max >= 10) {
         max /= 10;
         ++digits;
        }
        int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
        return space;
    }
    void setCursor(unsigned int line);
    unsigned int getLineSize() { return lines.size(); }

public slots:
    /// add text-line to this TextArea
    void appendLine(QString l) {
        lines.append(l);
        set_text();
    }

protected:
    /// set text do be displayed (from lines[] member)
    void set_text() {
        QString text;
        for(int n=0;n<lines.size();n++) 
            text += lines[n]+"\n";
        setPlainText(text);
    }
    /// resize widget
    void resizeEvent(QResizeEvent *e)  {
        QPlainTextEdit::resizeEvent(e);
        QRect cr = contentsRect();
        lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    }

private slots:
    void updateLineNumberAreaWidth(int newBlockCount) {// (int /* newBlockCount */)
     setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    }

    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int dy);

private:
    QWidget *lineNumberArea;
    QVarLengthArray<QString> lines;
};

/// helper-class to show line-numbers in the left margin of TextArea
class LineNumberArea : public QWidget {
 Q_OBJECT

public:
    /// create LineNumberArea for the given TextArea
    LineNumberArea(TextArea* area) : QWidget(area) {
        textArea = area;
    }
    /// sizeHint
    QSize sizeHint() const {
        return QSize(textArea->lineNumberAreaWidth(), 0);
    }

protected:
    /// forward paintEvent to TextArea
    void paintEvent(QPaintEvent *event) {
        textArea->lineNumberAreaPaintEvent(event);
    }

private:
    TextArea *textArea;
};

#endif
