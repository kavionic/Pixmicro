#ifndef MEASUREPOINTLIST_H
#define MEASUREPOINTLIST_H

#include "MeasureNode.h"

class MeasurePointList : public MeasureNode
{
    Q_OBJECT

public:
    MeasurePointList(VideoView* view, int32_t id, const QString& name, uint32_t measurements, const PMPoint& start, bool multiPoint);
    virtual double GetLength() const override
    {
        double length = 0.0;
        for (size_t i = 1 ; i < m_MeasurePoints.size() ; ++i) {
            length += PointLength(m_MeasurePoints[i-1] - m_MeasurePoints[i]);
        }
        return length;
    }

    virtual double GetRadius() const override { return 0.0; }
    virtual double GetArea() const override { return 0.0; }
    virtual double GetAngle() const override
    {
        PMPoint vector(m_MeasurePoints[m_MeasurePoints.size() - 1] - m_MeasurePoints[0]);
        return atan2(vector.x(), -vector.y());
    }

    virtual bool MouseDown(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale) override;
    virtual bool MouseUp(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale) override;
    virtual bool MouseMove(const PMPoint& pos, const PMPoint& currentScale) override;

    virtual void Transform(const QMatrix& transform) override;

    virtual void InsertPoint() override;
    virtual bool Intersects(const QPointF& point) const override;
    virtual bool Intersects(const QRectF& frame) const override;
    virtual double GetDistance() const override;

protected:
    friend class ECmdMoveMeasurePointListPoint;
    friend class ECmdAddMeasurePointListPoint;
    friend class ECmdDeleteMeasurePointListPoint;

    enum DragMode_e {
        e_DragNone,
        e_DragCreate,
        e_DragMultiPointCreate,
        e_DragMovePoint,
        e_DragMoveSegment
    };

    void UpdateBoundingBox();

    void AddPoint(int index, const PMPoint& pos);
    void RemovePoint(int index);
    void MovePoint(int index, const PMPoint& pos);

    double GetSegmentDistance(size_t firstPoint, const PMPoint& pos) const;

    QPolygonF            m_MeasurePoints;
    int                  m_MouseSegment;
    double               m_MouseDistance;
    int                  m_MousePoint;

    DragMode_e           m_DragMode;
    int                  m_DragPoint;
    PMPoint              m_StartDragPos;
};

#endif // MEASUREPOINTLIST_H
