#include <QtCore>
#include <QTimer>
#include <QMessageBox>

#define endOfLine "\n"

#include "windows.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"

#define numeroParametros 18
#define numeroParametrosProg 8

QMutex lock;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //condições iniciais
    this->statusThread = NULL;
    arduino = new QSerialPort;

    ui->setupUi(this);

    this->ui_desconectar();
    this->setupGraphs();
    this->minUmidade = 0;
    this->maxUmidade = 1;


    //    this->settingsWindow = new SettingsWindow(this);
}

MainWindow::~MainWindow()
{
    this->on_DisconnectButton_clicked();
    delete ui;
}

void MainWindow::plot() {
    if ((this->UMIDADE == 0) || (this->TEMPERATURA == 0) || (this->LUMINOSIDADE == 0) || (this->UMIDADE_AR == 0)) {
        return;
    }
    this->addPoint(this->UMIDADE, this->TEMPERATURA, this->LUMINOSIDADE, this->UMIDADE_AR);
    if (tempo > 50) this->ui->graphUMIDADE->xAxis->setRange(tempo - 48,tempo + 2);
    else ui->graphUMIDADE->xAxis->setRange(0,tempo + 5);
    this->ui->graphUMIDADE->graph(0)->setData(umidadeQVectorX, umidadeQVectorY);
    this->ui->graphUMIDADE->replot();
    this->ui->graphUMIDADE->update();

    if (tempo > 50) ui->graphTEMP->xAxis->setRange(tempo - 48,tempo + 2);
    else ui->graphTEMP->xAxis->setRange(0,tempo + 5);
    this->ui->graphTEMP->graph(0)->setData(temperaturaQVectorX, temperaturaQVectorY);
    this->ui->graphTEMP->replot();
    this->ui->graphTEMP->update();

    if (tempo > 50) ui->graphLUMINOSIDADE->xAxis->setRange(tempo - 48,tempo + 2);
    else ui->graphLUMINOSIDADE->xAxis->setRange(0,tempo + 5);
    this->ui->graphLUMINOSIDADE->graph(0)->setData(luminosidadeQVectorX, luminosidadeQVectorY);
    this->ui->graphLUMINOSIDADE->replot();
    this->ui->graphLUMINOSIDADE->update();

    if (tempo > 50) ui->graphUMIDADE_AR->xAxis->setRange(tempo - 48,tempo + 2);
    else ui->graphUMIDADE_AR->xAxis->setRange(0,tempo + 5);
    this->ui->graphUMIDADE_AR->graph(0)->setData(umidade_arQVectorX, umidade_arQVectorY);
    this->ui->graphUMIDADE_AR->replot();
    this->ui->graphUMIDADE_AR->update();

    this->ui->lcdUMIDADE->display(UMIDADE);
    this->ui->lcdTEMP->display(TEMPERATURA);
    this->ui->lcdLUMINOSIDADE->display(LUMINOSIDADE);
    this->ui->lcdUMIDADE_AR->display(UMIDADE_AR);
}

void MainWindow::addPoint(double h, double t, double l, double aH)
{
    this->umidadeQVectorX.append(tempo);
    this->temperaturaQVectorX.append(tempo);
    this->luminosidadeQVectorX.append(tempo);
    this->umidade_arQVectorX.append(tempo);
    this->tempo++;

    this->umidadeQVectorY.append(h);
    this->temperaturaQVectorY.append(t);
    this->luminosidadeQVectorY.append(l);
    this->umidade_arQVectorY.append(aH);
}

void MainWindow::setupGraphs()
{
    ui->graphUMIDADE->addGraph();
    ui->graphUMIDADE->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->graphUMIDADE->graph(0)->setPen(QPen(QColor(40, 40, 190)));
    ui->graphUMIDADE->xAxis->setLabel("k");
    ui->graphUMIDADE->yAxis->setLabel("Umidade");
    ui->graphUMIDADE->yAxis->setRange(0,100);

    ui->graphTEMP->addGraph();
    ui->graphTEMP->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->graphTEMP->graph(0)->setPen(QPen(QColor(190, 40, 40)));
    ui->graphTEMP->xAxis->setLabel("k");
    ui->graphTEMP->yAxis->setLabel("Temperatura");
    ui->graphTEMP->yAxis->setRange(10,50);

    ui->graphLUMINOSIDADE->addGraph();
    ui->graphLUMINOSIDADE->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->graphLUMINOSIDADE->graph(0)->setPen(QPen(QColor(40, 190, 40)));
    ui->graphLUMINOSIDADE->xAxis->setLabel("k");
    ui->graphLUMINOSIDADE->yAxis->setLabel("Luminosidade");
    ui->graphLUMINOSIDADE->yAxis->setRange(0,1000);

    ui->graphUMIDADE_AR->addGraph();
    ui->graphUMIDADE_AR->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->graphUMIDADE_AR->graph(0)->setPen(QPen(QColor(40, 40, 40)));
    ui->graphUMIDADE_AR->xAxis->setLabel("k");
    ui->graphUMIDADE_AR->yAxis->setLabel("Umidade do Ar");
    ui->graphUMIDADE_AR->yAxis->setRange(0,100);
}

