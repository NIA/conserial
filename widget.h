#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class Widget;
}
class QextSerialPort;

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = 0);

    ~Widget();

public slots:
    /*!
     * @brief write to console and auto-format it
     * @param text text to write
     */
    void write(QString text);

    /*!
     * \brief sends data to port and prints to screet
     * \param data data to send
     */
    void send(QByteArray data, bool print = true);

    /*!
     * \brief encode \a values and send them
     * \param values values to send
     */
    void sendValues(QVector<int> values, bool print = true);

    /*!
     * \brief generate data and send
     */
    void sendGenerated();

    /*!
     * \brief open port and setup handlers
     * \param portName system port name like COM6
     */
    void openPort(QString portName);
    
private:
    Ui::Widget *ui;
    QextSerialPort * port;

    QTimer autoTimer;
};

#endif // WIDGET_H
