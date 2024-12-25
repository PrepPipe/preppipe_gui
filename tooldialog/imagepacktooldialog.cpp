#include "imagepacktooldialog.h"
#include "ui_imagepacktooldialog.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QTextStream>
#include "ExecutionData.h"
#include "executewindow.h"
#include "auxiliarydialog/textwithcolorinputdialog.h"
#include "util/filepathvalidate.h"

QString ImagePackToolDialog::STATE_lastImagePath;
QColor  ImagePackToolDialog::STATE_lastColor(Qt::white);
QString ImagePackToolDialog::STATE_lastTextContent;
QColor  ImagePackToolDialog::STATE_lastTextColor(Qt::black);
QString ImagePackToolDialog::STATE_lastCreateYAMLPath;
QString ImagePackToolDialog::STATE_lastLoadZIPPath;
QString ImagePackToolDialog::STATE_lastSaveZIPPath;
QStringList ImagePackToolDialog::STATE_lastForkParams;
QString ImagePackToolDialog::STATE_lastExportPath;
ImagePackToolDialog::SourceOption ImagePackToolDialog::STATE_lastSourceOption = ImagePackToolDialog::SourceOption::SRC_PRESET;
bool ImagePackToolDialog::STATE_lastSaveOptionEnable = false;
bool ImagePackToolDialog::STATE_lastForkOptionEnable = true;
bool ImagePackToolDialog::STATE_lastExportOptionEnable = true;
bool ImagePackToolDialog::STATE_lastExportOverviewOptionEnable = true;

QString ImagePackToolDialog::STATE_presetQueryData;
QString ImagePackToolDialog::STATE_presetFromExecutablePath;
QString ImagePackToolDialog::STATE_lastSelectedPresetID;

