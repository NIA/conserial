#include "widget.h"
#include "ui_widget.h"
#include "qextserialport.h"
#include <QInputDialog>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

namespace {
    const QString DEFAULT_PORT = "COM9";
    const QByteArray DATA_PREFIX(5, '\xF0');
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
        send(data);
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

                // Prepare data packet
                QByteArray data = DATA_PREFIX;
                foreach(int val, values) {
                    data.append( (char*)&val, sizeof(val));
                }
                // Send it
                send(data.toHex());
            }
        }
    });
}

void Widget::send(QByteArray data) {
    write("<< " + data);
    port->write(QByteArray::fromHex(data));
    port->flush();
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
    delete ui;
}
