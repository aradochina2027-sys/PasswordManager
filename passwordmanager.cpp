#include "passwordmanager.h"
#include "ui_passwordmanager.h"
#include <QMessageBox>

PasswordManager::PasswordManager(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setupModel();

    ui->tableViewAccounts->setModel(model);
    ui->tableViewAccounts->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewAccounts->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
}

PasswordManager::~PasswordManager() {
    delete ui;
}

void PasswordManager::setupModel() {
    model = new QStandardItemModel(0, 3, this);
    model->setHeaderData(0, Qt::Horizontal, "Service");
    model->setHeaderData(1, Qt::Horizontal, "Login");
    model->setHeaderData(2, Qt::Horizontal, "Password");
}

void PasswordManager::on_actionNew_triggered() {
    QList<QStandardItem*> items;
    items << new QStandardItem("New Service")
          << new QStandardItem("user@mail.com")
          << new QStandardItem("********");
    model->appendRow(items);
}

void PasswordManager::on_actionEdit_triggered() {
    QModelIndex index = ui->tableViewAccounts->currentIndex();
    if (index.isValid()) {
        ui->tableViewAccounts->edit(index);
    }
}

void PasswordManager::on_actionDelete_triggered() {
    QModelIndex index = ui->tableViewAccounts->currentIndex();
    if (index.isValid()) {
        model->removeRow(index.row());
    }
}
