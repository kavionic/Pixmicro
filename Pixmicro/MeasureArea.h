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

#ifndef MEASUREAREA_H
#define MEASUREAREA_H

#include "MeasurePointList.h"

class MeasureArea : public MeasurePointList
{
    Q_OBJECT

public:
    static void Create(VideoView* view, int id, const QString& name, const PMPoint& startPos);
    MeasureArea(VideoView* view, int32_t id, const QString& name, const PMPoint& start);

    virtual void DataChanged() override;

//    virtual PMPoint GetDefaultLabelPos(QPainter& painter, const PMPoint& scale) const override;
    virtual double GetArea() const override;

    virtual bool Intersects(const QPointF& point) const override;
    virtual bool Intersects(const QRectF& frame) const override;

    virtual double GetDistance() const override;

    virtual void Paint(QPainter& painter, const PMPoint& scale) override;
private:
    QPainterPath     m_PolyConverter;
//    QList<QPolygonF> m_PolyList;
    double m_Area;

    static double GetPolyArea(const QPolygonF& points)
    {
        double area = 0.0;         // Accumulates area in the loop
        for (size_t i = 0, j = points.size() - 1, e = points.size(); i < e; j = i++)
        {
            area += (points[j].x() + points[i].x()) * (points[j].y()-points[i].y());
        }
        return abs(area * 0.5);
    }

};

#endif // MEASUREAREA_H
