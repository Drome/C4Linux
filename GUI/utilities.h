#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QStringList>

#include <unicode/utypes.h>
#include <unicode/ucsdet.h>

extern "C" {
#include <c4group.h>
#include <sys/stat.h>
}

void unpackC4GroupTo(const QString groupPath, const QString targetPath, bool createPath=true);
void unpackC4GroupTo(const C4GroupFile_t * grp, const QString targetPath, bool createPath=true);

QString heuristicStrEncoding(char *str);

#endif // UTILITIES_H
