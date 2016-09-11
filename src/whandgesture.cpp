#include "whandgesture.h"

#include <vector>
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
    QGridLayout *layoutSliders = new QGridLayout;
    
    /*
     * Create components
     */
    
    m_lblOriginalImage = new QLabel(this);
    m_lblThresholdImage = new QLabel(this);
    
    QLabel *lblLowBound = new QLabel(tr("Low bound"));
    QLabel *lblHigthBound = new QLabel(tr("High bound"));
    QLabel *lblHule = new QLabel(tr("Hule"));
    QLabel *lblSaturation = new QLabel(tr("Saturation"));
    QLabel *lblValue = new QLabel(tr("Value"));
    
    m_sliderLowerColor[ColorSliderHule] = new QSlider(Qt::Horizontal);
    m_sliderLowerColor[ColorSliderHule]->setRange(-360, 0);
    m_sliderLowerColor[ColorSliderHule]->setValue(-12);
    m_sliderUpperColor[ColorSliderHule] = new QSlider(Qt::Horizontal);
    m_sliderUpperColor[ColorSliderHule]->setRange(0, 360);
    m_sliderUpperColor[ColorSliderHule]->setValue(7);
    
    m_sliderLowerColor[ColorSliderSaturation] = new QSlider(Qt::Horizontal);
    m_sliderLowerColor[ColorSliderSaturation]->setRange(-100, 0);
    m_sliderLowerColor[ColorSliderSaturation]->setValue(-30);
    m_sliderUpperColor[ColorSliderSaturation] = new QSlider(Qt::Horizontal);
    m_sliderUpperColor[ColorSliderSaturation]->setRange(0, 100);
    m_sliderUpperColor[ColorSliderSaturation]->setValue(40);
    
    m_sliderLowerColor[ColorSliderValue] = new QSlider(Qt::Horizontal);
    m_sliderLowerColor[ColorSliderValue]->setRange(-100, 0);
    m_sliderLowerColor[ColorSliderValue]->setValue(-80);
    m_sliderUpperColor[ColorSliderValue] = new QSlider(Qt::Horizontal);
    m_sliderUpperColor[ColorSliderValue]->setRange(0, 100);
    m_sliderUpperColor[ColorSliderValue]->setValue(80);
    
    /*
     * Lay down components
     */
    
    layoutImages->addWidget(m_lblOriginalImage);
    layoutImages->addWidget(m_lblThresholdImage);
    
    layoutSliders->addWidget(lblLowBound, 0, 1, Qt::AlignCenter);
    layoutSliders->addWidget(lblHigthBound, 0, 2, Qt::AlignCenter);
    layoutSliders->addWidget(lblHule, 1, 0);
    layoutSliders->addWidget(lblSaturation, 2, 0);
    layoutSliders->addWidget(lblValue, 3, 0);
    layoutSliders->addWidget(m_sliderLowerColor[ColorSliderHule], 1, 1);
    layoutSliders->addWidget(m_sliderUpperColor[ColorSliderHule], 1, 2);
    layoutSliders->addWidget(m_sliderLowerColor[ColorSliderSaturation], 2, 1);
    layoutSliders->addWidget(m_sliderUpperColor[ColorSliderSaturation], 2, 2);
    layoutSliders->addWidget(m_sliderLowerColor[ColorSliderValue], 3, 1);
    layoutSliders->addWidget(m_sliderUpperColor[ColorSliderValue], 3, 2);
    
    layoutMain->addLayout(layoutSliders);
    layoutMain->addLayout(layoutImages);
    layoutMain->setStretch(1, 1);
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
    std::vector<std::vector<Point> > contours(1);
    
    makeBinary(matThreshold);
    if(!findContour(matThreshold, contours[0]))
        return;
    
    drawContours(matOriginal, contours, -1, Scalar(0, 255, 0), 3, 8);
    rectangle(matOriginal, boundingRect(Mat(contours[0])), Scalar(255, 0, 0), 3, 8);
    
    std::vector<std::vector<Point> > hull(1);
    std::vector<Vec4i> defects;
    int fingersCuont = countFingers(contours[0], hull[0], defects);
    drawContours(matOriginal, hull, -1, Scalar(0, 0, 255), 3, 8);
    
    for(int i = 0; i < defects.size(); i++)
    {
        Point pt = contours[0][defects[i].val[2]];
        circle(matOriginal, pt, 5, Scalar(255, 0, 0), CV_FILLED);
    }
    
    std::cout << fingersCuont << std::endl;
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
        Scalar lowerColor(m_color[0][0] + m_sliderLowerColor[ColorSliderHule]->value(),
                          m_color[0][1] + m_sliderLowerColor[ColorSliderSaturation]->value(),
                          m_color[0][2] + m_sliderLowerColor[ColorSliderValue]->value());
        Scalar upperColor(m_color[0][0] + m_sliderUpperColor[ColorSliderHule]->value(),
                          m_color[0][1] + m_sliderUpperColor[ColorSliderSaturation]->value(),
                          m_color[0][2] + m_sliderUpperColor[ColorSliderValue]->value());
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

int WHandGesture::countFingers(std::vector<Point> &contour,
                               std::vector<Point> &hull,
                               std::vector<Vec4i> &defects)
{
    std::vector<int> hullForDefects;
    convexHull(contour, hull, false, true);             // For drawing
    convexHull(contour, hullForDefects, false, false);  // For defects
    convexityDefects(contour, hullForDefects, defects);
    
    /*
     * Checking if defects betwean 2 fingers
     *  (couting fingers)
     */
    int fingerCount = 0;
    
    for(int i = 0; i < defects.size(); i++)
    {
        Point ptStart(contour[defects[i].val[0]]);
        Point ptEnd(contour[defects[i].val[1]]);
        Point ptFar(contour[defects[i].val[2]]);
        
        /*
         * Using dot product of vectors to find angle
         */
        float length1 = sqrt(pow(ptFar.x - ptStart.x, 2) +
                             pow(ptFar.y - ptStart.y, 2));
        float length2 = sqrt(pow(ptFar.x - ptStart.x, 2) +
                             pow(ptFar.y - ptStart.y, 2));
        float dotProduct = (ptStart.x - ptFar.x) * (ptStart.y - ptFar.y) +
                           (ptEnd.x - ptFar.x) * (ptEnd.y - ptFar.y);
        float angle = acos(dotProduct / (length1 * length2));
        angle = angle * 180 / CV_PI;    // Radian to degrease
        
        if(angle > 100 ||       // Checking angle and lengths
           length1 < 50 ||
           length2 < 50)
            continue;
        
        fingerCount++;
    }
    
    return fingerCount;
}
