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
#include "MeasureNode.h"
#include "ValuePrefixes.h"
#include "VideoView.h"
#include "EditCommands.h"
#include "VisualSettings.h"

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

MeasureNode::MeasureNode(VideoView* view, int32_t id, const QString& name, uint32_t measurements, uint32_t displayedMeasurements) : m_View(view), m_ID(id), m_Name(name), m_Measurements(measurements), m_DisplayedMeasurements(displayedMeasurements)
{
    m_IsSelected = false;
    m_IsHovered  = false;
    m_IsDraggingLabel = false;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureNode::CaptureMouse(bool capture)
{
    if (capture) {
        m_View->m_MouseCaptureMeasureNode = sharedFromThis();
    } else {
        if ( m_View->m_MouseCaptureMeasureNode == this ) {
            m_View->m_MouseCaptureMeasureNode.reset();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureNode::DataChanged()
{
    QPainter& painter = m_View->m_Painter;

    double lineHeight = painter.fontMetrics().height();
    m_LabelSize.setX(0.0);
    m_LabelSize.setY(lineHeight);


    if (m_DisplayedMeasurements & e_MeasureLength) {
        m_LabelLength = "L=" + ValuePrefixes::FormatValue(GetLength());
        double width = painter.fontMetrics().width(m_LabelLength);
        if (width > m_LabelSize.x()) m_LabelSize.setX(width);
        m_LabelSize.ry() += lineHeight;
    }
    if (m_DisplayedMeasurements & e_MeasureRadius) {
        m_LabelRadius = "r=" + ValuePrefixes::FormatValue(GetRadius());
        double width = painter.fontMetrics().width(m_LabelRadius);
        if (width > m_LabelSize.x()) m_LabelSize.setX(width);
        m_LabelSize.ry() += lineHeight;
    }
    if (m_DisplayedMeasurements & e_MeasureDiameter) {
        m_LabelDiameter = QChar(0x2300);
        m_LabelDiameter += "=" + ValuePrefixes::FormatValue(GetRadius() * 2.0);
        double width = painter.fontMetrics().width(m_LabelDiameter);
        if (width > m_LabelSize.x()) m_LabelSize.setX(width);
        m_LabelSize.ry() += lineHeight;
    }
    if (m_DisplayedMeasurements & e_MeasureArea) {
        m_LabelArea = "A=" + ValuePrefixes::FormatArea(GetArea());
        double width = painter.fontMetrics().width(m_LabelArea);
        if (width > m_LabelSize.x()) m_LabelSize.setX(width);
        m_LabelSize.ry() += lineHeight;
    }
    if (m_DisplayedMeasurements & e_MeasureAngle) {
        m_LabelAngle = QChar(0x2220);
        m_LabelAngle += "=" + ValuePrefixes::FormatAngle(GetAngle());
        double width = painter.fontMetrics().width(m_LabelAngle);
        if (width > m_LabelSize.x()) m_LabelSize.setX(width);
        m_LabelSize.ry() += lineHeight;
    }

    emit SignalDataChanged();
    m_View->update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureNode::SetBoundingBox(const QRectF& boundingBox)
{
    if (boundingBox != m_BoundingBox)
    {
        m_BoundingBox = boundingBox;
        emit SignalBoundingBoxChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureNode::UpdateLabelSizeAndPos(const PMPoint& size, const PMPoint& pos)
{
    m_LabelPos  = pos;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureNode::DrawLabel(QPainter& painter, const PMPoint& scale)
{
    m_ScaledLabelSize = m_LabelSize / scale;
    QPen pen;
    pen.setCosmetic(true);
    pen.setColor(VisualSettings::s_MeasureLabelColor);
    painter.setPen(pen);

    QTransform prevTransform = painter.transform();

    painter.resetTransform();

    double lineHeight = painter.fontMetrics().height();

    QPointF pos((m_LabelPos + m_LabelOffset) * scale - m_View->m_ImageOffset /** m_View->m_BitmapToViewScale*/); // = GetDefaultLabelPos(painter, scale);

    pos.ry() += painter.fontMetrics().ascent(); // lineHeight;

    painter.drawText(pos, GetName());
    pos.ry() += lineHeight;

    if (m_DisplayedMeasurements & e_MeasureLength) {
        painter.drawText(pos, m_LabelLength);
        pos.ry() += lineHeight;
    }
    if (m_DisplayedMeasurements & e_MeasureRadius) {
        painter.drawText(pos, m_LabelRadius);
        pos.ry() += lineHeight;
    }
    if (m_DisplayedMeasurements & e_MeasureDiameter) {
        painter.drawText(pos, m_LabelDiameter);
        pos.ry() += lineHeight;
    }
    if (m_DisplayedMeasurements & e_MeasureArea) {
        painter.drawText(pos, m_LabelArea);
        pos.ry() += lineHeight;
    }
    if (m_DisplayedMeasurements & e_MeasureAngle) {
        painter.drawText(pos, m_LabelAngle);
        pos.ry() += lineHeight;
    }
    painter.setTransform(prevTransform);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasureNode::HitTestLabel(const PMPoint& pos) const
{
    QRectF labelFrame(m_LabelPos + m_LabelOffset, QSizeF(m_ScaledLabelSize.x(), m_ScaledLabelSize.y()));
    return labelFrame.contains(pos);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasureNode::MouseDown(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale)
{
    if (mouseButton == Qt::LeftButton)
    {
        if (HitTestLabel(pos))
        {
            m_HitPos = pos;
            m_IsDraggingLabel = true;
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasureNode::MouseUp(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale)
{
    if ( mouseButton == Qt::LeftButton && m_IsDraggingLabel )
    {
        m_IsDraggingLabel = false;
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasureNode::MouseMove(const PMPoint& pos, const PMPoint& currentScale)
{
    m_MousePos = pos;
    if (m_IsDraggingLabel)
    {
        PMPoint delta = pos - m_HitPos;
        m_HitPos = pos;
        m_LabelOffset += delta;
        return true;
    }
    return false;
}

