/* 
 *
 * $Id$
 * Copyright (C) 2003 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2003 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef K3BAUDIOVIEW_H
#define K3BAUDIOVIEW_H

#include <k3bview.h>

#include <qstringlist.h>
#include <qptrlist.h>


class K3bAudioListView;
class K3bAudioListViewItem;
class QWidget;
class K3bAudioDoc;
class K3bAudioTrack;
class QListViewItem;
class KListView;
class K3bFillStatusDisplay;
class K3bAudioBurnDialog;
class K3bProjectBurnDialog;


/**
  *@author Sebastian Trueg
  */

class K3bAudioView : public K3bView  {

  Q_OBJECT
	
 public: 
  K3bAudioView( K3bAudioDoc* pDoc, QWidget* parent, const char *name = 0 );
  ~K3bAudioView();

  void burnDialog( bool );

 private:
  K3bAudioDoc* m_doc;
	
  K3bAudioListView* m_songlist;

  K3bFillStatusDisplay* m_fillStatusDisplay;
  K3bAudioBurnDialog* m_burnDialog;
};

#endif
