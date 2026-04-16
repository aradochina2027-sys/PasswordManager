#include "typingtrainer.h"
#include "ui_typingtrainer.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRandomGenerator>

TypingTrainer::TypingTrainer(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::TypingTrainer) {
    ui->setupUi(this);

    lessonsPath = QDir::currentPath() + "/lessons";
    settings = new QSettings("KHTA", "TypingTrainer", this);

    sessionTimer = new QTimer(this);
    connect(sessionTimer, &QTimer::timeout, this, &TypingTrainer::updateTimerLabel);

    // ВАЖЛИВО: забираємо фокус у всіх елементів, щоб головне вікно ловило клавіші
    ui->comboLesson->setFocusPolicy(Qt::NoFocus);
    ui->btnStart->setFocusPolicy(Qt::NoFocus);
    ui->btnRandom->setFocusPolicy(Qt::NoFocus);
    ui->btnRestart->setFocusPolicy(Qt::NoFocus);
    ui->btnReturn->setFocusPolicy(Qt::NoFocus);

    setupKeyboard();
    scanLessons();
    loadSettings();

    ui->stackScreens->setCurrentIndex(0);
}

TypingTrainer::~TypingTrainer() {
    saveSettings();
    delete ui;
}

void TypingTrainer::setupKeyboard() {
    QStringList rows = {"1,2,3,4,5,6,7,8,9,0", "Q,W,E,R,T,Y,U,I,O,P", "A,S,D,F,G,H,J,K,L", "Z,X,C,V,B,N,M"};
    for (int i = 0; i < rows.size(); ++i) {
        QStringList keys = rows[i].split(",");
        for (int j = 0; j < keys.size(); ++j) {
            QString keyName = keys[j];
            QPushButton* btn = new QPushButton(keyName);
            btn->setMinimumSize(40, 40);
            btn->setFocusPolicy(Qt::NoFocus);
            ui->layoutGridKeyboard->addWidget(btn, i, j);
            keyboardMap.insert(keyName.toUpper(), btn);
        }
    }

    QPushButton* spaceBtn = new QPushButton("Space");
    spaceBtn->setMinimumSize(200, 40);
    spaceBtn->setFocusPolicy(Qt::NoFocus);
    ui->layoutGridKeyboard->addWidget(spaceBtn, 4, 0, 1, 10, Qt::AlignCenter);
    keyboardMap.insert(" ", spaceBtn); // Реєструємо саме символ пробілу
}

void TypingTrainer::scanLessons() {
    ui->comboLesson->clear();
    QDir dir(lessonsPath);
    if (!dir.exists()) dir.mkpath(".");
    ui->comboLesson->addItems(dir.entryList({"*.txt"}, QDir::Files));
}

void TypingTrainer::loadLessonFile(const QString &fileName) {
    QFile file(lessonsPath + "/" + fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        currentText = QTextStream(&file).readAll().trimmed().simplified();
        file.close();
    } else {
        currentText = "Error: File not found.";
    }
    currentIndex = 0;
    errorCount = 0;
    updateDisplay();
}

void TypingTrainer::updateDisplay() {
    if (currentText.isEmpty()) return;
    QString typed = "<span style='color:green;'>" + currentText.left(currentIndex) + "</span>";
    QString current = "<span style='background-color:yellow; color:black; font-weight:bold;'>" + QString(currentText.at(currentIndex)) + "</span>";
    QString remaining = currentText.mid(currentIndex + 1);
    ui->textDisplay->setHtml(typed + current + remaining);
    ui->progressTraining->setValue((currentIndex * 100) / currentText.length());
    ui->lblAccuracy->setText("Помилки: " + QString::number(errorCount));
}

void TypingTrainer::updateTimerLabel() {
    qint64 ms = elapsedTimer.elapsed();
    int seconds = (ms / 1000) % 60;
    int minutes = (ms / 60000);
    ui->lblTime->setText(QString("Час: %1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));

    if (ms > 0) {
        int cpm = static_cast<int>((currentIndex * 60000.0) / ms);
        ui->lblSpeed->setText(QString("Швидкість: %1 CPM").arg(cpm));
    }
}

void TypingTrainer::keyPressEvent(QKeyEvent *event) {
    if (ui->stackScreens->currentIndex() != 1 || currentText.isEmpty()) {
        return;
    }

    // Обробка спеціальних клавіш
    if (event->key() == Qt::Key_Backspace) {
        if (currentIndex > 0) currentIndex--;
        updateDisplay();
        return;
    }

    // Обробка пробілу окремо, якщо event->text() порожній
    QString input = event->text();
    if (event->key() == Qt::Key_Space) input = " ";

    if (input.isEmpty()) return;

    highlightKey(input, true);

    if (input == currentText.at(currentIndex)) {
        currentIndex++;
        if (currentIndex >= currentText.length()) finishSession();
        else updateDisplay();
    } else {
        errorCount++;
        updateDisplay();
    }
}

void TypingTrainer::keyReleaseEvent(QKeyEvent *event) {
    QString input = event->text();
    if (event->key() == Qt::Key_Space) input = " ";
    highlightKey(input, false);
}

void TypingTrainer::highlightKey(const QString &key, bool pressed) {
    QString k = key.toUpper();
    if (keyboardMap.contains(k)) {
        if (pressed) keyboardMap[k]->setStyleSheet("background-color: lightblue; border: 2px solid blue;");
        else keyboardMap[k]->setStyleSheet("");
    }
}

void TypingTrainer::finishSession() {
    sessionTimer->stop();
    ui->stackScreens->setCurrentIndex(2);
    qint64 totalMs = elapsedTimer.elapsed();
    int cpm = (totalMs > 0) ? static_cast<int>((currentText.length() * 60000.0) / totalMs) : 0;
    ui->resTime->setText(ui->lblTime->text());
    ui->resSpeed->setText(QString("Швидкість: %1 CPM").arg(cpm));
    ui->resAccuracy->setText("Помилок: " + QString::number(errorCount));
}

void TypingTrainer::saveSettings() {
    settings->setValue("lastLesson", ui->comboLesson->currentText());
}

void TypingTrainer::loadSettings() {
    QString last = settings->value("lastLesson", "").toString();
    int index = ui->comboLesson->findText(last);
    if (index != -1) ui->comboLesson->setCurrentIndex(index);
}

void TypingTrainer::on_btnStart_clicked() {
    if (ui->comboLesson->currentText().isEmpty()) return;
    loadLessonFile(ui->comboLesson->currentText());
    ui->stackScreens->setCurrentIndex(1);
    elapsedTimer.start();
    sessionTimer->start(100);
}

void TypingTrainer::on_btnRandom_clicked() {
    if (ui->comboLesson->count() > 0) {
        ui->comboLesson->setCurrentIndex(QRandomGenerator::global()->bounded(ui->comboLesson->count()));
        on_btnStart_clicked();
    }
}

void TypingTrainer::on_btnRestart_clicked() { on_btnStart_clicked(); }
void TypingTrainer::on_btnReturn_clicked() { ui->stackScreens->setCurrentIndex(0); }
void TypingTrainer::on_actionExit_triggered() { close(); }
