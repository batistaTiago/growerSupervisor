#ifndef PROGRAMATICCONTROLSETTINGS_H
#define PROGRAMATICCONTROLSETTINGS_H

#include <QDialog>

namespace Ui {
class ProgramaticControlSettings;
}

class ProgramaticControlSettings : public QDialog
{
    Q_OBJECT

public:
    ProgramaticControlSettings(QString data, QString senderMode, QWidget *parent = 0);
    ~ProgramaticControlSettings();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    Ui::ProgramaticControlSettings *ui;
    QString senderMode;

signals:
    void okClicked(QString, QString);
};

#endif // PROGRAMATICCONTROLSETTINGS_H
