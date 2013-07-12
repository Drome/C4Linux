#ifndef INSPECTORDIALOG_H
#define INSPECTORDIALOG_H

#include <functional>
#include <memory>
#include <cstdlib>
#include <QDialog>
#include <QFileInfo>
#include <QFileSystemModel>

#include "utilities.h"
#include "c4fileiconprovider.h"
#include "filesortfilterproxymodel.h"

#include <memfs.h>
extern "C" {
#include <c4group.h>
#include <sys/stat.h>
}

#define VFS_TMP_DIR "/tmp/"

class io_error : std::exception {
    virtual const char* what() const noexcept { return "I/O Error. Most helpful error message ever !"; }
};

class c4grp_error : std::exception {
    virtual const char* what() const noexcept { return C4GROUP_getError(); }
};

QString heuristicStrEncoding(char * str);

namespace Ui {
class InspectorDialog;
}

class InspectorDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit InspectorDialog(QWidget *parent, const QFileInfo &finfo);
    ~InspectorDialog();
    
private:
    void unpackC4Grp();
    void setupVirtualFS();
    void destroyVirtualFS();

    Ui::InspectorDialog *ui;
    std::unique_ptr<C4FileIconProvider> iconProvider;
    std::unique_ptr<QFileSystemModel> fsModel;
    std::unique_ptr<FileSortFilterProxyModel> proxyModel;
    std::unique_ptr<QFileInfo> c4finfo;
    C4GroupFile_t *c4group;
    QString mountpath;

    MemFUSE::MemFS *memfs;
};

#endif // INSPECTORDIALOG_H
