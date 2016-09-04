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

#pragma once

class VideoView;

typedef QPointF PMPoint;

inline double  PointLengthSqr(const PMPoint& point) { return point.x() * point.x() + point.y() * point.y(); }
inline double  PointLength(const PMPoint& point) { return sqrt(PointLengthSqr(point)); }

inline void    NormalizePoint(PMPoint& point) { double length = PointLength(point); point *= 1.0/length; }
inline PMPoint GetNormalizedPoint(const PMPoint& point) { double length = PointLength(point); return point * (1.0/length); }

//inline float CrossProduct(const PMPoint &p, PMPoint &q, PMPoint &r) { return q.x() * (r.y() - p.y()) + p.x() * (q.y() - r.y()) + r.x() * (p.y() - q.y()); }
//inline float CrossProduct(const PMPoint &p, PMPoint &q, PMPoint &r) { return (q.x() * (r.y() - p.y()) *(q.y() - r.y()) + r.x() * (p.y() - q.y()));; }
/*inline float CrossProduct(const PMPoint &p, PMPoint &q, PMPoint &r)
{
    PMPoint v1(GetNormalizedPoint(q-p));
    PMPoint v2(GetNormalizedPoint(r-q));
    double angle = atan2(v2.x(), -v2.y()) - atan2(v1.x(), -v1.y());
    if (angle < -M_PI) {
        return angle + M_PI*2.0;
    }
    else if (angle > M_PI) {
        return angle - M_PI*2.0;
    }
    else {
        return angle;
    }
    return angle;
}*/

inline PMPoint operator*(const PMPoint& p1, const PMPoint& p2) { return PMPoint(p1.x() * p2.x(), p1.y() * p2.y()); }
inline PMPoint operator/(const PMPoint& p1, const PMPoint& p2) { return PMPoint(p1.x() / p2.x(), p1.y() / p2.y()); }
inline PMPoint& operator*=(PMPoint& p1, const PMPoint& p2) { p1.rx() *= p2.x(); p1.ry() *= p2.y(); return p1; }
inline PMPoint& operator/=(PMPoint& p1, const PMPoint& p2) { p1.rx() /= p2.x(); p1.ry() /= p2.y(); return p1; }

class MeasureNode : public QObject, public QEnableSharedFromThis<MeasureNode>
{
    Q_OBJECT

public:
    enum Measurements_e {
        e_MeasureLength   = 0x0001,
        e_MeasureRadius   = 0x0002,
        e_MeasureDiameter = 0x0004,
        e_MeasureArea     = 0x0008,
        e_MeasureAngle    = 0x0010
    };

    MeasureNode(VideoView* view, int32_t id, const QString& name, uint32_t measurements, uint32_t displayedMeasurements);

    virtual ~MeasureNode() {}

    int32_t GetID() const { return m_ID; }
    QString GetName() const { return m_Name; }
    uint32_t GetMeasurements() const { return m_Measurements; }
    uint32_t GetDisplayedMeasurements() const { return m_DisplayedMeasurements; }

    bool IsSelected() const { return m_IsSelected; }
    bool IsHovered() const { return m_IsHovered; }

    void CaptureMouse(bool capture);

    virtual void DataChanged(); // Called when any of the measurement values changed, or when the display units change.

    void SetBoundingBox(const QRectF& boundingBox);
    const QRectF& GetBoundingBox() const { return m_BoundingBox; }

    void UpdateLabelSizeAndPos(const PMPoint& size, const PMPoint& pos);
    PMPoint GetLabelSize(QPainter& painter) const { return m_LabelSize; }
    void DrawLabel(QPainter& painter, const PMPoint& scale);
    bool HitTestLabel(const PMPoint& pos) const;

    virtual double GetLength() const = 0;
    virtual double GetRadius() const = 0;
    virtual double GetArea() const = 0;
    virtual double GetAngle() const = 0;


    virtual bool MouseDown(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale);
    virtual bool MouseUp(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale);
    virtual bool MouseMove(const PMPoint& pos, const PMPoint& currentScale);
    virtual void Transform(const QMatrix& transform) = 0;

    virtual void InsertPoint() {}

    virtual void Paint(QPainter& painter, const PMPoint& scale) = 0;
    virtual bool Intersects(const QPointF& point) const = 0;
    virtual bool Intersects(const QRectF& frame) const = 0;

    virtual double GetDistance() const = 0;

signals:
    void SignalDataChanged();
    void SignalBoundingBoxChanged();

protected:
    VideoView* m_View;
    PMPoint    m_MousePos;

private:
    friend class VideoView;

    int32_t    m_ID;
    QString    m_Name;
    uint32_t   m_Measurements;
    uint32_t   m_DisplayedMeasurements;
    void SetSelect(bool isSelected) {
        if ( isSelected != m_IsSelected ) {
            m_IsSelected = isSelected;
            //emit SignalSelectionChanged(m_IsSelected);
        }
    }
    void SetHovered(bool isHovered) {
        m_IsHovered = isHovered;
    }

    bool    m_IsSelected;
    bool    m_IsHovered;    // This is the node under the mouse.
    QRectF  m_BoundingBox;
    PMPoint m_LabelSize;
    PMPoint m_ScaledLabelSize;
    bool    m_IsDraggingLabel;
    PMPoint m_HitPos;
    PMPoint m_LabelPos;
    PMPoint m_LabelOffset;

    QString m_LabelLength;
    QString m_LabelRadius;
    QString m_LabelDiameter;
    QString m_LabelArea;
    QString m_LabelAngle;

};