ImagePackToolDialog::ImagePackToolDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImagePackToolDialog)
{
    ui->setupUi(this);
    ui->createInputWidget->setFieldName(tr(u8"图片包配置文件(.yaml/.yml)"));
    ui->createInputWidget->setFilter(tr(u8"YAML 文本(*.yaml *.yml)"));
    ui->loadInputWidget->setFieldName(tr(u8"要读取的图片包(.zip)"));
    ui->loadInputWidget->setFilter(tr(u8"ZIP 压缩文件(*.zip)"));
    ui->savePathInputWidget->setIsOutputInsteadofInput(true);
    ui->savePathInputWidget->setFieldName(tr(u8"要保存的图片包(.zip)"));
    ui->savePathInputWidget->setFilter(tr(u8"ZIP 压缩文件(*.zip)"));
    ui->savePathInputWidget->setDefaultName("imagepack.zip");
    ui->exportPathInputWidget->setIsOutputInsteadofInput(true);
    ui->exportPathInputWidget->setDirectoryMode(true);
    ui->exportPathInputWidget->setFieldName(tr(u8"差分图片导出目录"));
    ui->exportOverviewPathInputWidget->setIsOutputInsteadofInput(true);
    ui->exportOverviewPathInputWidget->setFieldName(tr(u8"导出的概览图(.png)"));
    ui->exportOverviewPathInputWidget->setFilter(tr(u8"PNG 图片 (*.png)"));
    ui->exportOverviewPathInputWidget->setDefaultName(tr(u8"概览图.png"));

    // 先把初值设好
    if (!STATE_lastCreateYAMLPath.isEmpty()) {
        ui->createInputWidget->setCurrentPath(STATE_lastCreateYAMLPath);
    }
    if (!STATE_lastLoadZIPPath.isEmpty()) {
        ui->loadInputWidget->setCurrentPath(STATE_lastLoadZIPPath);
    }
    if (!STATE_lastSaveZIPPath.isEmpty()) {
        ui->savePathInputWidget->setCurrentPath(STATE_lastSaveZIPPath);
    }
    if (!STATE_lastForkParams.isEmpty()) {
        for (const QString& p : STATE_lastForkParams) {
            if (p.startsWith('#')) {
                addColorForkParamImpl(QColor(p));
            } else if (p == "None") {
                requestAddNoneForkParam();
            } else if (p.startsWith("<#")) {
                auto pos = p.indexOf('>');
                QString text = p.mid(pos+1);
                QColor color = QColor(p.mid(1, pos-1));
                addTextForkParamImpl(text, color);
            } else {
                addImageForkParamImpl(p);
            }
        }
    }
    if (!STATE_lastExportPath.isEmpty()) {
        ui->exportPathInputWidget->setCurrentPath(STATE_lastExportPath);
    }
    ui->createRadioButton->setChecked(STATE_lastSourceOption == SourceOption::SRC_CREATE);
    ui->loadRadioButton->setChecked(STATE_lastSourceOption == SourceOption::SRC_LOAD);
    ui->loadPresetRadioButton->setChecked(STATE_lastSourceOption == SourceOption::SRC_PRESET);
    ui->saveGroupBox->setChecked(STATE_lastSaveOptionEnable);
    ui->forkGroupBox->setChecked(STATE_lastForkOptionEnable);
    ui->exportGroupBox->setChecked(STATE_lastExportOptionEnable);
    ui->exportOverviewGroupBox->setChecked(STATE_lastExportOverviewOptionEnable);

    // 再开始 connect
    connect(ui->addForkColorPushButton, &QPushButton::clicked, this, &ImagePackToolDialog::requestAddColorForkParam);
    connect(ui->addForkImagePushButton, &QPushButton::clicked, this, &ImagePackToolDialog::requestAddImageForkParam);
    connect(ui->addForkTextPushButton,  &QPushButton::clicked, this, &ImagePackToolDialog::requestAddTextForkParam);
    connect(ui->addForkNonePushButton,  &QPushButton::clicked, this, &ImagePackToolDialog::requestAddNoneForkParam);
    connect(ui->deleteForkParamPushButton, &QPushButton::clicked, this, &ImagePackToolDialog::requestDeleteForkParam);
    connect(ui->forkParamListWidget, &QListWidget::itemDoubleClicked, this, &ImagePackToolDialog::handleForkParamEdit);
    connect(ui->presetInfoPushButton,   &QPushButton::clicked, this, &ImagePackToolDialog::showPresetInfo);

    connect(this, &ImagePackToolDialog::forkParamChanged, this, &ImagePackToolDialog::forkParamChangeSaver);
    connect(ui->createInputWidget, &FileSelectionWidget::filePathUpdated, this, [](const QString& newpath) {
        STATE_lastCreateYAMLPath = newpath;
    });
    connect(ui->loadInputWidget, &FileSelectionWidget::filePathUpdated, this, [](const QString& newpath) {
        STATE_lastLoadZIPPath = newpath;
    });
    connect(ui->savePathInputWidget, &FileSelectionWidget::filePathUpdated, this, [](const QString& newpath) {
        STATE_lastSaveZIPPath = newpath;
    });
    connect(ui->exportPathInputWidget, &FileSelectionWidget::filePathUpdated, this, [](const QString& newpath) {
        STATE_lastExportPath = newpath;
    });
    connect(ui->createRadioButton, &QRadioButton::toggled, this, [](bool checked){
        if (checked) {
            STATE_lastSourceOption = SourceOption::SRC_CREATE;
        }
    });
    connect(ui->loadRadioButton, &QRadioButton::toggled, this, [](bool checked){
        if (checked) {
            STATE_lastSourceOption = SourceOption::SRC_LOAD;
        }
    });
    connect(ui->loadPresetRadioButton, &QRadioButton::toggled, this, [](bool checked){
        if (checked) {
            STATE_lastSourceOption = SourceOption::SRC_PRESET;
        }
    });
    connect(ui->saveGroupBox, &QGroupBox::toggled, this, [](bool checked){
        STATE_lastSaveOptionEnable = checked;
    });
    connect(ui->forkGroupBox, &QGroupBox::toggled, this, [](bool checked){
        STATE_lastForkOptionEnable = checked;
    });
    connect(ui->exportGroupBox, &QGroupBox::toggled, this, [](bool checked){
        STATE_lastExportOptionEnable = checked;
    });
    connect(ui->exportOverviewGroupBox, &QGroupBox::toggled, this, [](bool checked){
        STATE_lastExportOverviewOptionEnable = checked;
    });

    forkParamDropHandler = new FileDropAccepter(this);
    forkParamDropHandler->setVerifyCallBack(FilePathValidate::isReadableImageFile);
    connect(forkParamDropHandler, &FileDropAccepter::fileDropped, this, [this](const QString& path, QObject* droppedTo) {
        if (droppedTo == ui->forkParamListWidget) {
            addImageForkParamImpl(path);
        }
    });
    ui->forkParamListWidget->installEventFilter(forkParamDropHandler);

    languageCode = "en";

    // 我们需要等创建本对话框的代码把 initialInfo 初始化后再执行此操作
    QMetaObject::invokeMethod(this, &ImagePackToolDialog::startRetrievePresetInfo, Qt::QueuedConnection);
}

