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


#ifndef _K3B_ISO9660_H_
#define _K3B_ISO9660_H_

#include <sys/stat.h>
#include <sys/types.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdict.h>


namespace K3bCdDevice {
  class CdDevice;
}

class K3bIso9660;


/**
 * Simplyfied primary descriptor which just contains the fields
 * used by K3b.
 */
class K3bIso9660SimplePrimaryDescriptor
{
 public:
  QString volumeId;
  QString systemId;
  QString volumeSetId;
  QString publisherId;
  QString preparerId;
  QString applicationId;
  int volumeSetSize;
  int volumeSetNumber;
  long logicalBlockSize;
  long long volumeSpaceSize;
};


/**
 * Base class for all entries in a K3bIso9660 archive. A lot has been copied
 * from KArchive.
 */
class K3bIso9660Entry
{
 public:
  K3bIso9660Entry( K3bIso9660* archive,
		   const QString& name,
		   int access,
		   int date,
		   int adate,
		   int cdate, 
		   const QString& user,
		   const QString& group,
		   const QString& symlink );
  virtual ~K3bIso9660Entry();

  int adate() const { return m_adate; }
  int cdate() const { return m_cdate; }

  /**
   * Creation date of the file.
   * @return the creation date
   */
  QDateTime datetime() const;

  /**
   * Creation date of the file.
   * @return the creation date in seconds since 1970
   */
  int date() const { return m_date; }

  /**
   * Name of the file without path.
   * @return the file name without path
   */
  const QString& name() const { return m_name; }

  /**
   * The permissions and mode flags as returned by the stat() function
   * in st_mode.
   * @return the permissions
   */
  mode_t permissions() const { return m_access; }

  /**
   * User who created the file.
   * @return the owner of the file
   */
  const QString& user() const { return m_user; }

  /**
   * Group of the user who created the file.
   * @return the group of the file
   */
  const QString& group() const { return m_group; }

  /**
   * Symlink if there is one.
   * @return the symlink, or QString::null
   */
  const QString& symlink() const { return m_symlink; }

  /**
   * Checks whether the entry is a file.
   * @return true if this entry is a file
   */
  virtual bool isFile() const { return false; }

  /**
   * Checks whether the entry is a directory.
   * @return true if this entry is a directory
   */
  virtual bool isDirectory() const { return false; }

  K3bIso9660* archive() const { return m_archive; }

 private:
  int m_adate;
  int m_cdate;
  QString m_name;
  int m_date;
  mode_t m_access;
  QString m_user;
  QString m_group;
  QString m_symlink;
  K3bIso9660* m_archive;
};


class K3bIso9660Directory : public K3bIso9660Entry
{
 public: 
  K3bIso9660Directory( K3bIso9660* archive, const QString& name, int access, int date,
		       int adate,int cdate, const QString& user, const QString& group,
		       const QString& symlink);
  ~K3bIso9660Directory();

  /**
   * Returns a list of sub-entries.
   * @return the names of all entries in this directory (filenames, no path).
   */
  QStringList entries() const;

  /**
   * Returns the entry with the given name.
   * @param name may be "test1", "mydir/test3", "mydir/mysubdir/test3", etc.
   * @return a pointer to the entry in the directory.
   */
  K3bIso9660Entry* entry( const QString& name );

  /**
   * Returns the entry with the given name.
   * @param name may be "test1", "mydir/test3", "mydir/mysubdir/test3", etc.
   * @return a pointer to the entry in the directory.
   */
  const K3bIso9660Entry* entry( const QString& name ) const;

  /**
   * @internal
   * Adds a new entry to the directory.
   */
  void addEntry( K3bIso9660Entry* );

  /**
   * Checks whether this entry is a directory.
   * @return true, since this entry is a directory
   */
  bool isDirectory() const { return true; }

 private:
  QDict<K3bIso9660Entry> m_entries;
};


