#include <QPushButton>
#include "textpad.h"
#include "replacedialog.h"
#include "ui_replacedialog.h"

ReplaceDialog::ReplaceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReplaceDialog)
{
    ui->setupUi(this);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

void ReplaceDialog::on_lineEditSearch_textChanged(const QString &arg1)
{
    ui->pushButtonFindNext->setDisabled(ui->lineEditSearch->text().isEmpty());
    ui->pushButtonReplace->setDisabled(ui->lineEditSearch->text().isEmpty());
    ui->pushButtonReplaceAll->setDisabled(ui->lineEditSearch->text().isEmpty());
}

void ReplaceDialog::on_pushButtonCancel_clicked()
{
    close();
}

void ReplaceDialog::on_pushButtonFindNext_clicked()
{
    ((TextPad*)(this->parent()))->find_next(this,
                                            ui->lineEditSearch->text(),
                                            ui->checkBoxMatchCase->isChecked(),
                                            ui->checkBoxWholeWord->isChecked(),
                                            ui->radioButtonDown->isChecked());
}

void ReplaceDialog::showEvent(QShowEvent *event)
{
    TextPad *parentWindow = (TextPad*)(this->parent());
    ui->checkBoxMatchCase->setChecked(parentWindow->getMatchCase());
    ui->checkBoxWholeWord->setChecked(parentWindow->getWholeWord());
    ui->lineEditSearch->setText(parentWindow->getSearch());
    ui->lineEditReplace->setText(parentWindow->getReplace());
    ui->radioButtonDown->setChecked(parentWindow->getDown());
    ui->radioButtonUp->setChecked(!parentWindow->getDown());
    ui->lineEditSearch->setFocus();

    QDialog::showEvent(event);
}

void ReplaceDialog::on_pushButtonReplace_clicked()
{
    if(!((TextPad*)(this->parent()))->replace_preface()){
        ((TextPad*)(this->parent()))->find_next(
                    this,
                    ui->lineEditSearch->text(),
                    ui->checkBoxMatchCase->isChecked(),
                    ui->checkBoxWholeWord->isChecked(),
                    ui->radioButtonDown->isChecked());
        return;
    }

    ((TextPad*)(this->parent()))->replace(ui->lineEditSearch->text(),
                                          ui->lineEditReplace->text(),
                                          ui->checkBoxMatchCase->isChecked(),
                                          ui->checkBoxWholeWord->isChecked(),
                                          ui->radioButtonDown->isChecked());

    ((TextPad*)(this->parent()))->find_next(this);
}

void ReplaceDialog::on_pushButtonReplaceAll_clicked()
{
    ((TextPad*)(this->parent()))->replace_all(ui->lineEditSearch->text(),
                                          ui->lineEditReplace->text(),
                                          ui->checkBoxMatchCase->isChecked(),
                                          ui->checkBoxWholeWord->isChecked(),
                                          ui->radioButtonDown->isChecked());
}
