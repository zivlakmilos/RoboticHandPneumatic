#ifndef _W_HAND_GESTURE_H_
#define _W_HAND_GESTURE_H_

#include <QWidget>

#include <opencv2/videoio.hpp>

class QLabel;
class QTimer;

class WHandGesture : public QWidget
{
    Q_OBJECT
    
public:
    explicit WHandGesture(QWidget *parent = 0);
    virtual ~WHandGesture(void);
    
    void start(void);
    
private:
    void setupGui(void);
    QImage matToImg(cv::Mat mat);
    
    QLabel *m_lblOriginalImage;
    QLabel *m_lblThresholdImage;
    QTimer *m_tmrFrameUpdater;
    
    cv::VideoCapture m_capture;
    
private slots:
    void updateFrame(void);
};

#endif // _W_HAND_GESTURE_H_