class K3bIso9660File : public K3bIso9660Entry
{
 public: 
  /**
   * @param pos start sector
   */
  K3bIso9660File( K3bIso9660* archive, 
		  const QString& name, 
		  int access, 
		  int date,
		  int adate,
		  int cdate, 
		  const QString& user, 
		  const QString& group,
		  const QString& symlink, 
		  unsigned int pos, 
		  unsigned int size );
  ~K3bIso9660File();

  bool isFile() const { return true; }

  void setZF( char algo[2], char parms[2], int realsize );
  int realsize() const { return m_realsize; }

  /**
   * @return size in bytes.
   */
  unsigned int size() const { return m_size; }

  /**
   * Returnes the startSector of the file.
   */
  unsigned int startSector() const { return m_startSector; }

  /**
   * Returnes the startOffset of the file in bytes.
   */
  unsigned long long startPostion() const { return (unsigned long long)m_startSector * 2048; }

  /**
   * @param pos offset in bytes
   * @param len max number of bytes to read
   */
  int read( unsigned int pos, char* data, int len ) const;

 private:
  char m_algo[2];
  char m_parms[2];
  int m_realsize;

  unsigned int m_curpos;
  unsigned int m_startSector;
  unsigned int m_size;
};


/**
 * This class is based on the KIso class by
 * Gy�rgy Szombathelyi <gyurco@users.sourceforge.net>.
 * A lot has been changed and bugfixed.
 * The API has been improved to be useful.
 *
 * Due to the stupid Qt which does not support large files as default
 * we cannot use QIODevice with DVDs! That's why we have our own 
 * reading code which is not allowed by KArchive (which is limited to int
 * by the way... who the hell designed this???)
 * I also removed the KArchive inheritance because of the named reasons.
 * So this stuff contains a lot KArchive code which has been made usable.
 */
class K3bIso9660
{
 public:
  /**
   * Creates an instance that operates on the given filename.
   * using the compression filter associated to given mimetype.
   *
   * @param filename is a local path (e.g. "/home/weis/myfile.tgz")
   */
  K3bIso9660( const QString& filename );

  /**
   * Special case which always reads the TOC from the specified sector
   * thus supporting multisession CDs.
   */
  K3bIso9660( K3bCdDevice::CdDevice* dev, unsigned int startSector = 0 );

  /**
   * @param fd open file descriptor
   */
  K3bIso9660( int fd );

  /**
   * If the .iso is still opened, then it will be
   * closed automatically by the destructor.
   */
  virtual ~K3bIso9660();


  /**
   * Opens the archive for reading.
   * Parses the directory listing of the archive
   * and creates the K3bIso9660Directory/K3bIso9660File entries.
   */
  bool open();

  /**
   * Closes everything.
   * This is also called in the destructor
   */
  void close();

  /**
   * @param sector startsector
   * @param len number of sectors
   * @return number of sectors read or -1 on error
   */
  int read( unsigned int sector, char* data, int len );

  /**
   * The name of the os file, as passed to the constructor
   * Null if you used the QIODevice constructor.
   */
  const QString& fileName() { return m_filename; }

  const K3bIso9660Directory* firstJolietDirEntry() const;
  const K3bIso9660Directory* firstIsoDirEntry() const;
  const K3bIso9660Directory* firstElToritoEntry() const;

  /**
   * @returns 0 if no joliet desc could be found
   *          the joliet level (1-3) otherwise
   */
  int jolietLevel() const { return m_joliet; }

  const K3bIso9660SimplePrimaryDescriptor& primaryDescriptor() const;

  void debug() const;

  int level;
  K3bIso9660Directory *dirent;

 private:
  /**
   * @internal
   */
  void addBoot(struct el_torito_boot_descriptor* bootdesc);
  void createSimplePrimaryDesc( struct iso_primary_descriptor* desc );

  void debugEntry( const K3bIso9660Entry*, int depth ) const;

  QString m_filename;
  int m_joliet;
 
 private:
  class Private;
  Private * d;
};

#endif
