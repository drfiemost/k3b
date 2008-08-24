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

#include "k3bexternalbinwidget.h"
#include "k3bexternalbinmanager.h"

#include <qpushbutton.h>
#include <QPixmap>
#include <kdebug.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <q3header.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qregexp.h>
#include <qfont.h>
#include <qpainter.h>
#include <qtooltip.h>

#include <qcursor.h>
#include <qapplication.h>
#include <qbitmap.h>

#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <keditlistbox.h>
#include <k3listview.h>
#include <kglobalsettings.h>




K3bExternalBinWidget::K3bExternalProgramViewItem::K3bExternalProgramViewItem( K3bExternalProgram* p, Q3ListView* parent )
    : K3bListViewItem( parent ), m_program(p)
{
    QFont f( listView()->font() );
    f.setBold(true);
    setFont( 0, f );
    setBackgroundColor( 0, parent->palette().alternateBase().color() );
    setBackgroundColor( 1, parent->palette().alternateBase().color() );
    setBackgroundColor( 2, parent->palette().alternateBase().color() );
    setText( 0, p->name() );
    setSelectable( false );
}


K3bExternalBinWidget::K3bExternalBinViewItem::K3bExternalBinViewItem( const K3bExternalBin* bin, K3bExternalProgramViewItem* parent )
    : K3bListViewItem( parent ), m_bin( bin ), m_parent( parent )
{
    setText( 0, bin->path );
    setText( 1, bin->version );
    setText( 2, bin->features().join(", ") );

    setDefault(false);
}


void K3bExternalBinWidget::K3bExternalBinViewItem::setDefault( bool b )
{
    static QPixmap s_emptyPix( (int)KIconLoader::SizeSmall, (int)KIconLoader::SizeSmall );
    static bool s_emptyPixInitialized = false;
    if( !s_emptyPixInitialized ) {
        QBitmap mask( (int)KIconLoader::SizeSmall, (int)KIconLoader::SizeSmall );
        mask.clear();
        s_emptyPix.setMask( mask );
        s_emptyPixInitialized = true;
    }

    m_default = b;
    if( b )
        setPixmap( 0, SmallIcon( "dialog-ok" ) );
    else
        setPixmap( 0, s_emptyPix );
}





// ///////////////////////////////////////////////////////////
//
// K3BEXTERNALBINWIDGET
//
// //////////////////////////////////////////////////////////


K3bExternalBinWidget::K3bExternalBinWidget( K3bExternalBinManager* manager, QWidget* parent )
    : QWidget( parent ), m_manager( manager )
{
    QGridLayout* mainGrid = new QGridLayout( this );
    mainGrid->setMargin( 0 );
    mainGrid->setSpacing( KDialog::spacingHint() );

    m_mainTabWidget = new QTabWidget( this );
    m_rescanButton = new QPushButton( i18n("&Search"), this );
    mainGrid->addWidget( m_mainTabWidget, 0, 0, 1, 2 );
    mainGrid->addWidget( m_rescanButton, 1, 1 );
    mainGrid->setColumnStretch( 0, 1 );
    mainGrid->setRowStretch( 0, 1 );


    // setup program tab
    // ------------------------------------------------------------
    QWidget* programTab = new QWidget( m_mainTabWidget );
    QGridLayout* programTabLayout = new QGridLayout( programTab );
    programTabLayout->setMargin( KDialog::marginHint() );
    programTabLayout->setSpacing( KDialog::spacingHint() );
    m_programView = new K3bListView( programTab );
    m_defaultButton = new QPushButton( i18n("Set Default"), programTab );
    m_defaultButton->setToolTip( i18n("Change the versions K3b should use.") );
    m_defaultButton->setWhatsThis( i18n("<p>If K3b finds more than one installed version of a program "
                                        "it will choose one as the <em>default</em>, which will be used "
                                        "to do the work. If you want to change the default, select the "
                                        "desired version and press this button.") );
    programTabLayout->addWidget( m_programView, 1, 0, 1, 2 );
    programTabLayout->addWidget( m_defaultButton, 0, 1 );
    programTabLayout->addWidget( new QLabel( i18n("Use the 'Default' button to change the versions K3b should use."),
                                             programTab ), 0, 0 );
    programTabLayout->setColumnStretch( 0, 1 );
    programTabLayout->setRowStretch( 1, 1 );

    m_programView->addColumn( i18n("Path") );
    m_programView->addColumn( i18n("Version") );
    m_programView->addColumn( i18n("Features") );
    m_programView->setAllColumnsShowFocus(true);
    m_programView->setFullWidth(true);
    m_programView->setAlternateBackground( QColor() );
    m_programView->setShadeSortColumn( false );
    m_mainTabWidget->addTab( programTab, i18n("Programs") );


    // setup parameters tab
    // ------------------------------------------------------------
    QWidget* parametersTab = new QWidget( m_mainTabWidget );
    QGridLayout* parametersTabLayout = new QGridLayout( parametersTab );
    parametersTabLayout->setMargin( KDialog::marginHint() );
    parametersTabLayout->setSpacing( KDialog::spacingHint() );
    m_parameterView = new K3bListView( parametersTab );
    parametersTabLayout->addWidget( m_parameterView, 1, 0 );
    parametersTabLayout->addWidget( new QLabel( i18n("User parameters have to be separated by space."), parametersTab ), 0, 0 );

    parametersTabLayout->setRowStretch( 1, 1 );

    m_parameterView->addColumn( i18n("Program") );
    m_parameterView->addColumn( i18n("Parameters") );
    m_parameterView->setAllColumnsShowFocus(true);
    m_parameterView->setFullWidth(true);
    m_parameterView->setDefaultRenameAction( Q3ListView::Accept );
    m_parameterView->setDoubleClickForEdit( false );

    m_mainTabWidget->addTab( parametersTab, i18n("User Parameters") );


    // setup search path tab
    // ------------------------------------------------------------
    QWidget* searchPathTab = new QWidget( m_mainTabWidget );
    QGridLayout* searchPathTabLayout = new QGridLayout( searchPathTab );
    searchPathTabLayout->setMargin( KDialog::marginHint() );
    searchPathTabLayout->setSpacing( KDialog::spacingHint() );
    m_searchPathBox = new KEditListBox( i18n("Search Path"), searchPathTab );
    m_searchPathBox->setCheckAtEntering( true );
    searchPathTabLayout->addWidget( m_searchPathBox, 0, 0 );
    searchPathTabLayout->addWidget( new QLabel( i18n("<qt><b>Hint:</b> to force K3b to use another than the "
                                                     "default name for the executable specify it in the search path.</qt>"),
                                                searchPathTab ), 1, 0 );

    searchPathTabLayout->setRowStretch( 0, 1 );

    m_mainTabWidget->addTab( searchPathTab, i18n("Search Path") );

    connect( m_rescanButton, SIGNAL(clicked()), this, SLOT(rescan()) );
    connect( m_defaultButton, SIGNAL(clicked()), this, SLOT(slotSetDefaultButtonClicked()) );
    connect( m_programView, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotProgramSelectionChanged(Q3ListViewItem*)) );

    slotProgramSelectionChanged( 0 );
}


