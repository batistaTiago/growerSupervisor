#include "settingswindow.h"
#include "ui_settingswindow.h"
#include <QDebug>

SettingsWindow::SettingsWindow(QStringList list, bool startConnection = false, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    for (int i = 0; i<list.count(); i++) {
        this->ui->portNamesComboBox->addItem(list[i]);
    }

    this->startConnection = startConnection;

    if (startConnection) {
        qDebug() << "Clicado por Connect";
        connect(this, SIGNAL(requestConnection()), parent, SLOT(on_ConnectButton_clicked()));
    }
    else {
        qDebug() << "Clicado por USB Settings";
    }
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::on_pushButton_clicked()
{
    QString portName = this->ui->portNamesComboBox->currentText();
    emit okClicked(portName);
    if (startConnection) emit requestConnection();
    SettingsWindow::close();
}

void SettingsWindow::on_pushButton_2_clicked()
{
    emit requestConnection();
    SettingsWindow::close();
}
