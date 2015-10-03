// This file is part of Pixmicro.
// 
// Copyright (C) 2015 Kurt Skauen <http://kavionic.com/>
// 
// Pixmicro is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Pixmicro is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Pixmicro. If not, see < http://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CaptureThread.h"
#include "CameraSettings.h"

CaptureThread::CaptureThread()
{
//    m_WantedCamera = 0;
//    m_CurrentCamera = -1;

//    OpenCamera(CameraSettings::GetInstance()->GetSelectedCamera());
}


CaptureThread::~CaptureThread()
{
}

void CaptureThread::SetThread( QThread* thread )
{
    moveToThread(thread);
    m_FrameTimer.moveToThread(thread);

    connect(thread, SIGNAL(started()), this, SLOT(SlotStarted()));
}
/*
void CaptureThread::SetCamera(int index)
{
    m_WantedCamera = index;
}
*/
void CaptureThread::SlotStarted()
{
    connect(&m_VideoCapturer, &VideoCapturer::SignalFrameCaptured, this, &CaptureThread::SlotFrameCaptured);
    //    connect(&m_FrameTimer, SIGNAL(timeout()), this, SLOT(SlotFrameTimer()));
//    m_FrameTimer.start(1000 / 300);
}

bool CaptureThread::OpenCamera(const QCameraInfo& cameraInfo)
{
    if (m_QCamera != nullptr)
    {
        m_QCamera->stop();
    }
    m_QCamera.reset(new QCamera(cameraInfo));
    //QSharedPointer<QCameraViewFinder>
    m_QCamera->setViewfinder(&m_VideoCapturer);
    

    m_QCamera->start();
    //    m_ImageCapture.reset(new QCameraImageCapture(m_QCamera.data()));

    QList<QCamera::FrameRateRange> frameRateList = m_QCamera->supportedViewfinderFrameRateRanges();

    qreal framerate = 5.0;
    for (int i = 0; i < frameRateList.length(); ++i)
    {
        auto& range = frameRateList[i];
        if (range.maximumFrameRate > framerate) framerate = range.maximumFrameRate;
    }
    if ( framerate > 0.0 )
    {
        QCameraViewfinderSettings viewFinderSettings = m_QCamera->viewfinderSettings();
        viewFinderSettings.setMaximumFrameRate(framerate);
        viewFinderSettings.setMinimumFrameRate(framerate);
        m_QCamera->setViewfinderSettings(viewFinderSettings);
    }
    return true;
}

void CaptureThread::CloseCamera()
{
//    m_ImageCapture.clear();
    m_QCamera.clear();
}

void CaptureThread::SlotImageCaptured(int id, const QImage &preview)
{

}

/*void CaptureThread::SlotFrameTimer()
{
    if (m_CurrentCamera != m_WantedCamera)
    {
        m_CurrentCamera = m_WantedCamera;
        m_Camera.open(m_CurrentCamera);
    }*/

void CaptureThread::SlotFrameCaptured(cv::Mat frame, qint64 captureTime)
{
    QElapsedTimer timer;
    timer.start();

//    qint64 captureTime;
    qint64 processTime;

    cv::Mat grayFrame;
    cv::Mat scaledFrame;

//    captureTime = timer.elapsed();
//    timer.start();
    //        cv::resize(frame, scaledFrame, cv::Size(160, 120));
    //        frame = scaledFrame;

//    cv::medianBlur(frame, frame, 5/*cv::Size(5, 5)*/);
//    cv::cvtColor(frame, frame, CV_BGRA2RGBA);
    cv::cvtColor(frame, grayFrame, CV_RGBA2GRAY);
    cv::blur(grayFrame, grayFrame, cv::Size(10, 10));
    //cv::medianBlur(grayFrame, grayFrame, 5);

    if (CameraSettings::GetInstance()->IsEdgeDetectEnabled())
    {
        QColor color = CameraSettings::GetInstance()->GetEdgeDetectColor();

        cv::Canny(grayFrame, grayFrame, CameraSettings::GetInstance()->GetEdgeDetectMinThres(), CameraSettings::GetInstance()->GetEdgeDetectMaxThres(), 3, true);

        scaledFrame = cv::Mat(grayFrame.size[0], grayFrame.size[1], CV_8UC4, cv::Scalar(color.blue(), color.green(), color.red()));
        cv::bitwise_not(grayFrame, grayFrame);
        frame.copyTo(scaledFrame, grayFrame);
        frame = scaledFrame;
    }

    emit SignalFrameReady(frame, captureTime, timer.elapsed());
}

bool VideoCapturer::present(const QVideoFrame &inFrame)
{
    QElapsedTimer timer;
    timer.start();
    QVideoFrame frame(inFrame);
    if (frame.map(QAbstractVideoBuffer::ReadOnly))
    {
        const uchar* data = frame.bits();
        auto bytesPerLine = frame.bytesPerLine();

        QVideoFrame::PixelFormat pixelFormat = frame.pixelFormat();
        cv::Mat cvFrame(cv::Size(frame.width(), frame.height()), (pixelFormat == QVideoFrame::Format_BGR24) ? CV_8UC3 : CV_8UC4, (void*)data);
        cvFrame = cvFrame.clone().reshape((pixelFormat == QVideoFrame::Format_BGR24) ? 3 : 4, frame.height());
        frame.unmap();

        if (pixelFormat == QVideoFrame::Format_RGB24) {
            cv::cvtColor(cvFrame, cvFrame, CV_RGB2RGBA);
        }
        emit SignalFrameCaptured(cvFrame, timer.elapsed());
    }
    return true;
}
