#include "filelistinputwidget.h"
#include "ui_filelistinputwidget.h"
#include "fileselectionwidget.h"
#include <QMimeData>
#include <QDragEnterEvent>
#include <QListWidgetItem>
#include <QFileDialog>

FileListInputWidget::FileListInputWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileListInputWidget)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    setDirectoryMode(isDirectoryMode);
    connect(ui->listWidget, &QListWidget::itemChanged, this, &FileListInputWidget::listChanged);
    connect(ui->addButton,      &QPushButton::clicked, this, &FileListInputWidget::itemAdd);
    connect(ui->removeButton,   &QPushButton::clicked, this, &FileListInputWidget::itemRemove);
    connect(ui->moveUpButton,   &QPushButton::clicked, this, &FileListInputWidget::itemMoveUp);
    connect(ui->moveDownButton, &QPushButton::clicked, this, &FileListInputWidget::itemMoveDown);
    // 嗯。。还是把上移下移给去掉吧，排版不好看
    ui->moveUpButton->hide();
    ui->moveDownButton->hide();
}

FileListInputWidget::~FileListInputWidget()
{
    delete ui;
}

void FileListInputWidget::setFieldName(const QString& name) {
    fieldName = name;
    ui->label->setText(name);
}

void FileListInputWidget::setDirectoryMode(bool v) {
    isDirectoryMode = v;
    verifyCB = FileSelectionWidget::getDefaultVerifier(v);
}

void FileListInputWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        QString path = e->mimeData()->urls().first().toLocalFile();
        if (!verifyCB || verifyCB(path)) {
            e->acceptProposedAction();
            return;
        }
    }
}

void FileListInputWidget::dropEvent(QDropEvent *event)
{
    for (const QUrl &url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        if (!verifyCB || verifyCB(path)) {
            addPath(path);
        }
    }
}

void FileListInputWidget::addPath(const QString& path)
{
    for (int i = 0, n = ui->listWidget->count(); i < n; ++i) {
        auto* curItem = ui->listWidget->item(i);
        if (curItem->text() == path) {
            return;
        }
    }
    // 然后再加
    QListWidgetItem* newItem = new QListWidgetItem(ui->listWidget);
    newItem->setText(path);
    newItem->setToolTip(path);
    ui->listWidget->addItem(newItem);
    lastAddedPath = path;
}

void FileListInputWidget::itemMoveUp()
{
    int currow = ui->listWidget->currentRow();
    if (currow > 0) {
        auto* item = ui->listWidget->takeItem(currow);
        ui->listWidget->insertItem(currow-1, item);
        ui->listWidget->setCurrentRow(currow-1);
    }
}
void FileListInputWidget::itemMoveDown()
{
    int currow = ui->listWidget->currentRow();
    if (currow >= 0 && currow + 1 < ui->listWidget->count()) {
        auto* item = ui->listWidget->takeItem(currow);
        ui->listWidget->insertItem(currow+1, item);
        ui->listWidget->setCurrentRow(currow+1);
    }
}
void FileListInputWidget::itemAdd()
{
    QString dialogTitle = tr(u8"请选择%1").arg(fieldName);
    QFileDialog* dialog = new QFileDialog(this, dialogTitle, lastAddedPath, filter);
    if (isDirectoryMode) {
        dialog->setFileMode(QFileDialog::Directory);
        dialog->setOption(QFileDialog::ShowDirsOnly, true);
    } else {
        dialog->setFileMode(isExistingOnly? QFileDialog::ExistingFile : QFileDialog::AnyFile);
    }
    connect(dialog, &QFileDialog::fileSelected, this, &FileListInputWidget::addPath);
    connect(dialog, &QFileDialog::finished, dialog, &QFileDialog::deleteLater);
    dialog->show();
}
void FileListInputWidget::itemRemove()
{
    int currow = ui->listWidget->currentRow();
    if (currow >= 0) {
        auto* item = ui->listWidget->item(currow);
        delete item;
        emit listChanged();
    }
}
QStringList FileListInputWidget::getCurrentList() const
{
    QStringList results;
    for (int i = 0, n = ui->listWidget->count(); i < n; ++i) {
        const auto* item = ui->listWidget->item(i);
        results.append(item->text());
    }
    return results;
}
