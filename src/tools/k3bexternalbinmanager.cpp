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

#include "k3bexternalbinmanager.h"

#include <kdebug.h>
#include <kprocess.h>
#include <kconfig.h>

#include <qstring.h>
#include <qregexp.h>
#include <qfile.h>
#include <qfileinfo.h>

#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>



QString K3bExternalBinManager::m_noPath = "";


// ///////////////////////////////////////////////////////////
//
// K3BEXTERNALBINVERSION
//
// ///////////////////////////////////////////////////////////

K3bExternalBinVersion::K3bExternalBinVersion()
  : m_majorVersion( -1 ),
    m_minorVersion( -1 ),
    m_patchLevel( -1 )
{
}

K3bExternalBinVersion::K3bExternalBinVersion( const K3bExternalBinVersion& v )
  : m_versionString( v.versionString() ),
    m_majorVersion( v.majorVersion() ),
    m_minorVersion( v.minorVersion() ),
    m_patchLevel( v.patchLevel() ),
    m_suffix( v.suffix() )
{
}

K3bExternalBinVersion::K3bExternalBinVersion( const QString& version )
{
  setVersion( version );
}

K3bExternalBinVersion::K3bExternalBinVersion( int majorVersion, 
					      int minorVersion, 
					      int patchlevel, 
					      const QString& suffix )
{
  setVersion( majorVersion, minorVersion, patchlevel, suffix );
}

void K3bExternalBinVersion::setVersion( const QString& v )
{
  QString suffix;
  splitVersionString( v, m_majorVersion, suffix );
  if( m_majorVersion >= 0 ) {
    if( suffix.startsWith(".") ) {
      suffix = suffix.mid( 1 );
      splitVersionString( suffix, m_minorVersion, suffix );
      if( m_minorVersion < 0 ) {
	kdDebug() << "(K3bExternalBinVersion) suffix must not start with a dot!" << endl;
	m_majorVersion = -1;
	m_minorVersion = -1;
	m_patchLevel = -1;
	m_suffix = "";
      }
      else {
	if( suffix.startsWith(".") ) {
	  suffix = suffix.mid( 1 );
	  splitVersionString( suffix, m_patchLevel, suffix );
	  if( m_patchLevel < 0 ) {
	    kdDebug() << "(K3bExternalBinVersion) suffix must not start with a dot!" << endl;
	    m_majorVersion = -1;
	    m_minorVersion = -1;
	    m_patchLevel = -1;
	    m_suffix = "";
	  }
	  else {
	    m_suffix = suffix;
	  }
	}
	else {
	  m_patchLevel = -1;
	  m_suffix = suffix;
	}
      }
    }
    else {
      m_minorVersion = -1;
      m_patchLevel = -1;
      m_suffix = suffix;
    }
  }

  m_versionString = createVersionString( m_majorVersion, m_minorVersion, m_patchLevel, m_suffix );
}


void K3bExternalBinVersion::splitVersionString( const QString& s, int& num, QString& suffix )
{
  int pos = s.find( QRegExp("\\D") );
  if( pos < 0 ) {
    num = s.toInt();
    suffix = "";
  }
  else if( pos == 0 ) {
    num = -1;
    suffix = s;
  }
  else {
    num = s.left( pos ).toInt();
    suffix = s.mid( pos );
  }
}


bool K3bExternalBinVersion::isValid() const
{
  return (m_majorVersion >= 0);
}


void K3bExternalBinVersion::setVersion( int majorVersion, 
					int minorVersion, 
					int patchlevel, 
					const QString& suffix )
{
  m_majorVersion = majorVersion;
  m_minorVersion = minorVersion;
  m_patchLevel = patchlevel;
  m_suffix = suffix;
  m_versionString = createVersionString( majorVersion, minorVersion, patchlevel, suffix );
}

K3bExternalBinVersion& K3bExternalBinVersion::operator=( const QString& v )
{
  setVersion( v );
  return *this;
}

