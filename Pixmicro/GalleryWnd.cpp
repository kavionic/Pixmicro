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
#include "GalleryWnd.h"


///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

GalleryWnd::GalleryWnd(QWidget *parent)
    : QDockWidget(parent)
{
    setupUi(this);

    connect(m_SnapshotButton, &QAbstractButton::clicked, this, &GalleryWnd::SlotMakeSnapshot);
    connect(m_DeleteButton, &QAbstractButton::clicked, this, &GalleryWnd::SlotDeleteSnapshot);
    connect(m_ImageListView, &QListWidget::currentItemChanged, this, &GalleryWnd::SlotSelectionChanged);

    m_DeleteButton->setEnabled(false);

    m_Path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/Pixmicro/";

    m_ImageListView->setSortingEnabled(true);

    GalleryItem* cameraItem = new GalleryItem("<Camera>", true);
    m_ImageListView->addItem(cameraItem);
    m_ImageListView->setItemSelected(cameraItem, true);

    QDir dir(m_Path, "*.png");

    QFileInfoList fileList = dir.entryInfoList();
    for (int i = 0; i < fileList.size(); ++i) {
        auto fileInfo = fileList.at(i);
        m_ImageListView->addItem(new GalleryItem(fileInfo.fileName(), false));
    }

    QSettings settings;
    m_SnapshotScale->setValue(settings.value("Gallery/snapshotScale", 1.0).toDouble());
    m_IncludeOverlayCB->setChecked(settings.value("Gallery/snapshotIncludeOverlay", false).toBool());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

GalleryWnd::~GalleryWnd()
{
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void GalleryWnd::SaveSettings()
{
    QSettings settings;

    settings.setValue("Gallery/snapshotScale", m_SnapshotScale->value());
    settings.setValue("Gallery/snapshotIncludeOverlay", m_IncludeOverlayCB->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void GalleryWnd::SlotMakeSnapshot()
{
    QDir dir(m_Path);
    if (dir.mkpath("."))
    {
        QString baseName = "PM" + QDateTime(QDate::currentDate(), QTime::currentTime()).toString("yy?!?MM?!?dd_hh?!?mm?!?ss").replace("?!?", "");

        for (int i = 0 ; ; ++i)
        {
            QString fileName = baseName;
            if ( i != 0 ) fileName += QString::asprintf("-%d", i);
            fileName += ".png";

            QString path = m_Path + fileName;
            if (!dir.exists(path))
            {
                bool result = false;
                emit SignalMakeSnapshot(&result, path, m_SnapshotScale->value(), m_IncludeOverlayCB->isChecked());
                if (result) {
                    m_ImageListView->addItem(new GalleryItem(fileName, false));
                } else {
                    QMessageBox(QMessageBox::Icon::Warning, "Error!", QString("Failed to create \"")  + path + QString("\"!"), QMessageBox::Ok, this).exec();
                }
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void GalleryWnd::SlotDeleteSnapshot()
{
    QListWidgetItem* selection = m_ImageListView->currentItem();

    if (selection != nullptr)
    {
        QString path = m_Path + selection->text();
        QMessageBox dlg(QMessageBox::Icon::Warning, "Warning!", QString("Are you sure you want to permanently delete \"")  + path + QString("\"?"), QMessageBox::Ok | QMessageBox::Cancel, this);

        if (dlg.exec() == QMessageBox::Ok)
        {
            if ( QDir(m_Path).remove(selection->text()) )
            {
                delete selection;
            }
            else
            {
                QMessageBox(QMessageBox::Icon::Warning, "Error!", QString("Failed to delete \"")  + path + QString("\"!"), QMessageBox::Ok, this).exec();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void GalleryWnd::SlotSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    GalleryItem* item = dynamic_cast<GalleryItem*>(current);

    if (item != nullptr)
    {
        if (item->m_IsCamera) {
            emit SignalSelectionChanged(QString::null);
            m_DeleteButton->setEnabled(false);
        } else {
            m_DeleteButton->setEnabled(true);
            emit SignalSelectionChanged(m_Path + current->text());
        }
    }
}
