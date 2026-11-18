#include <QCoreApplication>
#include <QDebug>
#include <QImage>
#include <QImageReader>
#include <QString>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const auto formats = QImageReader::supportedImageFormats();
    for (const QByteArray &format : {QByteArray("heif"), QByteArray("heic"), QByteArray("avif")}) {
        if (!formats.contains(format)) {
            qCritical() << "Missing image decoder:" << format << "available:" << formats;
            return 1;
        }
    }

    if (app.arguments().size() != 2) {
        qCritical() << "Usage: qv_imageformat_smoke <test-image.avif>";
        return 2;
    }

    QImageReader reader(app.arguments().at(1));
    const QImage image = reader.read();
    if (image.isNull()) {
        qCritical() << "Could not decode test image:" << reader.errorString();
        return 3;
    }
    if (image.size() != QSize(12, 8)) {
        qCritical() << "Unexpected AVIF image dimensions:" << image.size();
        return 4;
    }
    qInfo() << "HEIF/HEIC and AVIF plugins loaded; AVIF test image decoded:" << image.size();
    return 0;
}
