#ifndef FILESELECTIONWIDGET_H
#define FILESELECTIONWIDGET_H

#include <QWidget>
#include <functional>

namespace Ui {
class FileSelectionWidget;
}

class FileSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileSelectionWidget(QWidget *parent = nullptr);
    ~FileSelectionWidget();

    void setDirectoryMode(bool v);
    bool getIsDirectoryMode() const {return isDirectoryMode;}

    static std::function<bool(const QString&)> getDefaultVerifier(bool isDirectoryMode);

    void setVerifyCallBack(std::function<bool(const QString&)> cb) {
        verifyCB = cb;
    }
    void setIsOutputInsteadofInput(bool v) {
        isOutputInsteadofInput = v;
    }
    bool getIsOutputInsteadofInput() const {return isOutputInsteadofInput;}
    void setExistingOnly(bool v) {
        isExistingOnly = v;
    }
    void setFieldName(const QString& name) {
        fieldName = name;
        updateLabelText();
    }
    QString getFieldName() const {return fieldName;}
    void setFilter(const QString& filter) {
        this->filter = filter;
    }
    QString getFilter() const {return filter;}
    QString getCurrentPath() const {return currentPath;}

    void setDefaultName(const QString& name) {
        defaultName = name;
    }
    QString getDefaultName() const {return defaultName;}

public slots:
    void setCurrentPath(const QString& newpath);

private slots:
    void requestOpenDialog();
    void updateLabelText();

signals:
    void filePathUpdated(const QString& newpath);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *e) override;
    virtual void dropEvent(QDropEvent *event) override;

private:
    Ui::FileSelectionWidget *ui;
    bool isDirectoryMode = false;
    bool isOutputInsteadofInput = false;
    bool isExistingOnly = false;
    std::function<bool(const QString&)> verifyCB;
    QString currentPath;
    QString fieldName;
    QString filter;
    QString defaultName; // 只为了输出时可以不指定路径，使用临时文件
};

#endif // FILESELECTIONWIDGET_H
