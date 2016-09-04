#include "stdafx.h"
#include "MeasurePointList.h"
#include "EditCommands.h"
#include "VisualSettings.h"
#include "VideoView.h"


///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

MeasurePointList::MeasurePointList(VideoView* view, int32_t id, const QString& name, uint32_t measurements, const PMPoint& start, bool multiPoint) : MeasureNode(view, id, name, measurements, measurements), m_StartDragPos(start)
{
    m_DragMode = (multiPoint) ? e_DragMultiPointCreate : e_DragCreate;
    m_DragPoint = 1;
    m_MousePoint = 1;
    m_MouseSegment = -1;
    m_MeasurePoints.resize(2);
    m_MeasurePoints[0] = start;
    m_MeasurePoints[1] = start;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasurePointList::MouseDown(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale)
{
    if (m_DragMode != e_DragMultiPointCreate)
    {
        if (MeasureNode::MouseDown(mouseButton, pos, currentScale)) {
            return true;
        }
        if (mouseButton == Qt::LeftButton && m_MousePoint != -1)
        {
            m_DragMode = e_DragMovePoint;
            m_DragPoint = m_MousePoint;
            m_StartDragPos = m_MeasurePoints[m_DragPoint];
            CaptureMouse(true);
            return true;
        }
    }
    else
    {
        if (mouseButton == Qt::LeftButton)
        {
            m_MousePoint = m_MeasurePoints.size();
            m_DragPoint = m_MousePoint;
            AddPoint(m_MousePoint, pos);
            m_View->update();
            return true;
        }
        else if (mouseButton == Qt::RightButton)
        {
            m_DragMode = e_DragNone;
            m_View->update();
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasurePointList::MouseUp(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale)
{
    if (MeasureNode::MouseUp(mouseButton, pos, currentScale)) {
        return true;
    }
    if (m_DragMode != e_DragMultiPointCreate && m_DragPoint != -1)
    {
        CaptureMouse(false);

        if (m_DragMode != e_DragCreate && m_DragMode != e_DragMultiPointCreate) {
            QUndoCommand* cmd = new ECmdMoveMeasurePointListPoint(m_View, this, m_DragPoint, m_StartDragPos, m_MeasurePoints[m_DragPoint]);
            m_View->GetUndoStack()->push(cmd);
        }
        m_DragPoint = -1;
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasurePointList::MouseMove(const PMPoint& pos, const PMPoint& currentScale)
{
    if (MeasureNode::MouseMove(pos, currentScale)) {
        return true;
    }
    if ( m_DragPoint == -1 )
    {
        m_MouseDistance = std::numeric_limits<double>::max();

        for (size_t i = 0 ; i < m_MeasurePoints.size() - 1 ; ++i)
        {
            double distance = GetSegmentDistance(i, pos);
            if (distance < m_MouseDistance) {
                m_MouseDistance = distance;
                m_MouseSegment = i;
            }
        }
        double longestPointDistance = std::numeric_limits<double>::max();
        double maxSelectionDistance = VisualSettings::s_SelectionMaxDistance / currentScale.x();
        maxSelectionDistance *= maxSelectionDistance;

        m_MousePoint = -1;
        for (size_t i = 0 ; i < m_MeasurePoints.size() ; ++i)
        {
            double distance = PointLengthSqr(m_MeasurePoints[i] - pos);
            if (distance < maxSelectionDistance && distance < longestPointDistance) {
                longestPointDistance = distance;
                m_MousePoint = i;
            }
        }
    }
    else
    {
        MovePoint(m_DragPoint, pos);
        m_View->update();
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasurePointList::Transform(const QMatrix& transform)
{
    for (size_t i = 0 ; i < m_MeasurePoints.size() ; ++i)
    {
        m_MeasurePoints[i] = transform.map(m_MeasurePoints[i]);
    }
    UpdateBoundingBox();
    DataChanged();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasurePointList::InsertPoint()
{
    if (m_MousePoint == -1) {
        QUndoCommand* cmd = new ECmdAddMeasurePointListPoint(this, m_MouseSegment + 1, m_MousePos);
        m_View->GetUndoStack()->push(cmd);
    }
    else {
        QUndoCommand* cmd = new ECmdDeleteMeasurePointListPoint(this, m_MousePoint);
        m_View->GetUndoStack()->push(cmd);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasurePointList::UpdateBoundingBox()
{
    QRectF boundingBox(m_MeasurePoints[0], m_MeasurePoints[0]);

    for (size_t i = 1, e = m_MeasurePoints.size() ; i < e ; ++i) {
        const auto& point = m_MeasurePoints[i];
        if (point.x() < boundingBox.left()) boundingBox.setLeft(point.x());
        else if (point.x() > boundingBox.right()) boundingBox.setRight(point.x());

        if (point.y() < boundingBox.top()) boundingBox.setTop(point.y());
        else if (point.y() > boundingBox.bottom()) boundingBox.setBottom(point.y());
    }
    SetBoundingBox(boundingBox);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasurePointList::AddPoint(int index, const PMPoint& pos)
{
    m_MeasurePoints.insert(m_MeasurePoints.begin() + index, pos);
    UpdateBoundingBox();
    DataChanged();
    m_View->update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasurePointList::RemovePoint(int index)
{
    m_MeasurePoints.erase(m_MeasurePoints.begin() + index);
    UpdateBoundingBox();
    DataChanged();
    m_MousePoint = -1;
    m_MouseSegment = -1;
    m_View->update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasurePointList::MovePoint(int index, const PMPoint& pos)
{
    m_MeasurePoints[index] = pos;
    UpdateBoundingBox();
    DataChanged();
    m_View->update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

double MeasurePointList::GetSegmentDistance(size_t firstPoint, const PMPoint& pos) const
{
    PMPoint relPos = pos - m_MeasurePoints[firstPoint];
    PMPoint relEnd = m_MeasurePoints[firstPoint + 1] - m_MeasurePoints[firstPoint];

    // If the length is 0, return distance between pos and one line end
    if (PointLength(relEnd) <= std::numeric_limits<float>::epsilon()) return PointLength(relPos);

    PMPoint tmp = (pos - m_MeasurePoints[firstPoint]) * relEnd;
    float intersection = (tmp.x() + tmp.y()) / PointLengthSqr(relEnd);

    if (intersection < 0.0f) {
        relEnd = pos - m_MeasurePoints[firstPoint];
    }
    else if (intersection > 1.0f) {
        relEnd = pos - m_MeasurePoints[firstPoint+1];
    }
    else {
        relEnd = pos - (m_MeasurePoints[firstPoint] + relEnd * intersection);
    }
    return PointLength(relEnd);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasurePointList::Intersects(const QPointF& point) const
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool MeasurePointList::Intersects(const QRectF& frame) const
{
    if (GetBoundingBox().intersects(frame))
    {
        QPainterPath painterPath;
        painterPath.moveTo(m_MeasurePoints[0]);
        for (int i = 1, e = m_MeasurePoints.size() ; i < e ; ++i) {
            painterPath.lineTo(m_MeasurePoints[i]);
            painterPath.closeSubpath();
            if (painterPath.intersects(frame)) {
                return true;
            }
            painterPath = QPainterPath();
            painterPath.moveTo(m_MeasurePoints[i]);
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

double MeasurePointList::GetDistance() const
{
    if (HitTestLabel(m_MousePos)) {
        return 0.0;
    }
    return m_MouseDistance;
}