void MainWindow::startThread() {
    this->statusThread = new StatusThread(this);
    this->threadObject = new QThread;
    this->statusThread->setup(*threadObject);
    this->statusThread->arduino = this->arduino;
    int val = this->statusThread->getSampleFrequency();
    this->ui->sampleTimeSlider->setValue(val);
    this->ui->sampleTimerLabel->setText("Tempo de amostragem(ms): " + QString::number(val));
    connect(this->statusThread,SIGNAL(updateRequest(QStringList)),this,SLOT(atualizar(QStringList)));
    connect(this->statusThread,SIGNAL(threadFinished()),this,SLOT(ui_desconectar()));

    delay(200);
    this->statusThread->running = true;
    this->threadObject->start();
}

void MainWindow::stopThread()
{
    if (this->statusThread == NULL) {
        return;
    } else if (this->statusThread->running) {
        disconnect(this->statusThread,SIGNAL(updateRequest(QStringList)),this,SLOT(atualizar(QStringList)));
        disconnect(this->statusThread,SIGNAL(firstMessageReceived(QStringList)),this,SLOT(onFirstMessageReceived(QStringList)));
        this->statusThread->running = false;
        disconnect(this->statusThread,SIGNAL(threadFinished()),this,SLOT(ui_desconectar()));
        this->statusThread = NULL;
    }
}

void MainWindow::on_radioAUTOMATICO_clicked() //função para ativar controle automatico
{
    if (!this->ui->radioAUTOMATICO->hasFocus()) return;
    if (this->opMode == "24") return;
    qDebug() << "ativando modo auto";
    stopThread();
    this->needsOpModeUIUpdate = false;
    this->needsHumidityParametersUIUpdate = true;
    this->needsPIDParametersUIUpdate = true;
    opMode = "24";
    MainWindow::sendCommand("M:24");
    updateAutomaticUI(true);
    updateManualUI(false);
    updateProgramaticUI(false);
    this->startThread();
}

void MainWindow::on_radioMANUAL_clicked() //função para ativar controle manual
{
    if (!this->ui->radioMANUAL->hasFocus()) return;
    if (this->opMode == "25") return;
    qDebug() << "ativando modo man";
    this->stopThread();
    this->needsOpModeUIUpdate = false;
    this->needsPumpLightUIUpdate = true;
    opMode = "25";
    MainWindow::sendCommand("M:25");
    updateAutomaticUI(false);
    updateManualUI(true);
    updateProgramaticUI(false);
    this->startThread();
}

void MainWindow::on_radioPROGRAMADO_clicked()
{
    if (!this->ui->radioPROGRAMADO->hasFocus()) return;
    if (this->opMode == "26") return;
    qDebug() << "ativando modo prog";
    this->stopThread();
    this->needsOpModeUIUpdate = false;
    //    this->needsHumidityParametersUIUpdate = true;
    opMode = "26";
    MainWindow::sendCommand("M:26");
    updateAutomaticUI(false);
    updateManualUI(false);
    updateProgramaticUI(true);
    this->startThread();
}

void MainWindow::on_radioSIMULADO_clicked()
{
    if (!this->ui->radioSIMULADO->hasFocus()) return;
    if (this->opMode == "27") return;
    qDebug() << "ativando modo sim";
    this->stopThread();
    this->needsOpModeUIUpdate = false;
    //    this->needsPumpLightUIUpdate = true;
    opMode = "27";
    MainWindow::sendCommand("M:27");
    updateAutomaticUI(false);
    updateManualUI(false);
    updateProgramaticUI(false);
    this->startThread();
}

void MainWindow::on_ConnectButton_clicked()
{
    conectar();

    if (arduino->isWritable()) {
        this->tempo = 0;
        clearGraph();
    }
}

