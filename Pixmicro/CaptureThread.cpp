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

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

CaptureThread::CaptureThread()
{
    m_IsStreaming = true;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

CaptureThread::~CaptureThread()
{
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void CaptureThread::SetThread( QThread* thread )
{
    moveToThread(thread);

    connect(thread, SIGNAL(started()), this, SLOT(SlotStarted()));
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void CaptureThread::SlotStarted()
{
    connect(&m_VideoCapturer, &VideoCapturer::SignalFrameCaptured, this, &CaptureThread::SlotFrameCaptured);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool CaptureThread::OpenCamera(const QCameraInfo& cameraInfo)
{
    if (m_Camera != nullptr && m_IsStreaming)
    {
        m_Camera->stop();
    }
    m_Camera.reset(new QCamera(cameraInfo));
    //QSharedPointer<QCameraViewFinder>

    m_Camera->setViewfinder(&m_VideoCapturer);
    

    if (m_IsStreaming) {
        m_Camera->start();
    }
    QCameraViewfinderSettings settings; // = m_Camera->viewfinderSettings();
    QList<QSize> resolutions = m_Camera->supportedViewfinderResolutions();
//    settings.setResolution(resolutions[9/*resolutions.size()-2*/]);
    QList<QCamera::FrameRateRange> framerates = m_Camera->supportedViewfinderFrameRateRanges(settings);
    //    settings.setMaximumFrameRate(5);
//    settings.setMinimumFrameRate(5);
    
//    settings.setMinimumFrameRate(framerates[framerates.size()-1].minimumFrameRate);
//    settings.setMaximumFrameRate(framerates[framerates.size()-1].maximumFrameRate);
//    m_Camera->setViewfinderSettings(settings);
    //    m_ImageCapture.reset(new QCameraImageCapture(m_Camera.data()));

/*    QList<QCamera::FrameRateRange> frameRateList = m_Camera->supportedViewfinderFrameRateRanges();

    qreal framerate = 5.0;
    for (int i = 0; i < frameRateList.length(); ++i)
    {
        auto& range = frameRateList[i];
        if (range.maximumFrameRate > framerate) framerate = range.maximumFrameRate;
    }
    if ( framerate > 0.0 )
    {
        QCameraViewfinderSettings viewFinderSettings = m_Camera->viewfinderSettings();
        viewFinderSettings.setMaximumFrameRate(framerate);
        viewFinderSettings.setMinimumFrameRate(framerate);
        m_Camera->setViewfinderSettings(viewFinderSettings);
    }*/
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void CaptureThread::CloseCamera()
{
//    m_ImageCapture.clear();
    m_Camera.clear();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void CaptureThread::Start(bool doRun)
{
    if (doRun != m_IsStreaming)
    {
        if (!doRun) {
            m_Camera->stop();
        }
        else {
            m_Camera->start();
        }
        m_IsStreaming = doRun;
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void CaptureThread::SlotImageCaptured(int id, const QImage &preview)
{

}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void CaptureThread::SlotFrameCaptured(const QVideoFrame &frame, qint64 captureTime)
{
    emit SignalFrameReady(frame, captureTime, 0);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool VideoCapturer::present(const QVideoFrame &inFrame)
{
    emit SignalFrameCaptured(inFrame, 0);
    return true;
}
