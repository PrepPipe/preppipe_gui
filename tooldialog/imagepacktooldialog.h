#ifndef IMAGEPACKTOOLDIALOG_H
#define IMAGEPACKTOOLDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "ExecutionData.h"

namespace Ui {
class ImagePackToolDialog;
}

class ImagePackToolDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImagePackToolDialog(QWidget *parent = nullptr);
    ~ImagePackToolDialog();

    void setInitialExecutionInfo(const ExecutionInfo& info) {initialInfo = info;}
    virtual void accept() override;

private slots:
    void requestAddColorForkParam();
    void requestAddImageForkParam();
    void requestAddNoneForkParam();
    void requestDeleteForkParam();
    void addColorForkParamImpl(QColor color);
    void addImageForkParamImpl(const QString& path);
    void handleForkParamEdit(QListWidgetItem* item);
    void forkParamChangeSaver();

signals:
    void forkParamChanged();

private:
    static constexpr int FORKPARAMICON_SIZE = 64;

    static QString STATE_lastImagePath;
    static QColor  STATE_lastColor;
    static QString STATE_lastCreateYAMLPath;
    static QString STATE_lastLoadZIPPath;
    static QString STATE_lastSaveZIPPath;
    static QStringList STATE_lastFrokParams;
    static QString STATE_lastExportPath;
    static bool STATE_lastCreateOptionEnable;
    static bool STATE_lastSaveOptionEnable;
    static bool STATE_lastForkOptionEnable;
    static bool STATE_lastExportOptionEnable;
private:
    Ui::ImagePackToolDialog *ui;
    ExecutionInfo initialInfo;
};

#endif // IMAGEPACKTOOLDIALOG_H
