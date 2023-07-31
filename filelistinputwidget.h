#ifndef FILELISTINPUTWIDGET_H
#define FILELISTINPUTWIDGET_H

#include <QWidget>

namespace Ui {
class FileListInputWidget;
}

class FileListInputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileListInputWidget(QWidget *parent = nullptr);
    ~FileListInputWidget();

    void setDirectoryMode(bool v);
    void setVerifyCallBack(std::function<bool(const QString&)> cb) {
        verifyCB = cb;
    }
    void setExistingOnly(bool v) {
        isExistingOnly = v;
    }
    void setFieldName(const QString& name);
    QString getFieldName() const {return fieldName;}
    void setFilter(const QString& filter) {
        this->filter = filter;
    }
    QString getFilter() const {return filter;}
    QStringList getCurrentList() const;

private slots:
    void itemMoveUp();
    void itemMoveDown();
    void itemAdd();
    void itemRemove();

public slots:
    void addPath(const QString& path);

signals:
    void listChanged();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *e) override;
    virtual void dropEvent(QDropEvent *event) override;

private:
    bool isDirectoryMode = false;
    bool isExistingOnly = false;
    std::function<bool(const QString&)> verifyCB;
    QString fieldName;
    QString filter;
    QString lastAddedPath;

private:
    Ui::FileListInputWidget *ui;
};

#endif // FILELISTINPUTWIDGET_H
