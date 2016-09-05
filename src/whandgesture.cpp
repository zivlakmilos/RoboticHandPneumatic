#include "whandgesture.h"

#include <QtGui>
#include <opencv2/opencv.hpp>

#include "zexception.h"

using namespace cv;

WHandGesture::WHandGesture(QWidget *parent)
    : QWidget(parent)
{
    /*
     * Setup timer for update frames
     */
    m_tmrFrameUpdater = new QTimer(this);
    m_tmrFrameUpdater->setInterval(40);    // 4ms
    connect(m_tmrFrameUpdater, SIGNAL(timeout()),
            this, SLOT(updateFrame()));
    
    /*
     * Setup Open CV
     */
    m_capture = VideoCapture(0);
    
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
    flip(matOriginal, matOriginal, 1);
    
    /*
     * Draw image to UI
     */
    try {
        QImage imgOriginal = matToImg(matOriginal);
        //QImage imgThreshold = matToImg(matThreshold);
        m_lblOriginalImage->setPixmap(QPixmap::fromImage(imgOriginal));
        //m_lblThresholdImage->setPixmap(QPixmap::fromImage(imgThreshold));
    } catch(ZException ex) {
        m_tmrFrameUpdater->stop();
        QMessageBox::critical(this, tr("Error with proccess image"),
                              tr("Error:<br />%1").arg(ex.what()),
                              QMessageBox::Ok | QMessageBox::Default);
    }
}