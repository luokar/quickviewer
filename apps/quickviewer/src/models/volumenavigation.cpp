#include "volumenavigation.h"

#include <QDir>
#include <QFileInfo>

#include <algorithm>
#include <utility>

#include "fileloader.h"

std::optional<VolumeNavigationContext> VolumeNavigation::resolve(const QString &volumePath)
{
    // Keep the path by which the user entered the volume. In particular, a
    // junction/symlink must navigate among its *visible* siblings, not among
    // the siblings of its target elsewhere on disk.
    const QFileInfo currentInfo(QDir::cleanPath(QDir::fromNativeSeparators(volumePath)));
    if (!currentInfo.exists()) {
        return std::nullopt;
    }
    const QString currentName = currentInfo.fileName();
    if (currentName.isEmpty()) {
        return std::nullopt;
    }

    const QString parentPath =
        QDir::cleanPath(QDir::fromNativeSeparators(currentInfo.absolutePath()));
    // Refresh on each volume boundary so files added, renamed or removed by
    // another application are reflected without a separate refresh action.
    QStringList names = siblingNames(parentPath);
    const std::optional<int> currentIndex = indexOf(names, currentName);
    if (!currentIndex) {
        return std::nullopt;
    }

    return VolumeNavigationContext{parentPath, std::move(names), *currentIndex};
}

QStringList VolumeNavigation::siblingNames(const QString &parentPath)
{
    const QDir directory(parentPath);
    QStringList folders =
        directory.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Hidden, QDir::Unsorted);
    IFileLoader::sortFiles(folders);

    // An individual image is a page of its parent directory, not a volume.
    // Treating it as a volume would open the parent and jump out of the series.
    QStringList files =
        directory.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Hidden, QDir::Unsorted);
    files.erase(
        std::remove_if(files.begin(),
                       files.end(),
                       [](const QString &name) { return !IFileLoader::isArchiveFile(name); }),
        files.end());
    IFileLoader::sortFiles(files);
    folders.append(files);
    return folders;
}

std::optional<int> VolumeNavigation::indexOf(const QStringList &siblingNames,
                                             const QString &currentName)
{
    const auto uniqueMatch = [&siblingNames](const auto &matches) {
        int matchCount = 0;
        int matchIndex = -1;
        for (int index = 0; index < siblingNames.size(); ++index) {
            if (!matches(siblingNames.at(index))) {
                continue;
            }
            ++matchCount;
            matchIndex = index;
            if (matchCount > 1) {
                break;
            }
        }
        return std::pair<int, int>{matchCount, matchIndex};
    };

    const auto [exactCount, exactIndex] =
        uniqueMatch([&currentName](const QString &name) { return name == currentName; });
    if (exactCount == 1) {
        return exactIndex;
    }
    if (exactCount > 1) {
        return std::nullopt;
    }

    const QString normalizedCurrent = currentName.normalized(QString::NormalizationForm_C);
    const auto [normalizedCount, normalizedIndex] =
        uniqueMatch([&normalizedCurrent](const QString &name) {
            return name.normalized(QString::NormalizationForm_C) == normalizedCurrent;
        });
    if (normalizedCount == 1) {
        return normalizedIndex;
    }
    if (normalizedCount > 1) {
        return std::nullopt;
    }

    const QString foldedCurrent = normalizedCurrent.toCaseFolded();
    const auto [foldedCount, foldedIndex] = uniqueMatch([&foldedCurrent](const QString &name) {
        return name.normalized(QString::NormalizationForm_C).toCaseFolded() == foldedCurrent;
    });
    if (foldedCount == 1) {
        return foldedIndex;
    }
    return std::nullopt;
}
