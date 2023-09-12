#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ExecutionData.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct ExecuteInfo
{
    QStringList args;
    QHash<QString, QString> envs;
    QHash<int, UnspecifiedPathInfo> unspecfiedPaths;
    QStringList errors;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static constexpr int LONGSAYSPLITTING_MIN_START_LENGTH = 180;
    static constexpr int LONGSAYSPLITTING_TARGET_LENGTH = 60;

private slots:
    void search_exe();
    void settingsChanged();
    void inputListChanged();
    void requestLaunch();
    void requestTranslationExport();
    void requestTranslationExportImpl(const QString& path);

private:
    Ui::MainWindow *ui;
    ExecutionInfo curInfo;
};
#endif // MAINWINDOW_H
