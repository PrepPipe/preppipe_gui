#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ExecutionData.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static constexpr int LONGSAYSPLITTING_MIN_START_LENGTH = 180;
    static constexpr int LONGSAYSPLITTING_TARGET_LENGTH = 60;

    // 构建适用于任何流程（包括正常的编译流程和使用工具的部分）的初始命令信息
    // 如果关键项有问题（比如没有指定主程序）就返回 false
    bool populateInitialExecutionInfo(ExecutionInfo& info);

private slots:
    void search_exe();
    void settingsChanged();
    void inputListChanged();
    void requestLaunch();
    void requestTranslationExport();
    void requestTranslationExportImpl(const QString& path);
    void requestImagePackTool();
    void requestLaunchNewUI();

private:
    Ui::MainWindow *ui;
    ExecutionInfo curInfo;
};
#endif // MAINWINDOW_H
