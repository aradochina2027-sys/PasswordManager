#include "passwordmanager.h"
#include "databasemanager.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    if (!DatabaseManager::init()) return -1;
    PasswordManager w;
    w.show();
    return a.exec();
}