QString K3bExternalBinVersion::createVersionString( int majorVersion, 
						    int minorVersion, 
						    int patchlevel, 
						    const QString& suffix )
{
  if( majorVersion >= 0 ) {
    QString s = QString::number(majorVersion);
    
    if( minorVersion > -1 ) {
      s.append( QString(".%1").arg(minorVersion) );
      if( patchlevel > -1 )
	s.append( QString(".%1").arg(patchlevel) );
    }
    
    if( !suffix.isNull() )
      s.append( suffix );

    return s;
  }
  else
    return "";
}


bool operator<( const K3bExternalBinVersion& v1, const K3bExternalBinVersion& v2 )
{
  // both version objects need to be valid

  if( v1.majorVersion() == v2.majorVersion() ) {

    // 1 == 1.0
    if( ( v1.minorVersion() == v2.minorVersion() )
	||
	( v1.minorVersion() == -1 && v2.minorVersion() == 0 )
	||
	( v2.minorVersion() == -1 && v1.minorVersion() == 0 ) 
	)
      {
	// 1.0 == 1.0.0
	if( ( v1.patchLevel() == v2.patchLevel() )
	    ||
	    ( v1.patchLevel() == -1 && v2.patchLevel() == 0 )
	    ||
	    ( v2.patchLevel() == -1 && v1.patchLevel() == 0 )
	    )
	  {
	    return ( v1.suffix() < v2.suffix() );
	  }
	else
	  return ( v1.patchLevel() < v2.patchLevel() );
      }
    else
      return ( v1.minorVersion() < v2.minorVersion() );
  }
  else 
    return ( v1.majorVersion() < v2.majorVersion() );
}

bool operator>( const K3bExternalBinVersion& v1, const K3bExternalBinVersion& v2 )
{
  return operator<( v2, v1 );
}


bool operator==( const K3bExternalBinVersion& v1, const K3bExternalBinVersion& v2 )
{
  return (!operator<( v2, v1 ) && !operator<( v1, v2 ));
}


bool operator<=( const K3bExternalBinVersion& v1, const K3bExternalBinVersion& v2 )
{
  return operator<( v1, v2 ) || operator==( v1, v2 );
}

bool operator>=( const K3bExternalBinVersion& v1, const K3bExternalBinVersion& v2 )
{
  return operator>( v1, v2 ) || operator==( v1, v2 );
}


// ///////////////////////////////////////////////////////////
//
// K3BEXTERNALBIN
//
// ///////////////////////////////////////////////////////////

K3bExternalBin::K3bExternalBin( K3bExternalProgram* p )
  : m_program(p)
{
}


bool K3bExternalBin::isEmpty() const
{
  return !version.isValid();
}


const QString& K3bExternalBin::name() const
{
  return m_program->name();
}


bool K3bExternalBin::hasFeature( const QString& f ) const
{
  return m_features.contains( f );
}


void K3bExternalBin::addFeature( const QString& f )
{
  m_features.append( f );
}


const QStringList& K3bExternalBin::userParameters() const
{
  return m_program->userParameters();
}



// ///////////////////////////////////////////////////////////
//
// K3BEXTERNALPROGRAM
//
// ///////////////////////////////////////////////////////////


K3bExternalProgram::K3bExternalProgram( const QString& name )
  : m_name( name )
{
  m_bins.setAutoDelete( true );
}


K3bExternalProgram::~K3bExternalProgram()
{
}

void K3bExternalProgram::addBin( K3bExternalBin* bin )
{
  if( !m_bins.contains( bin ) ) {
    // insertion sort
    // the first bin in the list is always the one used 
    // so we default to using the newest one
    K3bExternalBin* oldBin = m_bins.first();
    while( oldBin && oldBin->version > bin->version )
      oldBin = m_bins.next();

    m_bins.insert( oldBin ? m_bins.at() : m_bins.count(), bin );
  }
}

void K3bExternalProgram::setDefault( K3bExternalBin* bin )
{
  if( m_bins.contains( bin ) )
    m_bins.take( m_bins.find( bin ) );

  // the first bin in the list is always the one used 
  m_bins.insert( 0, bin );
}


