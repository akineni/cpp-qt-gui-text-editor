#include <QPushButton>
#include <QMessageBox>
#include "textpad.h"
#include "finddialog.h"
#include "ui_finddialog.h"
#include "ui_textpad.h"

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    ui->pushButtonFindNext->setDefault(true);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::on_lineEdit_textChanged(const QString &arg1)
{
    ui->pushButtonFindNext->setDisabled(ui->lineEdit->text().isEmpty());
}

void FindDialog::showEvent(QShowEvent *event)
{
    TextPad *parentWindow = (TextPad*)(this->parent());
    ui->checkBoxMatchCase->setChecked(parentWindow->getMatchCase());
    ui->checkBoxWholeWord->setChecked(parentWindow->getWholeWord());
    ui->lineEdit->setText(parentWindow->getSearch());
    ui->radioButtonDown->setChecked(parentWindow->getDown());
    ui->radioButtonUp->setChecked(!parentWindow->getDown());
    ui->lineEdit->setFocus();

    QDialog::showEvent(event);
}

void FindDialog::on_pushButtonFindNext_clicked()
{

    ((TextPad*)(this->parent()))->find_next(this,
                                            ui->lineEdit->text(),
                                            ui->checkBoxMatchCase->isChecked(),
                                            ui->checkBoxWholeWord->isChecked(),
                                            ui->radioButtonDown->isChecked());
}

void FindDialog::on_pushButtonCancel_clicked()
{
    close();
}
