#ifndef MEASURECIRCLE_H
#define MEASURECIRCLE_H

#include "MeasureNode.h"

class MeasureCircle : public MeasureNode
{
    Q_OBJECT

public:
    enum DragMode_e {
        e_DragNone,
        e_DragCreate,
        e_DragMove,
        e_DragResize
    };

public:
    static void Create(VideoView* view, int id, const QString& name, const PMPoint& startPos, bool edgeMode);
    MeasureCircle(VideoView* view, int32_t id, const QString& name, const PMPoint& start, bool edgeMode);

    virtual double GetLength() const override { return m_Radius * 2.0; }
    virtual double GetRadius() const override { return m_Radius; }
    virtual double GetArea() const override { return 2.0 * M_PI * m_Radius * m_Radius; }
    virtual double GetAngle() const override { return 0.0; }


    virtual bool MouseDown(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale) override;
    virtual bool MouseUp(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale) override;
    virtual bool MouseMove(const PMPoint& pos, const PMPoint& currentScale) override;
    virtual void Transform(const QMatrix& transform) override;

    virtual void Paint(QPainter& painter, const PMPoint& scale) override;

    virtual bool Intersects(const QPointF& point) const override;
    virtual bool Intersects(const QRectF& frame) const override;

    virtual double GetDistance() const override;
private:
    friend class ECmdDragMeasureCircle;

    void SetPosAndRadius(const PMPoint& pos, double radius);

    PMPoint    m_Center;
    double     m_Radius;
    bool       m_DragEdge;
    double     m_MouseDistance;
    PMPoint    m_HitPos;
    PMPoint    m_StartDragPos;
    double     m_StartDragRadius;
    PMPoint    m_HitDirection;
    DragMode_e m_DragMode;
};

#endif // MEASURECIRCLE_H
