#include "configdialog.h"
#include "ui_editorconfdiag.h"

ConfigDialog::ConfigDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    QSettings settings("Drome", "C4Linux");
    ui->createBackupsCheckBox->setCheckState(settings.value("createbackups", true).toBool() ? Qt::Checked : Qt::Unchecked);
    ui->editPackedC4CheckBox->setCheckState(settings.value("editpackedc4", false).toBool() ? Qt::Checked : Qt::Unchecked);
    ui->clonkPathEdit->setText(settings.value("clonkpath").toString());
    ui->workPathEdit->setText(settings.value("workpath").toString());
}

void ConfigDialog::buttonBrowseClonkPathClicked() {
    QSettings settings("Drome", "C4Linux");
    QString clonkPath = QFileDialog::getExistingDirectory(this, "Clonk Path", settings.value("clonkpath").toString(), QFileDialog::ShowDirsOnly);
    if(!clonkPath.isNull())
        ui->clonkPathEdit->setText(clonkPath);
}

void ConfigDialog::buttonBrowseWorkPathClicked() {
    QSettings settings("Drome", "C4Linux");
    QString workPath = QFileDialog::getExistingDirectory(this, "Work Path", settings.value("workpath").toString(), QFileDialog::ShowDirsOnly);
    if(!workPath.isNull())
        ui->workPathEdit->setText(workPath);
}

void ConfigDialog::buttonApplyClicked(QAbstractButton * button) {
    if(button == ui->buttonBox->button(QDialogButtonBox::Apply))
        applySettings();
}

void ConfigDialog::accept() {
    applySettings();
    QDialog::accept();
}

void ConfigDialog::applySettings() {
    QSettings settings("Drome", "C4Linux");
    // General tab
    QString workPath = ui->workPathEdit->text();
    if(!workPath.isEmpty())
        settings.setValue("workpath", workPath);
    // Applications tab
    settings.setValue("editpackedc4", ui->editPackedC4CheckBox->checkState() == Qt::Checked);
    // Clonk tab
    QString clonkPath = ui->clonkPathEdit->text();
    if(!clonkPath.isEmpty())
        settings.setValue("clonkpath", clonkPath);
}
