#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QMainWindow>
#include <QStandardItemModel>

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

private:
    Ui::MainWindow *ui;
    QStandardItemModel *model;

    void setupModel();
};

#endif
