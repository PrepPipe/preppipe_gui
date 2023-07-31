#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDir>
#include <QProcessEnvironment>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->renpyOutputDirWidget->setDirectoryMode(true);
    ui->renpyTemplateSelectionWidget->setDirectoryMode(true);
    ui->pluginSelectionWidget->setDirectoryMode(true);
    ui->searchPathInputWidget->setDirectoryMode(true);
    ui->sayDumpLocationSelectionWidget->setDirectoryMode(true);

    ui->icfgExportSelectionWidget->setFieldName(tr(u8"控制流图输出文件(.dot)"));
    ui->icfgExportSelectionWidget->setFilter(tr(u8"Graphviz DOT 图片 (*.dot)"));
    ui->icfgExportSelectionWidget->setDefaultName("icfg.dot");

    ui->mainExecutableSelectionWidget->setFieldName(tr(u8"主程序路径"));
    ui->mainExecutableSelectionWidget->setFilter(tr(u8"语涵编译器命令行主程序，一般是 preppipe_cli.exe (*.exe)"));
    ui->mainExecutableSelectionWidget->setVerifyCallBack([](const QString& path)->bool{
        QFileInfo info(path);
        if (info.isExecutable()) {
            return true;
        }
        return false;
    });
    ui->assetUsageSelectionWidget->setFieldName(tr(u8"资源使用分析输出路径"));
    ui->assetUsageSelectionWidget->setFilter(tr(u8"分析报告 (*.txt)"));
    ui->assetUsageSelectionWidget->setDefaultName(tr(u8"资源使用分析报告.txt"));
    ui->inputWidget->setFieldName(tr(u8"输入文档"));
    ui->searchPathInputWidget->setFieldName(tr(u8"素材搜索目录"));
    ui->renpyOutputDirWidget->setFieldName(tr(u8"RenPy 输出目录"));
    ui->renpyOutputDirWidget->setDefaultName("game");
    ui->renpyTemplateSelectionWidget->setFieldName(tr(u8"RenPy 工程模板目录"));
    ui->pluginSelectionWidget->setFieldName(tr(u8"插件目录"));
    ui->sayDumpLocationSelectionWidget->setFieldName(tr(u8"发言信息导出目录"));
    ui->sayDumpLocationSelectionWidget->setDefaultName(tr("发言信息"));

    ui->languageComboBox->addItem(u8"简体中文 (zh_cn)", QString("zh_cn"));
    ui->languageComboBox->addItem(u8"繁體中文 (zh_hk)", QString("zh_hk"));
    ui->languageComboBox->addItem(u8"English (en)", QString("en"));

    ui->sayDumpPresetComboBox->addItem(tr(u8"默认"), QString(""));
    ui->sayDumpPresetComboBox->addItem(tr(u8"用于 GameCreator 对话批量导入工具 (gamecreator)"), QString("gamecreator"));

    ui->minlenSpinBox->setValue(LONGSAYSPLITTING_MIN_START_LENGTH);
    ui->targetlenSpinBox->setValue(LONGSAYSPLITTING_TARGET_LENGTH);

    connect(ui->inputWidget, &FileListInputWidget::listChanged, this, &MainWindow::settingsChanged);
    connect(ui->searchPathInputWidget, &FileListInputWidget::listChanged, this, &MainWindow::settingsChanged);
    connect(ui->outputSelectionTabWidget, &QTabWidget::currentChanged, this, &MainWindow::settingsChanged);
    connect(ui->renpyOutputDirWidget, &FileSelectionWidget::filePathUpdated, this, &MainWindow::settingsChanged);
    connect(ui->renpyUseTemplateCheckBox, &QCheckBox::stateChanged, this, &MainWindow::settingsChanged);
    connect(ui->renpyTemplateSelectionWidget, &FileSelectionWidget::filePathUpdated, this, &MainWindow::settingsChanged);
    connect(ui->mainExecutableSelectionWidget, &FileSelectionWidget::filePathUpdated, this, &MainWindow::settingsChanged);
    connect(ui->languageCheckBox, &QCheckBox::stateChanged, this, &MainWindow::settingsChanged);
    connect(ui->languageComboBox, &QComboBox::currentIndexChanged, this, &MainWindow::settingsChanged);
    connect(ui->verboseCheckBox, &QCheckBox::stateChanged, this, &MainWindow::settingsChanged);
    connect(ui->entryInferenceCheckBox, &QCheckBox::stateChanged, this, &MainWindow::settingsChanged);
    connect(ui->longSaySplittinggroupBox, &QGroupBox::toggled, this, &MainWindow::settingsChanged);
    connect(ui->minlenSpinBox, &QSpinBox::valueChanged, this, &MainWindow::settingsChanged);
    connect(ui->targetlenSpinBox, &QSpinBox::valueChanged, this, &MainWindow::settingsChanged);
    connect(ui->icfgGroupBox, &QGroupBox::toggled, this, &MainWindow::settingsChanged);
    connect(ui->icfgExportSelectionWidget, &FileSelectionWidget::filePathUpdated, this, &MainWindow::settingsChanged);
    connect(ui->assetUsageGroupBox, &QGroupBox::toggled, this, &MainWindow::settingsChanged);
    connect(ui->assetUsageSelectionWidget, &FileSelectionWidget::filePathUpdated, this, &MainWindow::settingsChanged);
    connect(ui->sayDumpGroupBox, &QGroupBox::toggled, this, &MainWindow::settingsChanged);
    connect(ui->sayDumpLocationSelectionWidget, &FileSelectionWidget::filePathUpdated, this, &MainWindow::settingsChanged);
    connect(ui->sayDumpPresetComboBox, &QComboBox::currentIndexChanged, this, &MainWindow::settingsChanged);
    connect(ui->customPassGroupBox, &QGroupBox::toggled, this, &MainWindow::settingsChanged);
    connect(ui->irPassLineEdit, &QLineEdit::textChanged, this, &MainWindow::settingsChanged);

    // 尝试从环境变量里读取插件目录
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.contains("PREPPIPE_PLUGINS")) {
        QString path = env.value("PREPPIPE_PLUGINS");
        if (path.length() > 0) {
            ui->pluginSelectionWidget->setCurrentPath(path);
        }
    }

    // 初始化时尝试寻找主程序
    QMetaObject::invokeMethod(this, &MainWindow::search_exe, Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::search_exe()
{
    if (ui->mainExecutableSelectionWidget->getCurrentPath().length() > 0)
        return;
    QDir curdir(QCoreApplication::applicationDirPath());
    QString selfName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();

    for (const QString& candidate : curdir.entryList(QDir::Files | QDir::Executable, QDir::NoSort)) {
        if (candidate == selfName)
            continue;
        if (candidate.startsWith("preppipe_cli")) {
            ui->mainExecutableSelectionWidget->setCurrentPath(candidate);
            return;
        }
    }
}

namespace {

// ChatGPT 出品
QList<QString> splitCommandLineArguments(const QString& inputString) {
    QList<QString> arguments;
    QString currentArgument;
    bool insideQuotes = false;

    for (int i = 0; i < inputString.length(); ++i) {
        QChar currentChar = inputString.at(i);

        if (currentChar == '\"') {
            // Toggle the insideQuotes flag when a quote is encountered
            insideQuotes = !insideQuotes;
        } else if (currentChar == ' ' && !insideQuotes) {
            // If space is encountered and not inside quotes, add the current argument to the list
            if (!currentArgument.isEmpty()) {
                arguments.append(currentArgument);
                currentArgument.clear();
            }
        } else {
            // Append the current character to the current argument
            currentArgument.append(currentChar);
        }
    }

    // Add the last argument if it's not empty (there might not be a space at the end)
    if (!currentArgument.isEmpty()) {
        arguments.append(currentArgument);
    }

    return arguments;
}

} // end anonymous namespace

void MainWindow::settingsChanged()
{
    ExecuteInfo newInfo;

    QStringList args;
    QHash<QString, QString> envs;
    QHash<int, UnspecifiedPathInfo> unspecifiedPaths;
    QStringList errors;
    QString unspecified = tr(u8"<未指定：%1>");

    // 从环境变量开始
    QString pluginpath = ui->pluginSelectionWidget->getCurrentPath();
    if (pluginpath.length() > 0) {
        envs.insert("PREPPIPE_PLUGINS", pluginpath);
    }
    auto populatePath = [&](FileSelectionWidget* w) -> void {
        QString path = w->getCurrentPath();
        if (path.length() == 0) {
            int index = args.size();
            args.append(unspecified.arg(w->getFieldName()));
            UnspecifiedPathInfo temp;
            temp.isDir = w->getIsDirectoryMode();
            temp.filter = w->getFilter();
            temp.defaultName= w->getDefaultName();
            unspecifiedPaths[index] = temp;
        } else {
            args.append(path);
        }
    };

    // 主程序
    populatePath(ui->mainExecutableSelectionWidget);

    // 设置部分
    if (ui->verboseCheckBox->isChecked()) {
        args.append("-v");
    }
    if (ui->languageCheckBox->isChecked()) {
        args.append("--language");
        auto data = ui->languageComboBox->currentData();
        args.append(data.toString());
    }

    // 搜索路径
    auto searchpaths = ui->searchPathInputWidget->getCurrentList();
    if (searchpaths.size() == 0) {
        // 没有搜索路径不能算错误
    } else {
        args.append("--searchpath");
        args.append(searchpaths);
    }

    // 输入
    args.append("--odf");
    auto inputs = ui->inputWidget->getCurrentList();
    if (inputs.size() == 0) {
        errors.append(tr(u8"请指定输入文档"));
        args.append(unspecified.arg(ui->inputWidget->getFieldName()));
    } else {
        args.append(inputs);
    }

    // 标准流程
    args.append("--cmdsyntax");
    args.append("--vnparse");
    args.append("--vncodegen");
    args.append("--vn-blocksorting");
    if (ui->entryInferenceCheckBox->isChecked()) {
        args.append("--vn-entryinference");
    }
    if (ui->longSaySplittinggroupBox->isChecked()) {
        args.append("--vn-longsaysplitting");
        if (ui->minlenSpinBox->value() != LONGSAYSPLITTING_MIN_START_LENGTH) {
            args.append("--longsaysplitting-length-split");
            args.append(QString::number(ui->minlenSpinBox->value()));
        }
        if (ui->targetlenSpinBox->value() != LONGSAYSPLITTING_TARGET_LENGTH) {
            args.append("--longsaysplitting-length-target");
            args.append(QString::number(ui->targetlenSpinBox->value()));
        }
    }

    // 自定义
    if (ui->customPassGroupBox->isChecked()) {
        auto splitted = splitCommandLineArguments(ui->irPassLineEdit->text());
        if (splitted.size() > 0) {
            args.append(splitted);
        }
    }

    // 控制流图
    if (ui->icfgGroupBox->isChecked()) {
        args.append("--dump-icfg");
        populatePath(ui->icfgExportSelectionWidget);
    }

    // 资源分析
    if (ui->assetUsageGroupBox->isChecked()) {
        args.append("--vn-assetusage");
        populatePath(ui->assetUsageSelectionWidget);
    }

    // 发言信息导出
    if (ui->sayDumpGroupBox->isChecked()) {
        args.append("--vn-saydump");
        populatePath(ui->sayDumpLocationSelectionWidget);
        QString preset = ui->sayDumpPresetComboBox->currentData().toString();
        if (preset.length() > 0) {
            args.append("--vnsaydump-preset");
            args.append(preset);
        }
    }

    // 如果某些路径选项没法通过新建临时文件、临时目录来解决的话，同样视作错误
    for (auto iter = unspecifiedPaths.begin(), iterEnd = unspecifiedPaths.end(); iter != iterEnd; ++iter) {
        if (iter.value().defaultName.length() == 0) {
            errors.append(tr(u8"请提供参数： ") + args.at(iter.key()));
        }
    }

    curInfo.args = args;
    curInfo.envs = envs;
    curInfo.unspecfiedPaths = unspecifiedPaths;
    curInfo.errors = errors;
    ui->executePushButton->setEnabled(errors.size() > 0);

    if (errors.size() > 0) {
        ui->statusbar->showMessage(errors.first());
    } else if (unspecifiedPaths.size() > 0) {
        ui->statusbar->showMessage(tr(u8"%1 项参数还未指定。如果现在执行，输出的路径、目录将会使用临时目录。").arg(QString::number(unspecifiedPaths.size())));
    } else {
        ui->statusbar->clearMessage();
    }

    QString mergedstr;
    for (const QString& a : args) {
        if (mergedstr.length() > 0) {
            mergedstr += ' ';
        }
        if (a.contains(' ')) {
            QString copy(a);
            copy.replace('"', "\\\"");
            mergedstr += '"';
            mergedstr += copy;
            mergedstr += '"';
        } else {
            mergedstr += a;
        }
    }
    ui->commandTextEdit->setPlainText(mergedstr);
}