ImagePackToolDialog::~ImagePackToolDialog()
{
    delete ui;
}

void ImagePackToolDialog::requestAddColorForkParam()
{
    QColor newColor = QColorDialog::getColor(STATE_lastColor, this, tr(u8"请选择颜色"));
    if (newColor.isValid()) {
        addColorForkParamImpl(newColor);
    }
}

void ImagePackToolDialog::addColorForkParamImpl(QColor color)
{
    QPixmap iconpixmap(FORKPARAMICON_SIZE, FORKPARAMICON_SIZE);
    iconpixmap.fill(color);
    QString str = color.name(QColor::HexRgb);
    QListWidgetItem* item = new QListWidgetItem(QIcon(iconpixmap), str);
    item->setToolTip(tr(u8"颜色参数，此参数所对应的选区将把其基础颜色改为所选颜色"));
    item->setData(Qt::UserRole, QVariant(static_cast<unsigned>(ForkParamType::TY_COLOR)));
    ui->forkParamListWidget->addItem(item);
    emit forkParamChanged();
    STATE_lastColor = color;
}

void ImagePackToolDialog::requestAddImageForkParam()
{
    QString path = QFileDialog::getOpenFileName(this, tr(u8"请选择图片"), STATE_lastImagePath, tr(u8"常见图片文件 (*.png *.jpg *.jpeg *.bmp *.gif);; 所有文件 (*.*)"));
    if (!path.isEmpty()) {
        addImageForkParamImpl(path);
    }
}

void ImagePackToolDialog::addImageForkParamImpl(const QString& path)
{
    QIcon icon = this->style()->standardIcon(QStyle::SP_FileIcon);
    QListWidgetItem* item = new QListWidgetItem(icon, path);
    item->setToolTip(tr(u8"图片参数，此参数所对应的选区将嵌入本项所指的图片文件"));
    item->setData(Qt::UserRole, QVariant(static_cast<unsigned>(ForkParamType::TY_IMAGE)));
    ui->forkParamListWidget->addItem(item);
    emit forkParamChanged();
    STATE_lastImagePath = path;
}

QPixmap ImagePackToolDialog::getIconForTextForkParam(QString text, QColor color)
{
    const unsigned iconsize = 32;
    QPixmap iconimg(iconsize, iconsize);
    QPainter p;
    p.begin(&iconimg);
    p.fillRect(0, 0, iconsize, iconsize, color);
    p.fillRect(0, iconsize/4, iconsize/3, iconsize/2, Qt::white);
    p.fillRect(iconsize-iconsize/3, iconsize/4, iconsize/3, iconsize/2, Qt::white);
    p.end();
    return iconimg;
}

QString ImagePackToolDialog::getMangledTextForTextForkParam(QString text, QColor color)
{
    QString mangledText;
    mangledText.append('<');
    mangledText.append(color.name(QColor::HexRgb));
    mangledText.append('>');
    mangledText.append(text);
    return mangledText;
}

void ImagePackToolDialog::requestAddTextForkParam()
{
    TextWithColorInputDialog* dialog = new TextWithColorInputDialog(STATE_lastTextColor, STATE_lastTextContent, this);
    connect(dialog, &QDialog::accepted, this, [=](){
        addTextForkParamImpl(dialog->getText(), dialog->getColor());
    });
    dialog->open();
}

