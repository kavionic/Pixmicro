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

#ifndef VISUALSETTINGS_H
#define VISUALSETTINGS_H

#include <QObject>

class VisualSettings : public QObject
{
    Q_OBJECT
public:
    VisualSettings(QObject* parent);
    ~VisualSettings();

    const VisualSettings& Get() { return *s_Instance; }

    static QColor s_CrosshairColor;
    static QColor s_CrosshairSelectedColor;
    static QColor s_CrosshairInnerCircleNormalColor;
    static QColor s_CrosshairInnerCircleHoleDetectedColor;
    static QColor s_CrosshairInnerCircleHoleNotDetectedColor;
    static QColor s_CrosshairInnerCircleSelectedColor;
    static QColor s_CrosshairOuterCircleColor;
    static QColor s_CrosshairOuterCircleSelectedColor;
    static QColor s_MeasureStrokeColor;
    static QColor s_MeasureStrokeSelectedColor;
    static QColor s_MeasureFillColor;
    static QColor s_MeasureSegmentMarkerColor;
    static QColor s_MeasureEndMarkerColor;
    static QColor s_MeasureLabelColor;

    static double s_MeasureSegmentMarkerLength;
    static double s_MeasureEndMarkerLength;

    static double s_SelectionMaxDistance;


private:
    static VisualSettings* s_Instance;
};

#endif // VISUALSETTINGS_H
