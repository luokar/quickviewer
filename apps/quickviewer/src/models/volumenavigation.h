#ifndef VOLUMENAVIGATION_H
#define VOLUMENAVIGATION_H

#include <QStringList>

#include <optional>

struct VolumeNavigationContext
{
    QString parentPath;
    QStringList siblingNames;
    int currentIndex = -1;
};

/** Resolves a volume against the naturally sorted volume entries beside it. */
class VolumeNavigation final
{
public:
    /**
     * Resolves the existing volume path using the logical (symlink-aware)
     * parent directory. Always refreshes the sibling names so changes made
     * outside the application are visible.
     */
    static std::optional<VolumeNavigationContext> resolve(const QString &volumePath);

    /** Lists immediate child directories first, then supported archive files. */
    static QStringList siblingNames(const QString &parentPath);

    /** Matches exact, NFC-equivalent, then case-fold-equivalent names uniquely. */
    static std::optional<int> indexOf(const QStringList &siblingNames, const QString &currentName);
};

#endif // VOLUMENAVIGATION_H
