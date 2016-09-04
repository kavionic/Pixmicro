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
#include "VisualSettings.h"
#include "HoleDetectWnd.h"
#include "MeasureWnd.h"
#include "ValuePrefixes.h"
#include "EditCommands.h"
#include "MeasureNode.h"

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

VideoView::VideoView(QWidget *parent) : QWidget(parent), m_CrosshairPos(0.5, 0.5), m_ToImageScale(1.0f, 1.0f), m_ToViewScale(1.0f, 1.0f)
{
    setupUi(this);

    m_UndoStack = new QUndoStack(this);

    m_LastMeasureID = 1;

    m_HoleDetectWnd       = nullptr;
    m_MeasureWnd          = nullptr;
    m_HoleDetectEnabled   = false;
    m_HoleColorFrozen     = false;
    m_Threshold           = 1.0;
    m_Roundness           = 1.0;
    m_HoleMarkerThickness = 1;

    m_ZoomScale = 1.0;
    m_HighlightCrosshairTarget = -1;
    m_HoleRadius = 0.0f;
    m_HoleDetected = false;

    QSettings settings;

    int size = settings.beginReadArray("Crosshair/targets");

    if (size == 0)
    {
        m_CrosshairTargets.push_back(CrosshairTarget(0.0315, Qt::green, true));
        m_CrosshairTargets.push_back(CrosshairTarget(0.1747, Qt::green, false));
    }
    else
    {
        m_CrosshairTargets.reserve(size);
        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            m_CrosshairTargets.push_back(CrosshairTarget(settings.value("radius").toDouble(), settings.value("color").value<QColor>(), settings.value("snapToHole").toBool()));
        }
    }
    settings.endArray();


    //    m_CrosshairRadius1 = settings.value("Crosshair/radius1", 0.005).toReal();
    //    m_CrosshairRadius2 = settings.value("Crosshair/radius2", 0.01).toReal();
    m_CrosshairPos = settings.value("Crosshair/position", m_CrosshairPos).value<PMPoint>();

    m_FlipH = CameraSettings::GetInstance()->IsHInverted();
    m_FlipV = CameraSettings::GetInstance()->IsVInverted();

    connect(CameraSettings::GetInstance(), &CameraSettings::SignalFlipHToggled, this, &VideoView::SlotFlipHToggled);
    connect(CameraSettings::GetInstance(), &CameraSettings::SignalFlipVToggled, this, &VideoView::SlotFlipVToggled);

    m_PanOffset = settings.value("Viewport/panOffset").value<PMPoint>() * m_ToViewScale;
    m_ZoomScale = settings.value("Viewport/zoom", 1.0).toDouble();

    setMouseTracking(true);

    m_SelectionAnimationTimer.setInterval(50);
    connect(&m_SelectionAnimationTimer, &QTimer::timeout, this, &VideoView::SlotSelectionAnimationTimer);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::AddToolbars(QMainWindow* window)
{
    QToolBar* measureToolbar = new QToolBar;

    

    m_MeasureModeActions = new QActionGroup(this);
    m_MeasureModeActions->setExclusive(true);

    m_MeasureNoneAction = m_MeasureModeActions->addAction(QIcon("Icons/MeasureNone.png"), "None");
    m_MeasureNoneAction->setCheckable(true);
    m_MeasureNoneAction->setChecked(true);
    m_MeasureNoneAction->setShortcut(Qt::Key_Escape);
    measureToolbar->addAction(m_MeasureNoneAction);

    m_MeasureDistanceAction = m_MeasureModeActions->addAction(QIcon("Icons/MeasureDistance.png"), "Distance");
    m_MeasureDistanceAction->setCheckable(true);
    connect(m_MeasureDistanceAction, &QAction::triggered, this, &VideoView::SlotMeasureDistance);
    measureToolbar->addAction(m_MeasureDistanceAction);

    m_MeasureAreaAction = m_MeasureModeActions->addAction(QIcon("Icons/MeasurePolygonArea.png"), "Area");
    m_MeasureAreaAction->setCheckable(true);
    measureToolbar->addAction(m_MeasureAreaAction);

    m_MeasureCircleCenterAction = m_MeasureModeActions->addAction(QIcon("Icons/MeasureCircleCenter.png"), "Circle - center");
    m_MeasureCircleCenterAction->setCheckable(true);
    measureToolbar->addAction(m_MeasureCircleCenterAction);

    m_MeasureCircleEdgeAction = m_MeasureModeActions->addAction(QIcon("Icons/MeasureCircleEdge.png"), "Circle - edge");
    m_MeasureCircleEdgeAction->setCheckable(true);
    measureToolbar->addAction(m_MeasureCircleEdgeAction);

    m_MeasureAngleAction = m_MeasureModeActions->addAction(QIcon("Icons/MeasureAngle.png"), "Angle");
    m_MeasureAngleAction->setCheckable(true);
    measureToolbar->addAction(m_MeasureAngleAction);

    m_InsertPointShortcut = new QShortcut('N', this);
    connect(m_InsertPointShortcut, &QShortcut::activated, this, &VideoView::SlotInsertPoint);

    m_UnitsDropdown = new QComboBox;

    for (int i = 0 ; i < ValuePrefixes::e_PrefixCount ; ++i)
    {
        ValuePrefixes::Prefixes_e prefix = ValuePrefixes::Prefixes_e(i);
        QString name = ValuePrefixes::GetPrefixName(prefix);
        if (!name.isEmpty()) {
            m_UnitsDropdown->addItem(name, i);
        }
    }
    connect(m_UnitsDropdown, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotUnitsChanged(int)));
    measureToolbar->addWidget(m_UnitsDropdown);
    window->addToolBar(Qt::TopToolBarArea, measureToolbar);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotMeasureDistance(bool checked)
{
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotInsertPoint()
{
    if ( m_SelectedMeasurements.size() == 1 ) {
        m_SelectedMeasurements[0]->InsertPoint();
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SetHoleDetectWnd(HoleDetectWnd* holeDetectWnd)
{
    m_HoleDetectWnd = holeDetectWnd;

    connect(holeDetectWnd, &HoleDetectWnd::SignalEnableChanged, this, &VideoView::SlotEnableChanged);
    connect(holeDetectWnd, &HoleDetectWnd::SignalFreezeHoleColorChanged, this, &VideoView::SlotFreezeHoleColorChanged);
    connect(holeDetectWnd, &HoleDetectWnd::SignalThresholdChanged, this, &VideoView::SlotThresholdChanged);
    connect(holeDetectWnd, &HoleDetectWnd::SignalRoundnessChanged, this, &VideoView::SlotRoundnessChanged);
    connect(holeDetectWnd, &HoleDetectWnd::SignalMarkerColorChanged, this, &VideoView::SlotMarkerColorChanged);
    connect(holeDetectWnd, &HoleDetectWnd::SignalMarkerWidthChanged, this, &VideoView::SlotMarkerWidthChanged);

    m_HoleDetectEnabled   = holeDetectWnd->IsEnabled();
    m_HoleColorFrozen     = holeDetectWnd->IsCenterColorFrozen();
    m_Threshold           = holeDetectWnd->GetThreshold();
    m_Roundness           = holeDetectWnd->GetRoundness();
    m_HoleMarkerThickness = holeDetectWnd->GetMarkerThickness();
    m_HoleMarkerColor     = holeDetectWnd->GetMarkerColor();

}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SetMeasureWnd(MeasureWnd* measureWnd)
{
    m_MeasureWnd = measureWnd;

    connect(measureWnd, &MeasureWnd::SignalSelectionChanged, this, &VideoView::SlotMeasureSelectionChanged);
    connect(measureWnd, &MeasureWnd::SignalMeasureDeleted, this, &VideoView::SlotMeasureDeleted);
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

void VideoView::SaveSettings()
{
    QSettings settings;

    settings.setValue("Viewport/panOffset", m_PanOffset / m_ToViewScale);
    settings.setValue("Viewport/zoom", m_ZoomScale);

    settings.setValue("Crosshair/position", m_CrosshairPos);
    settings.beginWriteArray("Crosshair/targets", m_CrosshairTargets.size());
    for (int i = 0; i < m_CrosshairTargets.size(); ++i)
    {
        const auto& target = m_CrosshairTargets[i];
        settings.setArrayIndex(i);

        settings.setValue("radius", target.m_Radius);
        settings.setValue("color", target.m_Color);
        settings.setValue("snapToHole", target.m_SnapToHoleSize);
    }
    settings.endArray();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotEnableChanged(bool enabled)
{
    m_HoleDetectEnabled = enabled;
    DetectHole();
    update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotFreezeHoleColorChanged(bool freeze)
{
    m_HoleColorFrozen = freeze;
    DetectHole();
    update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotThresholdChanged(double value)
{
    m_Threshold = value;
    DetectHole();
    update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotRoundnessChanged(double value)
{
    m_Roundness = value;
    DetectHole();
    update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotMarkerColorChanged(const QColor& color)
{
    m_HoleMarkerColor = color;
    update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotMarkerWidthChanged(int value)
{
    m_HoleMarkerThickness = value;
    update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotFlipHToggled(bool flip)
{
    m_FlipH = flip;
    m_CrosshairPos.setX(1.0f - m_CrosshairPos.x());
    m_HoleCenter.setX(1.0f - m_HoleCenter.x());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotFlipVToggled(bool flip)
{
    m_FlipV = flip;
    m_CrosshairPos.setY(1.0f - m_CrosshairPos.y());
    m_HoleCenter.setY(1.0f - m_HoleCenter.y());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotMeasureSelectionChanged(const std::vector<QSharedPointer<MeasureNode>>& list)
{
    for ( auto i : m_SelectedMeasurements )
    {
        i->SetSelect(false);
        disconnect(i.data(), &MeasureNode::SignalBoundingBoxChanged, this, &VideoView::SlotSelectedNodeBoundingBoxChanged);
    }
    m_SelectedMeasurements = list;
    for (auto i : m_SelectedMeasurements)
    {
        i->SetSelect(true);
        connect(i.data(), &MeasureNode::SignalBoundingBoxChanged, this, &VideoView::SlotSelectedNodeBoundingBoxChanged);
    }
    SlotSelectedNodeBoundingBoxChanged();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotSelectedNodeBoundingBoxChanged()
{
    if ( m_SelectedMeasurements.size() > 1 )
    {
        m_SelectionBoundingBox = m_SelectedMeasurements[0]->GetBoundingBox();
        for (int i = 1, e = m_SelectedMeasurements.size() ; i < e ; ++i)
        {
            m_SelectionBoundingBox |= m_SelectedMeasurements[i]->GetBoundingBox();
        }
        if (!m_SelectionAnimationTimer.isActive()) {
            m_SelectionAnimationTimer.start();
        }
    }
    else
    {
        if (m_SelectionAnimationTimer.isActive()) {
            m_SelectionAnimationTimer.stop();
        }
    }
    update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotSelectionAnimationTimer()
{
    m_DashOffset++;
    update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotMeasureDeleted(QSharedPointer<MeasureNode> measureNode)
{
    QUndoCommand* cmd = new ECmdDeleteMeasure(this, measureNode);
    m_UndoStack->push(cmd);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SlotUnitsChanged(int index)
{
    ValuePrefixes::Get().SetSelectedPrefix(ValuePrefixes::Prefixes_e(m_UnitsDropdown->itemData(index).toInt()));
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::DrawCrosshair(QPainter& painter)
{
    QPen pen;
    pen.setCosmetic(true);

    double deadZone = 3.0 / m_ToViewScale.x();

    if (m_HoleDetected)
    {
        pen.setStyle(Qt::SolidLine);
        pen.setColor(m_HoleMarkerColor);
        pen.setWidth(m_HoleMarkerThickness);
        painter.setPen(pen);

        PMPoint holeCenter(m_HoleCenter);

        PMPoint holeCenterFiltered(m_HoleCenterFiltered);

        painter.drawEllipse(holeCenter, m_HoleRadius, m_HoleRadius);

        pen.setStyle(Qt::DashDotLine);
        pen.setWidth(1);

        double aimSize = 15.0 / m_ToViewScale.x();
        double xOff = abs(holeCenterFiltered.x() - m_CrosshairPos.x());
        double yOff = abs(holeCenterFiltered.y() - m_CrosshairPos.y());
        if (xOff < aimSize && yOff < aimSize) {
            QPointF pos(m_CrosshairPos.x() + (holeCenterFiltered.x() - m_CrosshairPos.x()) * 3.0, m_CrosshairPos.y() + (holeCenterFiltered.y() - m_CrosshairPos.y()) * 3.0);

            double errOff1 = 1.0 / m_ToViewScale.x();
            double errOff2 = 3.0 / m_ToViewScale.x();

            pen.setColor((xOff > errOff2) ? Qt::red : (xOff > errOff1) ? Qt::yellow : Qt::green);
            painter.setPen(pen);
            painter.drawLine(QPointF(pos.x(), m_CrosshairPos.y() - deadZone), QPointF(pos.x(), m_CrosshairPos.y() - aimSize));
            painter.drawLine(QPointF(pos.x(), m_CrosshairPos.y() + deadZone), QPointF(pos.x(), m_CrosshairPos.y() + aimSize));

            pen.setColor((yOff > errOff2) ? Qt::red : (yOff > errOff1) ? Qt::yellow : Qt::green);
            painter.setPen(pen);
            painter.drawLine(QPointF(m_CrosshairPos.x() - deadZone, pos.y()), QPointF(m_CrosshairPos.x() - aimSize, pos.y()));
            painter.drawLine(QPointF(m_CrosshairPos.x() + deadZone, pos.y()), QPointF(m_CrosshairPos.x() + aimSize, pos.y()));
        }
    }

    pen.setStyle(Qt::DashDotLine);
    pen.setWidth(1);


    pen.setColor((m_IsCrosshairHighlighted) ? VisualSettings::s_CrosshairSelectedColor : VisualSettings::s_CrosshairColor);
    painter.setPen(pen);

    painter.drawLine(QPointF(m_CrosshairPos.x() - deadZone, m_CrosshairPos.y()), QPointF(-1.0, m_CrosshairPos.y()));
    painter.drawLine(QPointF(m_CrosshairPos.x() + deadZone, m_CrosshairPos.y()), QPointF(2.0, m_CrosshairPos.y()));

    painter.drawLine(QPointF(m_CrosshairPos.x(), m_CrosshairPos.y() - deadZone), QPointF(m_CrosshairPos.x(), -1.0));
    painter.drawLine(QPointF(m_CrosshairPos.x(), m_CrosshairPos.y() + deadZone), QPointF(m_CrosshairPos.x(), 2.0));

    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    painter.setPen(pen);

    for (int i = 0; i < m_CrosshairTargets.size(); ++i)
    {
        const auto& target = m_CrosshairTargets[i];

        double radius = target.m_Radius;

        if (target.m_SnapToHoleSize)
        {
            if (m_HoleDetected && !(m_MouseHandler != nullptr && i == m_HighlightCrosshairTarget)) {
                radius = m_HoleRadius;
                pen.setColor((i != m_HighlightCrosshairTarget) ? VisualSettings::s_CrosshairInnerCircleHoleDetectedColor : VisualSettings::s_CrosshairInnerCircleSelectedColor);
            } else {
                pen.setColor((i != m_HighlightCrosshairTarget) ? VisualSettings::s_CrosshairInnerCircleHoleNotDetectedColor : VisualSettings::s_CrosshairInnerCircleSelectedColor);
            }
        }
        else
        {
            pen.setColor((i != m_HighlightCrosshairTarget) ? VisualSettings::s_CrosshairOuterCircleColor : VisualSettings::s_CrosshairOuterCircleSelectedColor);
        }

        pen.setStyle((target.m_SnapToHoleSize && !m_HoleDetected) ? Qt::DashLine : Qt::SolidLine);
        painter.setPen(pen);

        painter.drawEllipse(m_CrosshairPos, radius, radius);
    }

    pen.setStyle(Qt::SolidLine);
    pen.setColor(Qt::white);
    pen.setWidth(1);
    painter.setPen(pen);

    for ( int i = 0 ; i < m_Measurements.size() ; ++i)
    {
        const auto& measurement = m_Measurements[i];
        measurement->Paint(painter, m_ToViewScale);
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

PMPoint VideoView::FindEdge(const QPointF& startPos, double dirX, double dirY, int maxSearchRadius)
{
    double x = startPos.x();
    double y = startPos.y();
    for (int i = 0; i < maxSearchRadius; ++i)
    {
        x += dirX;
        y += dirY;
        int xInt = int(x + 0.5);
        int yInt = int(y + 0.5);
        if (xInt < 0 || yInt < 0 || xInt >= m_CurrentRawImage.width() || yInt >= m_CurrentRawImage.height()) {
            return PMPoint(0.0, 0.0);
        }
        const uint32_t* line = (const uint32_t*)m_CurrentRawImage.scanLine(yInt);
        if ((ColToVec(line[xInt]) - m_HoleCenterColor).lengthSquared() > m_Threshold) {
            return PMPoint(i*dirX, i*dirY);
        }
    }
    return PMPoint(0.0, 0.0);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::DetectHole()
{
    m_HoleDetected = false;
    if (!m_HoleDetectEnabled) return;
    
    PMPoint searchPos = m_CrosshairPos * m_ToImageScale; // /* * m_CurrentRawImage.width(), m_CrosshairPos.y() * m_CurrentRawImage.height()*/);
    bool wasDetected = m_HoleDetected;

    static const int NUM_DIRECTIONS = 16;

    if (!m_HoleColorFrozen)
    {
        const uint32_t* scanLine = reinterpret_cast<const uint32_t*>(m_CurrentRawImage.constScanLine(searchPos.y()));
        uint32_t startColor = scanLine[int(searchPos.x() + 0.5)];
        m_HoleCenterColor = ColToVec(startColor);
        m_HoleDetectWnd->UpdateCenterColor(QColor(startColor));
    }
    else
    {
        m_HoleCenterColor = ColToVec(m_HoleDetectWnd->GetCenterColor().rgb());

    }
    int maxSearchRadius = 1000;

    double minRadius = std::numeric_limits<int>::max();
    double maxRadius = 0;
    double radius = 0.0;


    uint32_t* scanLine = (uint32_t*)m_CurrentRawImage.scanLine(searchPos.y());
    if (scanLine != nullptr)
    {
        int currentLeft = searchPos.x();
        int currentRight = currentLeft;


        double hDirections[] = { -1.0, 1.0, +0.0, 0.0, 1.0, -1.0, +1.0, -1.0, -1.0, +1.0f, -1.0f, +1.0f, -0.5f, +0.5f, +0.5f, -0.5f };
        double vDirections[] = { +0.0, 0.0, -1.0, 1.0, 1.0, -1.0, -1.0, +1.0, -0.5, +0.5f, +0.5f, -0.5f, -1.0f, +1.0f, -1.0f, +1.0f };

        double cX = searchPos.x();
        double cY = searchPos.y();

        std::vector<std::pair<int, PMPoint>> points;
        points.resize(NUM_DIRECTIONS);

        int iterations = 10;
        for (int k = 0; k < iterations; ++k)
        {
            QPointF center(cX, cY);
            for (int i = 0; i < points.size(); ++i)
            {
                PMPoint pos = FindEdge(center, hDirections[i], vDirections[i], maxSearchRadius);
                if (pos.x() == 0.0 && pos.y() == 0.0) {
                    return;
                }
                points[i] = std::make_pair(i, pos);
            }
            if (k > 8)
            {
                std::sort(points.begin(), points.end(), [](const std::pair<int, PMPoint>& l, const std::pair<int, PMPoint>& r) { return PointLengthSqr(l.second) < PointLengthSqr(r.second);  });

                for (int i = 0; i < 4; ++i)
                {
                    points[i].second.setX(0.0);
                    points[i].second.setY(0.0);
                    points[points.size() - 1 - i].second.setX(0.0);
                    points[points.size() - 1 - i].second.setY(0.0);
                }
                std::sort(points.begin(), points.end(), [](const std::pair<int, PMPoint>& l, const std::pair<int, PMPoint>& r) { return l.first < r.first;  });

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
                PMPoint pos = points[i].second;
                if (pos.x() != 0.0 || pos.y() != 0.0)
                {
                    cX += pos.x();
                    cY += pos.y();
                    radius += PointLength(pos);
                    validPoints++;
                }
            }
            cX /= validPoints;
            cY /= validPoints;
            radius /= validPoints;
            cX += center.x();
            cY += center.y();
        }

        if (m_Roundness > 0.0)
        {
            double minRadius = radius * m_Roundness;
            double maxRadius = radius / m_Roundness;

            for (int i = 0; i < points.size(); ++i)
            {
                PMPoint pos = points[i].second;
                if (pos.x() != 0.0 || pos.y() != 0.0)
                {
                    double curRadius = PointLength(pos);
                    if (curRadius < minRadius || curRadius > maxRadius) {
                        return;
                    }
                }
            }
        }

        m_HoleCenter.setX(cX / m_ToImageScale.x());
        m_HoleCenter.setY(cY / m_ToImageScale.y());
        m_HoleRadius = radius / m_ToImageScale.x();
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

void VideoView::UpdateTransform()
{
    if (m_CurrentRawImage.isNull()) return;

    QPointF viewSize(width(), height());
    QSizeF bitmapSize = m_CurrentRawImage.size();

    double bitmapToViewScale = m_ZoomScale; // std::min(viewSize.x() / bitmapSize.width(), viewSize.y() / bitmapSize.height()) * m_ZoomScale;;
    m_BitmapToViewScale.setX(bitmapToViewScale);
    m_BitmapToViewScale.setY(bitmapToViewScale);

    double scaledWidth = bitmapSize.width() * m_BitmapToViewScale.x();

    m_PanOffset /= m_ToViewScale;

    m_ToImageScale.setX(bitmapSize.width());
    m_ToImageScale.setY(bitmapSize.width());
    m_ToViewScale.setX(scaledWidth);
    m_ToViewScale.setY(scaledWidth);

    m_PanOffset *= m_ToViewScale;

    // Offset needed to center the image.
    m_ImageOffset.setX((bitmapSize.width()  * m_BitmapToViewScale.x() - viewSize.x()) * 0.5);
    m_ImageOffset.setY((bitmapSize.height() * m_BitmapToViewScale.y() - viewSize.y()) * 0.5);

    // Cap m_PanOffset to prevent scrolling outside image boundaries.
    if (m_ImageOffset.x() < 0.0f) {
        m_PanOffset.setX(0.0f);
    } else if (-m_PanOffset.x() > m_ImageOffset.x()) {
        m_PanOffset.setX(-m_ImageOffset.x());
    } else if (m_PanOffset.x() > m_ImageOffset.x()) {
        m_PanOffset.setX(m_ImageOffset.x());
    }
    if (m_ImageOffset.y() < 0.0f) {
        m_PanOffset.setY(0.0f);
    } else if (-m_PanOffset.y() > m_ImageOffset.y()) {
        m_PanOffset.setY(-m_ImageOffset.y());
    } else if (m_PanOffset.y() > m_ImageOffset.y()) {
        m_PanOffset.setY(m_ImageOffset.y());
    }
    // Calculate total offset to center / pan the image.
    m_ImageOffset -= m_PanOffset;

    m_ImageTransform.reset();
    m_ImageTransform.scale(m_BitmapToViewScale.x(), m_BitmapToViewScale.y());
    m_ImageTransform.translate(-m_ImageOffset.x() / m_BitmapToViewScale.x(), -m_ImageOffset.y() / m_BitmapToViewScale.y());

    m_RenderTransform.reset();
    m_RenderTransform.scale(m_ToViewScale.x(), m_ToViewScale.y());
    m_RenderTransform.translate(-m_ImageOffset.x() / m_ToViewScale.x(), -m_ImageOffset.y() / m_ToViewScale.y());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::Update(QImage* image, bool fromCamera)
{
    QSize prevSize = m_CurrentRawImage.size();
    if (fromCamera)
    {
        QTransform transform;
        transform.scale((m_FlipH) ? -1.0 : 1.0, (m_FlipV) ? 1.0 : -1.0);
        m_CurrentRawImage = image->transformed(transform);
    }
    else
    {
        m_CurrentRawImage = image->copy();
    }
    m_CurrentRawImage.bits(); // Workaround for Qt bug causing a access violation in QPainter.drawImage()

    if (m_CurrentRawImage.size() != prevSize) {
        UpdateTransform();
    }
    DetectHole();
    update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

bool VideoView::TakeSnapshot(const QString& path, double scale, bool withOverlay)
{
    if (!m_CurrentRawImage.isNull())
    {
        if (scale == 1.0 && !withOverlay)
        {
            return m_CurrentRawImage.save(path, "PNG");
        }
        else
        {
            QImage image = m_CurrentRawImage.scaled(m_CurrentRawImage.width() * scale, m_CurrentRawImage.height() * scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            if (withOverlay)
            {
                if (m_Painter.begin(&image)) {
//                    m_Painter.setRenderHint(QPainter::SmoothPixmapTransform);
                    m_Painter.setRenderHint(QPainter::Antialiasing);
                    m_Painter.setRenderHint(QPainter::TextAntialiasing);
                    DrawCrosshair(m_Painter);
                    m_Painter.end();
                }
            }
            return image.save(path, "PNG");
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::AddMeasureNode(QSharedPointer<MeasureNode> node)
{
    if (m_HighlightMeasureNode != nullptr) {
        m_HighlightMeasureNode->SetHovered(false);
    }
    m_Measurements.push_back(node);
    m_HighlightMeasureNode = node;
    node->SetHovered(true);
    m_MeasureWnd->AddMeasure(node);
    update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::RemoveMeasureNode(QSharedPointer<MeasureNode> node)
{
    auto i = std::find(m_Measurements.begin(), m_Measurements.end(), node);
    if (i != m_Measurements.end())
    {
        if ( node == m_HighlightMeasureNode )
        {
            node->SetHovered(false);
            m_HighlightMeasureNode.reset();
        }
        if (node == m_MouseCaptureMeasureNode)
        {
            m_MouseCaptureMeasureNode.reset();
        }
        m_Measurements.erase(i);
        UnselectMeasureNode(node);
        update();
        m_MeasureWnd->RemoveMeasure(node);
    }
    UpdateMouseItem(mapFromGlobal(QCursor::pos()));
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::SelectMeasureNode(QSharedPointer<MeasureNode> node, bool exclusive)
{
    if (node != nullptr && node->IsSelected()) return;
    
    disconnect(m_MeasureWnd, &MeasureWnd::SignalSelectionChanged, this, &VideoView::SlotMeasureSelectionChanged);
    if (exclusive) {
        for ( auto i = m_SelectedMeasurements.begin() ; i != m_SelectedMeasurements.end() ; ++i )
        {
            QSharedPointer<MeasureNode> curNode = *i;
            if ( curNode != node ) {
                curNode->SetSelect(false);
                disconnect(curNode.data(), &MeasureNode::SignalBoundingBoxChanged, this, &VideoView::SlotSelectedNodeBoundingBoxChanged);
                m_MeasureWnd->UpdateSelection(curNode, false);
            }
        }
        m_SelectedMeasurements.clear();
    }
    if (node != nullptr)
    {
        m_SelectedMeasurements.push_back(node);
        if (!node->IsSelected())
        {
            node->SetSelect(true);
            connect(node.data(), &MeasureNode::SignalBoundingBoxChanged, this, &VideoView::SlotSelectedNodeBoundingBoxChanged);
            m_MeasureWnd->UpdateSelection(node, true);
        }
    }
    SlotSelectedNodeBoundingBoxChanged();
    update();
    connect(m_MeasureWnd, &MeasureWnd::SignalSelectionChanged, this, &VideoView::SlotMeasureSelectionChanged);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::UnselectMeasureNode(QSharedPointer<MeasureNode> node)
{
    if (node->IsSelected())
    {
        auto i = std::find(m_SelectedMeasurements.begin(), m_SelectedMeasurements.end(), node);
        if (i != m_SelectedMeasurements.end())
        {
            m_SelectedMeasurements.erase(i);
            node->SetSelect(false);
            m_MeasureWnd->UpdateSelection(node, false);
            update();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::ClearSelection()
{
    SelectMeasureNode(QSharedPointer<MeasureNode>(), true);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::CalibrateCrosshair()
{
    if (m_HoleDetected) {
        m_CrosshairPos = m_HoleCenterFiltered;
        update();
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

int VideoView::GetClosestCrosshairTarget(const PMPoint& relativePos) const
{
    double testRadius = PointLength(relativePos - m_CrosshairPos);

    double closestRadius;
    int closestTarget = -1;
    double closestOffset = std::numeric_limits<double>::max();
    for (int i = 0; i < m_CrosshairTargets.size(); ++i)
    {
        const auto& target = m_CrosshairTargets[i];

        double curRadius = (target.m_SnapToHoleSize && m_HoleDetected) ? m_HoleRadius : target.m_Radius;
        double offset = abs(curRadius - testRadius);
        if (offset < closestOffset)
        {
            closestTarget = i;
            closestOffset = offset;
        }
    }
    return (closestOffset < 5.0 / m_ToViewScale.x()) ? closestTarget : -1;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

QSharedPointer<MeasureNode> VideoView::GetClosestMeasureNode(const PMPoint& relativePos)
{
    QSharedPointer<MeasureNode> closestNode;
    double closestOffset = std::numeric_limits<double>::max();

    for (int i = 0; i < m_Measurements.size(); ++i)
    {
        const auto& node = m_Measurements[i];

        node->MouseMove(relativePos, m_ToViewScale);
        double offset = node->GetDistance();
        if ( offset < closestOffset) {
            closestOffset = offset;
            closestNode = node;
        }
    }
    return (closestOffset < 5.0 / m_ToViewScale.x()) ? closestNode : QSharedPointer<MeasureNode>();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::UpdateMouseItem(const QPointF& mousePos)
{
    PMPoint relativePos((PMPoint(mousePos) + m_ImageOffset) / m_ToViewScale);

    QSharedPointer<MeasureNode> closestMeasureNode = GetClosestMeasureNode(relativePos);
    int closestTarget = -1;
    bool highlightCrosshair = false;
    bool needUpdate = false;

    if (closestMeasureNode == nullptr) {
        closestTarget = GetClosestCrosshairTarget(relativePos);
        if (closestTarget == -1) {
            highlightCrosshair = m_HighlightCrosshairTarget == -1 && (abs(relativePos.x() - m_CrosshairPos.x()) < 5.0 / m_ToViewScale.x() || abs(relativePos.y() - m_CrosshairPos.y()) < 5.0 / m_ToViewScale.x());
        }
    }
    if (closestMeasureNode != m_HighlightMeasureNode || closestTarget != m_HighlightCrosshairTarget || highlightCrosshair != m_IsCrosshairHighlighted)
    {
        if (closestMeasureNode != m_HighlightMeasureNode)
        {
            if (m_HighlightMeasureNode != nullptr) {
                m_HighlightMeasureNode->SetHovered(false);
            }
            m_HighlightMeasureNode = closestMeasureNode;
            if (m_HighlightMeasureNode != nullptr) {
                m_HighlightMeasureNode->SetHovered(true);
            }
        }
        m_HighlightCrosshairTarget = closestTarget;
        m_IsCrosshairHighlighted   = highlightCrosshair;
        update();
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::paintEvent(QPaintEvent* event)
{
    if (!m_CurrentRawImage.isNull())
    {
        m_Painter.begin(this);

//        m_Painter.setRenderHint(QPainter::SmoothPixmapTransform);
        m_Painter.setRenderHint(QPainter::Antialiasing);
        m_Painter.setRenderHint(QPainter::TextAntialiasing);
        m_Painter.setTransform(m_ImageTransform);
        m_Painter.drawImage(QPoint(0, 0), m_CurrentRawImage);

        m_Painter.setTransform(m_RenderTransform);
        DrawCrosshair(m_Painter);

        if ( m_IsSelecting || m_SelectedMeasurements.size() > 1 )
        {
            QPen pen;
            pen.setCosmetic(true);
            pen.setColor(Qt::white);
            QVector<qreal> dashPattern(2);
            dashPattern[0] = 4.0;
            dashPattern[1] = 4.0;
            pen.setDashPattern(dashPattern);
            pen.setDashOffset(-(m_DashOffset % 8));
            m_Painter.setPen(pen);
            m_Painter.setBrush(Qt::NoBrush);
            if ( m_IsSelecting ) {
                m_Painter.drawRect(m_SelectFrame);
            } else {
                m_Painter.drawRect(m_SelectionBoundingBox);
            }
        }
        m_Painter.end();
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::resizeEvent(QResizeEvent* event)
{
    UpdateTransform();
    update();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::mousePressEvent(QMouseEvent* event)
{
    PMPoint relativePos((PMPoint(event->localPos()) + m_ImageOffset) / m_ToViewScale);

    double selectPadding = 3.0 / m_ToViewScale.x();

    if (m_MouseCaptureMeasureNode != nullptr)
    {
        m_MouseCaptureMeasureNode->MouseDown(event->button(), relativePos, m_ToViewScale);
    }
    else
    {
        m_HitPos = event->localPos();
        if (event->button() == Qt::LeftButton)
        {
            if ((event->modifiers() & Qt::AltModifier) || m_MeasureModeActions->checkedAction() == m_MeasureDistanceAction )
            {
                QString name = QString::asprintf("L%02d", m_LastMeasureID);
                MeasureLength::Create(this, m_LastMeasureID, name, relativePos);
                m_LastMeasureID++;
            }
            else if (m_MeasureModeActions->checkedAction() == m_MeasureAreaAction)
            {
                QString name = QString::asprintf("A%02d", m_LastMeasureID);
                MeasureArea::Create(this, m_LastMeasureID, name, relativePos);
                m_LastMeasureID++;
            }
            else if (m_MeasureModeActions->checkedAction() == m_MeasureCircleCenterAction || m_MeasureModeActions->checkedAction() == m_MeasureCircleEdgeAction)
            {
                QString name = QString::asprintf("C%02d", m_LastMeasureID);
                MeasureCircle::Create(this, m_LastMeasureID, name, relativePos, m_MeasureModeActions->checkedAction() == m_MeasureCircleEdgeAction);
                m_LastMeasureID++;
            }
            else
            {
                if ( m_HighlightMeasureNode != nullptr )
                {
                    if ((event->modifiers() & Qt::ControlModifier) )
                    {
                        if ( m_HighlightMeasureNode->IsSelected() ) {
                            UnselectMeasureNode(m_HighlightMeasureNode);
                        } else {
                            SelectMeasureNode(m_HighlightMeasureNode, false);
                        }
                    }
                    else
                    {
                        SelectMeasureNode(m_HighlightMeasureNode, true);
                    }
                    if (m_SelectedMeasurements.size() == 1) {
                        m_HighlightMeasureNode->MouseDown(event->button(), relativePos, m_ToViewScale);
                    } else {
                        m_MouseHandler = QSharedPointer<MouseHandler>(new MouseHandlerMultiSelectMove(this, relativePos, m_ToViewScale));
                    }
                }
                else if (m_SelectedMeasurements.size() > 1 && m_SelectionBoundingBox.adjusted(-selectPadding, -selectPadding, selectPadding, selectPadding).contains(relativePos) )
                {
                    MouseHandlerMultiSelectScale::ScaleEdge_e edge = MouseHandlerMultiSelectScale::e_EdgeNone;

                    if ( abs(relativePos.x() - m_SelectionBoundingBox.left()) < selectPadding )
                    {
                        if (abs(relativePos.y() - m_SelectionBoundingBox.top()) < selectPadding) {
                            edge = MouseHandlerMultiSelectScale::e_EdgeTopLeft;
                        } else if (abs(relativePos.y() - m_SelectionBoundingBox.bottom()) < selectPadding) {
                            edge = MouseHandlerMultiSelectScale::e_EdgeBottomLeft;
                        } else {
                            edge = MouseHandlerMultiSelectScale::e_EdgeLeft;
                        }
                    }
                    else if (abs(relativePos.x() - m_SelectionBoundingBox.right()) < selectPadding)
                    {
                        if (abs(relativePos.y() - m_SelectionBoundingBox.top()) < selectPadding) {
                            edge = MouseHandlerMultiSelectScale::e_EdgeTopRight;
                        } else if (abs(relativePos.y() - m_SelectionBoundingBox.bottom()) < selectPadding) {
                            edge = MouseHandlerMultiSelectScale::e_EdgeBottomRight;
                        } else {
                            edge = MouseHandlerMultiSelectScale::e_EdgeRight;
                        }
                    }
                    else if (abs(relativePos.y() - m_SelectionBoundingBox.top()) < selectPadding)
                    {
                        edge = MouseHandlerMultiSelectScale::e_EdgeTop;
                    }
                    else if (abs(relativePos.y() - m_SelectionBoundingBox.bottom()) < selectPadding)
                    {
                        edge = MouseHandlerMultiSelectScale::e_EdgeBottom;
                    }
                    if (edge != MouseHandlerMultiSelectScale::e_EdgeNone) {
//                        m_MouseHandler = QSharedPointer<MouseHandler>(new MouseHandlerMultiSelectScale(this, relativePos, m_ToViewScale, edge));
                        m_MouseHandler = QSharedPointer<MouseHandler>(new MouseHandlerMultiSelectRotate(this, relativePos, m_ToViewScale));
                    }
                }
                else
                {
                    ClearSelection();
                    if (m_HighlightCrosshairTarget != -1) {
                        m_MouseHandler = QSharedPointer<MouseHandler>(new MouseHandlerResizeCrosshair(this, relativePos, m_ToViewScale));
                    } else if (m_IsCrosshairHighlighted) {
                        m_MouseHandler = QSharedPointer<MouseHandler>(new MouseHandlerDragCrosshair(this, relativePos, m_ToViewScale));
                    } else {
                        m_MouseHandler = QSharedPointer<MouseHandler>(new MouseHandlerSelect(this, relativePos, m_ToViewScale));
                    }
                }
            }
        }
        else
        {
            m_MouseHandler = QSharedPointer<MouseHandler>(new MouseHandlerPan(this, relativePos, m_ToViewScale));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::mouseReleaseEvent(QMouseEvent* event)
{
    PMPoint relativePos((PMPoint(event->localPos()) + m_ImageOffset) / m_ToViewScale);
    if (m_MouseCaptureMeasureNode != nullptr)
    {
        m_MouseCaptureMeasureNode->MouseUp(event->button(), relativePos, m_ToViewScale);
    }
    else if ( m_MouseHandler != nullptr )
    {
        m_MouseHandler->MouseUp(event->button(), relativePos, m_ToViewScale);
        m_MouseHandler.reset();
    }
    else
    {
        if (event->button() == Qt::LeftButton)
        {
            if (!m_SelectedMeasurements.empty()) {
                m_SelectedMeasurements[0]->MouseUp(event->button(), relativePos, m_ToViewScale);
                m_MeasureWnd->UpdateMeasure(m_SelectedMeasurements[0]);
            }
        }
        UpdateMouseItem(event->localPos());
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::mouseMoveEvent(QMouseEvent* event)
{
    PMPoint relativePos((PMPoint(event->localPos()) + m_ImageOffset) / m_ToViewScale);

    if (m_MouseCaptureMeasureNode != nullptr)
    {
        m_MouseCaptureMeasureNode->MouseMove(relativePos, m_ToViewScale);
        return;
    }

    if ( !(event->buttons() & Qt::LeftButton) )
    {
        UpdateMouseItem(event->localPos());
    }

    if ( m_MouseHandler != nullptr) {
        QSharedPointer<MouseHandler> handler = m_MouseHandler;
        PMPoint delta = relativePos - handler->m_PrevPos;
        handler->MouseMove(relativePos, delta, m_ToViewScale);
        handler->m_PrevPos = PMPoint((PMPoint(event->localPos()) + m_ImageOffset) / m_ToViewScale);
    }
    else if ( m_HighlightMeasureNode != nullptr )
    {
        m_HighlightMeasureNode->MouseMove(relativePos, m_ToViewScale);
        update();
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void VideoView::wheelEvent(QWheelEvent* event)
{
    m_PanOffset /= m_ZoomScale;
    m_ZoomScale *= 1.0 + event->angleDelta().y() * 0.001;
    if (m_ZoomScale < 1.0f) m_ZoomScale = 1.0f;
    m_PanOffset *= m_ZoomScale;
    UpdateTransform();
    update();
}

