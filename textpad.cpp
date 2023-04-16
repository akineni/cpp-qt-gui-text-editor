#include <QClipboard>
#include <QFontDialog>
#include <QDateTime>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextBlock>
#include <QTextStream>
#include <QDesktopServices>
#include "textpad.h"
#include "ui_textpad.h"
#include "aboutdialog.h"

#define SAVE_AS \
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));\
    if(!fileName.isEmpty()){\
        SAVE_FILE(fileName)\
    }else{\
        return;\
    }

#define ASK_SAVE_FILE \
    if(noteState == Unsaved) {\
        QMessageBox messageBox(\
                    QMessageBox::Question,\
                    QCoreApplication::applicationName(),\
                    "Do you want to save changes to " + currentFile + "?",\
                    QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,\
                    this);\
        messageBox.setButtonText(QMessageBox::Yes,"&Save");\
        messageBox.setButtonText(QMessageBox::No,"&Don't Save");\
        messageBox.setDefaultButton(QMessageBox::Yes);\
        switch (messageBox.exec()) {\
            case QMessageBox::Yes:\
                RESOLVE_SAVE_FILE\
            break;\
            case QMessageBox::Cancel:\
                return;\
        }\
    }

#define RESOLVE_SAVE_FILE \
    if(!firstSave){\
        SAVE_AS\
    }else{\
        SAVE_FILE(currentFile)\
    }

#define SAVE_FILE(fileName) \
    QFile file(fileName);\
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){\
        QMessageBox::critical(this,\
                              QCoreApplication::applicationName(),\
                              "Cannot to save file " + fileName + ": " + file.errorString());\
        return;\
    }\
\
    QTextStream out(&file);\
    out << ui->editor->toPlainText();\
\
    file.close();\
\
    firstSave = true;\
\
    currentFile = fileName;\
    emit filename_changed(currentFile);\
\
    noteState = Saved;\
    emit note_state_changed(noteState);

#define SET_COMMON_FIND_PARAMS \
    setSearch(search);\
    setMatchCase(match_case);\
    setWholeWord(whole_word);\
    setDown(down);

#define REPLACE_WITH_WHOLE_WORDS \
    int start_pos, end_pos;\
    QChar prev_char, next_char;\
    start_pos = textCursor.selectionStart();\
    end_pos = textCursor.selectionEnd();\
    prev_char = ui->editor->document()->characterAt(start_pos-1);\
    next_char = ui->editor->document()->characterAt(end_pos);\
    QRegExp regExp("\\W");\
    if(regExp.exactMatch(prev_char) && regExp.exactMatch(next_char))\
        return true;

TextPad::TextPad(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TextPad),
    printer(new QPrinter),
    printDialog(new QPrintDialog(printer,this)),
    pageSetupDialog(new QPageSetupDialog(printer, this)),
    findDialog(new FindDialog(this)),
    replaceDialog(new ReplaceDialog(this))
{
    ui->setupUi(this);

    QClipboard *clipboard = QGuiApplication::clipboard();

    QActionGroup *editorThemeActionGroup = new QActionGroup(this);
    editorThemeActionGroup->addAction(ui->actionTheme_Default);
    editorThemeActionGroup->addAction(ui->actionTextpad_Dark);

    //connect(clipboard,SIGNAL(dataChanged()),this,SLOT(enable_paste()));
    connect(clipboard,SIGNAL(changed(QClipboard::Mode)),this,SLOT(enable_paste()));
    connect(this,SIGNAL(filename_changed(QString&)),this,SLOT(set_window_title(QString&)));
    connect(this,SIGNAL(note_state_changed(NoteState&)),this,SLOT(update_statusbar(NoteState&)));
    connect(editorThemeActionGroup,SIGNAL(triggered(QAction*)),this,SLOT(activate_theme(QAction*)));

    ui->actionPaste->setEnabled(!clipboard->text().isEmpty());

    emit filename_changed(currentFile);

    QCoreApplication::setOrganizationName("akineni");

    ui->statusBar->addPermanentWidget(ui->editor_statusLbl);
    ui->statusBar->addPermanentWidget(ui->noteStateLbl);

    load_settings();
}

TextPad::~TextPad()
{
    delete ui;
    delete printer;
    delete pageSetupDialog;
    delete printDialog;
    delete findDialog;
    delete replaceDialog;
}

