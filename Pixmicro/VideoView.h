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

#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <QLabel>
#include "ui_VideoView.h"

class VideoView : public QLabel, public Ui::VideoView
{
    Q_OBJECT

public:
    VideoView(QWidget *parent = 0);
    ~VideoView();

    void Update(QImage* image, bool detectHole, bool holeColorFrozen, double holeThreshold, double holeRoundness);
    void CalibrateCrosshair();
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

private slots:
    void SlotFlipHToggled(bool flip);
    void SlotFlipVToggled(bool flip);
private:
    void      DrawCrosshair(QPixmap* bitmap, const QPointF& center);
    QVector2D FindEdge(QImage* image, const QPointF& startPos, double dirX, double dirY, int maxSearchRadius, double threshold);
    void      DetectHole(QImage* image, bool holeColorFrozen, const QPointF& searchPos, double threshold, double roundness);

    QPointF GetCrosshairViewPos(double scaleX, double scaleY) const;

    bool    m_FlipH;
    bool    m_FlipV;
    bool    m_IsPressed;
    QPointF m_HitPos;

    QPointF m_CrosshairPos;
    qreal   m_CrosshairRadius1;
    qreal   m_CrosshairRadius2;

    QVector3D m_HoleCenterColor;
    bool    m_HoleDetected;
    QPointF m_HoleCenter;
    QPointF m_HoleCenterFiltered;
    double  m_HoleRadius;
};

#endif // VIDEOVIEW_H
