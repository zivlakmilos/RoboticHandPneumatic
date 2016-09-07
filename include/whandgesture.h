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
    
    enum Mode {
        ModeSampling,
        ModeGesture
    };
    
    enum SamplingRectangle {
        SamplingRectangleWidth = 15,
        SamplingRectangleHeight = 15
    };
    
protected:
    virtual void keyPressEvent(QKeyEvent *event);
    
private:
    void setupGui(void);
    QImage matToImg(cv::Mat mat);
    void sampling(cv::Mat &matOriginal, cv::Mat &matThreshold);
    void gesture(cv::Mat &matOriginal, cv::Mat &matThreshold);
    void colorFromSamples(QVector<cv::Mat> &matRoi);
    void makeBinary(cv::Mat &matThreshold);
    
    QLabel *m_lblOriginalImage;
    QLabel *m_lblThresholdImage;
    QTimer *m_tmrFrameUpdater;
    
    cv::VideoCapture m_capture;
    Mode m_mode;
    QVector<cv::Scalar> m_color;
    cv::Scalar m_lowerColor;
    cv::Scalar m_upperColor;
    
private slots:
    void updateFrame(void);
};

#endif // _W_HAND_GESTURE_H_
