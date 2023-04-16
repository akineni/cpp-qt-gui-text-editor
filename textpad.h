#ifndef TEXTPAD_H
#define TEXTPAD_H

#include <QMainWindow>
#include <QPrinter>
#include <QPrintDialog>
#include <QPageSetupDialog>
#include "finddialog.h"
#include "replacedialog.h"

namespace Ui {
    class TextPad;
}

enum NoteState : unsigned char {
    Untouched, Saved, Unsaved
};

class TextPad : public QMainWindow
{
    Q_OBJECT

public:
    explicit TextPad(QWidget *parent = nullptr);
    ~TextPad();

    void find_next(QWidget* parent, QString search, bool match_case, bool whole_word, bool down);
    void find_next(QWidget *parent);

    void replace(QString search,
                 QString rep, bool match_case, bool whole_word, bool down);
    void replace_all(QString search, QString replace,
                    bool match_case, bool whole_word, bool down);

    bool getMatchCase() const;
    void setMatchCase(bool value);

    bool getWholeWord() const;
    void setWholeWord(bool value);

    bool getDown() const;
    void setDown(bool value);

    QString getSearch() const;
    void setSearch(const QString &value);

    QString getReplace() const;
    void setReplace(const QString &replace);

    bool replace_preface();

private slots:
    void on_actionExit_triggered();
    void on_actionSelect_All_triggered();
    void on_editor_copyAvailable(bool b);
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionDelete_triggered();
    void on_editor_undoAvailable(bool b);
    void on_actionUndo_triggered();
    void on_editor_redoAvailable(bool b);
    void on_actionRedo_triggered();
    void on_actionPaste_triggered();
    void enable_paste();
    void on_editor_textChanged();
    void on_actionFont_triggered();
    void on_actionNew_triggered();
    void on_actionTime_Date_triggered();
    void on_actionStatus_Bar_triggered(bool);
    void on_actionWord_Wrap_triggered(bool);
    void on_actionGo_To_triggered();
    void set_window_title(QString&);
    void on_actionView_Help_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_As_triggered();
    void on_actionSave_triggered();
    void on_editor_cursorPositionChanged();
    void update_statusbar(NoteState&);
    void on_actionPrint_triggered();
    void on_actionPage_Setup_triggered();
    void on_actionAbout_Textpad_triggered();
    void closeEvent(QCloseEvent*);
    void on_actionFind_triggered();
    void on_actionReplace_triggered();
    void on_actionFind_Next_triggered();
    void activate_theme(QAction*);

signals:
    void filename_changed(QString&);
    void note_state_changed(NoteState&);

private:
    Ui::TextPad *ui;
    QString currentFile = "Untitled";
    NoteState noteState;
    bool untouched = true, firstSave;
    bool matchCase, wholeWord, down = true;
    bool me;
    QString search, replace_;

    QPrinter *printer;
    QPrintDialog *printDialog;
    QPageSetupDialog *pageSetupDialog;
    FindDialog *findDialog;
    ReplaceDialog *replaceDialog;

    void load_settings();
    void sync_statusbar();
    void save_file(QString&);
};

#endif // TEXTPAD_H
