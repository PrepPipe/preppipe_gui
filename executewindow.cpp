#include "executewindow.h"
#include "ui_executewindow.h"
#include "outputwidget.h"

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
}

ExecuteWindow::~ExecuteWindow()
{
    delete ui;
}

void ExecuteWindow::init(const ExecutionInfo& info)
{
    static const QString SEPARATOR = QString("=").repeated(20);
    QStringList args = info.args;
    if (info.unspecfiedPaths.size() > 0) {
        if (!tmpdir.isValid()) {
            // 无法创建临时目录，无法执行
            ui->plainTextEdit->setPlainText(tr(u8"有未指定的输出且无法创建临时目录，无法执行"));
            return;
        }
    }
    QDir tmppath(tmpdir.path());
    for (auto iter = info.unspecfiedPaths.begin(), iterEnd = info.unspecfiedPaths.end(); iter != iterEnd; ++iter) {
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
    // getMergedCommand
    proc.setProgram(info.program);
    proc.setArguments(args);
    if (info.envs.size() > 0) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        for (auto iter = info.envs.begin(), iterEnd = info.envs.end(); iter != iterEnd; ++iter) {
            env.insert(iter.key(), iter.value());
        }
        proc.setProcessEnvironment(env);
    }
    proc.setProcessChannelMode(QProcess::MergedChannels);

    connect(&proc, &QProcess::readyReadStandardOutput, this, [this](){
        auto content = proc.readAllStandardOutput();
        //qDebug() << "output: " << content;
        ui->plainTextEdit->appendPlainText(QString::fromLocal8Bit(content));
    });

    connect(&proc, &QProcess::finished, this, [this](){
        auto content = proc.readAllStandardOutput();
        //qDebug() << "output: " << content;
        if (content.length() > 0) {
            ui->plainTextEdit->appendPlainText(QString::fromLocal8Bit(content));
        }
        ui->plainTextEdit->appendPlainText(SEPARATOR);
        ui->plainTextEdit->appendPlainText(tr(u8"执行结束(%1)").arg(QString::number(proc.exitCode())));
        ui->killButton->setEnabled(false);
    });
    connect(&proc, &QProcess::finished, this, &ExecuteWindow::executionFinished);
    QString mergedArgs = getMergedCommand(info.program, args);
    ui->plainTextEdit->appendPlainText(mergedArgs + "\n" + SEPARATOR + '\n');
    connect(ui->killButton, &QPushButton::clicked, &proc, &QProcess::kill);
    proc.start(QProcess::ReadOnly);
}
