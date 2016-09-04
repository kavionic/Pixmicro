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
#include "MeasureWnd.h"
#include "MeasureNode.h"
#include "ValuePrefixes.h"


///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

MeasureItem::MeasureItem(QSharedPointer<MeasureNode> node) : QTreeWidgetItem(), m_Node(node)
{
    Update();
}

void MeasureItem::Update()
{
    QSharedPointer<MeasureNode> node = m_Node;

    if (node != nullptr)
    {
        setText(e_ColName, node->GetName());
        setText(e_ColLength, ValuePrefixes::FormatValue(node->GetLength()));
        setText(e_ColRadius, ValuePrefixes::FormatValue(node->GetRadius()));
        setText(e_ColDiameter, ValuePrefixes::FormatValue(node->GetRadius() * 2.0));
        setText(e_ColArea, ValuePrefixes::FormatArea(node->GetArea()));
        setText(e_ColAngle, ValuePrefixes::FormatAngle(node->GetAngle()));
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

MeasureWnd::MeasureWnd(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi(this);

//    connect(m_SnapshotButton, &QAbstractButton::clicked, this, &MeasureWnd::SlotMakeSnapshot);
    connect(m_DeleteButton, &QAbstractButton::clicked, this, &MeasureWnd::SlotDeleteSnapshot);
    connect(m_ListView, &QTreeWidget::itemSelectionChanged, this, &MeasureWnd::SlotSelectionChanged);

    connect(m_MeasureLength, &QDoubleSpinBox::editingFinished, this, &MeasureWnd::SlotMeasureLengthValueChanged);

    m_ListView->setUniformRowHeights(true);
    m_ListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_ListView->setColumnCount(MeasureItem::e_ColCount);


    QStringList labels;
    labels << "Name";
    labels << "Length";
    labels << "Radius";
    labels << "Diameter";
    labels << "Area";
    labels << "Angle";

    m_ListView->setHeaderLabels(labels);

    m_MeasureLength->setSpecialValueText("---");

    m_DeleteButton->setEnabled(false);

    m_Path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/Pixmicro/";

    connect(&ValuePrefixes::Get(), &ValuePrefixes::SignalSelectedPrefixChanged, this, &MeasureWnd::SlotSelectedPrefixChanged);
    connect(&ValuePrefixes::Get(), &ValuePrefixes::SignalCalibrationChanged, this, &MeasureWnd::SlotCalibrationChanged);
    //    m_ListView->setSortingEnabled(true);



//    QSettings settings;
//    m_SnapshotScale->setValue(settings.value("Gallery/snapshotScale", 1.0).toDouble());
//    m_IncludeOverlayCB->setChecked(settings.value("Gallery/snapshotIncludeOverlay", false).toBool());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

MeasureWnd::~MeasureWnd()
{
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureWnd::SaveSettings()
{
//    QSettings settings;

//    settings.setValue("Gallery/snapshotScale", m_SnapshotScale->value());
//    settings.setValue("Gallery/snapshotIncludeOverlay", m_IncludeOverlayCB->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureWnd::AddMeasure(QSharedPointer<MeasureNode> node)
{
    MeasureItem* item = new MeasureItem(node);
//    m_ListView->setUpdatesEnabled(false);
    m_ListView->addTopLevelItem(item);
//    m_ListView->setUpdatesEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureWnd::RemoveMeasure(QSharedPointer<MeasureNode> node)
{
    for (int i = 0 ; i < m_ListView->topLevelItemCount() ; ++i)
    {
        MeasureItem* item = static_cast<MeasureItem*>(m_ListView->topLevelItem(i));
        if (item->GetNode() == node) {
            delete item;
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureWnd::UpdateMeasure(QSharedPointer<MeasureNode> node)
{
    for ( int i = 0 ; i < m_ListView->topLevelItemCount() ; ++i )
    {
        MeasureItem* item = static_cast<MeasureItem*>(m_ListView->topLevelItem(i));
        if ( item->GetNode() == node ) {
            item->Update();
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureWnd::UpdateSelection(QSharedPointer<MeasureNode>& node, bool isSelected)
{
    for (int i = 0 ; i < m_ListView->topLevelItemCount() ; ++i)
    {
        MeasureItem* item = static_cast<MeasureItem*>(m_ListView->topLevelItem(i));
        if (item->GetNode() == node) {
            item->setSelected(isSelected);
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureWnd::SlotDeleteSnapshot()
{
    MeasureItem* selection = dynamic_cast<MeasureItem*>(m_ListView->currentItem());

    if (selection != nullptr)
    {
        emit SignalMeasureDeleted(selection->GetNode());
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureWnd::SlotSelectionChanged()
{
    auto selectionList = m_ListView->selectedItems();

    if ( selectionList.length() > 0 )
    {
        m_DeleteButton->setEnabled(true);
        std::vector<QSharedPointer<MeasureNode>> nodeList;
        for (int i = 0, l = selectionList.length() ; i < l ; ++i) {
            MeasureItem* item = static_cast<MeasureItem*>(selectionList[i]);
            nodeList.push_back(item->GetNode());
        }
        emit SignalSelectionChanged(nodeList);
    }
    else
    {
        static std::vector<QSharedPointer<MeasureNode>> emptyNodeList;
        m_DeleteButton->setEnabled(false);
        emit SignalSelectionChanged(emptyNodeList);
    }
    UpdateCurrentMeasure();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureWnd::SlotMeasureLengthValueChanged()
{
    MeasureItem* item = static_cast<MeasureItem*>(m_ListView->currentItem());
    if (item != nullptr)
    {
        QSharedPointer<MeasureNode> node = item->GetNode();
        if (node != nullptr)
        {
            ValuePrefixes::Get().SetCalibrationScale(m_MeasureLength->value() * ValuePrefixes::GetPrefixScale(ValuePrefixes::Get().GetSelectedPrefix()) / node->GetLength());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureWnd::SlotCalibrationChanged()
{
    for (int i = 0 ; i < m_ListView->topLevelItemCount() ; ++i)
    {
        MeasureItem* item = static_cast<MeasureItem*>(m_ListView->topLevelItem(i));
        item->Update();
    }
    UpdateCurrentMeasure();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureWnd::SlotSelectedPrefixChanged()
{
    for (int i = 0 ; i < m_ListView->topLevelItemCount() ; ++i)
    {
        MeasureItem* item = static_cast<MeasureItem*>(m_ListView->topLevelItem(i));
        item->Update();
    }
    UpdateCurrentMeasure();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void MeasureWnd::UpdateCurrentMeasure()
{
    auto selectedItems = m_ListView->selectedItems();
    if ( selectedItems.length() == 1 )
    {
        MeasureItem* item = static_cast<MeasureItem*>(selectedItems[0]);
        m_MeasureName->setText(item->GetNode()->GetName());
        if (!m_MeasureLength->hasFocus()) m_MeasureLength->setValue(item->GetNode()->GetLength() * ValuePrefixes::Get().GetCalibrationScale() / ValuePrefixes::GetPrefixScale(ValuePrefixes::Get().GetSelectedPrefix()));
        m_MeasureLength->setSuffix(ValuePrefixes::GetPrefixName(ValuePrefixes::Get().GetSelectedPrefix()));
    }
    else
    {
        m_MeasureName->setText("---");
        m_MeasureLength->setValue(0.0);

    }
}
