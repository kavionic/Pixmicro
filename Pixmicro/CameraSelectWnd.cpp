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
#include "CameraSelectWnd.h"
#include "CameraSettings.h"

CameraSelectWnd::CameraSelectWnd(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi(this);

    const auto& cameraList = QCameraInfo::availableCameras();
    auto cameraCount = cameraList.count();
    for (int i = 0; i < cameraCount; ++i)
    {
        auto& camera = cameraList[i];
        QString desc = camera.description();
        m_CameraComboBox->addItem(desc);
    }

    m_FlipHorizontally->setChecked(CameraSettings::GetInstance()->IsHInverted());
    m_FlipVertically->setChecked(CameraSettings::GetInstance()->IsVInverted());

    connect(m_CameraComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotCameraSelectionChanged(int)));
    connect(CameraSettings::GetInstance(), &CameraSettings::SignalSelectedCameraChanged, this, &CameraSelectWnd::SlotSelectedCameraChanged);

    connect(m_FlipHorizontally, &QCheckBox::toggled, this, &CameraSelectWnd::SlotFlipHorizontallyToggled);
    connect(m_FlipVertically, &QCheckBox::toggled, this, &CameraSelectWnd::SlotFlipVerticallyToggled);
}

CameraSelectWnd::~CameraSelectWnd()
{

}

void CameraSelectWnd::SlotCameraSelectionChanged(int index)
{
    const auto& cameraList = QCameraInfo::availableCameras();
    if (index >= 0 && index < cameraList.length())
    {
        CameraSettings::GetInstance()->SelectCamera(cameraList[index]);
    }
//    m_CaptureWorker.SetCamera(index);
}

void CameraSelectWnd::SlotSelectedCameraChanged(const QCameraInfo& cameraInfo)
{
    const auto& cameraList = QCameraInfo::availableCameras();
    for ( int i = 0; i < cameraList.length(); ++i )
    {
        if (cameraList[i] == cameraInfo)
        {
            m_CameraComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void CameraSelectWnd::SlotFlipHorizontallyToggled(bool checked)
{
    CameraSettings::GetInstance()->SetInvertH(checked);
}

void CameraSelectWnd::SlotFlipVerticallyToggled(bool checked)
{
    CameraSettings::GetInstance()->SetInvertV(checked);
}