void ImagePackToolDialog::addTextForkParamImpl(QString text, QColor color)
{
    QIcon icon = QIcon(getIconForTextForkParam(text, color));
    QString mangledText = getMangledTextForTextForkParam(text, color);
    QListWidgetItem* item = new QListWidgetItem(icon, mangledText);
    item->setToolTip(tr(u8"文本参数，此参数所对应的选区将显示这里的文本"));
    item->setData(Qt::UserRole, QVariant(static_cast<unsigned>(ForkParamType::TY_TEXT)));
    ui->forkParamListWidget->addItem(item);
    emit forkParamChanged();
    STATE_lastTextContent = text;
    STATE_lastTextColor = color;
}

void ImagePackToolDialog::requestAddNoneForkParam()
{
    QListWidgetItem* item = new QListWidgetItem(this->style()->standardIcon(QStyle::SP_MessageBoxQuestion), "None");
    item->setToolTip(tr(u8"空参数，此参数所对应的选区将不会改变"));
    item->setData(Qt::UserRole, QVariant(static_cast<unsigned>(ForkParamType::TY_NONE)));
    ui->forkParamListWidget->addItem(item);
    emit forkParamChanged();
}

void ImagePackToolDialog::requestDeleteForkParam()
{
    auto items = ui->forkParamListWidget->selectedItems();
    if (!items.empty()) {
        for (auto item : items) {
            delete item;
        }
        emit forkParamChanged();
    }
}

void ImagePackToolDialog::handleForkParamEdit(QListWidgetItem* item)
{
    QString text = item->text();
    switch (static_cast<ForkParamType>(item->data(Qt::UserRole).toUInt())) {
    default: break;
    case ForkParamType::TY_NONE:
        return;
    case ForkParamType::TY_COLOR: {
        QColor oldColor = QColor(text);
        QColor newColor = QColorDialog::getColor(oldColor, this, tr(u8"请选择颜色"));
        if (newColor.isValid() && newColor != oldColor) {
            item->setText(newColor.name(QColor::HexRgb));
            QPixmap iconpixmap(FORKPARAMICON_SIZE, FORKPARAMICON_SIZE);
            iconpixmap.fill(newColor);
            item->setIcon(QIcon(iconpixmap));
        }
        emit forkParamChanged();
        return;
    } break;
    case ForkParamType::TY_IMAGE: {
        QString path = QFileDialog::getOpenFileName(this, tr(u8"请选择图片"), text, tr(u8"常见图片文件 (*.png *.jpg *.jpeg *.bmp *.gif);; 所有文件 (*.*)"));
        if (!path.isEmpty() && path != text) {
            item->setText(path);
            emit forkParamChanged();
        }
    }  break;
    case ForkParamType::TY_TEXT: {
        int pos = text.indexOf('>');
        QString content = text.mid(pos+1);
        QColor color = QColor(text.mid(1, pos-1));
        TextWithColorInputDialog* dialog = new TextWithColorInputDialog(color, content, this);
        if (dialog->exec() == QDialog::Accepted) {
            QString newContent = dialog->getText();
            QColor newColor = dialog->getColor();
            if (newContent != content || newColor != color) {
                QString mangledText = getMangledTextForTextForkParam(newContent, newColor);
                QIcon newIcon = getIconForTextForkParam(newContent, newColor);
                item->setIcon(newIcon);
                item->setText(mangledText);
                emit forkParamChanged();
            }
        }
    } break;
    }
}

void ImagePackToolDialog::startRetrievePresetInfo()
{
    if (STATE_presetQueryData.length() > 0 && initialInfo.program == STATE_presetFromExecutablePath) {
        presetQueryData = STATE_presetQueryData;
        handlePresetInfo();
        return;
    }
    ExecutionInfo info = initialInfo;
    info.envs["PREPPIPE_TOOL"] = "assetmanager";
    info.args.append("--dump-json");
    ExecuteWindow* w = new ExecuteWindow(this);
    w->init(info);
    connect(w, &ExecuteWindow::executionFinished, this, [=](){
        presetQueryData = w->getOutput();
        STATE_presetQueryData = presetQueryData;
        STATE_presetFromExecutablePath = initialInfo.program;
    });
    connect(w, &ExecuteWindow::executionFinished, this, &ImagePackToolDialog::handlePresetInfo);
    connect(w, &ExecuteWindow::executionFinished, w, &QObject::deleteLater);
}

