#include "editorwindow.h"
#include "ui_editorwindow.h"

editorWindow::editorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::editorWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/icons/windowIcons/dev.png"));
    QSettings settings("Drome", "C4Linux");
    if(settings.contains("geometry")) {
        this->restoreGeometry(settings.value("geometry").toByteArray());
    }

    if(!settings.contains("clonkpath")) {
        QMessageBox::information(0, "No Clonk path found!", "Please enter the path of your Clonk installation in the following dialog.", QMessageBox::Ok);
        QString inputPath = QFileDialog::getExistingDirectory(this, "Clonk Path", QDir::currentPath(), QFileDialog::ShowDirsOnly);
        if(inputPath.isNull())
            exit(EXIT_FAILURE);
        settings.setValue("clonkpath", inputPath);
    }
    QString clonkpath = settings.value("clonkpath").toString();
    if(!settings.contains("workpath"))
        settings.setValue("workpath", clonkpath);
    QString workpath = settings.value("workpath").toString();

    treeModel = new QFileSystemModel(this);
    proxyModel = new FileSortFilterProxyModel(this);
    iconProvider = new C4FileIconProvider;
    treeModel->setIconProvider(iconProvider);
    treeModel->setRootPath(workpath);
    proxyModel->setSourceModel(treeModel);
    //proxyModel->setDynamicSortFilter(true);
    ui->treeView->setModel(proxyModel);
    ui->treeView->setSortingEnabled(true);
    ui->treeView->sortByColumn(0, Qt::AscendingOrder);
    ui->treeView->header()->setStretchLastSection(false);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    //ui->treeView->hideColumn(1); ui->treeView->hideColumn(2); ui->treeView->hideColumn(3);
    ui->treeView->setRootIndex(proxyModel->mapFromSource(treeModel->index(workpath)));
}

editorWindow::~editorWindow()
{
    delete ui;
    delete treeModel;
    delete proxyModel;
    delete iconProvider;
}

void editorWindow::closeEvent(QCloseEvent * ce) {
    QSettings settings("Drome", "C4Linux");
    settings.setValue("geometry", saveGeometry());
    QWidget::closeEvent(ce);
}

void editorWindow::buttonStartClicked() {
    //int retVal = system("");
    ui->statusBar->showMessage("Started!", 5000);
}

void editorWindow::buttonEditClicked() {
    QModelIndexList mindl = ui->treeView->selectionModel()->selectedIndexes();
    if(mindl.isEmpty()) return;
    QFileInfo finfo = treeModel->fileInfo(proxyModel->mapToSource(mindl.first()));
    QString fsuffix = finfo.suffix();
    if((fsuffix.length() == 3) && (fsuffix.startsWith("c4", Qt::CaseInsensitive)) && (fsuffix != "c4k")) {
        if(finfo.isDir()) {
            QMessageBox::information(0, "Edit", "Direct editing of C4*-files is not implemented yet.", QMessageBox::Ok);
            return;
        }
        else {
            QSettings settings("Drome", "C4Linux");
            if(!settings.value("editpackedc4", false).toBool()) {
                QMessageBox::information(0, "Edit", "You should unpack your C4*-files before editing it. If you want to edit them externally, please enable this in the options.", QMessageBox::Ok);
                return;
            }
        }
    }
    if(finfo.isDir()) {
        QMessageBox::information(0, "Edit", "Editing directories is not a reasonable move. Please reconsider your past decisions and try again.", QMessageBox::Ok);
        return;
    }
    else {
        if(finfo.isExecutable()) {
            QMessageBox::information(0, "Edit", "Editing an executable? I'm not sure about that...", QMessageBox::Ok);
            return;
        }
    }
    int retVal = system(QString("xdg-open \"%1\"").arg(finfo.absoluteFilePath()).toStdString().c_str());
    if(!retVal)
        ui->statusBar->showMessage("External editor starting...", 5000);
    if(retVal == 1)
        ui->statusBar->showMessage("Commandline error. That was unexpected.", 5000);
    if(retVal == 2)
        ui->statusBar->showMessage("File does not exist. What?", 5000);
    if(retVal == 3)
        ui->statusBar->showMessage("No editor found!", 5000);
    if(retVal == 4)
        ui->statusBar->showMessage("Failure. Somehow.", 5000);
}

void editorWindow::actionOptionsActivated() {
    ConfigDialog diag(this);
    diag.exec();
    QSettings settings("Drome", "C4Linux");
    if(treeModel->rootPath() != settings.value("workpath").toString()) {
        ui->statusBar->showMessage("Reloading tree...");
        treeModel->setRootPath(settings.value("workpath").toString());
        ui->treeView->setRootIndex(proxyModel->mapFromSource(treeModel->index(settings.value("workpath").toString())));
    }
}

void editorWindow::actionAboutActivated() {
    QMessageBox::about(this, "About C4Linux", "C4Linux");
}

QFileInfo editorWindow::getSelectedFileInfo() {
    QModelIndexList mindl = ui->treeView->selectionModel()->selectedIndexes();
    if(mindl.isEmpty()) return QFileInfo();
    QFileInfo finfo = treeModel->fileInfo(proxyModel->mapToSource(mindl.first()));
    if(!finfo.exists()) {
        QMessageBox::information(0, "Inspector", "File does not exist. Invalid symlink maybe?", QMessageBox::Ok);
        return QFileInfo();
    }
    return finfo;
}

void editorWindow::actionPackActivated() {

}

void editorWindow::actionUnpackActivated() {
    QFileInfo finfo = getSelectedFileInfo();
    if(!finfo.exists()) return;
    if(!finfo.isFile()) {
        QMessageBox::information(0, "Unpack", "This is a directory. You do not need to unpack directories.", QMessageBox::Ok);
        return;
    }
    std::string targetPath = finfo.absoluteFilePath().toStdString();
    std::string backupPath = finfo.absoluteFilePath().append(QString(".%1.bak").arg(time(NULL))).toStdString();
    QMessageBox::information(0, "Inspector", QString::fromStdString(backupPath), QMessageBox::Ok);
    rename(targetPath.c_str(), backupPath.c_str());
    unpackC4GroupTo(QString::fromStdString(backupPath), QString::fromStdString(targetPath));
}

void editorWindow::actionInspectActivated() {
    QFileInfo finfo = getSelectedFileInfo();
    if(!finfo.exists()) return;
    if(!finfo.isFile()) {
        QMessageBox::information(0, "Inspector", "There is no need to inspect a directory.", QMessageBox::Ok);
        return;
    }
    try {
        InspectorDialog diag(this, finfo);
        diag.exec();
    }
    catch (std::exception e) {
    }
}

void editorWindow::actionAboutQtActivated(){
    QMessageBox::aboutQt(this, "About Qt");
}

void editorWindow::treeViewContextRequested() {
    QMenu contextMenu;
    contextMenu.addAction("Action0!");
    contextMenu.addAction("Action1!");
    contextMenu.addAction("Action2!");

    contextMenu.exec(QCursor::pos());
}
