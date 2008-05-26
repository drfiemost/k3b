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



#ifndef _K3B_MIXED_DIRTREEVIEW_H_
#define _K3B_MIXED_DIRTREEVIEW_H_

#include <k3bdatadirtreeview.h>
#include <QDropEvent>

class K3bView;
class K3bMixedDoc;
class QDropEvent;
class Q3ListViewItem;


class K3bMixedDirTreeView : public K3bDataDirTreeView
{
    Q_OBJECT

public:
    K3bMixedDirTreeView( K3bView* view, K3bMixedDoc* doc, QWidget* parent = 0 );
    ~K3bMixedDirTreeView();

Q_SIGNALS:
    void audioTreeSelected();
    void dataTreeSelected();

protected Q_SLOTS:
/*     void slotDropped( QDropEvent* e, Q3ListViewItem* after, Q3ListViewItem* parent ); */
  
private Q_SLOTS:
/*     void slotSelectionChanged( Q3ListViewItem* i ); */
/*     void slotNewAudioTracks(); */

private:
    K3bMixedDoc* m_doc;

    class PrivateAudioRootViewItem;
    PrivateAudioRootViewItem* m_audioRootItem;
};


#endif
