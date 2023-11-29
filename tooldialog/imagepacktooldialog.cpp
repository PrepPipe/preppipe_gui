#include "imagepacktooldialog.h"
#include "ui_imagepacktooldialog.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>
#include <QStyle>
#include "ExecutionData.h"
#include "executewindow.h"
#include "util/filepathvalidate.h"

QString ImagePackToolDialog::STATE_lastImagePath;
QColor  ImagePackToolDialog::STATE_lastColor(Qt::white);
QString ImagePackToolDialog::STATE_lastCreateYAMLPath;
QString ImagePackToolDialog::STATE_lastLoadZIPPath;
QString ImagePackToolDialog::STATE_lastSaveZIPPath;
QStringList ImagePackToolDialog::STATE_lastFrokParams;
QString ImagePackToolDialog::STATE_lastExportPath;
bool ImagePackToolDialog::STATE_lastCreateOptionEnable = true;
bool ImagePackToolDialog::STATE_lastSaveOptionEnable = true;
bool ImagePackToolDialog::STATE_lastForkOptionEnable = true;
bool ImagePackToolDialog::STATE_lastExportOptionEnable = true;

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
    if (!STATE_lastFrokParams.isEmpty()) {
        for (const QString& p : STATE_lastFrokParams) {
            if (p.startsWith('#')) {
                addColorForkParamImpl(QColor(p));
            } else if (p == "None") {
                requestAddNoneForkParam();
            } else {
                addImageForkParamImpl(p);
            }
        }
    }
    if (!STATE_lastExportPath.isEmpty()) {
        ui->exportPathInputWidget->setCurrentPath(STATE_lastExportPath);
    }
    ui->createRadioButton->setChecked(STATE_lastCreateOptionEnable);
    ui->loadRadioButton->setChecked(!STATE_lastCreateOptionEnable);
    ui->saveGroupBox->setChecked(STATE_lastSaveOptionEnable);
    ui->forkGroupBox->setChecked(STATE_lastForkOptionEnable);
    ui->exportGroupBox->setChecked(STATE_lastExportOptionEnable);

    // 再开始 connect
    connect(ui->addForkColorPushButton, &QPushButton::clicked, this, &ImagePackToolDialog::requestAddColorForkParam);
    connect(ui->addForkImagePushButton, &QPushButton::clicked, this, &ImagePackToolDialog::requestAddImageForkParam);
    connect(ui->addForkNonePushButton,  &QPushButton::clicked, this, &ImagePackToolDialog::requestAddNoneForkParam);
    connect(ui->deleteForkParamPushButton, &QPushButton::clicked, this, &ImagePackToolDialog::requestDeleteForkParam);
    connect(ui->forkParamListWidget, &QListWidget::itemDoubleClicked, this, &ImagePackToolDialog::handleForkParamEdit);

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
        STATE_lastCreateOptionEnable = checked;
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

    forkParamDropHandler = new FileDropAccepter(this);
    forkParamDropHandler->setVerifyCallBack(FilePathValidate::isReadableImageFile);
    connect(forkParamDropHandler, &FileDropAccepter::fileDropped, this, [this](const QString& path, QObject* droppedTo) {
        if (droppedTo == ui->forkParamListWidget) {
            addImageForkParamImpl(path);
        }
    });
    ui->forkParamListWidget->installEventFilter(forkParamDropHandler);
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
    ui->forkParamListWidget->addItem(item);
    emit forkParamChanged();
    STATE_lastImagePath = path;
}

void ImagePackToolDialog::requestAddNoneForkParam()
{
    QListWidgetItem* item = new QListWidgetItem(this->style()->standardIcon(QStyle::SP_MessageBoxQuestion), "None");
    item->setToolTip(tr(u8"空参数，此参数所对应的选区将不会改变"));
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
    if (text == "None") {
        return;
    }
    if (text.startsWith('#')) {
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
    }
    // 剩下的只可能是图片路径
    QString path = QFileDialog::getOpenFileName(this, tr(u8"请选择图片"), text, tr(u8"常见图片文件 (*.png *.jpg *.jpeg *.bmp *.gif);; 所有文件 (*.*)"));
    if (!path.isEmpty() && path != text) {
        item->setText(path);
        emit forkParamChanged();
    }
}

void ImagePackToolDialog::forkParamChangeSaver()
{
    STATE_lastFrokParams.clear();
    STATE_lastFrokParams.reserve(ui->forkParamListWidget->count());
    for (unsigned i = 0, n = ui->forkParamListWidget->count(); i < n; ++i) {
        STATE_lastFrokParams.append(ui->forkParamListWidget->item(i)->text());
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

    info.args.append("--debug");

    ExecuteWindow* w = new ExecuteWindow();
    w->init(info);
    w->show();
    QDialog::accept();
}
