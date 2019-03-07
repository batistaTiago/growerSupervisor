#include "statusthread.h"
#include "windows.h"
#include "mainwindow.h"
#include <QSerialPort>

#define numeroDeParametros 18
#define numeroParametrosFirstMessage 3

extern QMutex lock;

StatusThread::StatusThread(QObject *parent) : QObject(parent)
{
}

void StatusThread::setup(QThread &thr) {
    connect(&thr, SIGNAL(started()),this, SLOT(rodar()));
}

void StatusThread::rodar() {
    parsedData << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "0" << "-1" << "-1" << "-1" << "-1";
    qDebug() << "Thread started!";
    rawSerialData = "";
    QStringList lastValidData = this->parsedData;
    while (this->running) {
        if (!this->skipIteration) {
            if ((arduino->isWritable()) && (arduino->isReadable())) {
                requestData();
                QString response = readSerialData();
                rawSerialData += response;

                QStringList dataLines = rawSerialData.split("\r\n", QString::SkipEmptyParts);

                rawSerialData = "";
                int ind = 0;
//                qDebug() << "dataLines count = " << dataLines.count();
                while (ind < dataLines.length()) {
                    parsedData = parse(dataLines[ind]);

                    if (parsedData.count() == numeroDeParametros) {
                        score++;
                        emit updateRequest(parsedData);
                        lastValidData = parsedData;
//                        qDebug() << parsedData[3];
//                        qDebug() << "Plotado: " << dataLines[ind];
                    } else {
//                        qDebug() << "Numero de parametros nao confere: recebido(parsedData.count())" << parsedData.count() << "esperado " << numeroDeParametros;
                        emit updateRequest(lastValidData);
                        if (ind == (dataLines.length()-1))
                            rawSerialData = dataLines[ind];
                        else {
                        }
                    }
                    ind += 1;
                    delay(tempoAmostragem-50);
                }
//                qDebug() << "#############" << rawSerialData << "#############";
            }
        }
    }
    qDebug() << "Thread finished!";
    emit threadFinished();
}

//não funciona
bool StatusThread::verificarFirstMessage(QString data) {
    QStringList splitData = data.split(" ");
    if ((splitData[0].contains("MODE:"))) {
        if (splitData.count() == numeroParametrosFirstMessage) {
            qDebug() << "contagem certa!";
            QStringList *output = new QStringList();
            for (int i = 0; i<numeroParametrosFirstMessage; i++) {
                output->append(splitData[i+1]);
            }
            if (output->count() == numeroParametrosFirstMessage) {
                qDebug() << "contagem certa 2!";
                emit firstMessageReceived(*output);
                return true;
            }
        }
    }
    return false;
}

void StatusThread::requestData() {
    lock.lock();
    arduino->write("?\n");
    lock.unlock();
}

QString StatusThread::readSerialData() {
    lock.lock();
    QByteArray responseData = NULL;
    if (arduino->waitForReadyRead(1000)) {
        responseData = arduino->readAll();
        while (arduino->waitForReadyRead(20)) {
            responseData += arduino->readAll();
        }
        lock.unlock();
        return QString::fromUtf8(responseData);
    } else {
        lock.unlock();
        return "";
    }
}

QStringList StatusThread::parse(QString data) {
    QStringList dataParsed;
    if (data.contains("___") && data.contains("&&&")) {
        QStringList aux0 = data.split("___");
        QStringList aux1 = aux0[1].split("&&&");
        dataParsed = aux1[0].split("/");
    }
    return dataParsed;
}

void StatusThread::delay(int t)
{
    QTime dieTime= QTime::currentTime().addMSecs(t);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void StatusThread::setSampleFrequency(int newValue) {
    this->tempoAmostragem = newValue;
    qDebug() << "A new sample timer has been set: " + newValue;
}

int StatusThread::getSampleFrequency() {
    if (this != NULL) return 2000;
    return this->tempoAmostragem;
}

bool StatusThread::isNull() {
    if (this == NULL) {
        return true;
    } else {
        return false;
    }
}
