#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

namespace Ui {
    class ReplaceDialog;
}

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = nullptr);
    ~ReplaceDialog();

private slots:
    void on_lineEditSearch_textChanged(const QString &arg1);
    void on_pushButtonCancel_clicked();
    void on_pushButtonFindNext_clicked();

    void on_pushButtonReplace_clicked();

    void on_pushButtonReplaceAll_clicked();

private:
    Ui::ReplaceDialog *ui;
    void showEvent(QShowEvent *event);
};

#endif // REPLACEDIALOG_H
