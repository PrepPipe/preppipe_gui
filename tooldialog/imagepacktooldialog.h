#ifndef IMAGEPACKTOOLDIALOG_H
#define IMAGEPACKTOOLDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "ExecutionData.h"
#include "util/filedropaccepter.h"

namespace Ui {
class ImagePackToolDialog;
}

class ImagePackToolDialog : public QDialog
{
    Q_OBJECT

public:
    enum class ForkParamType : unsigned {
        TY_NONE = 0,
        TY_COLOR = 1,
        TY_IMAGE = 2,
        TY_TEXT = 3
    };
    enum class SourceOption : unsigned {
        SRC_PRESET = 0,
        SRC_CREATE = 1,
        SRC_LOAD = 2
    };

public:
    explicit ImagePackToolDialog(QWidget *parent = nullptr);
    ~ImagePackToolDialog();

    void setInitialExecutionInfo(const ExecutionInfo& info) {initialInfo = info;}
    void setLanguageCode(const QString& code) {languageCode = code;}
    virtual void accept() override;

private slots:
    void requestAddColorForkParam();
    void requestAddImageForkParam();
    void requestAddTextForkParam();
    void requestAddNoneForkParam();
    void requestDeleteForkParam();
    void addColorForkParamImpl(QColor color);
    void addImageForkParamImpl(const QString& path);
    void addTextForkParamImpl(QString text, QColor color);
    void handleForkParamEdit(QListWidgetItem* item);
    void forkParamChangeSaver();
    void showPresetInfo();
    void handlePresetInfo();
    void startRetrievePresetInfo();

signals:
    void forkParamChanged();

private:
    static constexpr int FORKPARAMICON_SIZE = 64;

    static QString STATE_lastImagePath;
    static QColor  STATE_lastColor;
    static QString STATE_lastTextContent;
    static QColor  STATE_lastTextColor;
    static QString STATE_lastCreateYAMLPath;
    static QString STATE_lastLoadZIPPath;
    static QString STATE_lastSaveZIPPath;
    static QStringList STATE_lastForkParams;
    static QString STATE_lastExportPath;
    static SourceOption STATE_lastSourceOption;
    static bool STATE_lastSaveOptionEnable;
    static bool STATE_lastForkOptionEnable;
    static bool STATE_lastExportOptionEnable;
    static bool STATE_lastExportOverviewOptionEnable;
    static QString STATE_presetQueryData;
    static QString STATE_presetFromExecutablePath;
    static QString STATE_lastSelectedPresetID;
private:
    QPixmap getIconForTextForkParam(QString text, QColor color);
    QString getMangledTextForTextForkParam(QString text, QColor color);
private:
    Ui::ImagePackToolDialog *ui;
    FileDropAccepter* forkParamDropHandler = nullptr;
    ExecutionInfo initialInfo;
    QHash<QString, QString> presetInfo;
    QString languageCode;
    QString presetQueryData;
};

#endif // IMAGEPACKTOOLDIALOG_H
