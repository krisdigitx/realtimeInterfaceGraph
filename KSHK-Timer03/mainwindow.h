#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBasicTimer>
#include <QLabel>
#include <QProcess>
#include <QHBoxLayout>
#include <qcustomplot.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void realTimeGraph(QCustomPlot *customplot);


private slots:
    void finished(int);
    void realtimeDataSlot();

private:
    Ui::MainWindow *ui;
    QBasicTimer m_timer;
    QLabel m_label;
    QProcess m_process;
    QHBoxLayout m_layout;
    void txMessage();
    void startTimer();
    void timerEvent(QTimerEvent * ev);
    QTimer dataTimer;
};

#endif // MAINWINDOW_H
