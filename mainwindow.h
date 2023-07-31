#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct UnspecifiedPathInfo {
    bool isDir = false;
    QString filter;
    QString defaultName;
};

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

private:
    Ui::MainWindow *ui;
    ExecuteInfo curInfo;
};
#endif // MAINWINDOW_H