void ImagePackToolDialog::handlePresetInfo()
{
    // qDebug() << "ImagePackToolDialog::handlePresetInfo(): " << presetQueryData;
    // 有可能会在 JSON 对象之前有报错内容（比如没找到 ffmpeg）
    int startpos = presetQueryData.indexOf('{');
    QString actualData = presetQueryData.mid(startpos);
    QByteArray jsonDump = actualData.toUtf8();
    QJsonDocument d = QJsonDocument::fromJson(jsonDump);
    QJsonObject topobj = d.object();
    int anonymousCount = 0;
    ui->presetComboBox->clear();
    for (auto iter = topobj.begin(), iterEnd = topobj.end(); iter != iterEnd; ++iter) {
        QString k = iter.key();
        if (!k.startsWith("imagepack")) {
            continue;
        }
        QJsonObject v = iter.value().toObject();
        if (v.isEmpty()) {
            // 应该是出问题了才会这样
            continue;
        }
        int width = v["width"].toInt(0);
        int height = v["height"].toInt(0);
        int numComposites = v["composites"].toArray().size();
        QString templateTypeName;
        QStringList maskNames;
        std::vector<bool> maskTypes;
        QJsonArray maskArray = v["masks"].toArray();
        for (auto iter = maskArray.begin(), iterEnd = maskArray.end(); iter != iterEnd; ++iter) {
            QJsonObject m = iter->toObject();
            QString name = m["name"].toString(QStringLiteral("<Anonymous>"));
            bool isScreen = m["projection"].toBool(false);
            maskNames.append(name);
            maskTypes.push_back(isScreen);
        }
        QString refName;
        // 开始尝试读取 descriptor 中的信息
        if (v.contains("descriptor")) {
            QJsonObject descriptor = v["descriptor"].toObject();
            QJsonObject refNameObj = descriptor["name"].toObject();
            refName = refNameObj[languageCode].toArray().first().toString();
            templateTypeName = descriptor["packtype"].toString();
        }
        // 整理已有内容
        if (refName.isEmpty()) {
            anonymousCount += 1;
            refName = tr(u8"未命名") + " " + QString::number(anonymousCount);
        }
        QString templateTypeForDisplay;
        if (templateTypeName == "BACKGROUND") {
            templateTypeForDisplay = tr(u8"背景");
        } else if (templateTypeName == "CHARACTER") {
            templateTypeForDisplay = tr(u8"立绘");
        } else {
            templateTypeForDisplay = tr(u8"未知类型");
        }
        QString label;
        {
            QTextStream s(&label, QIODevice::WriteOnly);
            s << '[' << templateTypeForDisplay << "] " << refName << " (" << width << '*' << height << ")";
        }
        ui->presetComboBox->addItem(label, QVariant(k));
        QString details;
        {
            QTextStream s(&details, QIODevice::WriteOnly);
            s << label << '\n';
            s << tr(u8"内部ID: ") << k << '\n';
            s << tr(u8"选区数量：") << ' ' << maskTypes.size() << '\n';
            for (int i = 0, n = maskTypes.size(); i < n; ++i) {
                bool isScreen = maskTypes.at(i);
                const QString& name = maskNames.at(i);
                s << "  [" << (isScreen? tr(u8"图片") : tr(u8"颜色")) << "] " << name << "\n";
            }
            s << tr(u8"差分组合数量：") << ' ' << numComposites << '\n';
        }
        presetInfo.insert(k, details);
    }
    if (ui->presetComboBox->count() > 0) {
        ui->presetComboBox->setEnabled(true);
        ui->presetInfoPushButton->setEnabled(true);
        if (STATE_lastSelectedPresetID.length() > 0) {
            for (int i = 0, n = ui->presetComboBox->count(); i < n; ++i) {
                if (ui->presetComboBox->itemData(i).toString() == STATE_lastSelectedPresetID) {
                    ui->presetComboBox->setCurrentIndex(i);
                }
            }
        }
    }
    // 这个需要在上面的载入完成之后再加上，否则给 QComboBox 加内容的时候就会触发此改动
    connect(ui->presetComboBox, &QComboBox::currentIndexChanged, this, [this](int index){
        STATE_lastSelectedPresetID = ui->presetComboBox->itemData(index).toString();
    });
}

