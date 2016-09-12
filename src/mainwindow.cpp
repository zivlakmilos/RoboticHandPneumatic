#include "mainwindow.h"

#include <QtGui>

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
    
    connect(m_actionStart, SIGNAL(triggered(bool)),
            this, SLOT(startHandGesture()));
    connect(m_actionAbout, SIGNAL(triggered(bool)),
            this, SLOT(about()));
    connect(m_actionExit, SIGNAL(triggered(bool)),
            qApp, SLOT(quit()));
    
    /*
     * Setup status bar
     */
    
    statusBar();
    
    /*
     * Setup central widget
     */
    m_centralWidget = new WHandGesture(this);
    setCentralWidget(m_centralWidget);
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
