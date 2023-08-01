#include "outputwidget.h"
#include "ui_outputwidget.h"
#include <QFileInfo>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDir>
#include <QProcess>

OutputWidget::OutputWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OutputWidget)
{
    ui->setupUi(this);
    connect(ui->openInExplorerPushButton, &QPushButton::clicked, this, &OutputWidget::requestOpenContainingDirectory);
    connect(ui->openPushButton, &QPushButton::clicked, this, &OutputWidget::requestOpen);
}

OutputWidget::~OutputWidget()
{
    delete ui;
}

void OutputWidget::setData(const QString &fieldName, const QString &path)
{
    this->fieldName = fieldName;
    this->path = path;
    ui->fieldNameLabel->setText(fieldName);
    ui->pathLabel->setText(path);
    ui->statusLabel->setText(tr(u8"初始"));
    QFileInfo f(path);
    if (f.exists()) {
        lastModified = f.lastModified();
    } else {
        lastModified = QDateTime();
    }
}

void OutputWidget::updateStatus()
{
    QFileInfo info(path);
    if (info.exists()) {
        QDateTime curtime = info.lastModified();
        if (!lastModified.isValid() || lastModified < curtime) {
            ui->statusLabel->setText(tr(u8"已生成"));
            return;
        }
        ui->statusLabel->setText(tr(u8"未更新"));
    } else {
        ui->statusLabel->setText(tr(u8"尚未生成"));
    }
}

void OutputWidget::requestOpenContainingDirectory()
{
    QFileInfo info(path);
    if (!info.exists()) {
        return;
    }

    // 参考 qt-creator/src/plugins/coreplugin/fileutils.cpp FileUtils::showInGraphicalShell()
#ifdef Q_OS_WIN32
    QString explorer = QStandardPaths::findExecutable("explorer.exe");
    if (explorer.length() == 0) {
        QMessageBox::warning(this, tr(u8"无法打开文件浏览器"), tr("PATH 中找不到 explorer.exe, 无法打开目录"));
        return;
    }
    QStringList param;
    param.append(QLatin1String("/select,"));
    param.append(QDir::toNativeSeparators(info.canonicalFilePath()));
    //qDebug() << "prog=" << explorer << ", args=" << param;
    QProcess::startDetached("explorer.exe", param);
#elif defined(Q_OS_MAC)
    QProcess::startDetached("/usr/bin/open", {"-R", info.canonicalFilePath()});
#else
    QMessageBox::warning(this, tr(u8"暂不支持"), tr("暂不支持在当前系统下打开目录"));
#endif
}
void OutputWidget::requestOpen()
{
    QFileInfo info(path);
    if (info.exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}
