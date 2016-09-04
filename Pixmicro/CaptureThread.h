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
    void SignalFrameCaptured(const QVideoFrame &frame, qint64 processingTime);
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

    void Start(bool doRun);
    bool IsStreaming() const { return m_IsStreaming; }

signals:
    void SignalFrameReady(const QVideoFrame &frame, qint64 captureTime, qint64 processTime);

private slots:
    void SlotStarted();
    void SlotFrameCaptured(const QVideoFrame &frame, qint64 processingTime);
    void SlotImageCaptured(int id, const QImage &preview);

private:
    QSharedPointer<QCamera> m_Camera;
    VideoCapturer           m_VideoCapturer;

    bool                    m_IsStreaming;
};

