#ifndef C4FILEICONPROVIDER_H
#define C4FILEICONPROVIDER_H

#include <QFileIconProvider>

class C4FileIconProvider : public QFileIconProvider
{
public:
    C4FileIconProvider() {}

    QIcon icon ( const QFileInfo & info ) const;
    QIcon icon (QFileIconProvider::IconType type) const { return QFileIconProvider::icon(type); }
};

#endif // C4FILEICONPROVIDER_H
