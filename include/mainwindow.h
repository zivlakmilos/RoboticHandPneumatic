#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <QMainWindow>

class WHandGesture;
class QAction;
class QextSerialPort;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow(void);
    
private:
    void setupGui(void);
    void send(QByteArray data);
    
    QAction *m_actionStart;
    QAction *m_actionAbout;
    QAction *m_actionExit;
    QList<QAction *> m_actionPorts;
    
    WHandGesture *m_centralWidget;
    
    QextSerialPort *m_port;
    
private slots:
    void startHandGesture(void);
    void about(void);
    void openSerialConnection(void);
    void handMoved(int move);
};

#endif // _MAIN_WINDOW_H_
