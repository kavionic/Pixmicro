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

#ifndef GALLERYWND_H
#define GALLERYWND_H

#include <QDockWidget>
#include "ui_GalleryWnd.h"

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

class GalleryItem : public QListWidgetItem
{
public:
    GalleryItem(const QString& name, bool isCamera) : QListWidgetItem(name), m_IsCamera(isCamera) {}

    virtual bool operator<(const QListWidgetItem &other) const
    {
        const GalleryItem* otherItem = dynamic_cast<const GalleryItem*>(&other);
        if ( otherItem != nullptr)
        {
            if ( otherItem->m_IsCamera != m_IsCamera ) {
                return m_IsCamera;
            } else {
                return text() > otherItem->text();
            }
        }
        return false;
    }

    bool m_IsCamera;
};

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

class GalleryWnd : public QDockWidget, public Ui::GalleryWnd
{
    Q_OBJECT

public:
    GalleryWnd(QWidget *parent = 0);
    ~GalleryWnd();
    void SaveSettings();

signals:
    void SignalMakeSnapshot(bool* result, const QString& path, double scale, bool includeOverlay);
    void SignalSelectionChanged(const QString& path);

private slots:
    void SlotMakeSnapshot();
    void SlotDeleteSnapshot();
    void SlotSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    QString m_Path;
};

#endif // GALLERYWND_H
