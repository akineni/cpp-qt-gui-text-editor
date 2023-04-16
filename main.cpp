#include "textpad.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TextPad t;
    t.show();

    return a.exec();
}
