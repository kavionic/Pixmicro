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

#ifndef HOLEDETECTWND_H
#define HOLEDETECTWND_H

#include <QWidget>
#include "ui_HoleDetectWnd.h"

class HoleDetectWnd : public QDockWidget, public Ui::HoleDetectWnd
{
    Q_OBJECT

signals:
    void SignalCalibrateCrosshair();
    void SignalEnableChanged(bool enabled);
    void SignalFreezeHoleColorChanged(bool freeze);
    void SignalThresholdChanged(double value);
    void SignalRoundnessChanged(double value);
    void SignalMarkerColorChanged(const QColor& color);
    void SignalMarkerWidthChanged(int value);


public:
    HoleDetectWnd(QWidget *parent = 0);
    ~HoleDetectWnd();

    bool   IsEnabled() const;
    bool   IsCenterColorFrozen() const;
    double GetThreshold() const;
    double GetRoundness() const;

    int GetMarkerThickness() const { return m_MarkerWidthSlider->value(); }
    QColor GetMarkerColor() const { return m_MarkerColor;  }

    void UpdateCenterColor(const QColor& color);
    QColor GetCenterColor() const { return m_CenterColor; }
private:
    void SlotEnableCBChanged(bool checked);
    void SlotFreezeHoleColorCBChanged(bool checked);
    void SlotMarkerAlphaValueChanged(int value);
    void SlotMarkerWidthValueChanged(int value);
    void SlotThresholdValueChanged(int value);
    void SlotRoundnessValueChanged(int value);

    void SlotMarkerColorButtonClicked();
    void SlotMarkerColorFinished(int result);
    void SlotMarkerColorChanged(const QColor& color);

    QColor         m_CenterColor;
    QColor         m_PrevMarkerColor;
    QColor         m_MarkerColor;

    QSharedPointer<QColorDialog> m_MarkerColorDialog;
};

#endif // HOLEDETECTWND_H
