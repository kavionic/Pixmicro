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
#include "VideoView.h"
#include "CameraSettings.h"
#include "HoleDetectWnd.h"

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

VideoView::VideoView(QWidget *parent) : QLabel(parent), m_CrosshairPos(0.5, 0.5)
{
    setupUi(this);

    m_IsPressed = false;
    m_HoleRadius = 0.0f;
    m_HoleDetected = false;

    QSettings settings;

    m_CrosshairRadius1 = settings.value("Crosshair/radius1", 0.005).toReal();
    m_CrosshairRadius2 = settings.value("Crosshair/radius2", 0.01).toReal();
    m_CrosshairPos = settings.value("Crosshair/position", m_CrosshairPos).toPointF();

    m_FlipH = CameraSettings::GetInstance()->IsHInverted();
    m_FlipV = CameraSettings::GetInstance()->IsVInverted();

    connect(CameraSettings::GetInstance(), &CameraSettings::SignalFlipHToggled, this, &VideoView::SlotFlipHToggled);
    connect(CameraSettings::GetInstance(), &CameraSettings::SignalFlipVToggled, this, &VideoView::SlotFlipVToggled);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

VideoView::~VideoView()
{
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotFlipHToggled(bool flip)
{
    m_FlipH = flip;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotFlipVToggled(bool flip)
{
    m_FlipV = flip;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::DrawCrosshair(QPixmap* bitmap, const QPointF& center)
{
    QPainter painter;

    if (painter.begin(bitmap))
    {
        QPen pen;
        if (m_HoleDetected)
        {
            QColor color = HoleDetectWnd::Get()->GetMarkerColor();
//            color.setAlphaF(0.3);
            pen.setStyle(Qt::SolidLine);
            pen.setColor(color);
            pen.setWidth(HoleDetectWnd::Get()->GetMarkerThickness());
            painter.setPen(pen);

            QPointF holeCenter(m_HoleCenter);

            if (m_FlipH)  holeCenter.setX(1.0 - holeCenter.x());
            if (!m_FlipV) holeCenter.setY(1.0 - holeCenter.y());
            holeCenter.rx() *= bitmap->width();
            holeCenter.ry() *= bitmap->height();

            QPointF holeCenterFiltered(m_HoleCenterFiltered);
            if (m_FlipH)  holeCenterFiltered.setX(1.0 - holeCenterFiltered.x());
            if (!m_FlipV) holeCenterFiltered.setY(1.0 - holeCenterFiltered.y());
            holeCenterFiltered.rx() *= bitmap->width();
            holeCenterFiltered.ry() *= bitmap->height();

            double holeRadius = m_HoleRadius * bitmap->width();
            painter.drawEllipse(holeCenter, holeRadius, holeRadius);

            pen.setStyle(Qt::DashDotLine);
            pen.setWidth(1);

            double aimSize = 15.0;
            double xOff = abs(holeCenterFiltered.x() - center.x());
            double yOff = abs(holeCenterFiltered.y() - center.y());
            if (xOff < aimSize && yOff < aimSize) {
                QPointF pos(center.x() + (holeCenterFiltered.x() - center.x()) * 3.0, center.y() + (holeCenterFiltered.y() - center.y()) * 3.0);
                
                pen.setColor((xOff>3) ? Qt::red : (xOff>1) ? Qt::yellow : Qt::green);
                painter.setPen(pen);
                painter.drawLine(pos.x(), center.y() - 3, pos.x(), center.y() - aimSize);
                painter.drawLine(pos.x(), center.y() + 3, pos.x(), center.y() + aimSize);

                pen.setColor((yOff>3) ? Qt::red : (yOff > 1) ? Qt::yellow : Qt::green);
                painter.setPen(pen);
                painter.drawLine(center.x() - 3, pos.y(), center.x() - aimSize, pos.y());
                painter.drawLine(center.x() + 3, pos.y(), center.x() + aimSize, pos.y());
            }
        }

        pen.setStyle(Qt::DashDotLine);
        pen.setColor(Qt::green);
        pen.setWidth(1);
        painter.setPen(pen);

        painter.drawLine(center.x() - 3, center.y(), 0, center.y());
        painter.drawLine(center.x() + 3, center.y(), bitmap->width(), center.y());

        painter.drawLine(center.x(), center.y() - 3, center.x(), 0);
        painter.drawLine(center.x(), center.y() + 3, center.x(), bitmap->height());

        pen.setStyle(Qt::SolidLine);
        pen.setColor(Qt::green);
        pen.setWidth(1);
        painter.setPen(pen);

        double radius1 = ((m_HoleDetected) ? m_HoleRadius : m_CrosshairRadius1) * bitmap->width();
        double radius2 = m_CrosshairRadius2 * bitmap->width();

        painter.drawEllipse(center, radius1, radius1);
        painter.drawEllipse(center, radius2, radius2);

        painter.end();
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

static QVector3D ColToVec(uint32_t color)
{
    return QVector3D(color & 0xff, (color >> 8) & 0xff, (color >> 16) & 0xff);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

QVector2D VideoView::FindEdge(QImage* image, const QPointF& startPos, double dirX, double dirY, int maxSearchRadius, double threshold)
{
    double x = startPos.x();
    double y = startPos.y();
    for (int i = 0; i < maxSearchRadius; ++i)
    {
        x += dirX;
        y += dirY;
        int xInt = int(x + 0.5);
        int yInt = int(y + 0.5);
        if (xInt < 0 || yInt < 0 || xInt >= image->width() || yInt >= image->height()) {
            return QVector2D(0.0, 0.0);
        }
        const uint32_t* line = (const uint32_t*)image->scanLine(yInt);
        if ((ColToVec(line[xInt]) - m_HoleCenterColor).lengthSquared() > threshold) {
            return QVector2D(i*dirX, i*dirY);
        }
    }
    return QVector2D(0.0, 0.0);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::DetectHole(QImage* image, bool holeColorFrozen, const QPointF& searchPos, double threshold, double roundness)
{
    bool wasDetected = m_HoleDetected;
    m_HoleDetected = false;

    static const int NUM_DIRECTIONS = 16;

    if (!holeColorFrozen)
    {
        const uint32_t* scanLine = reinterpret_cast<const uint32_t*>(image->constScanLine(searchPos.y()));
        uint32_t startColor = scanLine[int(searchPos.x() + 0.5)];
        m_HoleCenterColor = ColToVec(startColor);
        HoleDetectWnd::Get()->UpdateCenterColor(QColor(startColor));
    }
    else
    {
        m_HoleCenterColor = ColToVec(HoleDetectWnd::Get()->GetCenterColor().rgb());

    }
    int maxSearchRadius = 100;

    double minRadius = std::numeric_limits<int>::max();
    double maxRadius = 0;
    double radius = 0.0;


    uint32_t* scanLine = (uint32_t*)image->scanLine(searchPos.y());
    if (scanLine != nullptr)
    {
        int currentLeft = searchPos.x();
        int currentRight = currentLeft;


        double hDirections[] = { -1.0, 1.0, +0.0, 0.0, 1.0, -1.0, +1.0, -1.0, -1.0, +1.0f, -1.0f, +1.0f, -0.5f, +0.5f, +0.5f, -0.5f };
        double vDirections[] = { +0.0, 0.0, -1.0, 1.0, 1.0, -1.0, -1.0, +1.0, -0.5, +0.5f, +0.5f, -0.5f, -1.0f, +1.0f, -1.0f, +1.0f };


        double cX = searchPos.x();
        double cY = searchPos.y();

        std::vector<std::pair<int, QVector2D>> points;
        points.resize(NUM_DIRECTIONS);

        int iterations = 6;
        for (int k = 0; k < iterations; ++k)
        {
            QPointF center(cX, cY);
            for (int i = 0; i < points.size(); ++i)
            {
                QVector2D pos = FindEdge(image, center, hDirections[i], vDirections[i], maxSearchRadius, threshold);
                if (pos.x() == 0.0 && pos.y() == 0.0) {
                    return;
                }
                points[i] = std::make_pair(i, pos);
            }
            if (k > 3)
            {
                std::sort(points.begin(), points.end(), [](const std::pair<int, QVector2D>& l, const std::pair<int, QVector2D>& r) { return l.second.lengthSquared() < r.second.lengthSquared();  });

                for (int i = 0; i < 4; ++i)
                {
                    points[i].second.setX(0.0);
                    points[i].second.setY(0.0);
                    points[points.size() - 1 - i].second.setX(0.0);
                    points[points.size() - 1 - i].second.setY(0.0);
                }
                std::sort(points.begin(), points.end(), [](const std::pair<int, QVector2D>& l, const std::pair<int, QVector2D>& r) { return l.first < r.first;  });

                for (int i = 0; i < points.size(); ++i)
                {
                    if (points[i].second.x() == 0.0 && points[i].second.y() == 0.0)
                    {
                        if (i & 0x01) {
                            points[i].second = -points[i - 1].second;
                        }
                        else {
                            points[i].second = -points[i + 1].second;
                        }
                    }
                }
            }
            radius = 0.0;
            cX = 0;
            cY = 0;
            int validPoints = 0;
            for (int i = 0; i < points.size(); ++i)
            {
                QVector2D pos = points[i].second;
                if (pos.x() != 0.0 || pos.y() != 0.0)
                {
                    cX += pos.x();
                    cY += pos.y();
                    radius += pos.length();
                    validPoints++;
                }
            }
            cX /= validPoints;
            cY /= validPoints;
            radius /= validPoints;
            cX += center.x();
            cY += center.y();
        }

        if (roundness > 0.0)
        {
            double minRadius = radius * roundness;
            double maxRadius = radius / roundness;

            for (int i = 0; i < points.size(); ++i)
            {
                QVector2D pos = points[i].second;
                if (pos.x() != 0.0 || pos.y() != 0.0)
                {
                    double curRadius = pos.length();
                    if (curRadius < minRadius || curRadius > maxRadius) {
                        return;
                    }
                }
            }
        }

        m_HoleCenter.setX(cX / image->width());
        m_HoleCenter.setY(cY / image->height());
        m_HoleRadius = radius / image->width();
        m_HoleDetected = true;
        if (wasDetected)
        {
            m_HoleCenterFiltered += (m_HoleCenter - m_HoleCenterFiltered) * 0.6;
        }
        else
        {
            m_HoleCenterFiltered = m_HoleCenter;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

QPointF VideoView::GetCrosshairViewPos(double scaleX, double scaleY) const
{
    QPointF pos(m_CrosshairPos);

    if (m_FlipH)  pos.setX(1.0 - pos.x());
    if (!m_FlipV) pos.setY(1.0 - pos.y());
    return QPointF(pos.x() * scaleX, pos.y() * scaleY);
}


///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::Update(QImage* image, bool detectHole, bool holeColorFrozen, double holeThreshold, double holeRoundness)
{
    QTransform transform;
    QPointF viewSize(width(), height());
    QPointF bitmapSize(image->width(), image->height());

    if (detectHole)
    {
        DetectHole(image, holeColorFrozen, QPointF(m_CrosshairPos.x() * image->width(), m_CrosshairPos.y() * image->height()), holeThreshold, holeRoundness);
    }
    else
    {
        m_HoleDetected = false;
    }
    double scaleX = std::min(viewSize.x() / bitmapSize.x(), viewSize.y() / bitmapSize.y());
    double scaleY = scaleX;
    if (CameraSettings::GetInstance()->IsHInverted())  scaleX *= -1.0;
    if (!CameraSettings::GetInstance()->IsVInverted()) scaleY *= -1.0;
    transform.scale(scaleX, scaleY);

    QPixmap bitmap = QPixmap::fromImage(*image).transformed(transform, Qt::SmoothTransformation);

    DrawCrosshair(&bitmap, GetCrosshairViewPos(bitmap.width(), bitmap.height()));
    setPixmap(bitmap);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::CalibrateCrosshair()
{
    if (m_HoleDetected) {
        m_CrosshairPos = m_HoleCenterFiltered;
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::mousePressEvent(QMouseEvent* event)
{
    m_HitPos = event->localPos();
    m_IsPressed = true;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::mouseReleaseEvent(QMouseEvent* event)
{
    m_IsPressed = true;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_IsPressed)
    {
        QPointF delta = event->localPos() - m_HitPos;
        m_HitPos = event->localPos();
        delta.rx() /= width();
        delta.ry() /= height();
        if (m_FlipH)  delta.setX(-delta.x());
        if (!m_FlipV) delta.setY(-delta.y());

        m_CrosshairPos += delta;

        QSettings settings;
        settings.setValue("Crosshair/position", m_CrosshairPos);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::wheelEvent(QWheelEvent* event)
{
    qreal& radius = (event->modifiers() & Qt::ShiftModifier) ? m_CrosshairRadius2 : m_CrosshairRadius1;

    radius += event->angleDelta().y() / 100000.0;
    if (radius < 0.005) radius = 0.005;
    if (radius > 0.5)    radius = 0.5;

    QSettings settings;
    settings.setValue("Crosshair/radius1", m_CrosshairRadius1);
    settings.setValue("Crosshair/radius2", m_CrosshairRadius2);
}
