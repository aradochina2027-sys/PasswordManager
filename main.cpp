#include "passwordmanager.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    PasswordManager w;
    w.show();
    return a.exec();
}
