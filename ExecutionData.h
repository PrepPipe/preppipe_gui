#ifndef EXECUTIONDATA_H
#define EXECUTIONDATA_H

#include <QString>
#include <QList>
#include <QHash>
#include <QStringList>

struct OutputInfo {
    // 一个输出项的所有信息
    QString fieldName;
    int argindex = -1;
};

struct UnspecifiedPathInfo {
    bool isDir = false;
    QString filter;
    QString defaultName;
};

struct ExecutionInfo {
    QString program;
    QStringList args;
    QHash<QString, QString> envs;
    QHash<int, UnspecifiedPathInfo> unspecifiedPaths;
    QList<OutputInfo> specifiedOutputs;
};

inline QString getMergedCommand(const QString& program, const QStringList& args) {
    QString mergedstr;
    if (program.contains(' ')) {
        mergedstr += '"';
        mergedstr += program;
        mergedstr += '"';
    } else {
        mergedstr = program;
    }
    for (const QString& a : args) {
        if (mergedstr.length() > 0) {
            mergedstr += ' ';
        }
        if (a.contains(' ')) {
            QString copy(a);
            copy.replace('"', "\\\"");
            mergedstr += '"';
            mergedstr += copy;
            mergedstr += '"';
        } else {
            mergedstr += a;
        }
    }
    return mergedstr;
}

#endif // EXECUTIONDATA_H