void TextPad::on_actionExit_triggered()
{
    ASK_SAVE_FILE

    QApplication::quit();
}

void TextPad::on_actionSelect_All_triggered()
{
    ui->editor->selectAll();
}

void TextPad::on_editor_copyAvailable(bool b)
{
    ui->actionCut->setEnabled(b);
    ui->actionCopy->setEnabled(b);
    ui->actionDelete->setEnabled(b);
}

void TextPad::on_actionCut_triggered()
{
    ui->editor->cut();
}

void TextPad::on_actionCopy_triggered()
{
    ui->editor->copy();
}

void TextPad::on_actionDelete_triggered()
{

    ui->editor->textCursor().removeSelectedText();
}

void TextPad::on_editor_undoAvailable(bool b)
{
    ui->actionUndo->setEnabled(b);
}

void TextPad::on_actionUndo_triggered()
{
    ui->editor->undo();
}

void TextPad::on_editor_redoAvailable(bool b)
{
    ui->actionRedo->setEnabled(b);
}

void TextPad::on_actionRedo_triggered()
{
    ui->editor->redo();
}

void TextPad::on_actionPaste_triggered()
{
    ui->editor->paste();
}

void TextPad::enable_paste()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    ui->actionPaste->setEnabled(!clipboard->text().isEmpty());
}

void TextPad::on_editor_textChanged()
{
    ui->actionFind->setEnabled(!ui->editor->toPlainText().isEmpty());
    ui->actionFind_Next->setEnabled(!ui->editor->toPlainText().isEmpty());
    ui->actionReplace->setEnabled(!ui->editor->toPlainText().isEmpty());
    ui->actionGo_To->setEnabled(!ui->editor->toPlainText().isEmpty());
    ui->actionSelect_All->setEnabled(!ui->editor->toPlainText().isEmpty());

    sync_statusbar();

    if(untouched){
        noteState = Untouched;
        untouched = false;
    }else
        noteState = Unsaved;

    emit note_state_changed(noteState);

    //ctrl, all arrow keys trigger textChanged. .. why? ??
}

void TextPad::on_actionFont_triggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, ui->editor->font(), this, "Select Font");
    if(ok) ui->editor->setFont(font);

    QSettings settings;
    settings.setValue("editor/font",font);

    /*
     * When rich text(e.g coding editor text) is pasted into editor, Html
     * style-font attribute-property is set on elements preventing selected font
     * from rendering on all text.
    */
}

void TextPad::on_actionTime_Date_triggered()
{
    QDateTime d = QDateTime::currentDateTime();
    ui->editor->insertPlainText(d.toString("h:mm AP d/M/yyyy"));
}

void TextPad::on_actionStatus_Bar_triggered(bool checked)
{
    ui->statusBar->setVisible(checked);

    QSettings settings;
    settings.setValue("statusBar", checked);
}

void TextPad::on_actionWord_Wrap_triggered(bool checked)
{
    ui->editor->setWordWrapMode(checked?QTextOption::WrapAtWordBoundaryOrAnywhere
                                                               :QTextOption::NoWrap);
    QSettings settings;
    settings.setValue("editor/wordWrap", checked);
}

void TextPad::on_actionGo_To_triggered()
{
    QTextCursor textCursor = ui->editor->textCursor();
    bool ok;
    int ln_num = QInputDialog::getInt(this,
                                      "Go To Line",
                                      "Line number",
                                      textCursor.blockNumber()+1,
                                      1,ui->editor->document()->blockCount(),1,&ok);
    if(ok){
        textCursor.setPosition(
                    ui->editor->document()->findBlockByLineNumber(ln_num-1).position());
        ui->editor->setTextCursor(textCursor);
    }
}

void TextPad::set_window_title(QString& title)
{
    setWindowTitle(title + " - " + QCoreApplication::applicationName());
}

void TextPad::on_actionView_Help_triggered()
{
    QDir dir;
    QString dirName = "help";
    QString file = dirName + "/readme.html";

    if(!dir.exists(file)){
        dir.mkdir(dirName);
        QFile::copy(":/txt/help", file);
    }

    QDesktopServices::openUrl(QUrl(file));
}

