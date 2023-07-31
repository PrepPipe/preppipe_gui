#include "fileselectionwidget.h"
#include "ui_fileselectionwidget.h"
#include <QMimeData>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileInfo>

namespace {

bool defaultFileChecker(const QString& path){
    const QFileInfo info(path);
    if (info.exists() and info.isFile()) {
        return true;
    }
    return false;
}
bool defaultDirectoryChecker(const QString& path) {
    const QFileInfo info(path);
    if (info.exists() and info.isDir()) {
        return true;
    }
    return false;
}

} // end anonymous namespace

FileSelectionWidget::FileSelectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileSelectionWidget)
{
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::clicked, this, &FileSelectionWidget::requestOpenDialog);
    setAcceptDrops(true);
    setDirectoryMode(isDirectoryMode);
}

void FileSelectionWidget::setDirectoryMode(bool v) {
    isDirectoryMode = v;
    if (isDirectoryMode) {
        verifyCB = defaultDirectoryChecker;
    } else {
        verifyCB = defaultFileChecker;
    }
}

std::function<bool(const QString&)> FileSelectionWidget::getDefaultVerifier(bool isDirectoryMode)
{
    if (isDirectoryMode) {
        return defaultDirectoryChecker;
    } else {
        return defaultFileChecker;
    }
}

void FileSelectionWidget::updateLabelText()
{
    if (currentPath.length() == 0) {
        ui->pathLabel->setText(tr(u8"(%1: 未选择)").arg(fieldName));
    } else {
        ui->pathLabel->setText(currentPath);
    }
}

FileSelectionWidget::~FileSelectionWidget()
{
    delete ui;
}

void FileSelectionWidget::requestOpenDialog()
{
    QString dialogTitle = tr(u8"请选择%1").arg(fieldName);
    QFileDialog* dialog = new QFileDialog(this, dialogTitle, currentPath, filter);
    if (isDirectoryMode) {
        dialog->setFileMode(QFileDialog::Directory);
        dialog->setOption(QFileDialog::ShowDirsOnly, true);
    } else {
        dialog->setFileMode(isExistingOnly? QFileDialog::ExistingFile : QFileDialog::AnyFile);
    }
    connect(dialog, &QFileDialog::fileSelected, this, &FileSelectionWidget::setCurrentPath);
    connect(dialog, &QFileDialog::finished, dialog, &QFileDialog::deleteLater);
    dialog->show();
}

void FileSelectionWidget::setCurrentPath(const QString& newpath)
{
    currentPath = newpath;
    updateLabelText();
    emit filePathUpdated(currentPath);
}

void FileSelectionWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        QString path = e->mimeData()->urls().first().toLocalFile();
        if (!verifyCB || verifyCB(path)) {
            e->acceptProposedAction();
        }
    }
}

void FileSelectionWidget::dropEvent(QDropEvent *event)
{
    for (const QUrl &url : event->mimeData()->urls()) {
        QString path = url.toLocalFile();
        if (!verifyCB || verifyCB(path)) {
            setCurrentPath(path);
            return;
        }
    }
}