void MainWindow::conectar() {

    //    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
    //        qDebug() << serialPortInfo.portName();
    //    }

    //Loop que verifica os dispositivos conectados nas portas Serial do computador e retorna as PRODUCT e VENDOR ID do arduino conectado, caso exista um.
    //    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
    //        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
    //            if(serialPortInfo.vendorIdentifier() == arduino_uno_vendor_id){
    //                if(serialPortInfo.productIdentifier() == arduino_uno_product_id){
    //                    arduino_port_name = serialPortInfo.portName();
    //                    arduino_is_available = true;
    //                }
    //            }
    //            else{
    //                qDebug() <<  "Arduino not available";
    //                arduino_is_available = false;
    //            }
    //        }
    //    }

    if ((!this->arduino_port_name.isNull()) && (!this->arduino_port_name.isEmpty())) {
        arduino->setPortName(arduino_port_name);
        arduino->setBaudRate(9600);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setFlowControl(QSerialPort::SoftwareControl);
        arduino->open(QIODevice::ReadWrite);
        if ((this->arduino->isWritable()) && (this->arduino->isReadable())) {
            MainWindow::ui_conectar();
            this->startThread();
        }
        else {
            QMessageBox::warning(this, "Port Error", "Arduino OCUPADO!");
        }
    }
    else {
        this->launchUSBSettingsWindow(true);
    }
}

void MainWindow::testcode() {
    qDebug() << "number  of  ports: " << QSerialPortInfo::availablePorts().length() << "\n";
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        qDebug() << "description: " << serialPortInfo.description() << "\n";
        qDebug() << "vendor: " << serialPortInfo.hasVendorIdentifier() << "\n";
        qDebug() << "vendor id? " << serialPortInfo.vendorIdentifier() << "\n";
        qDebug() << "product: " <<  serialPortInfo.hasProductIdentifier() << "\n";
        qDebug() << "product id? " << serialPortInfo.productIdentifier() << "\n";
    }
}

void MainWindow::updateSensorReadings(QStringList valores)
{
    if (valores[0].toDouble()) this->UMIDADE = valores[0].toDouble();
    if (valores[1].toDouble()) this->UMIDADE_AR = valores[1].toDouble();
    if (valores[2].toDouble()) this->TEMPERATURA = valores[2].toDouble();
    if (valores[3].toDouble()) this->LUMINOSIDADE = valores[3].toDouble();
}

void MainWindow::updatePumpLightUI(QStringList valores)
{
    this->bombaOn = valores[4] == "1" ? true : false;
    this->ui->checkBox_bomba->setChecked(this->bombaOn);
    this->luzOn = valores[5] == "1" ? true : false;
    this->ui->checkBox_luzes->setChecked(this->luzOn);
    this->needsPumpLightUIUpdate = false;
    double coolerPowerSliderValue = valores[6].toDouble();
    this->ui->coolerPowerSlider->setValue(coolerPowerSliderValue);
    this->coolerPower = coolerPowerSliderValue;
    qDebug() << "Atualizando CheckBoxes de controle manual.";
}

void MainWindow::updateHumidityParameters(QStringList valores)
{
    double min = valores[8].toDouble();
    double max = valores[9].toDouble();
    if ((min != 0) && (max != 0)) {
        this->needsHumidityParametersUIUpdate = false;
        this->minUmidade = min;
        this->maxUmidade = max;
        this->ui->spinBox_controleumidade_min->setValue(min);
        this->ui->spinBox_controleumidade_max->setValue(max);
        qDebug() << "Atualizando SpinBoxes de controle automatico de umidade. Novos valores: " << min << " e " << max;
    }
}

void MainWindow::updatePIDParameters(QStringList valores) {

    QString setPointToParse = valores[14];
    QString kpToParse = valores[15];
    QString kiToParse = valores[16];
    QString kdToParse = valores[17];

    if ((kpToParse != "-1") && (kiToParse != "-1") && (kdToParse != "-1") && (setPointToParse != "-1")) {
        this->ui->setpointTextField->setText(setPointToParse);
        this->ui->kpTextField->setText(kpToParse);
        this->ui->kiTextField->setText(kiToParse);
        this->ui->kdTextField->setText(kdToParse);
        this->needsPIDParametersUIUpdate = false;
    }
}

