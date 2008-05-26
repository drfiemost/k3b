/* 
 *
 * Copyright (C) 2006 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef _K3B_TIMEOUT_WIDGET_H_
#define _K3B_TIMEOUT_WIDGET_H_

#include <qwidget.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>

class QPaintEvent;
class QResizeEvent;


class K3bTimeoutWidget : public QWidget
{
  Q_OBJECT

 public:
  K3bTimeoutWidget( QWidget* parent );
  ~K3bTimeoutWidget();

  QSize sizeHint() const;
  QSize minimumSizeHint() const;

 public Q_SLOTS:
  void setTimeout( int msecs );
  void start();
  void stop();
  void pause();
  void resume();

  Q_SIGNALS:
  void timeout();

 protected:
  void paintEvent( QPaintEvent* );
  void resizeEvent( QResizeEvent* );

 private Q_SLOTS:
  void timeStep();
  void startTimer();

 private:
  class Private;
  Private* d;
};

#endif