K3bExternalBinWidget::~K3bExternalBinWidget()
{
    qDeleteAll( m_programRootItems );
}

void K3bExternalBinWidget::rescan()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    saveSearchPath();
    m_manager->search();
    load();
    QApplication::restoreOverrideCursor();
}


void K3bExternalBinWidget::load()
{
    m_programView->clear();
    m_programRootItems.clear();
    m_parameterView->clear();

    // load programs
    const QMap<QString, K3bExternalProgram*>& map = m_manager->programs();
    for( QMap<QString, K3bExternalProgram*>::const_iterator it = map.begin(); it != map.end(); ++it ) {
        K3bExternalProgram* p = *it;

        K3bExternalProgramViewItem* pV = new K3bExternalProgramViewItem( p, m_programView );
        m_programRootItems.append( pV );
        // populate it
        foreach( const K3bExternalBin* b, p->bins() ) {
            K3bExternalBinViewItem* bV = new K3bExternalBinViewItem( b, pV );
            if( b == p->defaultBin() )
                bV->setDefault(true);

            pV->setOpen(true);
        }

        if( p->bins().isEmpty() )
            pV->setText( 0, p->name() + i18n(" (not found)") );

        // load user parameters
        if( p->supportsUserParameters() ) {
            K3bExternalProgramViewItem* paraV = new K3bExternalProgramViewItem( p, m_parameterView );
            paraV->setText( 1, p->userParameters().join( " " ) );
            paraV->setEditor( 1, K3bListViewItem::LINE );
        }
    }



    // load search path
    m_searchPathBox->clear();
    m_searchPathBox->insertStringList( m_manager->searchPath() );
}


void K3bExternalBinWidget::save()
{
    saveSearchPath();


    // save the default programs
    Q3ListViewItemIterator progIt( m_programView );
    while( progIt.current() ) {
        if( K3bExternalBinViewItem* bV = dynamic_cast<K3bExternalBinViewItem*>( progIt.current() ) ) {
            if( bV->isDefault() )
                bV->parentProgramItem()->program()->setDefault( bV->bin() );
        }

        ++progIt;
    }


    // save the user parameters
    Q3ListViewItemIterator paraIt( m_parameterView );
    QRegExp reSpace( "\\s" );
    while( paraIt.current() ) {
        K3bExternalProgramViewItem* pV = (K3bExternalProgramViewItem*)paraIt.current();
        pV->program()->setUserParameters( pV->text(1).split( reSpace ) );

        ++paraIt;
    }
}


void K3bExternalBinWidget::saveSearchPath()
{
    m_manager->setSearchPath( m_searchPathBox->items() );
}


void K3bExternalBinWidget::slotSetDefaultButtonClicked()
{
    // check if we are on a binItem
    K3bExternalBinViewItem* item = dynamic_cast<K3bExternalBinViewItem*>( m_programView->selectedItem() );
    if( item ) {
        // remove all default flags
        K3bExternalBinViewItem* bi = (K3bExternalBinViewItem*)item->parentProgramItem()->firstChild();
        Q3ListViewItemIterator it( bi );
        while( it.current() && it.current()->parent() == item->parentProgramItem() ) {
            ((K3bExternalBinViewItem*)it.current())->setDefault(false);
            ++it;
        }

        item->setDefault(true);
    }
}


void K3bExternalBinWidget::slotProgramSelectionChanged( Q3ListViewItem* item )
{
    K3bExternalBinViewItem* bV = dynamic_cast<K3bExternalBinViewItem*>( item );
    if( bV ) {
        if( bV->isDefault() )
            m_defaultButton->setEnabled(false);
        else
            m_defaultButton->setEnabled(true);
    }
    else
        m_defaultButton->setEnabled(false);
}

#include "k3bexternalbinwidget.moc"
