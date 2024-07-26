#include "textwithcolorinputdialog.h"
#include "ui_textwithcolorinputdialog.h"

#include <QColorDialog>

TextWithColorInputDialog::TextWithColorInputDialog(QColor color, QString text, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextWithColorInputDialog),
    curColor(color)
{
    ui->setupUi(this);
    ui->lineEdit->setText(text);
    colorChanged();
    connect(ui->colorPushButton, &QPushButton::clicked, this, &TextWithColorInputDialog::colorChangeRequest);
}

TextWithColorInputDialog::~TextWithColorInputDialog()
{
    delete ui;
}

void TextWithColorInputDialog::colorChangeRequest()
{
    QColor newColor = QColorDialog::getColor(curColor, this, tr(u8"请选择颜色"));
    if (curColor != newColor) {
        curColor = newColor;
        colorChanged();
    }
}

void TextWithColorInputDialog::colorChanged()
{
    ui->colorLabel->setText(curColor.name(QColor::HexRgb));

    QPalette palette = ui->colorPreviewLabel->palette();
    palette.setColor(ui->colorPreviewLabel->backgroundRole(), curColor);
    palette.setColor(ui->colorPreviewLabel->foregroundRole(), curColor);
    ui->colorPreviewLabel->setPalette(palette);
}

QString TextWithColorInputDialog::getText()
{
    return ui->lineEdit->text();
}
