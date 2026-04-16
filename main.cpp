#include "passwordmanager.h"
#include "databasemanager.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    if (!DatabaseManager::init()) {
        QMessageBox::critical(nullptr, "Error", "Could not open database");
        return -1;
    }

    PasswordManager w;
    w.show();
    return a.exec();
}
