#ifndef READPROGRESSSTORE_H
#define READPROGRESSSTORE_H

#include <QtGui>
#include <QtCore>
#include <QtConcurrent>
#include <QDir>
#include <QFileInfo>

#include "qvenums.h"

/**
 * Reading position and completion state for one volume.
 */
struct ReadProgress
{
    QString volumeTitle;
    QString volumePath;
    QString currentPageName;
    int totalPageCount;
    int resumePageIndex;
    bool completed;
};

Q_DECLARE_METATYPE(ReadProgress)

/**
 * Stores reading progress by volume path and persists it to progress.ini.
 */
class ReadProgressStore : public QObject
{
    Q_OBJECT
public:
    typedef QMap<QString, ReadProgress> ReadProgressMap;

    ReadProgressStore(QObject *parent);
    void save();

    static ReadProgressMap initializeAsync();

    /** Equivalent spellings of a filesystem volume share a reading position. */
    static QString normalizedPath(const QString &path)
    {
        if (path.isEmpty()) {
            return {};
        }
        QString normalized =
            QDir::cleanPath(QFileInfo(QDir::fromNativeSeparators(path)).absoluteFilePath());
#ifdef Q_OS_WIN
        normalized = normalized.toCaseFolded();
#endif
        return normalized;
    }

    bool contains(QString path)
    {
        path = normalizedPath(path);
        return m_sessionOverrides.contains(path) || m_progressByVolumePath.contains(path);
    }
    ReadProgress at(QString path)
    {
        path = normalizedPath(path);
        const auto sessionOverride = m_sessionOverrides.constFind(path);
        return sessionOverride == m_sessionOverrides.cend() ? m_progressByVolumePath[path]
                                                            : sessionOverride.value();
    }
    void insert(QString path, const ReadProgress &value)
    {
        path = normalizedPath(path);
        m_progressByVolumePath.insert(path, value);
        emit progressChanged(path);
    }
    void insertSessionOverride(QString path, const ReadProgress &value)
    {
        path = normalizedPath(path);
        m_sessionOverrides.insert(path, value);
        emit progressChanged(path);
    }
    void moveToThread(QThread *targetThread);

public slots:
    void handleInitializationFinished();

signals:
    /** The progress of the volume at \a path was inserted or replaced. */
    void progressChanged(QString path);
    /** The stored progress of every volume has been read from the settings. */
    void progressLoaded();

private:
    ReadProgressMap m_progressByVolumePath;
    ReadProgressMap m_sessionOverrides;
    QFutureWatcher<ReadProgressMap> m_initializeWatcher;
};

#endif // READPROGRESSSTORE_H
