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

#ifndef EDGEDETECTWND_H
#define EDGEDETECTWND_H

#include <QDockWidget>
#include "ui_EdgeDetectWnd.h"

class EdgeDetectWnd : public QDockWidget, public Ui::EdgeDetectWnd
{
    Q_OBJECT

private slots:
    void SlotEnableCBChanged();
    void SlotMinThresValueChanged(int value);
    void SlotMaxThresValueChanged(int value);
    void SlotColorButtonClicked();
    void SlotColorChanged(const QColor& color);

public:
    EdgeDetectWnd(QWidget *parent = 0);
    ~EdgeDetectWnd();
};

#endif // EDGEDETECTWND_H
