#include "programaticcontrolsettings.h"
#include "ui_programaticcontrolsettings.h"
#include "mainwindow.h"
#include <QDebug>

ProgramaticControlSettings::ProgramaticControlSettings(QString data, QString senderMode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgramaticControlSettings)
{
    ui->setupUi(this);

    //processar os dados recebidos da mainwindow
    if (data != "") {
        QStringList splitData = data.split("/");
        this->ui->dayOfWeekON->setCurrentIndex(splitData[0].toInt() - 1);
        this->ui->dayOfWeekOFF->setCurrentIndex(splitData[1].toInt() - 1);

        this->ui->horasON->setCurrentIndex(splitData[2].toInt());
        this->ui->minutosON->setCurrentIndex(splitData[3].toInt());
        this->ui->segundosON->setCurrentIndex(splitData[4].toInt());

        this->ui->horasOFF->setCurrentIndex(splitData[5].toInt());
        this->ui->minutosOFF->setCurrentIndex(splitData[6].toInt());
        this->ui->segundosOFF->setCurrentIndex(splitData[7].toInt());
    }

    this->senderMode = senderMode;
    if (senderMode == "pump") {
        QWidget::setWindowTitle("Cofigurações de Irrigação");
    } else if (senderMode == "light") {
        QWidget::setWindowTitle("Cofigurações de Iluminação");
    } else {
        QWidget::setWindowTitle("Cofigurações de Ventilação");
    }
}

ProgramaticControlSettings::~ProgramaticControlSettings()
{
    qDebug() << "Destructor!";
    delete ui;
}

void ProgramaticControlSettings::on_pushButton_2_clicked()
{
    //ler combo boxes e enviar para a mainwindow atualizar o arduino.
    QString schedulePatch;
    schedulePatch.append(QString::number(this->ui->dayOfWeekON->currentIndex() + 1));
    schedulePatch.append(":");
    schedulePatch.append(QString::number(this->ui->dayOfWeekOFF->currentIndex() + 1));
    schedulePatch.append(":");

    schedulePatch.append(QString::number(this->ui->horasON->currentIndex()));
    schedulePatch.append(":");
    schedulePatch.append(QString::number(this->ui->minutosON->currentIndex()));
    schedulePatch.append(":");
    schedulePatch.append(QString::number(this->ui->segundosON->currentIndex()));
    schedulePatch.append(":");

    schedulePatch.append(QString::number(this->ui->horasOFF->currentIndex()));
    schedulePatch.append(":");
    schedulePatch.append(QString::number(this->ui->minutosOFF->currentIndex()));
    schedulePatch.append(":");
    schedulePatch.append(QString::number(this->ui->segundosOFF->currentIndex()));

    emit okClicked(schedulePatch, this->senderMode);
    ProgramaticControlSettings::close();
}

void ProgramaticControlSettings::on_pushButton_clicked()
{
    emit okClicked("schedulePatch", this->senderMode);
    ProgramaticControlSettings::close();
    delete this;
}
