#include "whandgesture.h"

#include <QtGui>
#include <opencv2/opencv.hpp>

#include "zexception.h"

using namespace cv;

WHandGesture::WHandGesture(QWidget *parent)
    : QWidget(parent),
      m_capture(0),
      m_mode(ModeSampling)
{
    /*
     * Setup timer for update frames
     */
    m_tmrFrameUpdater = new QTimer(this);
    m_tmrFrameUpdater->setInterval(40);    // 4ms
    connect(m_tmrFrameUpdater, SIGNAL(timeout()),
            this, SLOT(updateFrame()));
    
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
    QVector<int> h;
    QVector<int> s;
    QVector<int> v;
    int i;
    
    for(i = 0; i < matRoi.size(); i++)
    {
        Mat1b mask(matRoi[i].rows, matRoi[i].cols);
        
        h.push_back(mean(matRoi[i], mask)[0]);
        s.push_back(mean(matRoi[i], mask)[1]);
        v.push_back(mean(matRoi[i], mask)[2]);
    }
    
    m_color[0] = median(h);
    m_color[1] = median(s);
    m_color[2] = median(v);
}

int WHandGesture::median(QVector<int> &values)
{
    int result;
    qSort(values.begin(), values.end());
    if(values.size() % 2)
        result = (values[values.size() / 2] + values[values.size() / 2 + 1]) / 2;
    else
        result = values[values.size() / 2 + 1];
    return result;
}

void WHandGesture::gesture(Mat &matOriginal, Mat &matThreshold)
{
}
