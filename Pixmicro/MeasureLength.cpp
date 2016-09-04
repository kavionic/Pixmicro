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
#include "MeasureLength.h"
#include "EditCommands.h"
#include "VideoView.h"
#include "VisualSettings.h"
#include "ValuePrefixes.h"

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureLength::Create(VideoView* view, int id, const QString& name, const PMPoint& startPos)
{
    QSharedPointer<MeasureNode> node(new MeasureLength(view, id, name, startPos));
    QUndoCommand* cmd = new ECmdAddMeasureNode(view, node);
    view->GetUndoStack()->push(cmd);
    node->CaptureMouse(true);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

MeasureLength::MeasureLength(VideoView* view, int32_t id, const QString& name, const PMPoint& start) : MeasurePointList(view, id, name, e_MeasureLength, start, false)
{
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

/*PMPoint MeasureArea::GetDefaultLabelPos(QPainter& painter, const PMPoint& scale) const
{
    PMPoint labelSize = GetLabelSize(painter) / scale;
    return PMPoint(m_BoundingBox.left() + (m_BoundingBox.width() - labelSize.x()) * 0.5, m_BoundingBox.top() - labelSize.y() - 5.0 / scale.y()) * scale;
}*/

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureLength::Paint(QPainter& painter, const PMPoint& scale)
{
    QPen pen;
    pen.setCosmetic(true);

    PMPoint startCapDirection;
    PMPoint prevDirection;

    for (size_t i = 0 ; i < m_MeasurePoints.size() - 1 ; ++i)
    {
        const bool isStart = i == 0;
        const bool isEnd   = i == m_MeasurePoints.size() - 2;
        pen.setStyle(Qt::SolidLine);
        pen.setColor((IsSelected() || IsHovered()) ? VisualSettings::s_MeasureStrokeSelectedColor : VisualSettings::s_MeasureStrokeColor);
        pen.setWidth(1);
        painter.setPen(pen);

        PMPoint start = m_MeasurePoints[i];
        PMPoint end   = m_MeasurePoints[i+1];

        PMPoint direction(end - start);
        if ( direction.manhattanLength() == 0.0 ) return;
        NormalizePoint(direction);
        PMPoint jointNormal;
        if ( !isStart ) {
            jointNormal = GetNormalizedPoint(direction + prevDirection);
        } else {
            jointNormal = direction;
            startCapDirection = PMPoint(direction.y(), -direction.x());
        }
        prevDirection = direction;

        PMPoint jointDirection(jointNormal.y(), -jointNormal.x());


        painter.drawLine(start, end);

        float endCapLength = (m_MeasurePoints.size() == 2 && IsHovered()) ? 100000.0 : (VisualSettings::s_MeasureEndMarkerLength * 0.5 / scale.x());
        pen.setStyle((m_MeasurePoints.size() == 2 && IsHovered()) ? Qt::DashDotLine : Qt::SolidLine);
        pen.setColor((m_MeasurePoints.size() == 2 && IsHovered()) ? VisualSettings::s_MeasureStrokeSelectedColor : VisualSettings::s_MeasureEndMarkerColor);

        painter.setPen(pen);


        painter.drawLine(start + jointDirection * endCapLength, start - jointDirection * endCapLength);
        if (isEnd)
        {
            PMPoint endCapDirection(direction.y(), -direction.x());
            float endCapLength = (m_MeasurePoints.size() == 2 && IsHovered()) ? 100000.0 : (VisualSettings::s_MeasureEndMarkerLength * 0.5 / scale.x());
            pen.setStyle((m_MeasurePoints.size() == 2 && IsHovered()) ? Qt::DashDotLine : Qt::SolidLine);
            pen.setColor((m_MeasurePoints.size() == 2 && IsHovered()) ? VisualSettings::s_MeasureStrokeSelectedColor : VisualSettings::s_MeasureEndMarkerColor);
            painter.setPen(pen);
            painter.drawLine(end + endCapDirection * endCapLength, end - endCapDirection * endCapLength);
        }
    }

    if ( m_MousePoint != -1 ) {
        const PMPoint& pos = m_MeasurePoints[m_MousePoint];
        double size = 6.0 / scale.x();
        pen.setStyle(Qt::SolidLine);
        pen.setColor(VisualSettings::s_MeasureStrokeSelectedColor);
        painter.setPen(pen);
        painter.drawRect(QRectF(pos.x() - size * 0.5, pos.y() - size * 0.5, size, size));
    }

    PMPoint labelSize = GetLabelSize(painter) / scale;

    float textPos = 0.25;
    float textDist = 20;
    double labelWidth = std::max(labelSize.x(), labelSize.y());
    PMPoint textPoint = (m_MeasurePoints[0] * (1.0f - textPos) + m_MeasurePoints[1] * textPos + startCapDirection * (labelWidth*0.5 + 5.0 / scale.x()));
    textPoint.setX(textPoint.x() - labelSize.x() * 0.5);
    textPoint.setY(textPoint.y() - labelSize.y() * 0.5);
    UpdateLabelSizeAndPos(labelSize, textPoint);
    DrawLabel(painter, scale);
}