void MainWindow::updateOPModeUI(QStringList valores)
{
    if ((this->opMode == "") && (valores[7] != "0")) {
        this->opMode = valores[7];
        qDebug() << "OpModeVar atualizado para: " << this->opMode << ".  atualizando radios";
    }

    if (opMode == "24") {
        if (!ui->radioAUTOMATICO->isChecked()) {
            ui->radioAUTOMATICO->setChecked(true);
            ui->radioMANUAL->setChecked(false);
            ui->radioPROGRAMADO->setChecked(false);
            ui->radioSIMULADO->setChecked(false);
            this->needsOpModeUIUpdate = false;
        }
        updateAutomaticUI(true);
        updateManualUI(false);
        updateProgramaticUI(false);
    }
    else if (opMode == "25") {
        if (!ui->radioMANUAL->isChecked()) {
            ui->radioAUTOMATICO->setChecked(false);
            ui->radioMANUAL->setChecked(true);
            ui->radioPROGRAMADO->setChecked(false);
            ui->radioSIMULADO->setChecked(false);
            this->needsOpModeUIUpdate = false;
        }
        updateAutomaticUI(false);
        updateManualUI(true);
        updateProgramaticUI(false);
    }
    else if (opMode == "26") {
        if (!ui->radioPROGRAMADO->isChecked()) {
            ui->radioAUTOMATICO->setChecked(false);
            ui->radioMANUAL->setChecked(false);
            ui->radioPROGRAMADO->setChecked(true);
            ui->radioSIMULADO->setChecked(false);
            this->needsOpModeUIUpdate = false;
        }
        updateAutomaticUI(false);
        updateManualUI(false);
        updateProgramaticUI(true);
    }
    else if (opMode == "27") {
        if (!ui->radioSIMULADO->isChecked()) {
            ui->radioAUTOMATICO->setChecked(false);
            ui->radioMANUAL->setChecked(false);
            ui->radioPROGRAMADO->setChecked(false);
            ui->radioSIMULADO->setChecked(true);
            this->needsOpModeUIUpdate = false;
        }
        updateAutomaticUI(false);
        updateManualUI(false);
        updateProgramaticUI(false);
    }
}

void MainWindow::atualizar(QStringList valores) {
    //Verificação se deve processar
    if (this->isUpdating) {
        qDebug() << "Em Atualização, retornando.";
        return;
    }

    if (valores.count() != numeroParametros) {
        qDebug() << "ERRO: N PARAMETROS = " << numeroParametros << ". N PARAMETROS RECEBIDOS = " << valores.count();
    }

    this->isUpdating = true;
    bool allZero = false;
    int count = valores.count();

    for (int i = 0; i<count;i++) {
        if (valores[i] != 0) {
            allZero = false;
            break;
        }
    }

    if (allZero) {
        qDebug() << "Dados invalidos, retornando.";
        this->isUpdating = false;
        return;
    }


    this->updateSensorReadings(valores);
    this->plot();

    //Verificação se deve continuar processando
    if (this->needsOpModeUIUpdate) {
        this->updateOPModeUI(valores);
    }


    if (this->needsPumpLightUIUpdate) {
        this->updatePumpLightUI(valores);
    }

    if (this->needsHumidityParametersUIUpdate) {
        this->updateHumidityParameters(valores);
    }

    if (this->needsPIDParametersUIUpdate) {
        this->updatePIDParameters(valores);
    }

    //encapsular em metodo 4
    if (this->statusThread == NULL) {
        this->isUpdating = false;
        return;
    }
    if (this->statusThread->running) {
        QStringList currentArduinoTime;
        for (int i = 13; i<17; i++) {
            currentArduinoTime.append(valores[i]);
        }
        updateArduinoClock(currentArduinoTime);
    }
    this->isUpdating = false;
}

void MainWindow::updateAutomaticUI(bool on) {
    this->ui->label_controleumidade->setEnabled(on);
    this->ui->label_controleumidade_max->setEnabled(on);
    this->ui->label_controleumidade_min->setEnabled(on);

    this->ui->spinBox_controleumidade_max->setEnabled(on);
    this->ui->spinBox_controleumidade_min->setEnabled(on);

    this->ui->kpTextField->setEnabled(on);
    this->ui->kiTextField->setEnabled(on);
    this->ui->kdTextField->setEnabled(on);
    this->ui->setpointTextField->setEnabled(on);

    if (on) {
        this->ui->spinBox_controleumidade_min->setValue(minUmidade);
        this->ui->spinBox_controleumidade_max->setValue(maxUmidade);
    }
}


void MainWindow::updateManualUI(bool on) {
    ui->checkBox_bomba->setEnabled(on);
    ui->checkBox_luzes->setEnabled(on);
    this->ui->coolerPowerSlider->setEnabled(on);

    if (on) {
        this->ui->checkBox_bomba->setChecked(bombaOn);
        this->ui->checkBox_luzes->setChecked(luzOn);
    }
}

void MainWindow::updateProgramaticUI(bool on) {
    this->ui->progPumpOptions->setEnabled(on);
    this->ui->progLightsOptions->setEnabled(on);
    this->ui->progCoolerOptions->setEnabled(on);
}


//Função que envia a string <comando> na serial
void MainWindow::sendCommand(QString command){
    lock.lock();
    if(arduino->isWritable()) {
        if ((command != "?") && (command[0] != 'R')) qDebug() << "escrevendo na serial: " + command;
        arduino->write(command.toStdString().c_str());
        arduino->write(endOfLine);
    }
    lock.unlock();
}