void ImagePackToolDialog::showPresetInfo()
{
    QVariant curPreset = ui->presetComboBox->currentData();
    if (!curPreset.isValid()) {
        return;
    }
    QString curPresetID = curPreset.toString();
    QString msgTitle = tr(u8"模板详情");
    auto iter = presetInfo.find(curPresetID);
    if (iter == presetInfo.end()) {
        QMessageBox::information(this, msgTitle, tr(u8"无法获取此模板的详情。"));
        return;
    }
    QString text = iter.value();
    QMessageBox* msgbox = new QMessageBox(QMessageBox::Icon::Information, msgTitle, text, QMessageBox::StandardButton::Ok, this);
    connect(msgbox, &QMessageBox::finished, msgbox, &QObject::deleteLater);
    msgbox->show();
}

void ImagePackToolDialog::forkParamChangeSaver()
{
    STATE_lastForkParams.clear();
    STATE_lastForkParams.reserve(ui->forkParamListWidget->count());
    for (unsigned i = 0, n = ui->forkParamListWidget->count(); i < n; ++i) {
        STATE_lastForkParams.append(ui->forkParamListWidget->item(i)->text());
    }
}

void ImagePackToolDialog::accept()
{
    ExecutionInfo info = initialInfo;
    info.envs["PREPPIPE_TOOL"] = "imagepack";

    QString unspecified = tr(u8"<未指定：%1>");
    auto populatePath = [&](FileSelectionWidget* w) -> void {
        if (w->getIsOutputInsteadofInput()) {
            OutputInfo outInfo;
            outInfo.argindex = info.args.size();
            outInfo.fieldName = w->getFieldName();
            info.specifiedOutputs.append(outInfo);
        }
        QString path = w->getCurrentPath();
        if (path.length() == 0) {
            int index = info.args.size();
            info.args.append(unspecified.arg(w->getFieldName()));
            UnspecifiedPathInfo temp;
            temp.isDir = w->getIsDirectoryMode();
            temp.filter = w->getFilter();
            temp.defaultName= w->getDefaultName();
            info.unspecifiedPaths[index] = temp;
        } else {
            info.args.append(path);
        }
    };

    // 输入部分
    if (ui->createRadioButton->isChecked()) {
        QString path = ui->createInputWidget->getCurrentPath();
        if (path.length() == 0) {
            QMessageBox::warning(this, tr(u8"需要输入"), tr(u8"请提供创建图片包所需的配置文件的路径。"));
            return;
        }
        info.args.append("--create");
        info.args.append(path);
    } else if (ui->loadRadioButton->isChecked()) {
        QString path = ui->loadInputWidget->getCurrentPath();
        if (path.length() == 0) {
            QMessageBox::warning(this, tr(u8"需要输入"), tr(u8"请提供要读取的图片包的路径。"));
            return;
        }
        info.args.append("--load");
        info.args.append(path);
    } else if (ui->loadPresetRadioButton->isChecked()) {
        info.args.append("--asset");
        QString preset = ui->presetComboBox->currentData().toString();
        if (preset.length() == 0) {
            QMessageBox::warning(this, tr(u8"需要输入"), tr(u8"模板暂不可用。"));
            return;
        }
        info.args.append(preset);
    } else {
        qFatal("should not happen");
    }

    // 保存部分
    if (ui->saveGroupBox->isChecked()) {
        info.args.append("--save");
        populatePath(ui->savePathInputWidget);
    }

    // 修改部分
    if (ui->forkGroupBox->isChecked()) {
        if (ui->forkParamListWidget->count() > 0) {
            info.args.append("--fork");
            for (unsigned i = 0, n = ui->forkParamListWidget->count(); i < n; ++i) {
                auto* item = ui->forkParamListWidget->item(i);
                info.args.append(item->text());
            }
        }
    }

    // 导出部分
    if (ui->exportGroupBox->isChecked()) {
        info.args.append("--export");
        populatePath(ui->exportPathInputWidget);
    }
    if (ui->exportOverviewGroupBox->isChecked()) {
        info.args.append("--export-overview");
        populatePath(ui->exportOverviewPathInputWidget);
    }

    info.args.append("--debug");

    ExecuteWindow* w = new ExecuteWindow();
    w->init(info);
    w->show();
    QDialog::accept();
}
