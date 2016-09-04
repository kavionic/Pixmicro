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

#include "MeasureArea.h"
#include "EditCommands.h"
#include "VideoView.h"
#include "VisualSettings.h"
#include "ValuePrefixes.h"

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureArea::Create(VideoView* view, int id, const QString& name, const PMPoint& startPos)
{
    QSharedPointer<MeasureNode> node(new MeasureArea(view, id, name, startPos));
    QUndoCommand* cmd = new ECmdAddMeasureNode(view, node);
    view->GetUndoStack()->push(cmd);
    node->CaptureMouse(true);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

MeasureArea::MeasureArea(VideoView* view, int32_t id, const QString& name, const PMPoint& start) : MeasurePointList(view, id, name, e_MeasureArea, start, true)
{
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureArea::DataChanged()
{
    MeasurePointList::DataChanged();

    m_PolyConverter = QPainterPath();
    //    m_PolyConverter.setFillRule(Qt::WindingFill);
    m_PolyConverter.addPolygon(m_MeasurePoints);
    m_Area = 0.0;
    for (auto& i : m_PolyConverter.simplified().toSubpathPolygons())
    {
        m_Area += GetPolyArea(i);
    }

}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

double MeasureArea::GetArea() const
{
    return m_Area;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasureArea::Intersects(const QPointF& point) const
{
    return GetBoundingBox().contains(point) && m_PolyConverter.contains(point);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasureArea::Intersects(const QRectF& frame) const
{
    return GetBoundingBox().intersects(frame) && m_PolyConverter.intersects(frame);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

double MeasureArea::GetDistance() const
{
    if ( m_PolyConverter.contains(m_MousePos)) {
        return 0.0;
    }
/*    for ( const auto& i : m_PolyList ) {
        if (i.containsPoint(m_MousePos, Qt::OddEvenFill)) {
            return 0.0;
        }
    }*/
    return MeasurePointList::GetDistance();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

double NormalizeAngle(double angle)
{
    if (angle < -M_PI) {
        return angle + M_PI*2.0;
    } else if (angle > M_PI) {
        return angle - M_PI*2.0;
    } else {
        return angle;
    }

}


///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureArea::Paint(QPainter& painter, const PMPoint& scale)
{
/*    QPolygonF polygon(m_MeasurePoints);

    for (size_t i = 0 ; i < m_MeasurePoints.size() ; ++i)
    {
        polygon2.append(m_MeasurePoints[i]);
    }*/


    QPen pen;
    QBrush brush(Qt::SolidPattern);
    brush.setColor(VisualSettings::s_MeasureFillColor);

    pen.setStyle(Qt::SolidLine);
    pen.setColor((IsSelected() || IsHovered()) ? VisualSettings::s_MeasureStrokeSelectedColor : VisualSettings::s_MeasureStrokeColor);
    pen.setWidth(1);
    pen.setCosmetic(true);
    painter.setPen(pen);

    painter.setBrush(brush);

    QColor colors[] ={ Qt::green,
        Qt::blue,
        Qt::cyan,
        Qt::magenta,
        Qt::yellow,
        Qt::darkRed,
        Qt::darkGreen,
        Qt::darkBlue,
        Qt::darkCyan,
        Qt::darkMagenta,
        Qt::darkYellow,
    };

    painter.drawPolygon(m_MeasurePoints/*, Qt::WindingFill*/);
    if (IsHovered())
    {
        pen.setStyle(Qt::SolidLine);
        pen.setColor(VisualSettings::s_MeasureStrokeSelectedColor);
        painter.setPen(pen);

        brush.setColor(VisualSettings::s_MeasureStrokeSelectedColor);
        painter.setBrush(brush);

        double size = 5.0 / scale.x();

        for (size_t i = 0 ; i < m_MeasurePoints.size() ; ++i)
        {
            const PMPoint& pos = m_MeasurePoints[i];
            if (i == m_MousePoint)
            {
                painter.setBrush(QBrush());
                painter.drawRect(QRectF(pos.x() - size * 0.5, pos.y() - size * 0.5, size, size));
                painter.setBrush(brush);
            }
            else
            {
                painter.drawEllipse(QRectF(pos.x() - size * 0.5, pos.y() - size * 0.5, size, size));
            }
        }
    }

    const QRectF& boundingBox = GetBoundingBox();
    PMPoint labelSize = GetLabelSize(painter) / scale;
    PMPoint labelPos = PMPoint(boundingBox.left() + (boundingBox.width() - labelSize.x()) * 0.5, boundingBox.top() - labelSize.y() - 5.0 / scale.y());
    UpdateLabelSizeAndPos(labelSize, labelPos);
    DrawLabel(painter, scale);
}
