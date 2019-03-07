#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    SettingsWindow(QStringList list, bool startConnection, QWidget *parent = 0);
    QStringList availablePorts;
    ~SettingsWindow();

signals:
    void okClicked(QString);
    void requestConnection();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::SettingsWindow *ui;
    bool startConnection = false;
};

#endif // SETTINGSWINDOW_H