void MainWindow::clearGraph(){
    this->umidadeQVectorX.clear();
    this->umidadeQVectorY.clear();


    this->temperaturaQVectorX.clear();
    this->temperaturaQVectorY.clear();

    this->tempo = 0;
    this->plot();

}

void MainWindow::ui_desconectar() {

    qDebug() << "Atualizando UI: Descontando!";

    //configs iniciais
    this->needsOpModeUIUpdate = true;
    this->needsPumpLightUIUpdate = true;
    this->needsHumidityParametersUIUpdate = true;
    this->needsPIDParametersUIUpdate = true;
    this->opMode = "";
    this->UMIDADE = this->TEMPERATURA = this->UMIDADE_AR = this->LUMINOSIDADE = this->minUmidade = this->maxUmidade = 0;

    this->ui->radioMANUAL->setAutoExclusive(false);
    this->ui->radioAUTOMATICO->setAutoExclusive(false);
    this->ui->radioPROGRAMADO->setAutoExclusive(false);
    this->ui->radioSIMULADO->setAutoExclusive(false);

    this->ui->checkBox_bomba->setDisabled(true);
    this->ui->checkBox_luzes->setDisabled(true);
    this->ui->coolerPowerSlider->setDisabled(true);

    this->ui->label_controleumidade->setDisabled(true);
    this->ui->label_controleumidade_max->setDisabled(true);
    this->ui->label_controleumidade_min->setDisabled(true);
    this->ui->spinBox_controleumidade_max->setDisabled(true);
    this->ui->spinBox_controleumidade_min->setDisabled(true);

    this->ui->kpTextField->setDisabled(true);
    this->ui->kiTextField->setDisabled(true);
    this->ui->kdTextField->setDisabled(true);
    this->ui->setpointTextField->setDisabled(true);

    this->ui->radioAUTOMATICO->setChecked(false);
    this->ui->radioMANUAL->setChecked(false);
    this->ui->radioPROGRAMADO->setChecked(false);
    this->ui->radioSIMULADO->setChecked(false);

    this->ui->ConnectButton->setDisabled(false);
    this->ui->DisconnectButton->setDisabled(true);
    this->ui->lcdTEMP->display("DC");
    this->ui->lcdUMIDADE->display("DC");
    this->ui->lcdLUMINOSIDADE->display("DC");
    this->ui->lcdUMIDADE_AR->display("DC");
    this->ui->lcdUMIDADE->setDisabled(true);
    this->ui->lcdTEMP->setDisabled(true);
    this->ui->lcdLUMINOSIDADE->setDisabled(true);
    this->ui->lcdUMIDADE_AR->setDisabled(true);

    this->ui->radioAUTOMATICO->setAutoExclusive(true);
    this->ui->radioMANUAL->setAutoExclusive(true);
    this->ui->radioPROGRAMADO->setAutoExclusive(true);
    this->ui->radioSIMULADO->setAutoExclusive(true);

    this->ui->radioAUTOMATICO->setDisabled(true);
    this->ui->radioMANUAL->setDisabled(true);
    this->ui->radioPROGRAMADO->setDisabled(true);
    this->ui->radioSIMULADO->setDisabled(true);


    int sampleTimer = 200;
    if (this->statusThread != NULL) {
        sampleTimer = this->statusThread->getSampleFrequency();
    }
    QString sampleTimerString = QString::number(sampleTimer);
    this->ui->sampleTimerLabel->setDisabled(true);
    this->ui->sampleTimerLabel->setText("Tempo de amostragem(ms): " + sampleTimerString);
    this->ui->sampleTimeSlider->setDisabled(true);
    this->ui->sampleTimeSlider->setValue(sampleTimer);

    this->updateProgramaticUI(false);
}

void MainWindow::ui_conectar() {
    ui->radioAUTOMATICO->setDisabled(false);
    ui->radioMANUAL->setDisabled(false);
    ui->radioPROGRAMADO->setDisabled(false);
    ui->radioSIMULADO->setDisabled(false);
    ui->DisconnectButton->setDisabled(false);
    ui->ConnectButton->setDisabled(true);
    ui->lcdUMIDADE->setDisabled(false);
    ui->lcdTEMP->setDisabled(false);
    ui->lcdLUMINOSIDADE->setDisabled(false);
    ui->lcdUMIDADE_AR->setDisabled(false);

    int sampleTimer = 200;
    if (this->statusThread != NULL) {
        sampleTimer = this->statusThread->getSampleFrequency();
    }
    QString sampleTimerString = QString::number(sampleTimer);
    ui->sampleTimerLabel->setDisabled(false);
    ui->sampleTimerLabel->setText("Tempo de amostragem(ms): " + sampleTimerString);
    ui->sampleTimeSlider->setDisabled(false);
    ui->sampleTimeSlider->setValue(sampleTimer);
}

