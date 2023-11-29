#ifndef FILEDROPACCEPTER_H
#define FILEDROPACCEPTER_H

#include <QObject>

class FileDropAccepter : public QObject
{
    Q_OBJECT
public:
    explicit FileDropAccepter(QObject *parent = nullptr);

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    void setVerifyCallBack(std::function<bool(const QString&)> cb) {
        verifyCB = cb;
    }

signals:
    void fileDropped(const QString& path, QObject* droppedTo);

private:
    std::function<bool(const QString&)> verifyCB;

};

#endif // FILEDROPACCEPTER_H
