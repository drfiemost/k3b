/***************************************************************************
                          k3bcddb.h  -  description
                             -------------------
    begin                : Sun Oct 7 2001
    copyright            : (C) 2001 by Sebastian Trueg
    email                : trueg@informatik.uni-freiburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef K3BCDDB_H
#define K3BCDDB_H

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>

struct cdrom_drive;

class CDDB;
class K3bToc;



class K3bCddb : public QObject {
    Q_OBJECT
  public:
    K3bCddb( );
    K3bCddb( bool, QString, unsigned int );
    ~K3bCddb(  );
    /**
	* Searches for a cd in a drive and open the drive.
	*/
    //struct cdrom_drive *pickDrive( QString newPath );
    /**
	* Reads the content and parses for cddb entries if enabled.
	*/
    void updateCD( struct cdrom_drive * );
    /**
	* Gets the disc id of the cd in the drive.
	*/
    unsigned int get_discid( struct cdrom_drive *drive );
    /**
    * Gets disc id, cd in the drive must be successful accessed one time.
    */
    unsigned int get_discid( ) { return m_discid; };
    /**
    *
    */
    void update( QString *device );
	/**
	* Closes the current connection to drive. Must be called if cd has changed.
	*/
    //void closeDrive(struct cdrom_drive *drive);

    void setServer(QString server) { m_cddbServer = server; };
    void setPort(unsigned int port) { m_cddbPort = port; };
    /**
    * Generates instance of cddb and enables cddb use.
    */
    void setUseCddb(bool useCddb);
    bool useCddb() { return m_useCddb; };

    QStringList getTitles() { return m_titles; };
    QString getAlbum() { return m_cd_album; };
    QString getArtist() { return m_cd_artist; };

    void setTitles( const QStringList& list) { m_titles = list; };
    void setAlbum( const QString& album ) {m_cd_album = album; };
    void setArtist( const QString& artist ) { m_cd_artist = artist; };

    static bool appendCddbInfo( K3bToc& );

 signals:
   void updatedCD();

  private:
    bool m_useCddb;
    CDDB *m_cddb;
    QString m_cddbServer;
    unsigned int m_cddbPort;
    struct cdrom_drive *m_drive;

    int m_trackIndex;
    unsigned int m_discid;
    int m_tracks;
    QString m_cd_album;
    QString m_cd_artist;
    QStringList m_titles;
    bool m_is_audio[100];
    //bool m_based_on_cddb;
    QString m_s_track;

    QValueList<int>* getTrackList( );
    void readQuery();

private slots:
    void prepareQuery( unsigned int );
    void queryTracks( );

};

#endif
