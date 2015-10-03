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
#include "EdgeDetectWnd.h"
#include "CameraSettings.h"

EdgeDetectWnd::EdgeDetectWnd(QWidget *parent) : QDockWidget(parent)
{
    setupUi(this);

    m_EnableCB->setChecked(CameraSettings::GetInstance()->IsEdgeDetectEnabled());
    connect(m_EnableCB, &QCheckBox::stateChanged, this, &EdgeDetectWnd::SlotEnableCBChanged);
    connect(m_SliderEdgeDetectMinThres, &QSlider::valueChanged, this, &EdgeDetectWnd::SlotMinThresValueChanged);
    connect(m_SliderEdgeDetectMaxThres, &QSlider::valueChanged, this, &EdgeDetectWnd::SlotMaxThresValueChanged);
    connect(m_OverlayColorView, &QPushButton::clicked, this, &EdgeDetectWnd::SlotColorButtonClicked);
//    QPalette palette = m_OverlayColorView->palette();
//    palette.setColor(QPalette::Window, Qt::red);
//    QColor color = Qt::red;
//    m_OverlayColorView->setStyleSheet(QString("background-color: %1; selection-background-color: %1;").arg(color.name()));
    m_OverlayColorView->SetColor(CameraSettings::GetInstance()->GetEdgeDetectColor());
}

EdgeDetectWnd::~EdgeDetectWnd()
{

}

void EdgeDetectWnd::SlotEnableCBChanged()
{
    CameraSettings::GetInstance()->EnableEdgeDetect(m_EnableCB->isChecked());
}

void EdgeDetectWnd::SlotMinThresValueChanged(int value)
{
    CameraSettings::GetInstance()->SetEdgeDetectMinThres(double(value));
    if (value > m_SliderEdgeDetectMaxThres->value()) {
        m_SliderEdgeDetectMaxThres->setValue(value);
    }
}

void EdgeDetectWnd::SlotMaxThresValueChanged(int value)
{
    CameraSettings::GetInstance()->SetEdgeDetectMaxThres(double(value));
    if (value < m_SliderEdgeDetectMinThres->value()) {
        m_SliderEdgeDetectMinThres->setValue(value);
    }
}

void EdgeDetectWnd::SlotColorButtonClicked()
{
    QColorDialog dlg;

    QColor oldColor = CameraSettings::GetInstance()->GetEdgeDetectColor();
    connect(&dlg, &QColorDialog::currentColorChanged, this, &EdgeDetectWnd::SlotColorChanged);
    if (dlg.exec() == QDialog::Accepted)
    {
        CameraSettings::GetInstance()->SaveEdgeDetectColorToPrefs();
        m_OverlayColorView->SetColor(dlg.selectedColor());
        CameraSettings::GetInstance()->SetEdgeDetectColor(dlg.selectedColor());
    }
    else
    {
        m_OverlayColorView->SetColor(oldColor);
        CameraSettings::GetInstance()->SetEdgeDetectColor(oldColor);
    }
}

void EdgeDetectWnd::SlotColorChanged(const QColor& color)
{
    m_OverlayColorView->SetColor(color);
    CameraSettings::GetInstance()->SetEdgeDetectColor(color);
}