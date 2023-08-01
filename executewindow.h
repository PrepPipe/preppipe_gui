#ifndef EXECUTEWINDOW_H
#define EXECUTEWINDOW_H

#include <QMainWindow>
#include <QTemporaryDir>
#include <QProcess>
#include <QVBoxLayout>
#include "ExecutionData.h"

namespace Ui {
class ExecuteWindow;
}

class ExecuteWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ExecuteWindow(QWidget *parent = nullptr);
    ~ExecuteWindow();

    void init(const ExecutionInfo& info);

signals:
    void executionFinished();

private:
    static int windowindex;

private:
    Ui::ExecuteWindow *ui;
    QVBoxLayout* outputLayout;
    QTemporaryDir tmpdir;
    QProcess proc;
};

#endif // EXECUTEWINDOW_H
