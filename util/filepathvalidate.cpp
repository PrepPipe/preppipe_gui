#include "filepathvalidate.h"

#include <QSet>
#include <QFileInfo>
namespace {

const QSet<QString> SupportedImageExtensions {
    QStringLiteral("blp"),
    QStringLiteral("bmp"),
    QStringLiteral("dds"),
    QStringLiteral("dib"),
    QStringLiteral("eps"),
    QStringLiteral("gif"),
    QStringLiteral("icns"),
    QStringLiteral("ico"),
    QStringLiteral("im"),
    QStringLiteral("jfif"),
    QStringLiteral("jpg"),
    QStringLiteral("jpeg"),
    QStringLiteral("j2k"),
    QStringLiteral("j2p"),
    QStringLiteral("j2x"),
    QStringLiteral("msp"),
    QStringLiteral("pcx"),
    QStringLiteral("png"),
    QStringLiteral("apng"),
    QStringLiteral("pbm"),
    QStringLiteral("pgm"),
    QStringLiteral("ppm"),
    QStringLiteral("pnm"),
    QStringLiteral("sgi"),
    QStringLiteral("spi"),
    QStringLiteral("tga"),
    QStringLiteral("tif"),
    QStringLiteral("tiff"),
    QStringLiteral("webp"),
    QStringLiteral("xbm"),
};

} // end anonymous namespace

bool FilePathValidate::isReadableImageFile(const QString& path)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isFile() || !info.isReadable())
        return false;
    return (SupportedImageExtensions.contains(info.suffix().toLower()));
}
