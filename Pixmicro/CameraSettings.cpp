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
#include "CameraSettings.h"

CameraSettings* CameraSettings::s_Instance;

CameraSettings::CameraSettings() : QObject()
{
    assert(s_Instance == NULL);
    s_Instance = this;

    m_SelectedCamera = QCameraInfo::defaultCamera();

    QSettings prefs;

    prefs.beginGroup("Camera");
    QString camDevice = prefs.value("device", "").toString();
    QCamera::Position camPos = prefs.value("position", QCamera::UnspecifiedPosition).value<QCamera::Position>();
    m_IsHInverted = prefs.value("invertH", false).toBool();
    m_IsVInverted = prefs.value("invertV", false).toBool();

    if (camDevice != "")
    {
        const auto& cameraList = QCameraInfo::availableCameras();
        auto cameraCount = cameraList.count();
        for (int i = 0; i < cameraCount; ++i)
        {
            auto& camera = cameraList[i];
            if (camera.deviceName() == camDevice && camera.position() == camPos)
            {
                m_SelectedCamera = camera;
                break;
            }
        }
    }
    prefs.endGroup();
    m_IsEdgeDetectEnabled = prefs.value("Filters/EdgeDetect/enabled", false).toBool();
    m_EdgeDetectMinThres  = prefs.value("Filters/EdgeDetect/min_threshold", 100.0).toDouble();
    m_EdgeDetectMaxThres  = prefs.value("Filters/EdgeDetect/max_threshold", 200.0).toDouble();
    m_EdgeDetectColor     = prefs.value("Filters/EdgeDetect/color", QColor(Qt::white)).value<QColor>();
}

CameraSettings::~CameraSettings()
{

}

void CameraSettings::SelectCamera(const QCameraInfo& cameraInfo)
{
    if (cameraInfo != m_SelectedCamera)
    {
        m_SelectedCamera = cameraInfo;

        QSettings prefs;
        prefs.beginGroup("Camera");
        prefs.setValue("device", cameraInfo.deviceName());
        prefs.setValue("position", cameraInfo.position());
        prefs.endGroup();

        SignalSelectedCameraChanged(m_SelectedCamera);
    }
}

QCameraInfo CameraSettings::GetSelectedCamera() const
{
    return m_SelectedCamera;
}

int CameraSettings::GetSelectedCameraIndex() const
{
    const auto& cameraList = QCameraInfo::availableCameras();
    auto cameraCount = cameraList.count();
    for (int i = 0; i < cameraCount; ++i)
    {
        auto& camera = cameraList[i];
        if (camera == m_SelectedCamera)
        {
            return i;
        }
    }
    return -1;
}

void CameraSettings::SetInvertH(bool invert)
{
    if (invert != m_IsHInverted)
    {
        m_IsHInverted = invert;
        QSettings().setValue("Camera/invertH", invert);
        emit SignalFlipHToggled(invert);
    }
}

void CameraSettings::SetInvertV(bool invert)
{
    if (invert != m_IsVInverted)
    {
        m_IsVInverted = invert;
        QSettings().setValue("Camera/invertV", invert);
        emit SignalFlipVToggled(invert);
    }
}

void CameraSettings::EnableEdgeDetect(bool enable)
{
    m_IsEdgeDetectEnabled = enable;
    QSettings().setValue("Filters/EdgeDetect/enabled", enable);
}

void CameraSettings::SetEdgeDetectMinThres(double value)
{
    m_EdgeDetectMinThres = value;
    QSettings().setValue("Filters/EdgeDetect/min_threshold", value);
}

void CameraSettings::SetEdgeDetectMaxThres(double value)
{
    m_EdgeDetectMaxThres = value;
    QSettings().setValue("Filters/EdgeDetect/max_threshold", value);
}

void CameraSettings::SetEdgeDetectColor(QColor color)
{
    m_EdgeDetectColor = color;
}

void CameraSettings::SaveEdgeDetectColorToPrefs()
{
    QSettings().setValue("Filters/EdgeDetect/color", m_EdgeDetectColor);
}