void MainWindow::on_DisconnectButton_clicked()
{
    this->stopThread();
    while(arduino->isOpen()){
        arduino->close();
    }
    this->ui_desconectar();
}


void MainWindow::on_checkBox_luzes_clicked(bool checked)
{

    this->needsPumpLightUIUpdate = false;

    this->luzOn = !checked;
    if (this->luzOn) {
        sendCommand("l");
    }
    else {
        sendCommand("L");
    }
}

void MainWindow::on_checkBox_bomba_clicked(bool checked)
{
    this->needsPumpLightUIUpdate = false;

    this->bombaOn = !checked;
    if (this->bombaOn) {
        sendCommand("b");
    }
    else {
        sendCommand("B");
    }
}

void MainWindow::delay(int t)
{
    QTime dieTime= QTime::currentTime().addMSecs(t);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::on_spinBox_controleumidade_min_valueChanged(QString arg1)
{
    if (this->isUpdating) {
        return;
    }

    this->needsHumidityParametersUIUpdate = false;

    if (ui->spinBox_controleumidade_min->hasFocus()) {
        this->isUpdating = true;
        if (this->statusThread != NULL) {
            this->statusThread->skipIteration = true;
        }
        minUmidade = arg1.toDouble();
        sendCommand("M:24:" +
                    QString::number(minUmidade) + ":" +
                    QString::number(maxUmidade) + ":" +
                    QString(this->ui->setpointTextField->text()) + ":" +
                    QString(this->ui->kpTextField->text()) + ":" +
                    QString(this->ui->kiTextField->text()) + ":" +
                    QString(this->ui->kdTextField->text()));
        if (this->statusThread != NULL) {
            this->statusThread->skipIteration = false;
        }
        this->isUpdating = false;
    }
}

void MainWindow::on_spinBox_controleumidade_max_valueChanged(QString arg1)
{
    if (this->isUpdating) {
        return;
    }

    this->needsHumidityParametersUIUpdate = false;

    if (ui->spinBox_controleumidade_max->hasFocus()) {
        this->isUpdating = true;
        if (this->statusThread != NULL) {
            this->statusThread->skipIteration = true;
        }
        maxUmidade = arg1.toDouble();
        sendCommand("M:24:" +
                    QString::number(minUmidade) + ":" +
                    QString::number(maxUmidade) + ":" +
                    QString(this->ui->setpointTextField->text()) + ":" +
                    QString(this->ui->kpTextField->text()) + ":" +
                    QString(this->ui->kiTextField->text()) + ":" +
                    QString(this->ui->kdTextField->text()));
        if (this->statusThread != NULL) {
            this->statusThread->skipIteration = false;
        }
        this->isUpdating = false;
    }
}

void MainWindow::on_spinBox_controleumidade_min_editingFinished()
{
    if (this->isUpdating) {
        return;
    }

    if (ui->spinBox_controleumidade_min->hasFocus()) {
        this->isUpdating = true;
        if (this->statusThread != NULL) {
            this->statusThread->skipIteration = true;
        }
        minUmidade = ui->spinBox_controleumidade_min->value();
        if (minUmidade < 1) {
            minUmidade = 1;
        }
        sendCommand("M:24:" +
                    QString::number(minUmidade) + ":" +
                    QString::number(maxUmidade) + ":" +
                    QString(this->ui->setpointTextField->text()) + ":" +
                    QString(this->ui->kpTextField->text()) + ":" +
                    QString(this->ui->kiTextField->text()) + ":" +
                    QString(this->ui->kdTextField->text()));
        if (this->statusThread != NULL) {
            this->statusThread->skipIteration = false;
        }
        this->isUpdating = false;
        ui->lcdUMIDADE->setFocus();
    }
}

void MainWindow::on_spinBox_controleumidade_max_editingFinished()
{
    if (this->isUpdating) {
        return;
    }

    if (ui->spinBox_controleumidade_max->hasFocus()) {
        this->isUpdating = true;
        if (this->statusThread != NULL) {
            this->statusThread->skipIteration = true;
        }
        maxUmidade = ui->spinBox_controleumidade_max->value();
        if (maxUmidade > 100) {
            maxUmidade = 100;
        }
        sendCommand("M:24:" +
                    QString::number(minUmidade) + ":" +
                    QString::number(maxUmidade) + ":" +
                    QString(this->ui->setpointTextField->text()) + ":" +
                    QString(this->ui->kpTextField->text()) + ":" +
                    QString(this->ui->kiTextField->text()) + ":" +
                    QString(this->ui->kdTextField->text()));
        if (this->statusThread != NULL) {
            this->statusThread->skipIteration = false;
        }
        this->isUpdating = false;
        ui->lcdUMIDADE->setFocus();
    }
}

void MainWindow::updateArduinoClock(QStringList currentArduinoTime) {
    //ver se precisa atualizar o relogio do arduino
    currentTime = QDateTime::currentDateTime().toString().split(" ");
    if ((currentTime.count() == 5) && (currentArduinoTime.count() == 4)) {

//        qDebug() << "hora Arduino: " << currentArduinoTime;
//        qDebug() << "hora PC: " << currentTime;

        QStringList parsedTime = currentTime[3].split(":");

        if (parsedTime.count() == 3) {

            QString hora = parsedTime[0];
            QString minuto = parsedTime[1];
            QString segundo = parsedTime[2];

            QString dow = currentTime[0];

            dow = convertFromFullString(currentTime[0]);

            if (dow == "0") return;

            if ((currentArduinoTime[0] != hora) ||
                    (currentArduinoTime[1] != minuto) ||
                    (currentArduinoTime[3] != dow)) {
                                qDebug() << "updating Clock";
                //                if (currentArduinoTime[0] != hora) qDebug() << "hora diferente!";
                //                if (currentArduinoTime[1] != minuto) qDebug() << "min diferente!";
                //                if (currentArduinoTime[3] != dow) qDebug() << "dow diferente!";
                sendCommand("R:" + segundo + ":" + minuto + ":" + hora + ":" + dow + ":" + "1" + ":" + "1" + ":" + "2001");
            }
        }
    }
}

QString MainWindow::convertFromFullString(QString dow) {
    if (dow == "Sun") {
        return "1";
    } else if (dow == "Mon") {
        return "2";
    } else if (dow == "Tue") {
        return "3";
    } else if (dow == "Wed") {
        return "4";
    } else if (dow == "Thr") {
        return "5";
    } else if (dow == "Fri") {
        return "6";
    } else if (dow == "Sat") {
        return "7";
    }
    return "0";
}

void MainWindow::on_sampleTimeSlider_sliderReleased()
{
    int value = ui->sampleTimeSlider->value();
    if (this->statusThread != NULL) {
        this->statusThread->setSampleFrequency(value);
    }
    ui->sampleTimerLabel->setText("Tempo de amostragem(ms): " + QString::number(value));
}

void MainWindow::on_sampleTimeSlider_valueChanged(int value)
{
    ui->sampleTimerLabel->setText("Tempo de amostragem(ms): " + QString::number(value));
}

void MainWindow::on_settingsButton_clicked()
{
    if (this->statusThread != NULL) {
        this->launchUSBSettingsWindow(this->statusThread->running);
    }

}

void MainWindow::updateArduinoPortName(QString portName) {
    this->arduino_port_name = portName;
    qDebug() << "Arduino portName updated.";
}

void MainWindow::launchUSBSettingsWindow(bool startConnection) {
    this->stopThread();
    QStringList aPorts;

    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        aPorts.append(serialPortInfo.portName());
    }

    if (aPorts.count() == 0) return;

    qDebug() << aPorts;

    this->settingsWindow = new SettingsWindow(aPorts, startConnection, this);


    connect(this->settingsWindow, SIGNAL(okClicked(QString)), this, SLOT(updateArduinoPortName(QString)));
    this->settingsWindow->setModal(true);
    this->settingsWindow->setFixedWidth(191);
    this->settingsWindow->setFixedHeight(131);
    this->settingsWindow->exec();
}


