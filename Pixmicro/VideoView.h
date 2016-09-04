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

#include "MeasureNode.h"

class HoleDetectWnd;
class MeasureWnd;
class MeasureNode;

struct CrosshairTarget
{
    CrosshairTarget(double radius, const QColor& color, bool snapToHoleSize) : m_Radius(radius), m_Color(color), m_SnapToHoleSize(snapToHoleSize) {}
    double m_Radius;
    QColor m_Color;
    bool   m_SnapToHoleSize;
};

class MouseHandler
{
public:
    MouseHandler(VideoView* view, const PMPoint& pos) : m_View(view), m_StartPos(pos), m_PrevPos(pos) {}
//    virtual void MouseDown(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale) = 0;
    virtual void MouseUp(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale) {}
    virtual void MouseMove(const PMPoint& pos, const PMPoint& delta, const PMPoint& currentScale) = 0;

protected:
    friend class VideoView;

    VideoView* m_View;
    PMPoint m_StartPos;
    PMPoint m_PrevPos;
};


class VideoView : public QWidget, public Ui::VideoView
{
    Q_OBJECT

public:
    VideoView(QWidget *parent = 0);
    ~VideoView();

    void AddToolbars(QMainWindow* window);
    QUndoStack* GetUndoStack() const { return m_UndoStack; }

    void SetHoleDetectWnd(HoleDetectWnd* holeDetectWnd);
    void SetMeasureWnd(MeasureWnd* measureWnd);
    void SaveSettings();

    void Update(QImage* image, bool fromCamera);
    void CalibrateCrosshair();
    bool TakeSnapshot(const QString& path, double scale, bool withOverlay);

    void AddMeasureNode(QSharedPointer<MeasureNode> node);
    void RemoveMeasureNode(QSharedPointer<MeasureNode> node);

    void SelectMeasureNode(QSharedPointer<MeasureNode> node, bool exclusive);
    void UnselectMeasureNode(QSharedPointer<MeasureNode> node);
    void ClearSelection();

    QFontMetrics GetLabelFontMetrics() const { return m_Painter.fontMetrics(); }

