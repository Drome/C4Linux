#include "inspectordialog.h"
#include "ui_editorinspectordiag.h"

using namespace MemFUSE;

InspectorDialog::InspectorDialog(QWidget *parent, const QFileInfo &finfo) :
    QDialog(parent),
    ui(new Ui::InspectorDialog)
{
    ui->setupUi(this);
    c4finfo = std::unique_ptr<QFileInfo>(new QFileInfo(finfo)); // This might be needed later on...

    c4group = NULL;
    unpackC4Grp();

    setupVirtualFS();

    fsModel = std::unique_ptr<QFileSystemModel>(new QFileSystemModel(this));
    proxyModel = std::unique_ptr<FileSortFilterProxyModel>(new FileSortFilterProxyModel(this));
    iconProvider = std::unique_ptr<C4FileIconProvider>(new C4FileIconProvider);
    fsModel->setIconProvider(iconProvider.get());
    fsModel->setRootPath(mountpath);
    proxyModel->setSourceModel(fsModel.get());
    //proxyModel->setDynamicSortFilter(true);
    ui->treeView->setModel(proxyModel.get());
    ui->treeView->setSortingEnabled(true);
    ui->treeView->sortByColumn(0, Qt::AscendingOrder);
    ui->treeView->header()->setStretchLastSection(false);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    //ui->treeView->hideColumn(1);
    //ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
    ui->treeView->setRootIndex(proxyModel->mapFromSource(fsModel->index(mountpath)));


    /*
    ui->setupUi(this);
    model = new QStandardItemModel(this);
    ui->treeView->setModel(model);
    iconProvider = new C4FileIconProvider();
    c4finfo = new QFileInfo(finfo);
    c4group = NULL;
    success = true;

    QStandardItem * root = new QStandardItem(iconProvider->icon(*c4finfo), (c4finfo->fileName()));
    root->setData(qVariantFromValue((void*) c4group));

    model->setItem(0,0,root);*/
}

void InspectorDialog::unpackC4Grp() {
    size_t fsize = (size_t) c4finfo->size();
    FILE * f = fopen(c4finfo->absoluteFilePath().toStdString().c_str(), "rb");

    if(!f) throw io_error();
    std::unique_ptr<unsigned char> buff = std::unique_ptr<unsigned char>(new unsigned char[fsize]); // Smart pointers + unsigned char buffers!
    if(fread(buff.get(),1,fsize,f) != fsize) throw io_error();
    if(ferror(f)) throw io_error();
    fclose(f);

    c4group = C4GROUP_unpack(buff.get(), fsize, true, true, c4finfo->fileName().toStdString().c_str());
    if(!c4group) throw c4grp_error();
}

QString heuristicStrEncoding(char *str) {
    return QString(str);
}

void InspectorDialog::setupVirtualFS() {
    mountpath.append(VFS_TMP_DIR);
    mountpath.append(QString("%1_%2").arg(c4finfo->fileName()).arg(rand()));
    mkdir(mountpath.toStdString().c_str(), 0777);

    memfs = new MemFS(mountpath.toStdString());

    std::function<void(Directory *,C4GroupFile_t *)> addFunc = [&addFunc](Directory *cd, C4GroupFile_t *grp) {
        for(unsigned int i=0; i<grp->header.fileCount; i++) {
            C4File_t f = grp->files[i];
            cd->addFile(std::unique_ptr<File>(new File(heuristicStrEncoding(f.name).toStdString(), f.content, f.size))); // rvalue references!
        }
        for(unsigned int i=0; i<grp->header.groupCount; i++) {
            C4GroupFile_t * subGrp = grp->groupFiles[i];
            std::unique_ptr<Directory> subDir = std::unique_ptr<Directory>(new Directory(heuristicStrEncoding(subGrp->header.name).toStdString()));
            addFunc(subDir.get(), subGrp);
            cd->addDir(subDir);
        }
    };
    addFunc(memfs->getRoot(), c4group);

    memfs->mount();
}

void InspectorDialog::destroyVirtualFS() {
    delete memfs;
}

InspectorDialog::~InspectorDialog()
{
    delete ui;
    destroyVirtualFS();
    if(c4group) C4GROUP_freeGroup(c4group, false);
    rmdir(mountpath.toStdString().c_str());
}
