/* 
 *
 * $Id: $
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

#include "k3btoolbox.h"

#include <kaction.h>

#include <qtoolbutton.h>
#include <qsizepolicy.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qlabel.h>


K3bToolBox::K3bToolBox( QWidget* parent, const char* name )
  : QFrame( parent, name )
{
  setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed) );

  m_mainLayout = new QGridLayout( this );
  m_mainLayout->setMargin( 1 );
  m_mainLayout->setSpacing( 0 );
}


K3bToolBox::~K3bToolBox()
{
}


void K3bToolBox::addButton( KAction* action )
{
  QToolButton* button = addClearButton( action );

  connect( button, SIGNAL(clicked()), action, SLOT(activate()) );
  connect( action, SIGNAL(enabled(bool)), button, SLOT(setEnabled(bool)) );
}


void K3bToolBox::addSpacing()
{
  int lastStretch = m_mainLayout->colStretch( m_mainLayout->numCols()-1 );
  m_mainLayout->setColStretch( m_mainLayout->numCols()-1, 0 );
  m_mainLayout->addColSpacing( m_mainLayout->numCols()-1, 5 );
  m_mainLayout->setColStretch( m_mainLayout->numCols(), lastStretch );
}


void K3bToolBox::addLabel( const QString& text )
{
  QLabel* label = new QLabel( text, this );
  label->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );

  addWidget( label );
}


void K3bToolBox::addWidget( QWidget* w )
{
  // TODO: reparent??
  m_mainLayout->setColStretch( m_mainLayout->numCols()-1, 0 );

  m_mainLayout->addWidget( w, 0, m_mainLayout->numCols()-1 );

  if( w->sizePolicy().horData() == QSizePolicy::Fixed || w->sizePolicy().horData() == QSizePolicy::Maximum )
    m_mainLayout->setColStretch( m_mainLayout->numCols(), 1 );
  else {
    m_mainLayout->setColStretch( m_mainLayout->numCols()-1, 1 );
    m_mainLayout->setColStretch( m_mainLayout->numCols(), 0 );
  }
}


void K3bToolBox::addToggleButton( KToggleAction* action )
{
  QToolButton* button = addClearButton( action );
  button->setToggleButton( true );

  if( action->isChecked() )
    button->toggle();

  connect( action, SIGNAL(toggled(bool)), button, SLOT(toggle()) );
  connect( button, SIGNAL(toggled(bool)), action, SLOT(setChecked(bool)) );
  connect( action, SIGNAL(enabled(bool)), button, SLOT(setEnabled(bool)) );
}


QToolButton* K3bToolBox::addClearButton( KAction* action )
{
  QToolButton* button = new QToolButton( this );
  button->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );

  button->setIconSet( action->iconSet() );
  button->setTextLabel( action->toolTip(), true );
  button->setTextLabel( action->text() );
  QWhatsThis::add( button, action->whatsThis() );

  button->setAutoRaise( true );

  addWidget( button );

  return button;
}


#include "k3btoolbox.moc"
