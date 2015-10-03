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

#ifndef CAMERASETTINGS_H
#define CAMERASETTINGS_H

#include <QObject>

class CameraSettings : public QObject
{
        Q_OBJECT

public:
    CameraSettings();
    ~CameraSettings();

    static CameraSettings* GetInstance() { return s_Instance; }

    void SelectCamera(const QCameraInfo& cameraInfo);
    QCameraInfo GetSelectedCamera() const;
    int GetSelectedCameraIndex() const;

    void SetInvertH(bool invert);
    bool IsHInverted() const { return m_IsHInverted;  }

    void SetInvertV(bool invert);
    bool IsVInverted() const { return m_IsVInverted; }

    void   EnableEdgeDetect(bool enable);
    bool   IsEdgeDetectEnabled() const { return m_IsEdgeDetectEnabled;  }
    void   SetEdgeDetectMinThres(double value);
    double GetEdgeDetectMinThres() const { return m_EdgeDetectMinThres; }

    void   SetEdgeDetectMaxThres(double value);
    double GetEdgeDetectMaxThres() const { return m_EdgeDetectMaxThres; }

    void   SetEdgeDetectColor(QColor color);
    QColor GetEdgeDetectColor() const { return m_EdgeDetectColor; }
    void   SaveEdgeDetectColorToPrefs();

signals:
    void SignalSelectedCameraChanged(const QCameraInfo& cameraInfo);
    void SignalFlipHToggled(bool invert);
    void SignalFlipVToggled(bool invert);
private:
    static CameraSettings* s_Instance;
    QCameraInfo m_SelectedCamera;
    bool   m_IsHInverted;
    bool   m_IsVInverted;

    bool   m_IsEdgeDetectEnabled;
    double m_EdgeDetectMinThres;
    double m_EdgeDetectMaxThres;
    QColor m_EdgeDetectColor;
};

#endif // CAMERASETTINGS_H
