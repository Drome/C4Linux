#include "filesortfilterproxymodel.h"

FileSortFilterProxyModel::FileSortFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

bool FileSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    //const QFileSystemModel * model = qobject_cast<const QFileSystemModel *>(left.model());
    //if(( (model->isDir(left)) && (model->isDir(right)) ) || ( (!model->isDir(left)) && (!model->isDir(right))))
        return left.data().toString().toCaseFolded() < right.data().toString().toCaseFolded();
    //else
    //    return (model->isDir(left));
}
