#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>
#include <QBasicTimer>
#include <QHBoxLayout>
#include <QLabel>

double value0;
double lvalue0 = 0.00;
double nvalue0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    m_layout.addWidget(&m_label);
    realTimeGraph(ui->customplot);
    startTimer();
    connect(&m_process, SIGNAL(finished(int)), SLOT(finished(int)));

}

void MainWindow::timerEvent(QTimerEvent * ev)
{
    if (ev->timerId() == m_timer.timerId() ) txMessage();
}

void MainWindow::txMessage()
{
    m_timer.stop();
    m_process.start("netstat", QStringList() << "-i", QProcess::ReadOnly);
}

void MainWindow::finished(int rc)
{
    startTimer();
    if (rc != 0)
    {
        m_label.setText("Error");
    }
    else
    {
        QString output = QString::fromLocal8Bit(m_process.readAll());
        QStringList lines = output.split("\n", QString::SkipEmptyParts);

        foreach (QString line, lines) {
           if (!line.contains("ens33")) continue;
           QStringList args = line.split(' ', QString::SkipEmptyParts);
           if (args.count() >= 2)
           {
               //m_label.setText(args.at(3));
               value0 = args.at(2).toDouble();

               nvalue0 = value0 - lvalue0;
               ui->ens33_rx->setText(args.at(2));
               ui->ens33_tx->setText(args.at(6));
               ui->rate_value->setText(QString::number(nvalue0) + "kb/s");
               lvalue0 = value0;
               return;
           }
        }
    }
    m_label.setText("...");
}

void MainWindow::startTimer()
{
    //m_timer.start(1000, Qt::CoarseTimer, this);
    m_timer.start(1000, this);
}

void MainWindow::realTimeGraph(QCustomPlot *customplot)
{
    customplot->addGraph();
    customplot->graph(0)->setPen(QPen(Qt::blue));
    customplot->addGraph();
    customplot->graph(1)->setPen(QPen(Qt::blue));
    customplot->graph(1)->setLineStyle(QCPGraph::lsNone);
    customplot->graph(1)->setScatterStyle(QCPScatterStyle::ssDisc);

    customplot->addGraph(); // blue dot
    customplot->graph(2)->setPen(QPen(Qt::blue));
    customplot->graph(2)->setLineStyle(QCPGraph::lsNone);
    customplot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);

    customplot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customplot->xAxis->setDateTimeFormat("hh:mm:ss");
    customplot->xAxis->setAutoTickStep(false);
    customplot->xAxis->setTickStep(2);
    customplot->axisRect()->setupFullAxesBox();

    connect(customplot->xAxis, SIGNAL(rangeChanged(QCPRange)), customplot->xAxis2, SLOT(setRange(QCPRange)));
    connect(customplot->yAxis, SIGNAL(rangeChanged(QCPRange)), customplot->yAxis2, SLOT(setRange(QCPRange)));

    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(0);
}

void MainWindow::realtimeDataSlot()
{
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.01)
    {
        //double value0 = qSin(key);

        ui->customplot->graph(0)->addData(key, nvalue0);
        ui->customplot->graph(1)->clearData();
        ui->customplot->graph(1)->addData(key, nvalue0);
        // remove data of lines that's outside visible range:
        ui->customplot->graph(0)->removeDataBefore(key-8);
        ui->customplot->graph(1)->removeDataBefore(key-8);
        // rescale value (vertical) axis to fit the current data:
        ui->customplot->graph(0)->rescaleValueAxis();
        ui->customplot->graph(1)->rescaleValueAxis(true);
        lastPointKey = key;
    }
    ui->customplot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
    ui->customplot->replot();
    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
      ui->statusBar->showMessage(
            QString("%1 FPS, Total Data points: %2")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(ui->customplot->graph(0)->data()->count()+ui->customplot->graph(1)->data()->count())
            , 0);
      lastFpsKey = key;
      frameCount = 0;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
