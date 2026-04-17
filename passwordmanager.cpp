#include "passwordmanager.h"
#include "ui_passwordmanager.h"
#include <QSqlQuery>
#include <QCryptographicHash>
#include <QNetworkRequest>
#include <QEventLoop>

PasswordManager::PasswordManager(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    networkManager = new QNetworkAccessManager(this);

    setupModel();
    loadData();

    ui->tableViewAccounts->setModel(model);
    ui->tableViewAccounts->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(model, &QStandardItemModel::dataChanged, this, &PasswordManager::on_dataChanged);

    // Перевірка, чи існує кнопка в UI
    if (ui->btnCheckLeak) {
        connect(ui->btnCheckLeak, &QPushButton::clicked, this, &PasswordManager::on_btnCheckAll_clicked);
    }
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

bool PasswordManager::checkPasswordSync(const QString &password) {
    QString hash = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha1).toHex()).toUpper();
    QString prefix = hash.left(5);
    QString suffix = hash.mid(5);

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl("https://api.pwnedpasswords.com/range/" + prefix)));

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    bool leaked = false;
    if (reply->error() == QNetworkReply::NoError) {
        leaked = reply->readAll().contains(suffix.toUtf8());
    }
    reply->deleteLater();
    return leaked;
}

void PasswordManager::on_btnCheckAll_clicked() {
    if (ui->statusbar) ui->statusbar->showMessage("Background check started...");

    for (int i = 0; i < model->rowCount(); ++i) {
        QString service = model->item(i, 1)->text();
        QString password = model->item(i, 3)->text();

        QtConcurrent::run([this, service, password]() {
            bool leaked = checkPasswordSync(password);
            QMetaObject::invokeMethod(this, "handleCheckResult", Qt::QueuedConnection,
                                      Q_ARG(QString, service), Q_ARG(bool, leaked));
        });
    }
}

void PasswordManager::handleCheckResult(const QString &service, bool leaked) {
    if (leaked && ui->statusbar) {
        ui->statusbar->showMessage("WARNING: Password for " + service + " leaked!", 5000);
    }
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
