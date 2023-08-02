#ifndef OUTPUTWIDGET_H
#define OUTPUTWIDGET_H

#include <QWidget>
#include <QDateTime>
#include "ExecutionData.h"

namespace Ui {
class OutputWidget;
}

class OutputWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OutputWidget(QWidget *parent = nullptr);
    ~OutputWidget();

    void setData(const QString& fieldName, const QString& path);

    static QDateTime getLatestModificationInDir(const QString& dirpath);

public slots:
    void updateStatus();
    void requestOpenContainingDirectory();
    void requestOpen();

private:
    Ui::OutputWidget *ui;
    QString fieldName;
    QString path;
    QDateTime lastModified; // 保存刚刚创建时给定路径文件的修改时间，这样可以发现覆盖操作
};

#endif // OUTPUTWIDGET_H
