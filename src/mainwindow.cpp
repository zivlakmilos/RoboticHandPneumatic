#include "mainwindow.h"

#include <QtGui>

#include "qextserialport.h"
#include "qextserialenumerator.h"

#include "zexception.h"
#include "whandgesture.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    /*
     * Setpu application data
     */
    QCoreApplication::setApplicationName("Robotic Hand Pneumatic");
    QCoreApplication::setOrganizationName("ZI");
    
    setWindowTitle(tr("%1 %2 %3 2016").arg("\xA9",
                                           QCoreApplication::applicationName(),
                                           QCoreApplication::organizationName()));
    
    /*
     * Create connect Port object
     */
    m_port = new QextSerialPort(QextSerialPort::EventDriven, this);
    m_port->setBaudRate(BAUD9600);
    m_port->setDataBits(DATA_8);
    m_port->setStopBits(STOP_1);
    m_port->setParity(PAR_NONE);
    m_port->setFlowControl(FLOW_OFF);
    
    setupGui();
}

MainWindow::~MainWindow(void)
{
}

void MainWindow::setupGui(void)
{
    /*
     * Setup menus
     */
    
    m_actionStart = menuBar()->addAction(tr("&Start"));
    QMenu *menuConnect = menuBar()->addMenu(tr("&Connect"));
    m_actionAbout = menuBar()->addAction(tr("&About"));
    m_actionExit = menuBar()->addAction(tr("E&xit"));
    m_actionExit->setShortcut(tr("Ctrl+Q"));
    
    QList<QextPortInfo> ports =  QextSerialEnumerator::getPorts();
    for(QextPortInfo &port : ports)
        m_actionPorts.append(menuConnect->addAction(port.physName));
    m_actionPorts.append(menuConnect->addAction(tr("Disconnect")));
    
    connect(m_actionStart, SIGNAL(triggered(bool)),
            this, SLOT(startHandGesture()));
    connect(m_actionAbout, SIGNAL(triggered(bool)),
            this, SLOT(about()));
    connect(m_actionExit, SIGNAL(triggered(bool)),
            qApp, SLOT(quit()));
    
    for(QAction *actionPort : m_actionPorts)
        connect(actionPort, SIGNAL(triggered(bool)),
                this, SLOT(openSerialConnection()));
    
    /*
     * Setup status bar
     */
    
    statusBar();
    
    /*
     * Setup central widget
     */
    
    m_centralWidget = new WHandGesture(this);
    setCentralWidget(m_centralWidget);
    
    connect(m_centralWidget, SIGNAL(handMoved(int)),
            this, SLOT(handMoved(int)));
}

void MainWindow::startHandGesture(void)
{
    try {
        m_centralWidget->start();
    } catch(ZException ex) {
        QMessageBox::critical(this, tr("Error when start hand gesture"),
                              tr("Error:<br />%1").arg(ex.what()));
        QApplication::quit();
    }
}

void MainWindow::about(void)
{
    QMessageBox::about(this, tr("%1").arg(QCoreApplication::applicationName()),
                       tr("<b>%1</b><br /><p>"
                          "Milos Zivlak (zivlakmilos@gmail.com)<br />"
                          "Pavle Kukavica</p>").arg(QCoreApplication::applicationName()));
}

void MainWindow::openSerialConnection(void)
{
    QAction *actionSender = qobject_cast<QAction *>(sender());
    if(!actionSender)
        return;
    
    QString portName = actionSender->text();
    if(m_port->isOpen())
        m_port->close();
    if(portName == tr("Disconnect"))
        return;
    
    m_port->setPortName(portName);
    m_port->open(QIODevice::ReadWrite);
    
    if(!m_port->isOpen())
        QMessageBox::warning(this, QCoreApplication::applicationName(),
                             tr("Can't open connection to %1 port").arg(portName),
                             QMessageBox::Ok | QMessageBox::Default);
}

void MainWindow::send(QByteArray data)
{
    QByteArray dataToSend;
    dataToSend.append(';');
    
    if(!m_port->isOpen())
        return;
    
    dataToSend.append(data);
    dataToSend.append('?');
    m_port->write(dataToSend);
    m_port->flush();
}

void MainWindow::handMoved(int move)
{
    QString data = QString::number(move);
    send(data.toAscii());
}
