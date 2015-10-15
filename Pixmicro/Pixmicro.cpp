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
#include "Pixmicro.h"
#include "CameraSelectWnd.h"
#include "EdgeDetectWnd.h"
#include "HoleDetectWnd.h"
#include "CameraSettings.h"

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

Pixmicro::Pixmicro(QWidget *parent) : QMainWindow(parent)
{
    qRegisterMetaType<cv::Mat>();
    setupUi(this);

    QDockWidget* dockWnd;

    dockWnd = new CameraSelectWnd();
    addDockWidget(Qt::RightDockWidgetArea, dockWnd);
    m_MenuWindows->addAction(dockWnd->toggleViewAction());

    m_HoleDetectWnd = new HoleDetectWnd();
    addDockWidget(Qt::RightDockWidgetArea, m_HoleDetectWnd);
    m_MenuWindows->addAction(m_HoleDetectWnd->toggleViewAction());
    connect(m_HoleDetectWnd, &HoleDetectWnd::SignalCalibrateCrosshair, this, &Pixmicro::SlotCalibrateCrosshair);

    dockWnd = new EdgeDetectWnd();
    dockWnd->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, dockWnd);
    m_MenuWindows->addAction(dockWnd->toggleViewAction());


    connect(&m_CaptureWorker, &CaptureThread::SignalFrameReady, this, &Pixmicro::SlotFrameReady);

    m_CaptureWorker.SetThread(&m_CaptureThread);

    StartCapture();
    connect(CameraSettings::GetInstance(), &CameraSettings::SignalSelectedCameraChanged, this, &Pixmicro::SlotSelectedCameraChanged);
    m_CaptureWorker.OpenCamera(CameraSettings::GetInstance()->GetSelectedCamera());

    const auto& cameraList = QCameraInfo::availableCameras();
    auto cameraCount = cameraList.count();
    for (int i = 0; i < cameraCount; ++i)
    {
        auto& camera = cameraList[i];
        QString desc = camera.description();
        QAction* action = m_MenuCamera->addAction(desc);
        action->setData(i);
        action->setCheckable(true);
        //            printf("%s, %s\n", name.data(), desc.data());
    }
    connect(m_MenuCamera, &QMenu::triggered, this, &Pixmicro::SlotCameraMenuTriggered);
    connect(m_MenuCamera, &QMenu::aboutToShow, this, &Pixmicro::SlotCameraMenuOpening);

    QSettings prefs;

    restoreGeometry(prefs.value("MainWindow/geometry").toByteArray());
    restoreState(prefs.value("MainWindow/windowState").toByteArray());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

Pixmicro::~Pixmicro()
{
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::closeEvent(QCloseEvent* event)
{
    QSettings prefs;

    prefs.setValue("MainWindow/geometry", saveGeometry());
    prefs.setValue("MainWindow/windowState", saveState());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::SlotCalibrateCrosshair()
{
    m_VideoView->CalibrateCrosshair();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::SlotCameraMenuOpening()
{
    const QList<QAction*>& actionList = m_MenuCamera->actions();
    int cameraIndex = CameraSettings::GetInstance()->GetSelectedCameraIndex();
    for (int i = 0; i < actionList.length(); ++i)
    {
        actionList[i]->setChecked(i == cameraIndex);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::SlotCameraMenuTriggered(QAction* action)
{
    int index = action->data().toInt();
    const auto& cameraList = QCameraInfo::availableCameras();
    if (index >= 0 && index < cameraList.length())
    {
        CameraSettings::GetInstance()->SelectCamera(cameraList[index]);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::SlotSelectedCameraChanged(const QCameraInfo& cameraInfo)
{
    m_CaptureWorker.OpenCamera(cameraInfo);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::StartCapture()
{
    m_CaptureThread.start();
    timer.start();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::SlotFrameReady(cv::Mat image, qint64 captureTime, qint64 processTime)
{
//	QImage img((const unsigned char*)(image.data), image.cols, image.rows, QImage::Format_Grayscale8);
    QImage img((const unsigned char*)(image.data), image.cols, image.rows, QImage::Format_RGB32);
    
    m_VideoView->Update(&img, m_HoleDetectWnd->IsEnabled(), m_HoleDetectWnd->IsCenterColorFrozen(), m_HoleDetectWnd->GetThreshold(), m_HoleDetectWnd->GetRoundness());

    qint64 elapsed = timer.elapsed();
    timer.start();
    QString statusText;
    statusText.sprintf("Scale time: %d (%.1f), %d, %d", elapsed, 1000.0f / double(elapsed), captureTime, processTime);

    m_StatusBar->showMessage(statusText);
}

