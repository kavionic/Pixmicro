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

#pragma once

class VideoCapturer : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const override
    {
        QList<QVideoFrame::PixelFormat> list;
        list.append(QVideoFrame::Format_RGB32);
        list.append(QVideoFrame::Format_BGR24);
        return list;

    }
    virtual bool present(const QVideoFrame &frame) override;

signals:
    void SignalFrameCaptured(cv::Mat frame, qint64 processingTime);
};

class CaptureThread : public QObject
{
    Q_OBJECT

public:
    CaptureThread();

    virtual ~CaptureThread();
    void SetThread(QThread* thread);

    bool OpenCamera(const QCameraInfo& cameraInfo);
    void CloseCamera();

//    void SetCamera(int index);

signals:
    void SignalFrameReady(cv::Mat frame, qint64 captureTime, qint64 processTime);

private slots:
    void SlotStarted();
//    void SlotFrameTimer();
    void SlotFrameCaptured(cv::Mat frame, qint64 processingTime);
    void SlotImageCaptured(int id, const QImage &preview);

private:

    QTimer m_FrameTimer;
//    QAtomicInt m_WantedCamera;
//    int        m_CurrentCamera;

    QSharedPointer<QCamera>             m_QCamera;
//    QSharedPointer<QCameraImageCapture> m_ImageCapture;
    VideoCapturer                       m_VideoCapturer;
    cv::VideoCapture m_Camera; // open the default camera

};

