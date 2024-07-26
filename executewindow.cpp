#include "executewindow.h"
#include "ui_executewindow.h"
#include "outputwidget.h"
#include <QOperatingSystemVersion>

int ExecuteWindow::windowindex = 0;

ExecuteWindow::ExecuteWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ExecuteWindow),
    outputLayout(nullptr),
    proc(this)
{
    ui->setupUi(this);
    outputLayout = new QVBoxLayout();
    ui->outputGroupBox->setLayout(outputLayout);
    windowindex += 1;
    QString title = tr(u8"#%1 执行窗口").arg(QString::number(windowindex));
    setWindowTitle(title);
    tmpdir.setAutoRemove(true);
    setAttribute(Qt::WA_DeleteOnClose);

    QOperatingSystemVersion osver = QOperatingSystemVersion::current();
    auto version = osver.version();
    QString osinfo = osver.name() + " " + version.toString();
    // Windows 11 会有版本号 >= 10.0.22000...
    if (osver.type() == QOperatingSystemVersion::Windows && version.majorVersion() == 10 && version.minorVersion() == 0 && version.microVersion() >= 22000) {
        osinfo += " (Windows 11)";
    }
    // 如果当前系统默认的编码无法显示中文或是其他非 ASCII 字符，就使用 UTF-8 而不是默认编码
    if (osver.type() == QOperatingSystemVersion::Windows && version.majorVersion() == 10 && version.minorVersion() == 0 && version.microVersion() < 22000) {
        isUTF8Fallback = true;
    }
    ui->plainTextEdit->appendPlainText(osinfo);
}

ExecuteWindow::~ExecuteWindow()
{
    delete ui;
}

void ExecuteWindow::closeEvent(QCloseEvent* e)
{
    if (!isCanClose) {
        e->ignore();
        return;
    }
    QMainWindow::closeEvent(e);
}

void ExecuteWindow::init(const ExecutionInfo& info)
{
    isCanClose = false;
    static const QString SEPARATOR = QString("=").repeated(20);
    QStringList args = info.args;
    if (info.unspecifiedPaths.size() > 0) {
        if (!tmpdir.isValid()) {
            // 无法创建临时目录，无法执行
            ui->plainTextEdit->setPlainText(tr(u8"有未指定的输出且无法创建临时目录，无法执行"));
            return;
        }
    }
    QDir tmppath(tmpdir.path());
    for (auto iter = info.unspecifiedPaths.begin(), iterEnd = info.unspecifiedPaths.end(); iter != iterEnd; ++iter) {
        int argindex = iter.key();
        const auto& info = iter.value();
        QString fullpath = tmppath.filePath(info.defaultName);
        args[argindex] = fullpath;
    }
    for (const auto& out : info.specifiedOutputs) {
        QString value = args.at(out.argindex);
        OutputWidget* w = new OutputWidget();
        outputLayout->addWidget(w);
        w->setData(out.fieldName, value);
        connect(this, &ExecuteWindow::executionFinished, w, &OutputWidget::updateStatus);
    }
    proc.setProgram(info.program);
    proc.setArguments(args);
    if (info.envs.size() > 0 || isUTF8Fallback) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        for (auto iter = info.envs.begin(), iterEnd = info.envs.end(); iter != iterEnd; ++iter) {
            env.insert(iter.key(), iter.value());
        }
        if (isUTF8Fallback) {
            // 如果系统定的默认输出编码不能显示中文或是其他非ASCII字符，让程序输出 UTF-8 而不是报错
            env.insert("PYTHONIOENCODING", "utf-8");
            env.insert("PYTHONLEGACYWINDOWSSTDIO", "utf-8");
            env.insert("PYTHONUTF8", "1");
        }
        proc.setProcessEnvironment(env);
    }
    proc.setProcessChannelMode(QProcess::MergedChannels);

    connect(&proc, &QProcess::finished, this, &ExecuteWindow::executionFinished);
    connect(&proc, &QProcess::readyReadStandardOutput, this, &ExecuteWindow::handleProcOutput);
    connect(&proc, &QProcess::finished, this, [this](){
        ExecuteWindow::handleProcOutput();
        ui->plainTextEdit->appendPlainText(SEPARATOR);
        int exitCode = proc.exitCode();
        if (exitCode == 0) {
            ui->plainTextEdit->appendPlainText(tr(u8"执行完成(%1)").arg(QString::number(exitCode)));
        } else {
            ui->plainTextEdit->appendPlainText(tr(u8"执行出错(%1)，如有疑问请联系开发者").arg(QString::number(exitCode)));
        }
        ui->killButton->setEnabled(false);
        isCanClose = true;
    });

    QString mergedArgs = getMergedCommand(info.program, args);
    ui->plainTextEdit->appendPlainText(mergedArgs + "\n" + SEPARATOR + '\n');
    connect(ui->killButton, &QPushButton::clicked, &proc, &QProcess::kill);
    proc.start(QProcess::ReadOnly);
}

void ExecuteWindow::handleProcOutput()
{
    auto content = proc.readAllStandardOutput();
    if (content.length() == 0)
        return;

    QString decoded;
    if (!isUTF8Fallback) {
        decoded = QString::fromLocal8Bit(content);
    } else {
        decoded = QString::fromUtf8(content);
    }
    ui->plainTextEdit->appendPlainText(decoded);
    ui->plainTextEdit->moveCursor(QTextCursor::End);
    progOutput += decoded;
}