void K3bExternalProgram::setDefault( const QString& path )
{
  for( QPtrListIterator<K3bExternalBin> it( m_bins ); it.current(); ++it ) {
    if( it.current()->path == path ) {
      setDefault( it.current() );
      return;
    }
  }
}


void K3bExternalProgram::addUserParameter( const QString& p )
{
  if( !m_userParameters.contains( p ) )
    m_userParameters.append(p);
}


K3bExternalProgram::OutputCollector::OutputCollector( KProcess* p )
  : m_process(0)
{
  setProcess( p );
}

void K3bExternalProgram::OutputCollector::setProcess( KProcess* p )
{
  if( m_process )
    disconnect( m_process );

  m_process = p;
  if( p ) {
    connect( p, SIGNAL(receivedStdout(KProcess*, char*, int)), this, SLOT(slotGatherOutput(KProcess*, char*, int)) );
    connect( p, SIGNAL(receivedStderr(KProcess*, char*, int)), this, SLOT(slotGatherOutput(KProcess*, char*, int)) );
  }

  m_gatheredOutput = "";
}

void K3bExternalProgram::OutputCollector::slotGatherOutput( KProcess*, char* data, int len )
{
  m_gatheredOutput.append( QString::fromLatin1( data, len ) );
}



// ///////////////////////////////////////////////////////////
//
// K3BEXTERNALBINMANAGER
//
// ///////////////////////////////////////////////////////////


K3bExternalBinManager::K3bExternalBinManager()
  : QObject()
{
}


K3bExternalBinManager::~K3bExternalBinManager()
{
  clear();
}


bool K3bExternalBinManager::readConfig( KConfig* c )
{
  loadDefaultSearchPath();

  if( c->hasKey( "search path" ) )
    setSearchPath( c->readListEntry( "search path" ) );

  search();

  for ( QMap<QString, K3bExternalProgram*>::iterator it = m_programs.begin(); it != m_programs.end(); ++it ) {
    K3bExternalProgram* p = it.data();
    if( c->hasKey( p->name() + " default" ) ) {
      p->setDefault( c->readEntry( p->name() + " default" ) );
    }
    if( c->hasKey( p->name() + " user parameters" ) ) {
      QStringList list = c->readListEntry( p->name() + " user parameters" );
      for( QStringList::iterator strIt = list.begin(); strIt != list.end(); ++strIt )
	p->addUserParameter( *strIt );
    }
  }

  return true;
}

bool K3bExternalBinManager::saveConfig( KConfig* c )
{
  c->writeEntry( "search path", m_searchPath );

  for ( QMap<QString, K3bExternalProgram*>::iterator it = m_programs.begin(); it != m_programs.end(); ++it ) {
    K3bExternalProgram* p = it.data();
    if( p->defaultBin() )
      c->writeEntry( p->name() + " default", p->defaultBin()->path );

    c->writeEntry( p->name() + " user parameters", p->userParameters() );
  }

  return true;
}


bool K3bExternalBinManager::foundBin( const QString& name )
{
  if( m_programs.find( name ) == m_programs.end() )
    return false;
  else
    return (m_programs[name]->defaultBin() != 0);
}


const QString& K3bExternalBinManager::binPath( const QString& name )
{
  if( m_programs.find( name ) == m_programs.end() )
    return m_noPath;

  if( m_programs[name]->defaultBin() != 0 )
    return m_programs[name]->defaultBin()->path;
  else
    return m_noPath;
}


const K3bExternalBin* K3bExternalBinManager::binObject( const QString& name )
{
  if( m_programs.find( name ) == m_programs.end() )
    return 0;

  return m_programs[name]->defaultBin();
}


void K3bExternalBinManager::addProgram( K3bExternalProgram* p )
{
  m_programs.insert( p->name(), p );
}


void K3bExternalBinManager::clear()
{
  for( QMap<QString, K3bExternalProgram*>::Iterator it = m_programs.begin(); it != m_programs.end(); ++it )
    delete it.data();
  m_programs.clear();
}


