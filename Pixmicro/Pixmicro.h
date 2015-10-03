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

#ifndef PIXMICRO_H
#define PIXMICRO_H

#include <QtWidgets/QMainWindow>
#include "ui_Pixmicro.h"

#include "CaptureThread.h"

class HoleDetectWnd;

class Pixmicro : public QMainWindow, private Ui::PixmicroClass
{
    Q_OBJECT

public:
    Pixmicro(QWidget *parent = 0);
    ~Pixmicro();

    void closeEvent(QCloseEvent* event);
    void StartCapture();


private slots:
    void SlotCameraMenuOpening();
    void SlotCameraMenuTriggered(QAction* action);
    void SlotSelectedCameraChanged(const QCameraInfo& cameraInfo);
    void SlotCalibrateCrosshair();
    void SlotFrameReady(cv::Mat image, qint64 captureTime, qint64 processTime);

private:

    CaptureThread m_CaptureWorker;
    QThread       m_CaptureThread;
    QElapsedTimer timer;

    HoleDetectWnd* m_HoleDetectWnd;
};

Q_DECLARE_METATYPE(cv::Mat);

#endif // PIXMICRO_H
