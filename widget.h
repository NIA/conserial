#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}
class QextSerialPort;

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = 0);

    /*!
     * @brief write to console and auto-format it
     * @param text text to write
     */
    void write(QString text);

    /*!
     * \brief sends data to port and prints to screet
     * \param data data to send
     */
    void send(QByteArray data);

    /*!
     * \brief open port and setup handlers
     * \param portName system port name like COM6
     */
    void openPort(QString portName);

    ~Widget();
    
private:
    Ui::Widget *ui;
    QextSerialPort * port;
};

#endif // WIDGET_H
