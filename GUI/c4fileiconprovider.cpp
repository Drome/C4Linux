#include "c4fileiconprovider.h"

QIcon C4FileIconProvider::icon(const QFileInfo &info) const {
    if(info.suffix().endsWith("c", Qt::CaseInsensitive))
        return QIcon::fromTheme("text-x-script");
    if(info.suffix().endsWith("c4d", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4d.png");
    if(info.suffix().endsWith("c4f", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4f.png");
    if(info.suffix().endsWith("c4g", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4g.png");
    if(info.suffix().endsWith("c4k", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4k.png");
    if(info.suffix().endsWith("c4p", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4p.png");
    if(info.suffix().endsWith("c4s", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4s.png");
    if(info.suffix().endsWith("c4u", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4u.png");
    if(info.suffix().endsWith("c4i", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4i.ico");
    if(info.suffix().endsWith("c4l", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4l.ico");
    if(info.suffix().endsWith("c4b", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4b.ico");
    if(info.suffix().endsWith("c4m", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4m.ico");
    if(info.suffix().endsWith("c4v", Qt::CaseInsensitive))
        return QIcon(":/icons/fileIcons/c4v.ico");
    if((!info.fileName().compare("Clonk", Qt::CaseInsensitive)) || (!info.fileName().compare("Clonk.exe", Qt::CaseInsensitive)) || (!info.fileName().compare("Clonk.app", Qt::CaseInsensitive)) || (!info.fileName().compare("Clonk64", Qt::CaseInsensitive)))
        return QIcon(":/icons/fileIcons/c4x.ico");
    return QFileIconProvider::icon(info);
}
