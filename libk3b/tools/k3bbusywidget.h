/* 
 *
 * Copyright (C) 2003-2008 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2008 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef K3B_BUSY_WIDGET_H
#define K3B_BUSY_WIDGET_H


#include <q3frame.h>
#include "k3b_export.h"

class QTimer;


class LIBK3B_EXPORT K3bBusyWidget : public QFrame
{
    Q_OBJECT

public:
    K3bBusyWidget( QWidget* parent = 0 );
    ~K3bBusyWidget();

    void showBusy( bool b );

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    void paintEvent( QPaintEvent* );

private Q_SLOTS:
    void animateBusy();

private:
    bool m_bBusy;
    int m_iBusyPosition;

    QTimer* m_busyTimer;
};


#endif
