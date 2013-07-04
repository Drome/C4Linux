#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QSettings>
#include <QAbstractButton>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

private:
    Ui::ConfigDialog *ui;
    void applySettings();

signals:
    
public slots:
    void buttonBrowseWorkPathClicked();
    void buttonBrowseClonkPathClicked();

    void buttonApplyClicked(QAbstractButton *);
    void accept();
};

#endif // CONFIGDIALOG_H
