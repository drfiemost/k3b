/*
 *
 * Copyright (C) 2003-2008 Sebastian Trueg <trueg@k3b.org>
 * Copyright (C) 2009      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (C) 2009-2010 Michal Malek <michalm@jabster.pl>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "k3bmixedview.h"
#include "k3baudiodoc.h"
#include "k3baudioprojectmodel.h"
//#include "k3baudiotrackplayer.h"
#include "k3baudioviewimpl.h"
#include "k3bdatadoc.h"
#include "k3bdataprojectmodel.h"
#include "k3bdataviewimpl.h"
#include "k3bdirproxymodel.h"
#include "k3bmetaitemmodel.h"
#include "k3bmixeddoc.h"
#include "k3bmixedburndialog.h"
#include "k3bvolumenamewidget.h"

#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KLocale>
#include <KMenu>
#include <KMessageBox>
#include <KToolBar>

#include <QSplitter>
#include <QStackedWidget>
#include <QTreeView>

K3b::MixedView::MixedView( K3b::MixedDoc* doc, QWidget* parent )
:
    View( doc, parent ),
    m_doc( doc ),
    m_audioViewImpl( new AudioViewImpl( this, m_doc->audioDoc(), actionCollection() ) ),
    m_dataViewImpl( new DataViewImpl( this, m_doc->dataDoc(), actionCollection() ) ),
    m_model( new MetaItemModel( this ) ),
    m_dirProxy( new DirProxyModel( this ) ),
    m_dirView( new QTreeView( this ) ),
    m_fileViewWidget( new QStackedWidget( this ) )
{
    m_model->addSubModel( i18n("Data Section"), KIcon("media-optical-data"), m_dataViewImpl->model(), true );
    m_model->addSubModel( i18n("Audio Section"), KIcon("media-optical-audio"), m_audioViewImpl->model() );
    m_dirProxy->setSourceModel( m_model );

    // Dir panel
    m_dirView->setHeaderHidden( true );
    m_dirView->setAcceptDrops( true );
    m_dirView->setDragEnabled( true );
    m_dirView->setDragDropMode( QTreeView::DragDrop );
    m_dirView->setSelectionMode( QTreeView::SingleSelection );
    m_dirView->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
    m_dirView->setModel( m_dirProxy );
    m_dirView->expandToDepth( 1 ); // Show first-level directories directories by default

    m_fileViewWidget->addWidget( m_dataViewImpl->view() );
    m_fileViewWidget->addWidget( m_audioViewImpl->view() );

    QSplitter* splitter = new QSplitter( this );
    splitter->addWidget( m_dirView );
    splitter->addWidget( m_fileViewWidget );
    splitter->setStretchFactor( 0, 1 );
    splitter->setStretchFactor( 1, 3 );
    setMainWidget( splitter );

    connect( actionCollection()->action( "parent_dir" ), SIGNAL( triggered() ),
             this, SLOT(slotParentDir()) );
    connect( m_dirView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(slotCurrentDirChanged()) );
    connect( m_dataViewImpl, SIGNAL(setCurrentRoot(QModelIndex)),
             this, SLOT(slotSetCurrentRoot(QModelIndex)) );

    // Setup toolbar
    m_audioActions.push_back( actionCollection()->action( "project_audio_convert" ) );
    m_dataActions.push_back( actionCollection()->action( "parent_dir" ) );
    QList<QAction*> audioPluginActions = createPluginsActions( doc->audioDoc()->type() );
    QList<QAction*> dataPluginActions = createPluginsActions( doc->dataDoc()->type() );

    toolBox()->addActions( m_audioActions );
    toolBox()->addActions( m_dataActions );
    toolBox()->addSeparator();
    toolBox()->addActions( audioPluginActions );
    toolBox()->addActions( dataPluginActions );
    toolBox()->addSeparator();
    m_dataActions.push_back( toolBox()->addWidget( new VolumeNameWidget( doc->dataDoc(), toolBox() ) ) );

    m_audioActions += audioPluginActions;
    m_dataActions += dataPluginActions;

    if( m_dirProxy->rowCount() > 0 )
        m_dirView->setCurrentIndex( m_dirProxy->index( 0, 0 ) );

#ifdef __GNUC__
#warning enable player once ported to Phonon
#endif
//   toolBox()->addAction( m_audioListView->player()->action( K3b::AudioTrackPlayer::ACTION_PLAY ) );
//   toolBox()->addAction( m_audioListView->player()->action( K3b::AudioTrackPlayer::ACTION_PAUSE ) );
//   toolBox()->addAction( m_audioListView->player()->action( K3b::AudioTrackPlayer::ACTION_STOP ) );
//   toolBox()->addSpacing();
//   toolBox()->addAction( m_audioListView->player()->action( K3b::AudioTrackPlayer::ACTION_PREV ) );
//   toolBox()->addAction( m_audioListView->player()->action( K3b::AudioTrackPlayer::ACTION_NEXT ) );
//   toolBox()->addSpacing();
//   m_audioListView->player()->action( K3b::AudioTrackPlayer::ACTION_SEEK )->plug( toolBox() );
//   toolBox()->addSeparator();
}


K3b::MixedView::~MixedView()
{
}


void K3b::MixedView::slotBurn()
{
    if( m_doc->audioDoc()->numOfTracks() == 0 || m_doc->dataDoc()->size() == 0 ) {
        KMessageBox::information( this, i18n("Please add files and audio titles to your project first."),
                                  i18n("No Data to Burn") );
    }
    else {
        K3b::ProjectBurnDialog* dlg = newBurnDialog( this );
        if( dlg ) {
            dlg->execBurnDialog(true);
            delete dlg;
        }
        else {
            kDebug() << "(K3b::Doc) Error: no burndialog available.";
        }
    }
}


void K3b::MixedView::addUrls( const KUrl::List& urls )
{
    if( m_fileViewWidget->currentWidget() == m_dataViewImpl->view() ) {
        QModelIndex parent = m_model->mapToSubModel( m_dirProxy->mapToSource( m_dirView->currentIndex() ) );
        m_dataViewImpl->addUrls( parent, urls );
    }
    else if( m_fileViewWidget->currentWidget() == m_audioViewImpl->view() ) {
        m_audioViewImpl->addUrls( urls );
    }
}


void K3b::MixedView::slotParentDir()
{
    m_dirView->setCurrentIndex( m_dirView->currentIndex().parent() );
}


void K3b::MixedView::slotCurrentDirChanged()
{
    QModelIndex newRoot = m_dirProxy->mapToSource( m_dirView->currentIndex() );

    QAbstractItemModel* currentSubModel = m_model->subModelForIndex( newRoot );
    if( currentSubModel != 0 ) {
        m_model->setSupportedDragActions( currentSubModel->supportedDragActions() );
    }

    if( currentSubModel == m_dataViewImpl->model() ) {
        m_dataViewImpl->slotCurrentRootChanged( m_model->mapToSubModel( newRoot ) );
        if( m_fileViewWidget->currentWidget() != m_dataViewImpl->view() ) {
            m_fileViewWidget->setCurrentWidget( m_dataViewImpl->view() );
        }
        Q_FOREACH( QAction* action, m_dataActions ) {
            action->setVisible( true );
        }
        Q_FOREACH( QAction* action, m_audioActions ) {
            action->setVisible( false );
        }
    }
    else if( currentSubModel == m_audioViewImpl->model() ) {
        if( m_fileViewWidget->currentWidget() != m_audioViewImpl->view() ) {
            m_fileViewWidget->setCurrentWidget( m_audioViewImpl->view() );
        }
        Q_FOREACH( QAction* action, m_dataActions ) {
            action->setVisible( false );
        }
        Q_FOREACH( QAction* action, m_audioActions ) {
            action->setVisible( true );
        }
    }
}


void K3b::MixedView::slotSetCurrentRoot( const QModelIndex& index )
{
    m_dirView->setCurrentIndex( m_dirProxy->mapFromSource( m_model->mapFromSubModel( index ) ) );
}


K3b::ProjectBurnDialog* K3b::MixedView::newBurnDialog( QWidget* parent )
{
    return new K3b::MixedBurnDialog( m_doc, parent );
}

#include "k3bmixedview.moc"