void MainWindow::on_progPumpOptions_clicked()
{
    if (!(this->arduino->isWritable()) || !(this->arduino->isReadable())) {
        return;
    }
    this->launchProgramaticSettings("pump");
}

void MainWindow::on_progLightsOptions_clicked()
{
    if (!(this->arduino->isWritable()) || !(this->arduino->isReadable())) {
        return;
    }
    this->launchProgramaticSettings("light");
}

void MainWindow::launchProgramaticSettings(QString sender) {
    if (this->statusThread != NULL) {
        if (this->statusThread->running) {
            this->stopThread();
        }
    }
    QString command;

    if (sender == "pump") {
        command = "#";
    } else if (sender == "light") {
        command = "$";
    } else {
        command = "@";
    }

    QString response = this->fetchProgrammaticSettings(command);

    /* if (this->validateProgData(response)) {
        qDebug() << "Resposta otida: " << response;
        QMessageBox::warning(this, "Port Error", "Erro de comunicação 2, tente novamente!");
        return;
    }*/

    int i = 0;
    while (response == "" || i < 5) {
        response = this->fetchProgrammaticSettings(command);
        i++;
    }

    qDebug() << "Resposta valida: " << response;
    if (response == "") {
        QMessageBox::warning(this, "Port Error", "Não foi possível carregar os valores atuais, reconfigurando.");
    }
    this->controlSettings = new ProgramaticControlSettings(response, sender, this);

    connect(this->controlSettings, SIGNAL(okClicked(QString, QString)), this, SLOT(progDataCallback(QString, QString)));

    this->controlSettings->setModal(true);
    this->controlSettings->setFixedWidth(669);
    this->controlSettings->setFixedHeight(124);
    this->controlSettings->exec();
}

