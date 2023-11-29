#include "filedropaccepter.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

FileDropAccepter::FileDropAccepter(QObject *parent)
    : QObject{parent}
{

}

bool FileDropAccepter::eventFilter(QObject *watched, QEvent *event)
{
    // 因为 QDragEnterEvent 继承自 QDragMoveEvent, 该事件又继承自 QDropEvent
    // 所以这里 if-else 的顺序不能调换
    if (QDragEnterEvent* e = dynamic_cast<QDragEnterEvent*>(event)) {
        if (e->mimeData()->hasUrls()) {
            QString path = e->mimeData()->urls().first().toLocalFile();
            if (!verifyCB || verifyCB(path)) {
                e->acceptProposedAction();
                return true;
            }
        }
    } else if (QDragMoveEvent* e = dynamic_cast<QDragMoveEvent*>(event)) {
        if (e->mimeData()->hasUrls()) {
            return true;
        }
    } else if (QDropEvent* e = dynamic_cast<QDropEvent*>(event)) {
        bool isHandled = false;
        for (const QUrl &url : e->mimeData()->urls()) {
            QString path = url.toLocalFile();
            isHandled = true;
            if (!verifyCB || verifyCB(path)) {
                emit fileDropped(path, watched);
            }
        }
        if (isHandled) {
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}
