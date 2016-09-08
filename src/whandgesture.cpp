#include "whandgesture.h"

#include <vector>
#include <QtGui>
#include <opencv2/opencv.hpp>

#include "zexception.h"

using namespace cv;

WHandGesture::WHandGesture(QWidget *parent)
    : QWidget(parent),
      m_capture(0),
      m_mode(ModeSampling),
      m_lowerColor(12, 30, 80),
      m_upperColor(7, 40, 80)
{
    /*
     * Setup timer for update frames
     */
    m_tmrFrameUpdater = new QTimer(this);
    m_tmrFrameUpdater->setInterval(40);    // 4ms
    connect(m_tmrFrameUpdater, SIGNAL(timeout()),
            this, SLOT(updateFrame()));
    
    setFocusPolicy(Qt::StrongFocus);    // For key event
    
    /*
     * Setup GUI
     */
    setupGui();
}

WHandGesture::~WHandGesture(void)
{
}

void WHandGesture::setupGui(void)
{
    /*
     * Create layouts
     */
    
    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    QHBoxLayout *layoutImages = new QHBoxLayout;
    
    /*
     * Create components
     */
    
    m_lblOriginalImage = new QLabel(this);
    m_lblThresholdImage = new QLabel(this);
    
    /*
     * Lay down components
     */
    
    layoutImages->addWidget(m_lblOriginalImage);
    layoutImages->addWidget(m_lblThresholdImage);
    
    layoutMain->addLayout(layoutImages);
}

void WHandGesture::start(void)
{
    if(!m_capture.isOpened())
        throw ZException("Can't open capture device");
    m_tmrFrameUpdater->start();
}

QImage WHandGesture::matToImg(Mat mat)
{
    if(mat.empty())
        throw ZException("Mat is empty");
    
    QImage result;
    
    switch(mat.type())
    {
        case CV_8UC4:
            result = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB32);
            break;
        case CV_8UC3:
            result = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
            result.rgbSwapped();
            break;
        case CV_8UC1:
            static QVector<QRgb> colorTable;
            for(int i = 0; i < 256; i++)
                colorTable.push_back(qRgb(i, i, i));
            result = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
            result.setColorTable(colorTable);
            break;
        default:
            throw ZException("Unsupported mat type");
            break;
    }
    
    return result;
}

void WHandGesture::updateFrame(void)
{
    Mat matOriginal;
    Mat matThreshold;
    
    m_capture.read(matOriginal);
    flip(matOriginal, matOriginal, 1);  // Flip image for easyer usage
    
    cvtColor(matOriginal, matThreshold, CV_BGR2HSV); // Convert image to hsv format
    
    switch(m_mode)
    {
        case ModeSampling:
            sampling(matOriginal, matThreshold);
            break;
        case ModeGesture:
            gesture(matOriginal, matThreshold);
            break;
    }
    
    /*
     * Draw image to UI
     */
    try {
        QImage imgOriginal = matToImg(matOriginal);
        QImage imgThreshold = matToImg(matThreshold);
        m_lblOriginalImage->setPixmap(QPixmap::fromImage(imgOriginal));
        m_lblThresholdImage->setPixmap(QPixmap::fromImage(imgThreshold));
    } catch(ZException ex) {
        m_tmrFrameUpdater->stop();
        QMessageBox::critical(this, tr("Error with proccess image"),
                              tr("Error:<br />%1").arg(ex.what()),
                              QMessageBox::Ok | QMessageBox::Default);
    }
}

void WHandGesture::sampling(Mat &matOriginal, Mat &matThreshold)
{
    int centerX = matOriginal.cols / 2;
    int centerY = matOriginal.rows / 2;
    QVector<Rect> roi;              // Region of interest
    QVector<Mat> matRoi;            // Mat of roi
    
    roi.push_back(Rect(centerX, centerY, SamplingRectangleWidth, SamplingRectangleHeight));
    roi.push_back(Rect(centerX, centerY - 100, SamplingRectangleWidth, SamplingRectangleHeight));
    roi.push_back(Rect(centerX, centerY - 200, SamplingRectangleWidth, SamplingRectangleHeight));
    roi.push_back(Rect(centerX - 100, centerY - 50, SamplingRectangleWidth, SamplingRectangleHeight));
    roi.push_back(Rect(centerX + 25, centerY - 25, SamplingRectangleWidth, SamplingRectangleHeight));
    roi.push_back(Rect(centerX + 25, centerY + 25, SamplingRectangleWidth, SamplingRectangleHeight));
    
    for(int i = 0; i < roi.size(); i++)
    {
        rectangle(matOriginal, roi[i], Scalar(255, 0, 0), 2);
        rectangle(matThreshold, roi[i], Scalar(255, 0, 0), 2);
        Mat matR = matThreshold(Range(roi[i].x, roi[i].x + roi[i].width),
                                Range(roi[i].y, roi[i].y + roi[i].height));
        matRoi.push_back(matR);
    }
    
    colorFromSamples(matRoi);
}

void WHandGesture::colorFromSamples(QVector<Mat> &matRoi)
{
    m_color.clear();
    for(int i = 0; i < matRoi.size(); i++)
    {
        Mat1b mask(matRoi[i].rows, matRoi[i].cols);
        m_color.push_back(mean(matRoi[i], mask));
    }
}

void WHandGesture::gesture(Mat &matOriginal, Mat &matThreshold)
{
    std::vector<Point> contour;
    
    makeBinary(matThreshold);
    if(findContour(matThreshold, contour))
    {
        std::vector<std::vector<Point> > tmpContours;
        tmpContours.push_back(contour);
        drawContours(matOriginal, tmpContours, -1, Scalar(0, 255, 0), 3, 8);
        rectangle(matOriginal, boundingRect(Mat(contour)), Scalar(255, 0, 0), 3, 8);
    }
}

void WHandGesture::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if(m_mode == ModeSampling)
                m_mode = ModeGesture;
            else
                m_mode = ModeSampling;
            break;
    }
}

void WHandGesture::makeBinary(Mat &matThreshold)
{
    int i;
    
    QVector<Mat> matBinarys;
    for(i = 0; i < m_color.size(); i++)
    {
        Scalar lowerColor(m_color[0][0] - m_lowerColor[0],
                          m_color[0][1] - m_lowerColor[1],
                          m_color[0][2] - m_lowerColor[2]);
        Scalar upperColor(m_color[0][0] + m_upperColor[0],
                          m_color[0][1] + m_upperColor[1],
                          m_color[0][2] + m_upperColor[2]);
        matBinarys.push_back(Mat(matThreshold.rows, matThreshold.cols, CV_8U));
        inRange(matThreshold, lowerColor, upperColor, matBinarys[i]);
    }
    
    matThreshold = matBinarys[0].clone();
    for(i = 1; i < matBinarys.size(); i++)
        matThreshold += matBinarys[0];
    
    medianBlur(matThreshold, matThreshold, 7);
}

bool WHandGesture::findContour(Mat &matThreshold, std::vector<Point> &contour)
{
    Mat matWorking = matThreshold.clone();
    std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarcy;
    
    findContours(matWorking, contours, hierarcy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    
    int idx = -1;   // Biggest contour
    for(int i = 0; i < contours.size(); i++)
    {
        if(idx < 0)
        {
            idx = i;
            continue;
        }
        if(contours[i].size() > contours[idx].size())
            idx = i;
    }
    
    if(idx < 0)
        return false;
    
    contour.clear();
    for(int i = 0; i < contours[idx].size(); i++)
    {
        contour.push_back(contours[idx][i]);
    }
    
    return true;
}