void K3bExternalBinManager::search()
{
  if( m_searchPath.isEmpty() )
    loadDefaultSearchPath();

  for( QMap<QString, K3bExternalProgram*>::iterator it = m_programs.begin(); it != m_programs.end(); ++it ) {
    it.data()->clear();
  }

  // do not search one path twice
  QStringList paths;
  for( QStringList::const_iterator it = m_searchPath.begin(); it != m_searchPath.end(); ++it ) {
    QString p = *it;
    if( p[p.length()-1] == '/' )
      p.truncate( p.length()-1 );
    if( !paths.contains( p ) && !paths.contains( p + "/" ) )
      paths.append(p);
  }

  // get the environment path variable
  char* env_path = ::getenv("PATH");
  if( env_path ) {
    QStringList env_pathList = QStringList::split(":", QString::fromLocal8Bit(env_path));
    for( QStringList::const_iterator it = env_pathList.begin(); it != env_pathList.end(); ++it ) {
      QString p = *it;
      if( p[p.length()-1] == '/' )
	p.truncate( p.length()-1 );
      if( !paths.contains( p ) && !paths.contains( p + "/" ) )
	paths.append(p);
    }
  }


  for( QStringList::const_iterator it = paths.begin(); it != paths.end(); ++it )
    for( QMap<QString, K3bExternalProgram*>::iterator pit = m_programs.begin(); pit != m_programs.end(); ++pit )
      pit.data()->scan(*it);

  // TESTING
  // /////////////////////////
  const K3bExternalBin* bin = program("cdrecord")->defaultBin();

  if( !bin ) {
    kdDebug() << "(K3bExternalBinManager) Probing cdrecord failed" << endl;
  }
  else {
    kdDebug() << "(K3bExternalBinManager) Cdrecord " << bin->version << " features: "
	      << bin->features().join( ", " ) << endl;

    if( bin->version >= K3bExternalBinVersion("1.11a02") )
      kdDebug() << "(K3bExternalBinManager) "
		<< bin->version.majorVersion() << " " << bin->version.minorVersion() << " " << bin->version.patchLevel()
		<< " " << bin->version.suffix()
		<< " seems to be cdrecord version >= 1.11a02, using burnfree instead of burnproof" << endl;
    if( bin->version >= K3bExternalBinVersion("1.11a31") )
      kdDebug() << "(K3bExternalBinManager) seems to be cdrecord version >= 1.11a31, support for Just Link via burnfree "
		<< "driveroption" << endl;
  }
}


K3bExternalProgram* K3bExternalBinManager::program( const QString& name ) const
{
  if( m_programs.find( name ) == m_programs.end() )
    return 0;
  else
    return m_programs[name];
}


void K3bExternalBinManager::loadDefaultSearchPath()
{
  static const char* defaultSearchPaths[] = { "/usr/bin/",
					      "/usr/local/bin/",
					      "/usr/sbin/",
					      "/usr/local/sbin/",
					      "/opt/schily/bin/",
					      "/sbin",
					      0 };

  m_searchPath.clear();
  for( int i = 0; defaultSearchPaths[i]; ++i ) {
    m_searchPath.append( defaultSearchPaths[i] );
  }
}


void K3bExternalBinManager::setSearchPath( const QStringList& list )
{
  loadDefaultSearchPath();

  for( QStringList::const_iterator it = list.begin(); it != list.end(); ++it ) {
    if( !m_searchPath.contains( *it ) )
      m_searchPath.append( *it );
  }
}


void K3bExternalBinManager::addSearchPath( const QString& path )
{
  if( !m_searchPath.contains( path ) )
    m_searchPath.append( path );
}



K3bExternalBinManager* K3bExternalBinManager::self()
{
  static K3bExternalBinManager* instance = 0;
  if( !instance )
    instance = new K3bExternalBinManager();
  return instance;
}


#include "k3bexternalbinmanager.moc"

