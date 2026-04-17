#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtConcurrent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PasswordManager : public QMainWindow {
    Q_OBJECT

public:
    PasswordManager(QWidget *parent = nullptr);
    ~PasswordManager();

private slots:
    void on_actionNew_triggered();
    void on_actionEdit_triggered();
    void on_actionDelete_triggered();
    void on_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void on_lineEditSearch_textChanged(const QString &text);
    void on_btnCheckAll_clicked();
    void handleCheckResult(const QString &service, bool leaked);

private:
    Ui::MainWindow *ui;
    QStandardItemModel *model;
    QNetworkAccessManager *networkManager;

    void setupModel();
    void loadData(const QString &filter = "");
    bool checkPasswordSync(const QString &password);
};

#endif
