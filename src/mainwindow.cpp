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
    
    setWindowTitle("Robotic Hand Pneumatic");
    
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
    
    connect(m_actionStart, SIGNAL(triggered(bool)),
            this, SLOT(startHandGesture()));
    
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
