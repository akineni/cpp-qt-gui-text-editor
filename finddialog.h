#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

namespace Ui {
    class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);
    ~FindDialog();

private slots:
    void on_lineEdit_textChanged(const QString &arg1);
    void on_pushButtonFindNext_clicked();
    void on_pushButtonCancel_clicked();

private:
    Ui::FindDialog *ui;

    void showEvent(QShowEvent *event);
};

#endif // FINDDIALOG_H
