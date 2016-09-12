#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <QMainWindow>

class WHandGesture;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow(void);
    
private:
    void setupGui(void);
    
    QAction *m_actionStart;
    QAction *m_actionAbout;
    QAction *m_actionExit;
    
    WHandGesture *m_centralWidget;
    
private slots:
    void startHandGesture(void);
    void about(void);
};

#endif // _MAIN_WINDOW_H_
