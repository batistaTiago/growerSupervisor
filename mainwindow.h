#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include <QMutex>
#include "statusthread.h"
#include "qcustomplot.h"
#include "settingswindow.h"
#include "programaticcontrolsettings.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int acesso = 0;
    QSerialPort *arduino;
    QStringList availablePorts;
    void stopThread();

    void setupGraphs();

    void startThread();

private slots:

    void progDataCallback(QString, QString);

    bool validateProgData(QString);

    void launchProgramaticSettings(QString sender);

    void launchUSBSettingsWindow(bool startConnection);

    void updateArduinoPortName(QString);

    void updateSensorReadings(QStringList valores);

    void updatePumpLightUI(QStringList valores);

    void updateHumidityParameters(QStringList valores);

    void updateOPModeUI(QStringList valores);

    void on_radioMANUAL_clicked();
    void on_radioAUTOMATICO_clicked();
    void on_radioPROGRAMADO_clicked();
    void on_radioSIMULADO_clicked();

    void on_ConnectButton_clicked();
    void on_DisconnectButton_clicked();

    void conectar();
    void ui_conectar();
    void ui_desconectar();
    void updateAutomaticUI(bool on);
    void updateManualUI(bool on);
    void updateProgramaticUI(bool on);

    void atualizar(QStringList valores);

    void plot();
    void addPoint(double, double, double, double);
    void clearGraph();

    void sendCommand(QString command);

    void testcode();

    void on_checkBox_luzes_clicked(bool checked);

    void on_checkBox_bomba_clicked(bool checked);

    void on_spinBox_controleumidade_min_valueChanged(QString arg1);

    void on_spinBox_controleumidade_max_valueChanged(QString arg1);

    void delay(int t);

    void on_spinBox_controleumidade_min_editingFinished();

    void on_spinBox_controleumidade_max_editingFinished();

    void updateArduinoClock(QStringList currentArduinoTime);

    QString convertFromFullString(QString dow);

    void on_sampleTimeSlider_sliderReleased();

    void on_sampleTimeSlider_valueChanged(int value);

    void on_settingsButton_clicked();

    void on_progPumpOptions_clicked();

    void on_progLightsOptions_clicked();

    void on_kpTextField_returnPressed();

    void on_kiTextField_returnPressed();

    void on_kdTextField_returnPressed();

    void on_setpointTextField_returnPressed();

    void updatePIDParameters(QStringList);

    void on_coolerPowerSlider_sliderReleased();

    void on_progCoolerOptions_clicked();

    QString fetchProgrammaticSettings(QString);

signals:

private:
    SettingsWindow *settingsWindow;
    ProgramaticControlSettings *controlSettings;
    Ui::MainWindow *ui;
    StatusThread *statusThread;
    QThread *threadObject;
    int arduino_uno_vendor_id; //talvez remover
    int arduino_uno_product_id; //talvez remover
    QString arduino_port_name; //talvez remover
    bool arduino_is_available = false; //talvez remover
    int stop;
    QString  opMode;
    double setPoint;
    double kp;
    double ki;
    double kd;
    double coolerPower;
    QVector<double> umidadeQVectorX, umidadeQVectorY,
                    temperaturaQVectorX, temperaturaQVectorY,
                    luminosidadeQVectorX, luminosidadeQVectorY,
                    umidade_arQVectorX, umidade_arQVectorY;
    int tempo;
    float UMIDADE, TEMPERATURA, UMIDADE_AR, LUMINOSIDADE, minUmidade, maxUmidade;
    bool bombaOn, luzOn;
    bool isUpdating = false;
    QStringList currentTime;
    bool needsOpModeUIUpdate = true;
    bool needsPumpLightUIUpdate = true;
    bool needsHumidityParametersUIUpdate = true;
    bool needsPIDParametersUIUpdate = true;
};

#endif // MAINWINDOW_H