    virtual void paintEvent(QPaintEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;

    virtual void wheelEvent(QWheelEvent* event) override;

private slots:
    void SlotEnableChanged(bool enabled);
    void SlotFreezeHoleColorChanged(bool freeze);
    void SlotThresholdChanged(double value);
    void SlotRoundnessChanged(double value);
    void SlotMarkerColorChanged(const QColor& color);
    void SlotMarkerWidthChanged(int value);
    void SlotFlipHToggled(bool flip);
    void SlotFlipVToggled(bool flip);

    void SlotSelectedNodeBoundingBoxChanged();
    void SlotSelectionAnimationTimer();

    void SlotUnitsChanged(int index);
    void SlotMeasureSelectionChanged(const std::vector<QSharedPointer<MeasureNode>>& list);
    void SlotMeasureDeleted(QSharedPointer<MeasureNode> measureNode);

    void SlotMeasureDistance(bool checked);
    void SlotInsertPoint();

private:
    friend class MeasureNode;
    friend class MouseHandlerSelect;
    friend class MouseHandlerDragCrosshair;
    friend class MouseHandlerResizeCrosshair;
    friend class MouseHandlerPan;
    friend class MouseHandlerMultiSelectMove;
    friend class MouseHandlerMultiSelectScale;
    friend class MouseHandlerMultiSelectRotate;

    void      UpdateTransform();
    void      DrawCrosshair(QPainter& painter);
    PMPoint   FindEdge(const QPointF& startPos, double dirX, double dirY, int maxSearchRadius);
    void      DetectHole();
    int       GetClosestCrosshairTarget(const PMPoint& relativePos) const;
    QSharedPointer<MeasureNode> GetClosestMeasureNode(const PMPoint& relativePos);
    void      UpdateMouseItem(const QPointF& mousePos);

    int m_LastMeasureID;



    HoleDetectWnd* m_HoleDetectWnd;
    MeasureWnd*    m_MeasureWnd;
    QComboBox*     m_UnitsDropdown;

    QActionGroup* m_MeasureModeActions;

    QAction* m_MeasureNoneAction;
    QAction* m_MeasureDistanceAction;
    QAction* m_MeasureAreaAction;
    QAction* m_MeasureCircleCenterAction;
    QAction* m_MeasureCircleEdgeAction;
    QAction* m_MeasureAngleAction;

    QShortcut* m_InsertPointShortcut;

    QUndoStack* m_UndoStack;

    QImage  m_CurrentRawImage;

    QPainter m_Painter;

    bool    m_HoleDetectEnabled;
    bool    m_HoleColorFrozen;
    double  m_Threshold;
    double  m_Roundness;

    QColor m_HoleMarkerColor;
    int    m_HoleMarkerThickness;

    PMPoint  m_ToImageScale;      // Scale from normalized coordinates to raw image coordinates.
    PMPoint  m_ToViewScale;       // Scale from normalized coordinates to scaled/zoomed image (view) coordinates.
    PMPoint  m_BitmapToViewScale; // Scale from raw image coordinates to scaled/zoomed image coordinates.
    PMPoint  m_ImageOffset;       // Scroll offset ((0, 0) is centered).
    QTransform m_ImageTransform;
    QTransform m_RenderTransform;

    bool    m_FlipH;
    bool    m_FlipV;

    QSharedPointer<MouseHandler> m_MouseHandler;

    bool m_IsSelecting = false;
    QRectF m_SelectFrame;

    bool    m_IsCrosshairHighlighted;
    int     m_HighlightCrosshairTarget;
    QPointF m_HitPos;

    double    m_ZoomScale;
    PMPoint   m_PanOffset;

    std::vector<QSharedPointer<MeasureNode>> m_Measurements;
    std::vector<QSharedPointer<MeasureNode>> m_SelectedMeasurements;
    QSharedPointer<MeasureNode>              m_HighlightMeasureNode;
    QSharedPointer<MeasureNode>              m_MouseCaptureMeasureNode;

    QRectF m_SelectionBoundingBox;
    QTimer m_SelectionAnimationTimer;
    int    m_DashOffset = 0;

    PMPoint m_CrosshairPos;

    std::vector<CrosshairTarget> m_CrosshairTargets;

    QVector3D m_HoleCenterColor;
    bool      m_HoleDetected;
    PMPoint   m_HoleCenter;
    PMPoint   m_HoleCenterFiltered;
    double    m_HoleRadius;
};

class MouseHandlerSelect : public MouseHandler
{
public:
    MouseHandlerSelect(VideoView* view, const PMPoint& pos, const PMPoint& currentScale) : MouseHandler(view, pos)
    {
        m_View->m_IsSelecting = true;
        m_View->m_SelectFrame.setTopLeft(pos);
        m_View->m_SelectFrame.setBottomRight(pos);
    }
    virtual void MouseUp(Qt::MouseButton mouseButton, const PMPoint& pos, const PMPoint& currentScale)
    {
        m_View->m_IsSelecting = false;
        m_View->update();
    }

    virtual void MouseMove(const PMPoint& pos, const PMPoint& delta, const PMPoint& currentScale) override
    {
        m_View->m_SelectFrame.setBottomRight(pos);

        QRectF selectFrame = m_View->m_SelectFrame.normalized();

        std::vector<QSharedPointer<MeasureNode>> selectedMeasurements;
        for ( auto i : m_View->m_Measurements )
        {
            if (i->Intersects(selectFrame)) {
                selectedMeasurements.push_back(i);
            }
        }
        m_View->SlotMeasureSelectionChanged(selectedMeasurements);
        m_View->update();
    }
};


class MouseHandlerDragCrosshair : public MouseHandler
{
public:
    MouseHandlerDragCrosshair(VideoView* view, const PMPoint& pos, const PMPoint& currentScale) : MouseHandler(view, pos) {}

