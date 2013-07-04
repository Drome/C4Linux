#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>

#include "filesortfilterproxymodel.h"
#include "configdialog.h"
#include "inspectordialog.h"
#include "c4fileiconprovider.h"

#include <exception>
extern "C" {
#include <stdlib.h>
}

namespace Ui {
class editorWindow;
}

class editorWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit editorWindow(QWidget *parent = 0);
    ~editorWindow();

protected:
    void closeEvent(QCloseEvent *);
    
private:
    Ui::editorWindow *ui;
    C4FileIconProvider *iconProvider;
    QFileSystemModel *treeModel;
    FileSortFilterProxyModel *proxyModel;

protected slots:
    void buttonStartClicked();
    void buttonEditClicked();

    void actionOptionsActivated();
    void actionAboutActivated();
    void actionAboutQtActivated();
    void actionPackActivated();
    void actionUnpackActivated();
    void actionInspectActivated();

    void treeViewContextRequested();
};

#endif // EDITORWINDOW_H
