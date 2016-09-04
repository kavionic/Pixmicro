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

#ifndef MEASUREWND_H
#define MEASUREWND_H

#include <QDockWidget>
#include "ui_MeasureWnd.h"

class MeasureNode;

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

class MeasureItem : public QTreeWidgetItem
{
public:
    enum Columns_e
    {
        e_ColName,
        e_ColLength,
        e_ColRadius,
        e_ColDiameter,
        e_ColArea,
        e_ColAngle,
        e_ColCount
    };

    MeasureItem(QSharedPointer<MeasureNode> node);

    void Update();

    QSharedPointer<MeasureNode> GetNode() const { return m_Node; }
//    int32_t GetID() const { return m_ID; }

private:
    QWeakPointer<MeasureNode> m_Node;
};

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

class MeasureWnd : public QDockWidget, public Ui::MeasureWnd
{
    Q_OBJECT

public:

    MeasureWnd(QWidget *parent = 0);
    ~MeasureWnd();
    void SaveSettings();

    void AddMeasure(QSharedPointer<MeasureNode> node);
    void RemoveMeasure(QSharedPointer<MeasureNode> node);
    void UpdateMeasure(QSharedPointer<MeasureNode> node);
    void UpdateSelection(QSharedPointer<MeasureNode>& node, bool isSelected);

signals:
    void SignalMeasureDeleted(QSharedPointer<MeasureNode>);
    void SignalSelectionChanged(const std::vector<QSharedPointer<MeasureNode>>& nodeList);

private slots:
    void SlotDeleteSnapshot();
    void SlotSelectionChanged();
    void SlotMeasureLengthValueChanged();
    void SlotCalibrationChanged();
    void SlotSelectedPrefixChanged();
    void UpdateCurrentMeasure();

private:
    QString m_Path;
};

#endif // MEASUREWND_H
