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
#include "VisualSettings.h"

VisualSettings* VisualSettings::s_Instance;

QColor VisualSettings::s_CrosshairColor(Qt::green);
QColor VisualSettings::s_CrosshairSelectedColor(Qt::white);
QColor VisualSettings::s_CrosshairInnerCircleNormalColor(Qt::green);
QColor VisualSettings::s_CrosshairInnerCircleHoleDetectedColor(Qt::green);
QColor VisualSettings::s_CrosshairInnerCircleHoleNotDetectedColor(Qt::red);
QColor VisualSettings::s_CrosshairInnerCircleSelectedColor(Qt::white);
QColor VisualSettings::s_CrosshairOuterCircleColor(Qt::green);
QColor VisualSettings::s_CrosshairOuterCircleSelectedColor(Qt::white);
QColor VisualSettings::s_MeasureStrokeColor(Qt::yellow);
QColor VisualSettings::s_MeasureStrokeSelectedColor(Qt::white);
QColor VisualSettings::s_MeasureFillColor(0x32, 0x9b, 0xff, 0x33);
QColor VisualSettings::s_MeasureSegmentMarkerColor(Qt::white);
QColor VisualSettings::s_MeasureEndMarkerColor(Qt::red);
QColor VisualSettings::s_MeasureLabelColor(Qt::white);

double VisualSettings::s_MeasureSegmentMarkerLength = 8.0;
double VisualSettings::s_MeasureEndMarkerLength = 10.0;
double VisualSettings::s_SelectionMaxDistance = 5.0;


VisualSettings::VisualSettings(QObject *parent) : QObject(parent)
{

}

VisualSettings::~VisualSettings()
{

}
