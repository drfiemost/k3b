/*
 *
 * Copyright (C) 2008 Sebastian Trueg <trueg@k3b.org>
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

#include "k3baudioprojectmodel.h"
#include "k3baudiotrackaddingdialog.h"

#include <k3baudiodoc.h>
#include <k3baudiotrack.h>
#include <k3baudiodatasource.h>
#include <k3baudiofile.h>

#include <KLocale>

#include <QtCore/QMimeData>
#include <QtGui/QApplication>


// we have K3bAudioTracks in the first level and K3bAudioDataSources in the second level

class K3b::AudioProjectModel::Private
{
public:
    Private( K3bAudioDoc* doc, AudioProjectModel* parent )
        : project( doc ),
          q( parent ) {
    }

    K3bAudioDoc* project;

    void _k_trackChanged( K3bAudioTrack* );
    void _k_docChanged();

private:
    AudioProjectModel* q;
};


void K3b::AudioProjectModel::Private::_k_docChanged()
{
    q->reset();
}


K3b::AudioProjectModel::AudioProjectModel( K3bAudioDoc* doc, QObject* parent )
    : QAbstractItemModel( parent ),
      d( new Private( doc, this ) )
{
    connect( doc, SIGNAL( changed() ), this, SLOT( _k_docChanged() ) );
}


K3b::AudioProjectModel::~AudioProjectModel()
{
    delete d;
}


K3bAudioDoc* K3b::AudioProjectModel::project() const
{
    return d->project;
}


K3bAudioTrack* K3b::AudioProjectModel::trackForIndex( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        QObject* o = static_cast<QObject*>( index.internalPointer() );
        if ( K3bAudioTrack* track = qobject_cast<K3bAudioTrack*>( o ) )
             return track;
    }

    return 0;
}


QModelIndex K3b::AudioProjectModel::indexForTrack( K3bAudioTrack* track ) const
{
    return createIndex( track->trackNumber()-1, 0, track );
}


K3bAudioDataSource* K3b::AudioProjectModel::sourceForIndex( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        QObject* o = static_cast<QObject*>( index.internalPointer() );
        if ( K3bAudioDataSource* source = qobject_cast<K3bAudioDataSource*>( o ) )
             return source;
    }

    return 0;
}


QModelIndex K3b::AudioProjectModel::indexForSource( K3bAudioDataSource* source ) const
{
    int row = 0;
    K3bAudioDataSource* s = source->track()->firstSource();
    while ( s && s != source ) {
        ++row;
    }
    return createIndex( row, 0, source );
}


int K3b::AudioProjectModel::columnCount( const QModelIndex& ) const
{
    return NumColumns;
}


QVariant K3b::AudioProjectModel::data( const QModelIndex& index, int role ) const
{
    if ( index.isValid() ) {
        K3bAudioTrack* track = trackForIndex( index );
        K3bAudioDataSource* source = sourceForIndex( index );

        // track
        if ( !source ) {
            switch( index.column() ) {
            case TrackNumberColumn:
                if( role == Qt::DisplayRole ) {
                    return track->trackNumber();
                }
                break;

            case ArtistColumn:
                if( role == Qt::DisplayRole ||
                    role == Qt::EditRole ) {
                    return track->artist();
                }
                break;

            case TitleColumn:
                if( role == Qt::DisplayRole ||
                    role == Qt::EditRole ) {
                    return track->title();
                }
                break;

            case TypeColumn:
                if( role == Qt::DisplayRole ) {
                    return track->firstSource()->type();
                }
                break;

            case LengthColumn:
                if( role == Qt::DisplayRole ) {
                    return track->length().toString();
                }
                break;

            case FilenameColumn:
                if( role == Qt::DisplayRole ) {
                    return track->firstSource()->sourceComment();
                }
                break;
            }
        }

        // source
        else {
            if( role == Qt::DisplayRole ) {
                switch( index.column() ) {
                case TypeColumn:
                    return source->type();

                case LengthColumn:
                    return source->length().toString();

                case FilenameColumn:
                    return source->sourceComment();
                }
            }
        }
    }

    // some formatting
    switch( role ) {
    case Qt::FontRole:
        switch( index.column() ) {
        case TypeColumn: {
            QFont f;
            f.setItalic( true );
            return f;
        }
        case FilenameColumn: {
            QFont f;
            f.setPointSize( f.pointSize() - 2 );
            return f;
        }
        break;
        }

    case Qt::ForegroundRole:
        if ( index.column() == FilenameColumn ) {
            return QPalette().color( QPalette::Disabled, QPalette::Text );
        }
        break;

    case Qt::TextAlignmentRole:
        if ( index.column() == TypeColumn ||
             index.column() == LengthColumn ) {
            return Qt::AlignHCenter;
        }
        break;
    }

    return QVariant();
}


QVariant K3b::AudioProjectModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    Q_UNUSED( orientation );
    if ( role == Qt::DisplayRole ) {
        switch( section ) {
        case TrackNumberColumn:
            return i18nc("audio track number", "No.");
        case ArtistColumn:
            return i18n("Artist (CD-Text)");
        case TitleColumn:
            return i18n("Title (CD-Text)");
        case TypeColumn:
            return i18nc("audio type like mp3 or whatever", "Type");
        case LengthColumn:
            return i18nc("audio track length", "Length");
        case FilenameColumn:
            return i18n("Filename");
        }
    }

    return QVariant();
}


Qt::ItemFlags K3b::AudioProjectModel::flags( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        Qt::ItemFlags f = Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled;
        // only tracks are editable, sources are not
        if ( !sourceForIndex( index ) &&
             ( index.column() == ArtistColumn ||
               index.column() == TitleColumn ) ) {
            f |= Qt::ItemIsEditable;
        }
        return f;
    }
    else {
        return QAbstractItemModel::flags(index)|Qt::ItemIsDropEnabled;
    }
}


QModelIndex K3b::AudioProjectModel::index( int row, int column, const QModelIndex& parent ) const
{
    // source
    if ( parent.isValid() ) {
        if ( K3bAudioTrack* track = trackForIndex( parent ) ) {
            if ( K3bAudioDataSource* source = track->getSource( row ) ) {
                return createIndex( row, column, source );
            }
        }
    }

    // track
    else if ( row >= 0 && row < d->project->numOfTracks() ) {
        return createIndex( row, column, d->project->getTrack( row+1 ) );
    }

    return QModelIndex();
}


QModelIndex K3b::AudioProjectModel::parent( const QModelIndex& index ) const
{
    if ( index.isValid() ) {
        if ( K3bAudioDataSource* source = sourceForIndex( index ) ) {
            return indexForTrack( source->track() );
        }
    }

    return QModelIndex();
}


int K3b::AudioProjectModel::rowCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) {
        if ( K3bAudioTrack* track = trackForIndex( parent ) ) {
            // first level
            return track->numberSources();
        }
        else  {
            // second level
            return 0;
        }
    }
    else {
        return d->project->numOfTracks();
    }
}


bool K3b::AudioProjectModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if ( index.isValid() ) {
        if ( K3bAudioTrack* track = trackForIndex( index ) ) {
            if ( role == Qt::EditRole ) {
                switch( index.column() ) {
                case ArtistColumn:
                    track->setArtist( value.toString() );
                    return true;
                case TitleColumn:
                    track->setTitle( value.toString() );
                    return true;
                }
            }
        }
    }
    return false;
}


QMimeData* K3b::AudioProjectModel::mimeData( const QModelIndexList& indexes ) const
{
    QMimeData* mime = new QMimeData();

    QSet<K3bAudioTrack*> tracks;
    QSet<K3bAudioDataSource*> sources;
    KUrl::List urls;
    foreach( const QModelIndex& index, indexes ) {
        if ( K3bAudioTrack* track = trackForIndex( index ) ) {
            tracks << track;
            K3bAudioDataSource* source = track->firstSource();
            while ( source ) {
                if ( K3bAudioFile* file = dynamic_cast<K3bAudioFile*>( source ) ) {
                    if ( !urls.contains( KUrl( file->filename() ) ) ) {
                        urls.append( KUrl( file->filename() ) );
                    }
                }
                source = source->next();
            }
        }
        else if ( K3bAudioDataSource* source = sourceForIndex( index ) ) {
            sources << source;
            if ( K3bAudioFile* file = dynamic_cast<K3bAudioFile*>( source ) ) {
                if ( !urls.contains( KUrl( file->filename() ) ) ) {
                    urls.append( KUrl( file->filename() ) );
                }
            }
        }
    }
    urls.populateMimeData( mime );

    // the easy road: encode the pointers
    if ( !tracks.isEmpty() ) {
        QByteArray trackData;
        QDataStream trackDataStream( &trackData, QIODevice::WriteOnly );
        foreach( K3bAudioTrack* track, tracks ) {
            trackDataStream << ( qint64 )track;
        }
        mime->setData( "application/x-k3baudiotrack", trackData );
    }
    if ( !sources.isEmpty() ) {
        QByteArray sourceData;
        QDataStream sourceDataStream( &sourceData, QIODevice::WriteOnly );
        foreach( K3bAudioDataSource* source, sources ) {
            sourceDataStream << ( qint64 )source;
        }
        mime->setData( "application/x-k3baudiodatasource", sourceData );
    }

    return mime;
}


Qt::DropActions K3b::AudioProjectModel::supportedDropActions() const
{
    return Qt::CopyAction|Qt::MoveAction;
}


QStringList K3b::AudioProjectModel::mimeTypes() const
{
    QStringList s = KUrl::List::mimeDataTypes();
    s += QString::fromLatin1( "application/x-k3baudiotrack" );
    s += QString::fromLatin1( "application/x-k3baudiodatasource" );
    return s;
}


bool K3b::AudioProjectModel::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
    //
    // Some hints:
    // * parent index is a track
    //   -> if row == -1 something has been dropped onto the track
    //   -> if row >= 0 in between two sources of the track
    //
    // * parent index is a source
    //   -> something has been dropped onto the source
    //
    // * parent index is invalid
    //   -> if row == -1, something has been dropped onto the view
    //   -> if row >= 0 in between two tracks, i.e. tracks row and row+1
    //

    //
    // Actions we perform:
    // * dropping onto a track
    //   -> add new tracks before the track (except if the track already has multiple sources)
    //
    // * dropping between tracks
    //   -> add new tracks after the first track (row)
    //
    // * dropping onto source
    //   -> add sources before the source
    //
    // * dropping onto the view
    //   -> add new tracks at the end
    //

    Q_UNUSED( column );

    if ( action == Qt::IgnoreAction )
        return true;

    K3bAudioTrack* dropTrackParent = 0;
    K3bAudioTrack* dropTrackAfter = 0;
    K3bAudioDataSource* dropSourceAfter = 0;
    if ( K3bAudioTrack* track = trackForIndex( parent ) ) {
        if ( row >= 0 ) {
            dropTrackParent = track;
            dropSourceAfter = track->getSource( row-1 );
        }
        else {
            dropTrackAfter = track->prev();
        }
    }
    else if ( K3bAudioDataSource* source = sourceForIndex( parent ) ) {
        dropTrackParent = source->track();
        dropSourceAfter = source->prev();
    }
    else if ( row >= 0 ) {
        dropTrackAfter = d->project->getTrack( row );
    }
    else {
        dropTrackAfter = d->project->lastTrack();
    }

    bool copyItems = ( action == Qt::CopyAction );

    //
    // decode data
    //
    QList<K3bAudioTrack*> tracks;
    QList<K3bAudioDataSource*> sources;
    KUrl::List urls;
    if ( data->hasFormat( "application/x-k3baudiotrack" ) ||
         data->hasFormat( "application/x-k3baudiodatasource" )) {

        QByteArray trackData = data->data( "application/x-k3baudiotrack" );
        QDataStream trackDataStream( trackData );
        while ( !trackDataStream.atEnd() ) {
            qint64 p;
            trackDataStream >> p;
            tracks << ( K3bAudioTrack* )p;
        }

        QByteArray sourceData = data->data( "application/x-k3baudiosource" );
        QDataStream sourceDataStream( sourceData );
        while ( !sourceDataStream.atEnd() ) {
            qint64 p;
            sourceDataStream >> p;
            sources << ( K3bAudioDataSource* )p;
        }

        //
        // remove all sources which belong to one of the selected tracks since they will be
        // moved along with their tracks
        //
        QList<K3bAudioDataSource*>::iterator srcIt = sources.begin();
        while( srcIt != sources.end() ) {
            if( tracks.contains( ( *srcIt )->track() ) )
                srcIt = sources.erase( srcIt );
            else
                ++srcIt;
        }


        //
        // Now move (or copy) all the tracks
        //
        for( QList<K3bAudioTrack*>::iterator it = tracks.begin(); it != tracks.end(); ++it ) {
            K3bAudioTrack* track = *it;
            if( dropTrackParent ) {
                dropTrackParent->merge( copyItems ? track->copy() : track, dropSourceAfter );
            }
            else if( dropTrackAfter ) {
                if( copyItems )
                    track->copy()->moveAfter( dropTrackAfter );
                else
                    track->moveAfter( dropTrackAfter );
            }
            else {
                if( copyItems )
                    track->copy()->moveAhead( d->project->firstTrack() );
                else
                    track->moveAhead( d->project->firstTrack() );
            }
        }

        //
        // now move (or copy) the sources
        //
        for( QList<K3bAudioDataSource*>::iterator it = sources.begin(); it != sources.end(); ++it ) {
            K3bAudioDataSource* source = *it;
            if( dropTrackParent ) {
                if( dropSourceAfter ) {
                    if( copyItems )
                        source->copy()->moveAfter( dropSourceAfter );
                    else
                        source->moveAfter( dropSourceAfter );
                }
                else {
                    if( copyItems )
                        source->copy()->moveAhead( dropTrackParent->firstSource() );
                    else
                        source->moveAhead( dropTrackParent->firstSource() );
                }
            }
            else {
                // create a new track
                K3bAudioTrack* track = new K3bAudioTrack( d->project );

                // special case: the source we remove from the track is the last and the track
                // will be deleted.
                if( !copyItems && dropTrackAfter == source->track() && dropTrackAfter->numberSources() == 1 )
                    dropTrackAfter = dropTrackAfter->prev();

                if( copyItems )
                    track->addSource( source->copy() );
                else
                    track->addSource( source );

                if( dropTrackAfter ) {
                    track->moveAfter( dropTrackAfter );
                    dropTrackAfter = track;
                }
                else {
                    track->moveAhead( d->project->firstTrack() );
                    dropTrackAfter = track;
                }
            }
        }

        return true;
    }

    //
    // handle tracks from the audio cd view
    //
#warning FIXME: handle dragndrop from the audio cd ripping view

    //
    // add new tracks
    //
    else if ( KUrl::List::canDecode( data ) ) {
        kDebug() << "url list drop";
        KUrl::List urls = KUrl::List::fromMimeData( data );
        K3bAudioTrackAddingDialog::addUrls( urls, d->project, dropTrackAfter, dropTrackParent, dropSourceAfter, qApp->activeWindow() );
        return true;
    }

    return false;
}

#include "k3baudioprojectmodel.moc"
