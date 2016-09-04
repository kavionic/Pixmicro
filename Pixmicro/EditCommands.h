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

#include "QSharedPointer"
#include "MeasureNode.h"
#include "MeasureLength.h"
#include "MeasureArea.h"
#include "MeasureCircle.h"

class VideoView;

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

class ECmdAddMeasureNode : public QUndoCommand
{
public:
    ECmdAddMeasureNode(VideoView* view, QSharedPointer<MeasureNode> node);

    virtual void undo() override;
    virtual void redo() override;
    
private:
    VideoView*     m_View;
    QSharedPointer<MeasureNode> m_Node;
};

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

class ECmdDeleteMeasure : public QUndoCommand
{
public:
    ECmdDeleteMeasure(VideoView* view, QSharedPointer<MeasureNode> node);
    virtual void undo() override;
    virtual void redo() override;

private:
    VideoView*   m_View;
    QSharedPointer<MeasureNode> m_Node;
};

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

class ECmdMoveMeasurePointListPoint : public QUndoCommand
{
public:
    ECmdMoveMeasurePointListPoint(VideoView* view, MeasurePointList* node, int measurePoint, const PMPoint& startPos, const PMPoint& endPos);

    virtual void undo() override;
    virtual void redo() override;

private:
    MeasurePointList* m_Node;
    int               m_MeasurePoint;
    PMPoint         m_StartPos;
    PMPoint         m_EndPos;
};

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

class ECmdAddMeasurePointListPoint : public QUndoCommand
{
public:
    ECmdAddMeasurePointListPoint(MeasurePointList* node, int index, const PMPoint& pos) : m_Node(node), m_Index(index), m_Pos(pos) {}

    virtual void undo() override { m_Node->RemovePoint(m_Index); }
    virtual void redo() override { m_Node->AddPoint(m_Index, m_Pos); }

private:
    MeasurePointList* m_Node;
    int               m_Index;
    PMPoint           m_Pos;
};

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

class ECmdDeleteMeasurePointListPoint : public QUndoCommand
{
public:
    ECmdDeleteMeasurePointListPoint(MeasurePointList* node, int index) : m_Node(node), m_Index(index) { m_Pos = m_Node->m_MeasurePoints[index]; }

    virtual void undo() override { m_Node->AddPoint(m_Index, m_Pos); }
    virtual void redo() override { m_Node->RemovePoint(m_Index); }

private:
    MeasurePointList* m_Node;
    int               m_Index;
    PMPoint         m_Pos;
};

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

class ECmdDragMeasureCircle : public QUndoCommand
{
public:
    ECmdDragMeasureCircle(VideoView* view, MeasureCircle* node, const PMPoint& startPos, const PMPoint& endPos, double startRadius, double endRadius);

    virtual void undo() override;
    virtual void redo() override;

private:
    VideoView*                m_View;
    MeasureCircle*            m_Node;
    PMPoint                 m_StartPos;
    PMPoint                 m_EndPos;
    double                    m_StartRadius;
    double                    m_EndRadius;
};

