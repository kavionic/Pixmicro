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

#include "EditCommands.h"
#include "VideoView.h"

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

ECmdAddMeasureNode::ECmdAddMeasureNode(VideoView* view, QSharedPointer<MeasureNode> node)
{
    m_View = view;
    m_Node = node;
    setText(QObject::tr("Create measure '%1'").arg(m_Node->GetName()));
}

void ECmdAddMeasureNode::undo()
{
    m_View->RemoveMeasureNode(m_Node);
}

void ECmdAddMeasureNode::redo()
{
    m_View->AddMeasureNode(m_Node);
    m_View->SelectMeasureNode(m_Node, true);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

ECmdDeleteMeasure::ECmdDeleteMeasure(VideoView* view, QSharedPointer<MeasureNode> node)
{
    m_View = view;
    m_Node = node;
    setText(QObject::tr("Delete measure '%1'").arg(node->GetName()));
}

void ECmdDeleteMeasure::undo()
{
    m_View->AddMeasureNode(m_Node);
    m_View->SelectMeasureNode(m_Node, true);
}

void ECmdDeleteMeasure::redo()
{
    m_View->RemoveMeasureNode(m_Node);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

ECmdMoveMeasurePointListPoint::ECmdMoveMeasurePointListPoint(VideoView* view, MeasurePointList* node, int measurePoint, const PMPoint& startPos, const PMPoint& endPos) : m_StartPos(startPos), m_EndPos(endPos)
{
    m_Node = node;
    m_MeasurePoint = measurePoint;
    setText(QObject::tr("Move length measure '%1'").arg(node->GetName()));
}

void ECmdMoveMeasurePointListPoint::undo()
{
    m_Node->MovePoint(m_MeasurePoint, m_StartPos);
}

void ECmdMoveMeasurePointListPoint::redo()
{
    m_Node->MovePoint(m_MeasurePoint, m_EndPos);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

ECmdDragMeasureCircle::ECmdDragMeasureCircle(VideoView* view, MeasureCircle* node, const PMPoint& startPos, const PMPoint& endPos, double startRadius, double endRadius) : m_StartPos(startPos), m_EndPos(endPos), m_StartRadius(startRadius), m_EndRadius(endRadius)
{
    m_View = view;
    m_Node = node;
    setText(QObject::tr("Move circle measure '%1'").arg(node->GetName()));
}

void ECmdDragMeasureCircle::undo()
{
    m_Node->SetPosAndRadius(m_StartPos, m_StartRadius);
}

void ECmdDragMeasureCircle::redo()
{
    m_Node->SetPosAndRadius(m_EndPos, m_EndRadius);
}