void TextPad::load_settings()
{
    QSettings settings;

    ui->actionStatus_Bar->setChecked(settings.value("statusBar",true).toBool());
    ui->statusBar->setVisible(settings.value("statusBar",true).toBool());

    ui->actionWord_Wrap->setChecked(settings.value("editor/wordWrap",true).toBool());
    ui->editor->setWordWrapMode(
                settings.value("editor/wordWrap",true).toBool()?
                    QTextOption::WrapAtWordBoundaryOrAnywhere
                  :QTextOption::NoWrap);

    ui->editor->setFont(settings.value("editor/font", QFont()).value<QFont>());
    ui->editor->setTextColor(settings.value("editor/textColor",QColor()).value<QColor>());
}

void TextPad::sync_statusbar()
{
    QTextCursor textCursor = ui->editor->textCursor();
    ui->editor_statusLbl->setText("Ln: " + QString::number(textCursor.blockNumber()+1)
                              + "  Col: " + QString::number(textCursor.columnNumber()+1)
                            + "  Total ln: " + QString::number(ui->editor->document()->lineCount())
                            + "  word-cnt: " + QString::number(ui->editor->toPlainText()
                             .split(QRegExp("(\\s|\\n|\\r)+")).count())
                            + "  char-cnt: " + QString::number(
                                      ui->editor->document()->characterCount()
                                      ));
}

void TextPad::find_next(QWidget *parent, QString search, bool match_case, bool whole_word, bool down)
{

    SET_COMMON_FIND_PARAMS

    QTextDocument::FindFlags findFlags;
    if(match_case) findFlags = QTextDocument::FindCaseSensitively;
    if(!down) findFlags = findFlags | QTextDocument::FindBackward;
    if(whole_word) findFlags = findFlags | QTextDocument::FindWholeWords;

    if(!ui->editor->find(search, findFlags))
        QMessageBox::information(parent, QCoreApplication::applicationName(),
                    "Cannot find \"" + search + "\"");
}

void TextPad::find_next(QWidget *parent)
{
    find_next(parent,getSearch(),getMatchCase(),getWholeWord(),getDown());
}

void TextPad::on_actionNew_triggered()
{
    ASK_SAVE_FILE

    ui->editor->clear();    //clears undo/redo history

    firstSave = false;     //Show save as dialog on first save

    currentFile = "Untitled";
    emit filename_changed(currentFile);

    noteState = Untouched;
    emit note_state_changed(noteState);
}

void TextPad::on_actionOpen_triggered()
{
    ASK_SAVE_FILE

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));

    if(!fileName.isEmpty()) {

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox::critical(this,
                                  QCoreApplication::applicationName(),
                                  "Cannot to open file " + fileName + ": " + file.errorString());
            return;
        }

        QTextStream in(&file);
        QString str = in.readAll();

        ui->editor->clear();    //clears the undo/redo history
        ui->editor->setText(str);

        file.close();

        firstSave = true;      //Do not open save as dialog

        currentFile = fileName;
        emit filename_changed(currentFile);

        noteState = Untouched;
        emit note_state_changed(noteState);
    }
}

void TextPad::on_actionSave_triggered()
{
    RESOLVE_SAVE_FILE
}

void TextPad::on_actionSave_As_triggered()
{
    SAVE_AS
}

void TextPad::on_editor_cursorPositionChanged()
{
    sync_statusbar();
}

void TextPad::update_statusbar(NoteState &state)
{
    QString str;

    switch (state) {
        case Untouched:
        str = QString::fromUtf8("<table>"
                                    "<tr>"
                                        "<td><img width=16 src=:/icon/check/></td>"
                                        "<td style='color:forestgreen;'> Ready</td>"
                                    "</tr>"
                                "</table>");
        break;

        case Saved:
        str = QString::fromUtf8("<table>"
                                    "<tr>"
                                        "<td><img src=:/icon/ready/></td>"
                                        "<td style='color:darkgreen;'> Saved</td>"
                                    "</tr>"
                                "</table>");
        break;

        case Unsaved:
        str = QString::fromUtf8("<table>"
                                    "<tr>"
                                        "<td><img src=:/icon/unsaved2/></td>"
                                        "<td style='color:#ff3232;'> Unsaved</td>"
                                    "</tr>"
                                "</table>");
    }

    ui->noteStateLbl->setText(str);
}

void TextPad::on_actionPrint_triggered()
{
    if (printDialog->exec() == QDialog::Accepted)
        ui->editor->print(printer);
}

