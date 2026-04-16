#include "passwordmanager.h"
#include "ui_passwordmanager.h"
#include <QSqlQuery>
#include <QSqlRecord>

PasswordManager::PasswordManager(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setupModel();
    loadData();

    ui->tableViewAccounts->setModel(model);
    ui->tableViewAccounts->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(model, &QStandardItemModel::dataChanged, this, &PasswordManager::on_dataChanged);
}

PasswordManager::~PasswordManager() {
    delete ui;
}

void PasswordManager::setupModel() {
    model = new QStandardItemModel(0, 4, this);
    model->setHeaderData(1, Qt::Horizontal, "Service");
    model->setHeaderData(2, Qt::Horizontal, "Login");
    model->setHeaderData(3, Qt::Horizontal, "Password");
}

void PasswordManager::loadData() {
    model->removeRows(0, model->rowCount());
    QSqlQuery query("SELECT id, service, login, password FROM accounts");
    while (query.next()) {
        QList<QStandardItem*> items;
        items << new QStandardItem(query.value(0).toString());
        items << new QStandardItem(query.value(1).toString());
        items << new QStandardItem(query.value(2).toString());
        items << new QStandardItem(query.value(3).toString());
        model->appendRow(items);
    }
    ui->tableViewAccounts->setColumnHidden(0, true);
}

void PasswordManager::on_actionNew_triggered() {
    QSqlQuery query;
    query.prepare("INSERT INTO accounts (service, login, password) VALUES (?, ?, ?)");
    query.addBindValue("New Service");
    query.addBindValue("login");
    query.addBindValue("password");

    if (query.exec()) loadData();
}

void PasswordManager::on_dataChanged(const QModelIndex &topLeft, const QModelIndex &) {
    int row = topLeft.row();
    QString id = model->item(row, 0)->text();
    QString service = model->item(row, 1)->text();
    QString login = model->item(row, 2)->text();
    QString password = model->item(row, 3)->text();

    QSqlQuery query;
    query.prepare("UPDATE accounts SET service=?, login=?, password=? WHERE id=?");
    query.addBindValue(service);
    query.addBindValue(login);
    query.addBindValue(password);
    query.addBindValue(id);
    query.exec();
}

void PasswordManager::on_actionDelete_triggered() {
    QModelIndex index = ui->tableViewAccounts->currentIndex();
    if (index.isValid()) {
        QString id = model->item(index.row(), 0)->text();
        QSqlQuery query;
        query.prepare("DELETE FROM accounts WHERE id=?");
        query.addBindValue(id);
        if (query.exec()) loadData();
    }
}

void PasswordManager::on_actionEdit_triggered() {
    QModelIndex index = ui->tableViewAccounts->currentIndex();
    if (index.isValid()) ui->tableViewAccounts->edit(index);
}
