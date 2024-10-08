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



#ifndef K3BDISKINFOVIEW_H
#define K3BDISKINFOVIEW_H

#include "k3bmediacontentsview.h"

//class QWebView;

namespace K3b {
class DiskInfoView : public MediaContentsView
{
    Q_OBJECT

public:
    DiskInfoView( QWidget* parent = 0 );
    ~DiskInfoView();

private:
    void reloadMedium();
    void updateTitle();

    QString createMediaInfoItems( const Medium& medium );
    QString createIso9660InfoItems( const Iso9660SimplePrimaryDescriptor& iso );
    QString createTrackItems( const Medium& medium );

    //QWebView* m_infoView;
};
}


#endif
