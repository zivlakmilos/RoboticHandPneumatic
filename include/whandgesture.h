#ifndef _W_HAND_GESTURE_H_
#define _W_HAND_GESTURE_H_

#include <QWidget>

#include <opencv2/videoio.hpp>

class QLabel;
class QTimer;
class QSlider;

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
    
    enum ColorSliders {
        ColorSliderHule = 0,
        ColorSliderSaturation,
        ColorSliderValue,
        ColorSliderSize         // Must be last!
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
    bool findContour(cv::Mat &matThreshold, std::vector<cv::Point> &contour);
    int countFingers(std::vector<cv::Point> &contour,
                     std::vector<cv::Point> &hull,
                     std::vector<cv::Vec4i> &defects);
    
    QLabel *m_lblOriginalImage;
    QLabel *m_lblThresholdImage;
    QTimer *m_tmrFrameUpdater;
    
    cv::VideoCapture m_capture;
    Mode m_mode;
    QVector<cv::Scalar> m_color;
    
    QSlider *m_sliderLowerColor[ColorSliderSize];
    QSlider *m_sliderUpperColor[ColorSliderSize];
    QSlider *m_sliderBlur;
    
private slots:
    void updateFrame(void);
    
    void sliderBlurValueChanged(int value);     // Calculating value for median blur (only odds)
};

#endif // _W_HAND_GESTURE_H_