QString MainWindow::fetchProgrammaticSettings(QString command) {


    QString response = "";
    bool found = false;
    this->sendCommand(command);
    lock.lock();
    if (this->arduino->waitForReadyRead(50)) {
        QByteArray responseData = arduino->readAll();
        QStringList splitData = QString::fromUtf8(responseData).split("\r\n");
        response += splitData[splitData.count() - 1];
    }
    lock.unlock();

    delay(100);

    int i = 0;
    while (!found) {
        if (i >= 5) {
            response = "";
            break;
        }
        i++;
        this->sendCommand(command);
        lock.lock();
        if (this->arduino->waitForReadyRead(150)) {
            QByteArray responseData = arduino->readAll();
            QStringList splitData = QString::fromUtf8(responseData).split("\r\n");
            response += splitData[0];
        }
        lock.unlock();
        QStringList data = response.split("/");
        if (data.count() == numeroParametrosProg) {
            found = true;

        }
        delay(200);
    }
    return response;
}

bool MainWindow::validateProgData(QString response) {
    QStringList data = response.split("/");
    if (data[0].toInt() >= 1) {
        if (data[1].toInt() <= 7) {
            if (data[2].toInt() >= 0) {
                if (data[3].toInt() >= 0) {
                    if (data[4].toInt() >= 0) {
                        if (data[5].toInt() <= 59) {
                            if (data[6].toInt() <= 59) {
                                if (data[7].toInt() <= 59) {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

void MainWindow::progDataCallback(QString data, QString senderMode) {
    if (data == "schedulePatch") {
        this->startThread();
        return;
    }
    QString command;
    if (senderMode == "pump") {
        command = "M:26:#:";
    }
    else if (senderMode == "light") {
        command = "M:26:$:";
    } else if (senderMode == "cooler") {
        command = "M:26:@:";
    }

    command.append(data);
    //M:26:#:1:7:0:23:0:59:0:29
    //M:26:$:1:7:0:23:0:59:30:59

    //M:26:$:1:7:00:00:00:23:59:59
    //M:26:#:2:7:00:00:00:23:59:59
    this->sendCommand(command);

    this->startThread();
}

void MainWindow::on_kpTextField_returnPressed()
{
    sendCommand("M:24:" +
                QString::number(minUmidade) + ":" +
                QString::number(maxUmidade) + ":" +
                QString(this->ui->setpointTextField->text()) + ":" +
                QString(this->ui->kpTextField->text()) + ":" +
                QString(this->ui->kiTextField->text()) + ":" +
                QString(this->ui->kdTextField->text()));
}

void MainWindow::on_kiTextField_returnPressed()
{
    sendCommand("M:24:" +
                QString::number(minUmidade) + ":" +
                QString::number(maxUmidade) + ":" +
                QString(this->ui->setpointTextField->text()) + ":" +
                QString(this->ui->kpTextField->text()) + ":" +
                QString(this->ui->kiTextField->text()) + ":" +
                QString(this->ui->kdTextField->text()));
}

void MainWindow::on_kdTextField_returnPressed()
{
    sendCommand("M:24:" +
                QString::number(minUmidade) + ":" +
                QString::number(maxUmidade) + ":" +
                QString(this->ui->setpointTextField->text()) + ":" +
                QString(this->ui->kpTextField->text()) + ":" +
                QString(this->ui->kiTextField->text()) + ":" +
                QString(this->ui->kdTextField->text()));
}

void MainWindow::on_setpointTextField_returnPressed()
{
    sendCommand("M:24:" +
                QString::number(minUmidade) + ":" +
                QString::number(maxUmidade) + ":" +
                QString(this->ui->setpointTextField->text()) + ":" +
                QString(this->ui->kpTextField->text()) + ":" +
                QString(this->ui->kiTextField->text()) + ":" +
                QString(this->ui->kdTextField->text()));
}

void MainWindow::on_coolerPowerSlider_sliderReleased()
{

    int value = this->ui->coolerPowerSlider->value();
    if (value <= 64) {
        this->sendCommand("c");
    } else {
        QString command = "C:" + QString::number(value);
        this->sendCommand(command);
    }
}

void MainWindow::on_progCoolerOptions_clicked()
{
    if (!(this->arduino->isWritable()) || !(this->arduino->isReadable())) {
        return;
    }
    this->launchProgramaticSettings("cooler");
}
