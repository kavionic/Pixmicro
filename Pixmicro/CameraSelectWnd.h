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

#ifndef CAMERASELECTWND_H
#define CAMERASELECTWND_H

#include <QWidget>
#include "ui_CameraSelectWnd.h"

class CameraSelectWnd : public QDockWidget, public Ui::CameraSelectWnd
{
    Q_OBJECT

private slots:
    void SlotCameraSelectionChanged(int index);
    void SlotSelectedCameraChanged(const QCameraInfo& cameraInfo);
    void SlotFlipHorizontallyToggled(bool checked);
    void SlotFlipVerticallyToggled(bool checked);
public:
    CameraSelectWnd(QWidget *parent = 0);
    ~CameraSelectWnd();
};

#endif // CAMERASELECTWND_H
