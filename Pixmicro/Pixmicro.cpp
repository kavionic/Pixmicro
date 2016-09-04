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
#include "GalleryWnd.h"
#include "MeasureWnd.h"
#include "ValuePrefixes.h"
#include "CameraSettings.h"

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

Pixmicro::Pixmicro(QWidget *parent) : QMainWindow(parent)
{
//    qRegisterMetaType<cv::Mat>();
    setupUi(this);

    CreateDockWindows();
    CreateToolbars();

    connect(&m_CaptureWorker, &CaptureThread::SignalFrameReady, this, &Pixmicro::SlotFrameReady);


    m_CaptureWorker.SetThread(&m_CaptureThread);

    StartCapture();
    connect(CameraSettings::GetInstance(), &CameraSettings::SignalSelectedCameraChanged, this, &Pixmicro::SlotSelectedCameraChanged);
    m_CaptureWorker.OpenCamera(CameraSettings::GetInstance()->GetSelectedCamera());


    QUndoStack* undoStack = m_VideoView->GetUndoStack();
    QAction* undoAction = undoStack->createUndoAction(this);
    QAction* redoAction = undoStack->createRedoAction(this);
    
    undoAction->setShortcut(QKeySequence::Undo);
    redoAction->setShortcut(QKeySequence::Redo);

    m_MenuEdit->addAction(undoAction);
    m_MenuEdit->addAction(redoAction);

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

    connect(qApp, &QGuiApplication::applicationStateChanged, this, &Pixmicro::SlotApplicationStateChanged);
    UpdateScreensaverSetting();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::CreateDockWindows()
{
    QDockWidget* dockWnd;

    dockWnd = new CameraSelectWnd();
    addDockWidget(Qt::RightDockWidgetArea, dockWnd);
    m_MenuWindows->addAction(dockWnd->toggleViewAction());

    m_HoleDetectWnd = new HoleDetectWnd();
    addDockWidget(Qt::RightDockWidgetArea, m_HoleDetectWnd);
    m_MenuWindows->addAction(m_HoleDetectWnd->toggleViewAction());
    connect(m_HoleDetectWnd, &HoleDetectWnd::SignalCalibrateCrosshair, this, &Pixmicro::SlotCalibrateCrosshair);

    m_GalleryWnd = new GalleryWnd();
    m_GalleryWnd->setVisible(true);
    addDockWidget(Qt::RightDockWidgetArea, m_GalleryWnd);
    m_MenuWindows->addAction(m_GalleryWnd->toggleViewAction());
    connect(m_GalleryWnd, &GalleryWnd::SignalMakeSnapshot, this, &Pixmicro::SlotTakeSnapshot);
    connect(m_GalleryWnd, &GalleryWnd::SignalSelectionChanged, this, &Pixmicro::SlotGallerySelectionChanged);

    m_MeasureWnd = new MeasureWnd();
    m_MeasureWnd->setVisible(true);
    addDockWidget(Qt::RightDockWidgetArea, m_MeasureWnd);
    m_MenuWindows->addAction(m_MeasureWnd->toggleViewAction());

    dockWnd = new EdgeDetectWnd();
    dockWnd->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, dockWnd);
    m_MenuWindows->addAction(dockWnd->toggleViewAction());

    m_VideoView->SetHoleDetectWnd(m_HoleDetectWnd);
    m_VideoView->SetMeasureWnd(m_MeasureWnd);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::CreateToolbars()
{
    m_VideoView->AddToolbars(this);
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

    m_VideoView->SaveSettings();
    m_GalleryWnd->SaveSettings();
    m_MeasureWnd->SaveSettings();
    ValuePrefixes::Get().SaveSettings();

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

void Pixmicro::SlotTakeSnapshot(bool* result, const QString& path, double scale, bool includeOverlay)
{
    *result = m_VideoView->TakeSnapshot(path, scale, includeOverlay);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::SlotGallerySelectionChanged(const QString& path)
{
    if (path.isEmpty()) {
        m_CaptureWorker.Start(true);
    } else {
        m_CaptureWorker.Start(false);

        QImage image;
        image.load(path);
        m_VideoView->Update(&image, false);
        m_StatusBar->showMessage(path);

    }
    UpdateScreensaverSetting();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::UpdateScreensaverSetting()
{
    if (m_CaptureWorker.IsStreaming() && qApp->applicationState() == Qt::ApplicationActive)
    {
#ifdef Q_OS_WIN32
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
#endif
    }
    else
    {
#ifdef Q_OS_WIN32
        SetThreadExecutionState(ES_CONTINUOUS);
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void Pixmicro::SlotApplicationStateChanged(Qt::ApplicationState state)
{
    UpdateScreensaverSetting();
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

void Pixmicro::SlotFrameReady(const QVideoFrame &inFrame, qint64 captureTime, qint64 processTime)
{
    QVideoFrame frame(inFrame);
    if (m_CaptureWorker.IsStreaming()) // Ignore updates that was lingering in the queue when capturing was stopped
    {
        if (frame.map(QAbstractVideoBuffer::ReadOnly))
        {
            QImage img(frame.bits(), frame.width(), frame.height(), QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()));
            m_VideoView->Update(&img, true);
            frame.unmap();

            qint64 elapsed = timer.elapsed();
            timer.start();
            QString statusText;
            statusText.sprintf("Scale time: %d (%.1f), %d, %d", elapsed, 1000.0f / double(elapsed), captureTime, processTime);

            m_StatusBar->showMessage(statusText);
        }
    }
}

