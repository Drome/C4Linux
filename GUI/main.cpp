#include <QApplication>
#include <memfs.h>
#include "editorwindow.h"

int main(int argc, char *argv[])
{
    MemFUSE::init();

    QApplication a(argc, argv);
    editorWindow w;
    w.show();
    
    return a.exec();
}
