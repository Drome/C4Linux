#ifndef FILESORTFILTERPROXYMODEL_H
#define FILESORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QFileSystemModel>

class FileSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FileSortFilterProxyModel(QObject *parent = 0);
    
protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // FILESORTFILTERPROXYMODEL_H