    virtual void MouseMove(const PMPoint& pos, const PMPoint& delta, const PMPoint& currentScale) override
    {
        m_View->m_CrosshairPos += delta;
        m_View->DetectHole();
        m_View->update();
    }
};

class MouseHandlerResizeCrosshair : public MouseHandler
{
public:
    MouseHandlerResizeCrosshair(VideoView* view, const PMPoint& pos, const PMPoint& currentScale) : MouseHandler(view, pos) {
        m_View->m_CrosshairTargets[m_View->m_HighlightCrosshairTarget].m_Radius = PointLength(pos - m_View->m_CrosshairPos);
    }

    virtual void MouseMove(const PMPoint& pos, const PMPoint& delta, const PMPoint& currentScale) override
    {
        m_View->m_CrosshairTargets[m_View->m_HighlightCrosshairTarget].m_Radius = PointLength(pos - m_View->m_CrosshairPos);
        m_View->update();
    }
};

class MouseHandlerPan : public MouseHandler
{
public:
    MouseHandlerPan(VideoView* view, const PMPoint& pos, const PMPoint& currentScale) : MouseHandler(view, pos) {}

    virtual void MouseMove(const PMPoint& pos, const PMPoint& delta, const PMPoint& currentScale) override
    {
        m_View->m_PanOffset += delta * currentScale;
        m_View->UpdateTransform();
        m_View->update();
    }
};

class MouseHandlerMultiSelectMove : public MouseHandler
{
public:
    MouseHandlerMultiSelectMove(VideoView* view, const PMPoint& pos, const PMPoint& currentScale) : MouseHandler(view, pos) {}

    virtual void MouseMove(const PMPoint& pos, const PMPoint& delta, const PMPoint& currentScale) override
    {
        QMatrix transform;
        transform.translate(delta.x(), delta.y());

        for (auto node : m_View->m_SelectedMeasurements)
        {
            node->Transform(transform);
        }
        m_View->update();
    }
};

class MouseHandlerMultiSelectScale : public MouseHandler
{
public:
    enum ScaleEdge_e
    {
        e_EdgeNone,
        e_EdgeLeft,
        e_EdgeRight,
        e_EdgeTop,
        e_EdgeBottom,
        e_EdgeTopLeft,
        e_EdgeTopRight,
        e_EdgeBottomLeft,
        e_EdgeBottomRight
    };
    MouseHandlerMultiSelectScale(VideoView* view, const PMPoint& pos, const PMPoint& currentScale, ScaleEdge_e edge) : MouseHandler(view, pos), m_Edge(edge) {}

