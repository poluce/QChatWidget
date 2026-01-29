#include "qss_utils.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QWidget>

namespace QssUtils {
QString resolveStyleSheetPath(const QString& fileNameOrPath)
{
    if (fileNameOrPath.startsWith(":/")) {
        return fileNameOrPath;
    }
    if (QFile::exists(fileNameOrPath)) {
        return QFileInfo(fileNameOrPath).absoluteFilePath();
    }

    const QString fileName = QFileInfo(fileNameOrPath).fileName();
    if (fileName.isEmpty()) {
        return QString();
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QDir(appDir).filePath("resources/styles/" + fileName),
        QDir(appDir).filePath("../resources/styles/" + fileName),
        QDir(QDir::currentPath()).filePath("resources/styles/" + fileName),
        QDir(QDir::currentPath()).filePath("../resources/styles/" + fileName)
    };

    for (const QString& path : candidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    return QString();
}

QString loadStyleSheetFile(const QString& fileNameOrPath)
{
    if (fileNameOrPath.trimmed().isEmpty()) {
        return QString();
    }

    const QString resolved = resolveStyleSheetPath(fileNameOrPath);
    const QString fileName = QFileInfo(fileNameOrPath).fileName();
    QString targetPath;

    if (!resolved.isEmpty()) {
        targetPath = resolved;
    } else if (!fileName.isEmpty()) {
        targetPath = QString(":/styles/%1").arg(fileName);
    } else {
        return QString();
    }

    QFile file(targetPath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return QString();
    }
    const QString content = QString::fromUtf8(file.readAll());
    file.close();
    return content;
}

QString buildCombinedStyleSheet(const QString& specificFileNameOrPath, const QString& inlineStyle, const QString& globalFileName)
{
    QString combined = loadStyleSheetFile(globalFileName);

    if (!inlineStyle.trimmed().isEmpty()) {
        if (!combined.isEmpty()) {
            combined += "\n";
        }
        combined += inlineStyle;
        return combined;
    }

    const QString specific = loadStyleSheetFile(specificFileNameOrPath);
    if (!specific.isEmpty()) {
        if (!combined.isEmpty()) {
            combined += "\n";
        }
        combined += specific;
    }
    return combined;
}

bool applyStyleSheetFromFile(QWidget* target, const QString& fileNameOrPath, const QString& globalFileName)
{
    if (!target) {
        return false;
    }

    const QString combined = buildCombinedStyleSheet(fileNameOrPath, QString(), globalFileName);
    target->setStyleSheet(combined);
    return true;
}
} // namespace QssUtils
