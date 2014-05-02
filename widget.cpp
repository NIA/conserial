#include "widget.h"
#include "ui_widget.h"
#include "qextserialport.h"
#include <QInputDialog>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <qmath.h>

namespace {
    const QString DEFAULT_PORT = "COM9";
    const QByteArray DATA_PREFIX(5, '\xF0');
    const unsigned CHANNELS_NUM = 3;

    // AUTO MODE generation params
    const int    DATA_SIZE = 200;
    const double PERIOD_MS = 1000;
    const double OMEGA1 = 3*M_PI/1000;
    const double OMEGA2 = OMEGA1/15;
    const double AMP = 9999999;
}

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget), port(NULL)
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0, 5);
    ui->splitter->setStretchFactor(1, 1);

    write(tr("Asking for port name"));
    bool ok;
    QString portName =
        QInputDialog::getText(this, tr("Specify port"), tr("Serial port name:"),
                              QLineEdit::Normal, DEFAULT_PORT, &ok );
    if( ! ok ) {
        close();
    } else {
        openPort(portName);
    }
}

void Widget::openPort(QString portName) {
    write(tr("Opening port %1").arg(portName));
    port = new QextSerialPort(portName);
    port->setBaudRate(BAUD57600);
    connect(port, &QextSerialPort::readyRead, [=](){
        QByteArray data = port->readAll();
        write(">> " + data.toHex());
    });

    if(port->open(QextSerialPort::ReadWrite)) {
        write(tr("Opened!"));
    } else {
        write(tr("Failed :("));
    }

    connect(ui->send, &QPushButton::clicked, [=](){
        QByteArray data = ui->dataInput->toPlainText().toLocal8Bit();
        send(QByteArray::fromHex(data));
    });

    connect(ui->sendFile, &QPushButton::clicked, [=](){
        QString fileName = QFileDialog::getOpenFileName(this, tr("Choose data file"));
        if( ! fileName.isEmpty() ) {
            QFile file(fileName);
            if( file.open(QFile::ReadOnly | QFile::Text) ) {
                QVector<int> values;
                // Read file
                QTextStream in(&file);
                QString line;
                while( ! (line = in.readLine()).isNull() ) {
                    if(line.isEmpty()) { continue; }

                    bool ok;
                    int val = line.toInt(&ok);
                    if(ok) {
                        values << val;
                    }
                }
                sendValues(values);
            }
        }
    });

    connect(&autoTimer, &QTimer::timeout, this, &Widget::sendGenerated);
    connect(ui->autoMode, &QPushButton::clicked, [=](){
        if (autoTimer.isActive()) {
            autoTimer.stop();
        } else {
            autoTimer.start(PERIOD_MS);
        }
    });
}

void Widget::send(QByteArray data, bool print) {
    if (print) {
        write("<< " + data.toHex());
    }
    port->write(data);
    port->flush();
}

void Widget::sendValues(QVector<int> values, bool print) {
    // Prepare data packet
    QByteArray data = DATA_PREFIX;
    foreach(int val, values) {
        for(unsigned ch = 0; ch < CHANNELS_NUM; ++ch) {
            // By now just repeats each data point CHANNELS_NUM times (the same data for all channelds)
            data.append( (char*)&val, sizeof(val));
        }
    }
    // Send it
    send(data, print);
}

void Widget::sendGenerated() {
    QVector<int> values(DATA_SIZE);
    double t0 = QDateTime::currentMSecsSinceEpoch() - PERIOD_MS;
    double dt = PERIOD_MS / double(DATA_SIZE);
    double t = t0;
    for(int i = 0; i < DATA_SIZE; ++i) {
        values[i] = AMP*qSin(OMEGA1*t)*qCos(OMEGA2*t);
        t += dt;
    }
    // send, but not print
    sendValues(values, false);
    // print stats instead
    write(QString("<< ... (sent %1 items with t0 = %2)").arg(DATA_SIZE).arg(t0,0,'f',0));
}

void Widget::write(QString text) {
    QString color;
    if(text.startsWith('>')) {
        color = "green";
    } else if(text.startsWith('<')) {
        color = "blue";
    }
    ui->console->appendHtml(QString("<font color=%1>%2</font>").arg(color).arg(text.toHtmlEscaped()));
}

Widget::~Widget()
{
    autoTimer.disconnect();
    autoTimer.stop();
    delete ui;
}
