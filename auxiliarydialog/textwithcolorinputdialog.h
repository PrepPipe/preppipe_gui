#ifndef TEXTWITHCOLORINPUTDIALOG_H
#define TEXTWITHCOLORINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class TextWithColorInputDialog;
}

class TextWithColorInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextWithColorInputDialog(QColor color, QString text, QWidget *parent = nullptr);
    ~TextWithColorInputDialog();

    QColor getColor() {return curColor;}
    QString getText();

private slots:
    void colorChangeRequest();
    void colorChanged();

private:
    Ui::TextWithColorInputDialog *ui;
    QColor curColor;
};

#endif // TEXTWITHCOLORINPUTDIALOG_H
