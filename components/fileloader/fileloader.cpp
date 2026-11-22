#include "fileloader.h"

#include <algorithm>
#include <QCollator>

#include <QImageReader>

#include "imageformat.h"

#ifdef Q_OS_WIN
#    include <Shlwapi.h>
#endif

namespace {

QString fileSuffix(const QString &path)
{
    return QFileInfo(path).suffix().toLower();
}

const QList<QByteArray> &supportedImageFormats()
{
    static const QList<QByteArray> formats = [] {
        QList<QByteArray> result = QImageReader::supportedImageFormats();
        const QList<QByteArray> extraFormats{
            "jpe",
            "jif",
            "jfif",
            "jfi",
            "heic",
            "heif",
        };
        for (const QByteArray &format : extraFormats) {
            if (!result.contains(format)) {
                result.append(format);
            }
        }
        return result;
    }();
    return formats;
}

const QStringList &exifRawImageFormats()
{
    static const QStringList formats{"crw", "cr2", "arw", "nef", "raf", "dng"};
    return formats;
}

const QStringList &archiveFormats()
{
    // Keep recognition in sync with the format aliases used by VolumeLoader.
    // Without this list the loader can open an archive that Folder cannot
    // display, bookmark or navigate to.
    static const QStringList formats{"zip",
                                     "7z",
                                     "rar",
                                     "cbr",
                                     "cbz",
                                     "cb7",
                                     "tar",
                                     "tar.gz",
                                     "tar.bz2",
                                     "tar.xz",
                                     "tgz",
                                     "tbz2",
                                     "txz"};
    return formats;
}

} // namespace

bool IFileLoader::isImageFile(QString path)
{
    return supportedImageFormats().contains(fileSuffix(path).toLatin1());
}

bool IFileLoader::supportsImageFormat(const QByteArray &format)
{
    return QImageReader::supportedImageFormats().contains(format.toLower());
}

QByteArray IFileLoader::jpegQtFormatName()
{
    if (supportsImageFormat(turboJpegFormatName())) {
        return QByteArray(turboJpegFormatName());
    }
    return QByteArrayLiteral("jpg");
}

bool IFileLoader::isArchiveFile(QString path)
{
    const QFileInfo info(path);
    return archiveFormats().contains(info.suffix().toLower()) ||
           archiveFormats().contains(info.completeSuffix().toLower());
}

bool IFileLoader::isExifJpegImageFile(QString path)
{
    return imageFormatFromPath(path) == ImageFormat::Jpeg;
}

bool IFileLoader::isExifRawImageFile(QString path)
{
    return exifRawImageFormats().contains(fileSuffix(path));
}

bool IFileLoader::isAnimatedImageFile(QString path)
{
    const ImageFormat format = imageFormatFromPath(path);
    return format == ImageFormat::Gif || format == ImageFormat::Apng;
}

void IFileLoader::sortFiles(QStringList &filenames)
{
#ifdef Q_OS_WIN
    std::stable_sort(filenames.begin(), filenames.end(), caseInsensitiveLessThan);
#else
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);
    std::stable_sort(
        filenames.begin(), filenames.end(), [&collator](const QString &left, const QString &right) {
            return collator.compare(left, right) < 0;
        });
#endif
}

#ifdef Q_OS_WIN
bool IFileLoader::caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
    return ::StrCmpLogicalW(reinterpret_cast<LPCWSTR>(s1.utf16()),
                            reinterpret_cast<LPCWSTR>(s2.utf16())) < 0;
}
#else
bool IFileLoader::caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
    return s1.toCaseFolded() < s2.toCaseFolded();
}
#endif

quint64 IFileLoader::getFileSize(QString filename) const
{
    Q_UNUSED(filename);
    return 0;
}

QDateTime IFileLoader::getFileModified(QString filename) const
{
    Q_UNUSED(filename);
    return QDateTime();
}