void TextPad::on_actionPage_Setup_triggered()
{
    if(pageSetupDialog->exec() == QDialog::Accepted) {
        //do something that i don't know yet
    }
}

void TextPad::on_actionAbout_Textpad_triggered()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}

void TextPad::closeEvent(QCloseEvent *event)
{
    event->ignore();
    on_actionExit_triggered();
}

void TextPad::on_actionFind_triggered()
{
    if(findDialog->isHidden() && replaceDialog->isHidden())
        findDialog->show();
    else if(findDialog->isVisible())
        findDialog->activateWindow();
}

void TextPad::on_actionReplace_triggered()
{
    if(replaceDialog->isHidden() && findDialog->isHidden())
        replaceDialog->show();
    else if(replaceDialog->isVisible())
        replaceDialog->activateWindow();
}

void TextPad::on_actionFind_Next_triggered()
{
    if(getSearch().isEmpty()){
        on_actionFind_triggered();
        return;
    }

    find_next(this);
}

void TextPad::activate_theme(QAction *action)
{
    if(action->iconText() == "Textpad Dark") {}
    else {}
}

QString TextPad::getReplace() const
{
    return replace_;
}

void TextPad::setReplace(const QString &replace)
{
    replace_ = replace;
}

void TextPad::replace(QString search,
                      QString replace, bool match_case, bool whole_word, bool down)
{
    SET_COMMON_FIND_PARAMS
    setReplace(replace);

    QTextCursor textCursor = ui->editor->textCursor();
    textCursor.insertText(getReplace());
    ui->editor->setTextCursor(textCursor);
}

void TextPad::replace_all(QString search, QString replace,
                         bool match_case, bool whole_word, bool down)
{
    SET_COMMON_FIND_PARAMS
    setReplace(replace);

    QTextCursor textCursor = ui->editor->textCursor();
    textCursor.movePosition(QTextCursor::Start);
    ui->editor->setTextCursor(textCursor);

    QTextDocument::FindFlags findFlags;
    if(getMatchCase()) findFlags = QTextDocument::FindCaseSensitively;
    if(getWholeWord()) findFlags = findFlags | QTextDocument::FindWholeWords;

    if(!ui->editor->find(search, findFlags)){
        QMessageBox::information(replaceDialog, QCoreApplication::applicationName(),
                    "Cannot find \"" + search + "\"");
        return;
    }

    textCursor.movePosition(QTextCursor::Start);
    textCursor.beginEditBlock();

    ui->editor->setTextCursor(textCursor);

    while(ui->editor->find(search, findFlags)){
        textCursor = ui->editor->textCursor();
        textCursor.insertText(getReplace());
    }

    textCursor.endEditBlock();
    ui->editor->setTextCursor(textCursor);
}

QString TextPad::getSearch() const
{
    return search;
}

void TextPad::setSearch(const QString &value)
{
    search = value;
}

bool TextPad::getDown() const
{
    return down;
}

void TextPad::setDown(bool value)
{
    down = value;
}

bool TextPad::getWholeWord() const
{
    return wholeWord;
}

void TextPad::setWholeWord(bool value)
{
    wholeWord = value;
}

bool TextPad::getMatchCase() const
{
    return matchCase;
}

void TextPad::setMatchCase(bool value)
{
    matchCase = value;
}

bool TextPad::replace_preface() {
    QTextCursor textCursor = ui->editor->textCursor();

    if(textCursor.hasSelection()){
       if(QString::compare(textCursor.selectedText(), getSearch(), Qt::CaseInsensitive) == 0){
           if(getMatchCase() && !getWholeWord()
                   && QString::compare(
                       textCursor.selectedText(),
                       getSearch()
                       ) == 0)
           {
               return true;
           }else if (!getMatchCase() && getWholeWord()){
               REPLACE_WITH_WHOLE_WORDS
           }else if (getMatchCase() && getWholeWord() && QString::compare(
                         textCursor.selectedText(),
                         getSearch()
                         ) == 0)
           {
               REPLACE_WITH_WHOLE_WORDS
           }else if (!getMatchCase() && !getWholeWord()) {
                return true;
           }
        }
    }
    return false;
}
/*
 * Advanced features to implement...
 *  -Tabs
 *  -Left line number bar
 *  -Side bar
 */