    virtual void MouseMove(const PMPoint& pos, const PMPoint& delta, const PMPoint& currentScale) override
    {
        QMatrix transform;

        PMPoint offset;
        PMPoint scale(1.0, 1.0);
        switch (m_Edge)
        {
        case MouseHandlerMultiSelectScale::e_EdgeNone:
            break;
        case MouseHandlerMultiSelectScale::e_EdgeLeft:
            transform.translate(m_View->m_SelectionBoundingBox.right(), 0.0);
            transform.scale(1.0 - delta.x() / m_View->m_SelectionBoundingBox.width(), 1.0);
            transform.translate(-m_View->m_SelectionBoundingBox.right(), 0.0);
            break;
        case MouseHandlerMultiSelectScale::e_EdgeRight:
            transform.translate(m_View->m_SelectionBoundingBox.left(), 0.0);
            transform.scale(1.0 + delta.x() / m_View->m_SelectionBoundingBox.width(), 1.0);
            transform.translate(-m_View->m_SelectionBoundingBox.left(), 0.0);
            break;
        case MouseHandlerMultiSelectScale::e_EdgeTop:
            transform.translate(0.0, m_View->m_SelectionBoundingBox.bottom());
            transform.scale(1.0, 1.0 - delta.y() / m_View->m_SelectionBoundingBox.height());
            transform.translate(0.0, -m_View->m_SelectionBoundingBox.bottom());
            break;
        case MouseHandlerMultiSelectScale::e_EdgeBottom:
            transform.translate(0.0, m_View->m_SelectionBoundingBox.top());
            transform.scale(1.0, 1.0 + delta.y() / m_View->m_SelectionBoundingBox.height());
            transform.translate(0.0, -m_View->m_SelectionBoundingBox.top());
            break;
        case MouseHandlerMultiSelectScale::e_EdgeTopLeft:
            transform.translate(m_View->m_SelectionBoundingBox.right(), m_View->m_SelectionBoundingBox.bottom());
            transform.scale(1.0 - delta.x() / m_View->m_SelectionBoundingBox.width(), 1.0 - delta.y() / m_View->m_SelectionBoundingBox.height());
            transform.translate(-m_View->m_SelectionBoundingBox.right(), -m_View->m_SelectionBoundingBox.bottom());
            break;
        case MouseHandlerMultiSelectScale::e_EdgeTopRight:
            transform.translate(m_View->m_SelectionBoundingBox.left(), m_View->m_SelectionBoundingBox.bottom());
            transform.scale(1.0 + delta.x() / m_View->m_SelectionBoundingBox.width(), 1.0 - delta.y() / m_View->m_SelectionBoundingBox.height());
            transform.translate(-m_View->m_SelectionBoundingBox.left(), -m_View->m_SelectionBoundingBox.bottom());
            break;
        case MouseHandlerMultiSelectScale::e_EdgeBottomLeft:
            transform.translate(m_View->m_SelectionBoundingBox.right(), m_View->m_SelectionBoundingBox.top());
            transform.scale(1.0 - delta.x() / m_View->m_SelectionBoundingBox.width(), 1.0 + delta.y() / m_View->m_SelectionBoundingBox.height());
            transform.translate(-m_View->m_SelectionBoundingBox.right(), -m_View->m_SelectionBoundingBox.top());
            break;
        case MouseHandlerMultiSelectScale::e_EdgeBottomRight:
            transform.translate(m_View->m_SelectionBoundingBox.left(), m_View->m_SelectionBoundingBox.top());
            transform.scale(1.0 + delta.x() / m_View->m_SelectionBoundingBox.width(), 1.0 + delta.y() / m_View->m_SelectionBoundingBox.height());
            transform.translate(-m_View->m_SelectionBoundingBox.left(), -m_View->m_SelectionBoundingBox.top());
            break;
        default:
            break;
        }

        for (auto node : m_View->m_SelectedMeasurements) {
            node->Transform(transform);
        }
        m_View->update();
    }

private:
    ScaleEdge_e m_Edge;
};

class MouseHandlerMultiSelectRotate : public MouseHandler
{
public:
    MouseHandlerMultiSelectRotate(VideoView* view, const PMPoint& pos, const PMPoint& currentScale) : MouseHandler(view, pos) {}

    virtual void MouseMove(const PMPoint& pos, const PMPoint& delta, const PMPoint& currentScale) override
    {
        QMatrix transform;

        PMPoint center(m_View->m_SelectionBoundingBox.center());
        PMPoint vector1 = pos - delta - center;
        PMPoint vector2 = pos - center;
        transform.translate(center.x(), center.y());
        transform.rotate((atan2(vector2.x(), -vector2.y()) - atan2(vector1.x(), -vector1.y())) * 180.0 / M_PI);
        transform.translate(-center.x(), -center.y());


        for (auto node : m_View->m_SelectedMeasurements)
        {
            node->Transform(transform);
        }
        m_View->update();
    }
};

#endif // VIDEOVIEW_H
