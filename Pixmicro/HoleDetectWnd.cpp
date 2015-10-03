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
#include "HoleDetectWnd.h"

HoleDetectWnd* HoleDetectWnd::s_Instance;

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

HoleDetectWnd::HoleDetectWnd(QWidget *parent) : QDockWidget(parent)
{
    s_Instance = this;
    setupUi(this);

    QSettings prefs;

    prefs.beginGroup("HoleDetect");

    m_EnableCB->setChecked(prefs.value("Enabled", true).toBool());
    m_FreezeHoleColor->setChecked(prefs.value("FreezeCenterColor", false).toBool());

    m_ThresholdSlider->setValue(prefs.value("Threshold", 250).toInt());
    m_RoundnessSlider->setValue(prefs.value("Roundness", 750).toInt());

    m_MarkerColor = Qt::blue;
    m_MarkerColor.setAlpha(80);
    m_MarkerColor = prefs.value("MarkerColor", m_MarkerColor).value<QColor>();
    m_MarkerColorView->SetColor(m_MarkerColor);
    m_MarkerAlphaSlider->setValue(m_MarkerColor.alpha());
    m_MarkerWidthSlider->setValue(prefs.value("MarkerWidth", 6).toInt());

    m_CenterColor = prefs.value("CenterColor", m_CenterColor).value<QColor>();
    m_CenterColorView->SetColor(m_CenterColor);

    prefs.endGroup();

    connect(m_CalibrateButton, &QAbstractButton::clicked, this, &HoleDetectWnd::SignalCalibrateCrosshair);
    connect(m_MarkerColorView, &QPushButton::clicked, this, &HoleDetectWnd::SlotMarkerColorButtonClicked);
    connect(m_EnableCB, &QCheckBox::stateChanged, this, &HoleDetectWnd::SlotEnableCBChanged);
    connect(m_FreezeHoleColor, &QCheckBox::stateChanged, this, &HoleDetectWnd::SlotFreezeHoleColorCBChanged);
    connect(m_MarkerAlphaSlider, &QSlider::valueChanged, this, &HoleDetectWnd::SlotMarkerAlphaValueChanged);
    connect(m_MarkerWidthSlider, &QSlider::valueChanged, this, &HoleDetectWnd::SlotMarkerWidthValueChanged);
    connect(m_ThresholdSlider, &QSlider::valueChanged, this, &HoleDetectWnd::SlotThresholdValueChanged);
    connect(m_RoundnessSlider, &QSlider::valueChanged, this, &HoleDetectWnd::SlotRoundnessValueChanged);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

HoleDetectWnd::~HoleDetectWnd()
{

}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool HoleDetectWnd::IsEnabled() const
{
    return m_EnableCB->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool HoleDetectWnd::IsCenterColorFrozen() const
{
    return m_FreezeHoleColor->isChecked();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

double HoleDetectWnd::GetThreshold() const
{
    double sliderValue = double(m_ThresholdSlider->value()) / double(m_ThresholdSlider->maximum());
    return (sliderValue * sliderValue) * 50000.0;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

double HoleDetectWnd::GetRoundness() const
{
    return double(m_RoundnessSlider->value()) / double(m_RoundnessSlider->maximum());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void HoleDetectWnd::UpdateCenterColor(const QColor& color)
{
    m_CenterColor = color;
    m_CenterColorView->SetColor(m_CenterColor);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void HoleDetectWnd::SlotEnableCBChanged(bool checked)
{
    QSettings().setValue("HoleDetect/Enabled", checked);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void HoleDetectWnd::SlotFreezeHoleColorCBChanged(bool checked)
{
    QSettings().setValue("HoleDetect/FreezeCenterColor", checked);
    if (checked) {
        QSettings().setValue("HoleDetect/CenterColor", m_CenterColor);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void HoleDetectWnd::SlotMarkerAlphaValueChanged(int value)
{
    m_MarkerColor.setAlpha(value);
    QSettings().setValue("HoleDetect/MarkerColor", m_MarkerColor);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void HoleDetectWnd::SlotMarkerWidthValueChanged(int value)
{
    QSettings().setValue("HoleDetect/MarkerWidth", value);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void HoleDetectWnd::SlotThresholdValueChanged(int value)
{
    QSettings().setValue("HoleDetect/Threshold", value);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void HoleDetectWnd::SlotRoundnessValueChanged(int value)
{
    QSettings().setValue("HoleDetect/Roundness", value);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void HoleDetectWnd::SlotMarkerColorButtonClicked()
{
    m_MarkerColorDialog.reset(new QColorDialog(), &QObject::deleteLater);
    m_PrevMarkerColor = m_MarkerColor;
    connect(m_MarkerColorDialog.data(), &QColorDialog::currentColorChanged, this, &HoleDetectWnd::SlotMarkerColorChanged);
    connect(m_MarkerColorDialog.data(), &QColorDialog::finished, this, &HoleDetectWnd::SlotMarkerColorFinished);
    m_MarkerColorDialog->open();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void HoleDetectWnd::SlotMarkerColorFinished(int result)
{
    if (result == QDialog::Accepted)
    {
        m_MarkerColor = m_MarkerColorDialog->selectedColor();
        m_MarkerColor.setAlpha(m_MarkerAlphaSlider->value());
        m_MarkerColorView->SetColor(m_MarkerColor);

        QSettings().setValue("HoleDetect/MarkerColor", m_MarkerColor);
    }
    else
    {
        m_MarkerColorView->SetColor(m_PrevMarkerColor);
        m_MarkerColor = m_PrevMarkerColor;
        m_MarkerColor.setAlpha(m_MarkerAlphaSlider->value());
    }
    m_MarkerColorDialog.reset();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void HoleDetectWnd::SlotMarkerColorChanged(const QColor& color)
{
    m_MarkerColor = color;
    m_MarkerColorView->SetColor(color);
    m_MarkerColor.setAlpha(m_MarkerAlphaSlider->value());
}
