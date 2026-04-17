#include "passwordmanager.h"
#include "ui_passwordmanager.h"
#include <QSqlQuery>
#include <QCryptographicHash>
#include <QNetworkRequest>

PasswordManager::PasswordManager(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &PasswordManager::onNetworkReply);

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

void PasswordManager::loadData(const QString &filter) {
    model->removeRows(0, model->rowCount());
    QSqlQuery query;
    if (filter.isEmpty()) {
        query.prepare("SELECT id, service, login, password FROM accounts");
    } else {
        query.prepare("SELECT id, service, login, password FROM accounts WHERE service LIKE ? OR login LIKE ?");
        query.addBindValue("%" + filter + "%");
        query.addBindValue("%" + filter + "%");
    }
    query.exec();
    while (query.next()) {
        QList<QStandardItem*> items;
        for(int i=0; i<4; ++i) items << new QStandardItem(query.value(i).toString());
        model->appendRow(items);
    }
    ui->tableViewAccounts->setColumnHidden(0, true);
}

QString PasswordManager::sha1Hash(const QString &input) {
    return QString(QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha1).toHex()).toUpper();
}

void PasswordManager::on_btnCheckLeak_clicked() {
    QModelIndex index = ui->tableViewAccounts->currentIndex();
    if (!index.isValid()) return;

    QString password = model->item(index.row(), 3)->text();
    QString hash = sha1Hash(password);
    QString prefix = hash.left(5);

    if(ui->statusbar) ui->statusbar->showMessage("Checking password leak...");
    networkManager->get(QNetworkRequest(QUrl("https://api.pwnedpasswords.com/range/" + prefix)));
}

void PasswordManager::onNetworkReply(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        if(ui->statusbar) ui->statusbar->showMessage("Check complete. See console.");
    } else {
        if(ui->statusbar) ui->statusbar->showMessage("Network error.");
    }
    reply->deleteLater();
}

void PasswordManager::on_lineEditSearch_textChanged(const QString &text) { loadData(text); }

void PasswordManager::on_actionNew_triggered() {
    QSqlQuery query;
    query.prepare("INSERT INTO accounts (service, login, password) VALUES ('New', 'user', 'pass')");
    if (query.exec()) loadData(ui->lineEditSearch->text());
}

void PasswordManager::on_dataChanged(const QModelIndex &topLeft, const QModelIndex &) {
    int row = topLeft.row();
    QSqlQuery query;
    query.prepare("UPDATE accounts SET service=?, login=?, password=? WHERE id=?");
    query.addBindValue(model->item(row, 1)->text());
    query.addBindValue(model->item(row, 2)->text());
    query.addBindValue(model->item(row, 3)->text());
    query.addBindValue(model->item(row, 0)->text());
    query.exec();
}

void PasswordManager::on_actionDelete_triggered() {
    QModelIndex index = ui->tableViewAccounts->currentIndex();
    if (index.isValid()) {
        QSqlQuery query;
        query.prepare("DELETE FROM accounts WHERE id=?");
        query.addBindValue(model->item(index.row(), 0)->text());
        if (query.exec()) loadData(ui->lineEditSearch->text());
    }
}

void PasswordManager::on_actionEdit_triggered() {
    QModelIndex index = ui->tableViewAccounts->currentIndex();
    if (index.isValid()) ui->tableViewAccounts->edit(index);
}
