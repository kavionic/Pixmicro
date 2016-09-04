#include "stdafx.h"
#include "MeasureCircle.h"

#include "VideoView.h"
#include "EditCommands.h"
#include "VisualSettings.h"

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureCircle::Create(VideoView* view, int id, const QString& name, const PMPoint& startPos, bool edgeMode)
{
    QSharedPointer<MeasureNode> node(new MeasureCircle(view, id, name, startPos, edgeMode));
    QUndoCommand* cmd = new ECmdAddMeasureNode(view, node);
    view->GetUndoStack()->push(cmd);
    node->CaptureMouse(true);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

MeasureCircle::MeasureCircle(VideoView* view, int32_t id, const QString& name, const PMPoint& start, bool edgeMode) : MeasureNode(view, id, name, e_MeasureRadius | e_MeasureDiameter | e_MeasureArea, e_MeasureDiameter | e_MeasureArea), m_HitPos(start)
{
    m_Center = start;
    m_Radius = 0.0;
    m_DragEdge = edgeMode;
    m_DragMode = e_DragCreate;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasureCircle::MouseDown(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale)
{
    if ( MeasureNode::MouseDown(mouseButton, pos, currentScale) ) {
        return true;
    }
    m_HitPos = pos;
    m_HitDirection = pos - m_Center;
    NormalizePoint(m_HitDirection);

    m_StartDragPos = m_Center;
    m_StartDragRadius = m_Radius;

    double maxDistance = 5.0 / currentScale.x();

    double centerOffset = PointLength(pos - m_Center);
    double circleOffset = abs(centerOffset - m_Radius);

    if (centerOffset < circleOffset) {
        m_DragMode = e_DragMove;
    } else {
        m_DragMode = e_DragResize;
    }

    CaptureMouse(true);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasureCircle::MouseUp(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale)
{
    if (MeasureNode::MouseUp(mouseButton, pos, currentScale)) {
        return true;
    }
    if (m_DragMode != e_DragNone)
    {
        if (m_DragMode != e_DragCreate)
        {
            QUndoCommand* cmd = new ECmdDragMeasureCircle(m_View, this, m_StartDragPos, m_Center, m_StartDragRadius, m_Radius);
            m_View->GetUndoStack()->push(cmd);
        }
        m_DragMode = e_DragNone;
        CaptureMouse(false);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasureCircle::MouseMove(const PMPoint& pos, const PMPoint& currentScale)
{
    if (MeasureNode::MouseMove(pos, currentScale)) {
        return true;
    }

    if ( m_DragMode == e_DragNone )
    {
        double centerOffset = PointLength(pos - m_Center);
        m_MouseDistance = std::min(abs(centerOffset - m_Radius), centerOffset);
    }
    else
    {
        PMPoint delta = pos - m_HitPos;
        m_HitPos = pos;
        QUndoStack* stack = m_View->GetUndoStack();

        m_HitDirection = GetNormalizedPoint(delta);

        PMPoint deltaPos;
        double    deltaRadius = 0.0;

        switch (m_DragMode)
        {
            case MeasureCircle::e_DragMove:
                deltaPos = delta;;
                break;
            case MeasureCircle::e_DragCreate:
            case MeasureCircle::e_DragResize:
                if (m_DragEdge)
                {
                    deltaRadius = (PointLength(pos - m_Center) - m_Radius);
                    deltaRadius *= 0.5;
                    deltaPos = m_HitDirection * abs(deltaRadius);
                }
                else
                {
                    deltaRadius = PointLength(pos - m_Center) - m_Radius;
                }
                break;
            default:
                break;
        }

        SetPosAndRadius(m_Center + deltaPos, m_Radius + deltaRadius);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureCircle::Transform(const QMatrix& transform)
{
    PMPoint prevEdge(m_Center);
    prevEdge.rx() += m_Radius;
    m_Center = transform.map(m_Center);
    m_Radius = PointLength(transform.map(prevEdge) - m_Center);

    SetBoundingBox(QRectF(m_Center.x() - m_Radius, m_Center.y() - m_Radius, m_Radius * 2.0, m_Radius * 2.0));
    DataChanged();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureCircle::Paint(QPainter& painter, const PMPoint& scale)
{
    QPen pen;
    pen.setCosmetic(true);
    QBrush brush(Qt::SolidPattern);
    brush.setColor(VisualSettings::s_MeasureFillColor);

    pen.setStyle(Qt::SolidLine);
    pen.setColor((IsSelected() || IsHovered()) ? VisualSettings::s_MeasureStrokeSelectedColor : VisualSettings::s_MeasureStrokeColor);
    pen.setWidth(1);
    painter.setPen(pen);

    painter.setBrush(brush);
    painter.drawEllipse(m_Center, m_Radius, m_Radius);

    PMPoint viewCenter = m_Center;
    double crossSize = 5.0 / scale.x();
    painter.drawLine(PMPoint(viewCenter.x() - crossSize, viewCenter.y()), PMPoint(viewCenter.x() + crossSize, viewCenter.y()));
    painter.drawLine(PMPoint(viewCenter.x(), viewCenter.y() - crossSize), PMPoint(viewCenter.x(), viewCenter.y() + crossSize));

    PMPoint labelSize = GetLabelSize(painter) / scale;
    PMPoint labelPos = m_Center;
    labelPos.setX(labelPos.x() - labelSize.x() * 0.5);
    labelPos.setY(labelPos.y() - m_Radius - labelSize.y() - 5.0 / scale.y());
    UpdateLabelSizeAndPos(labelSize, labelPos);
    DrawLabel(painter, scale);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasureCircle::Intersects(const QPointF& point) const
{
    return PointLengthSqr(point - m_Center) < m_Radius * m_Radius;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasureCircle::Intersects(const QRectF& frame) const
{
    if (GetBoundingBox().intersects(frame))
    {
        QPainterPath painterPath;
        painterPath.addEllipse(m_Center, m_Radius, m_Radius);
        return painterPath.intersects(frame);
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

double MeasureCircle::GetDistance() const
{
    if (HitTestLabel(m_MousePos)) {
        return 0.0;
    }
    return m_MouseDistance;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureCircle::SetPosAndRadius(const PMPoint& pos, double radius)
{
    m_Center = pos;
    m_Radius = radius;

    SetBoundingBox(QRectF(m_Center.x() - m_Radius, m_Center.y() - m_Radius, m_Radius * 2.0, m_Radius * 2.0));
    DataChanged();
}
