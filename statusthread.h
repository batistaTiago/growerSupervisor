#ifndef STATUSTHREAD_H
#define STATUSTHREAD_H

#include <QObject>
#include <QtCore>
#include <QSerialPort>

class StatusThread : public QObject
{
    Q_OBJECT
public:

    explicit StatusThread(QObject *parent = 0);
    void setup(QThread &thr);
    QSerialPort *arduino;
    bool skipIteration = false;
    bool running = false;

signals:
    void updateRequest(QStringList list);
    void firstMessageReceived(QStringList list);
    void threadFinished();

public slots:
    bool isNull();
    void setSampleFrequency(int newValue);
    int getSampleFrequency();
private:
    bool verificarFirstMessage(QString data);
    void requestData();
    QString readSerialData();
    QStringList parse(QString data);
    void delay(int t);
    double amostra = 0;
    double score = 0;

    QString rawSerialData;
    QStringList parsedData;
    int tempoAmostragem = 200;

private slots:
    void rodar();
};

#endif // STATUSTHREAD_H
