/* 
 *
 * $Id$
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2005 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "k3baudiomaxspeedjob.h"
#include "k3baudiotrack.h"
#include "k3baudiodatasource.h"
#include "k3baudiodoc.h"
#include <k3bdevice.h>

#include <k3bthread.h>

#include <kdebug.h>
#include <klocale.h>

#include <qdatetime.h>


class K3bAudioMaxSpeedJob::WorkThread : public K3bThread
{
public:
  WorkThread( K3bAudioDoc* doc )
    : K3bThread(),
      m_doc(doc),
      m_canceled(false) {
    m_buffer = new char[2352*10];
  }

  ~WorkThread() {
    delete [] m_buffer;
  }

  void run() {
    m_canceled = false;

    emitStarted();

    K3bAudioTrack* track = m_doc->firstTrack();
    K3bAudioDataSource* source = track->firstSource();
    bool success = true;
    QTime t;
    maxSpeed = 175*1000;

    while( source && !m_canceled ) {
      if( !source->seek(0) ) {
	kdDebug() << "(K3bAudioMaxSpeedJob) seek failed." << endl;
	success = false;
	break;
      }
      
      // start the timer
      t.start();

      // read some data
      int data = speedTest( source );

      // elapsed millisec
      int usedT = t.elapsed();

      if( data < 0 ) {
	success = false;
	break;
      }

      // KB/sec (add 1 millisecond to avoid division by 0)
      int throughput = (data*1000+usedT)/(usedT+1)/1024;
      kdDebug() << "(K3bAudioMaxSpeedJob) throughput: " << throughput 
		<< " (" << data << "/" << usedT << ")" << endl;

      // update the max speed
      maxSpeed = QMIN( maxSpeed, throughput );

      // next source
      source = source->next();
      if( !source ) {
	track = track->next();
	if( track )
	  source = track->firstSource();
      }
    }

    if( m_canceled ) {
      emitCanceled();
      success = false;
    }

    if( success )
      kdDebug() << "(K3bAudioMaxSpeedJob) max speed: " << maxSpeed << endl;

    emitFinished( success );
  }

  // returns the amount of data read from this source
  int speedTest( K3bAudioDataSource* source ) {
    int dataRead = 0;
    int r = 0;
    // read ten seconds of audio data. This is some value which seemed about right. :)
    while( dataRead < 2352*75*10 && (r = source->read( m_buffer, 2352*10 )) > 0 ) {
      dataRead += r;
    }

    if( r < 0 ) {
      kdDebug() << "(K3bAudioMaxSpeedJob) read failure." << endl;
      return -1;
    }

    return dataRead;
  }

  void cancel() {
    m_canceled = true;
  }

  int maxSpeedByMedia() const {
    int s = 0;
    
    QValueList<int> speeds = m_doc->burner()->determineSupportedWriteSpeeds();
    // simply use what we have and let the writer decide if the speeds are empty
    if( !speeds.isEmpty() ) {
      // start with the highest speed and go down the list until we are below our max
      QValueListIterator<int> it = speeds.end();
      --it;
      while( *it > maxSpeed && it != speeds.begin() )
	--it;
      
      // this is the first valid speed or the lowest supported one
      s = *it;
      kdDebug() << "(K3bAudioMaxSpeedJob) using speed factor: " << (s/175) << endl;
    }

    return s;
  }

  int maxSpeed;

private:
  K3bAudioDoc* m_doc;
  bool m_canceled;
  char* m_buffer;
};


K3bAudioMaxSpeedJob::K3bAudioMaxSpeedJob( K3bAudioDoc* doc, K3bJobHandler* jh, QObject* parent, const char* name )
  : K3bThreadJob( jh, parent, name )
{
  m_thread = new WorkThread( doc );
  setThread( m_thread );
}


K3bAudioMaxSpeedJob::~K3bAudioMaxSpeedJob()
{
  delete m_thread;
}


int K3bAudioMaxSpeedJob::maxSpeed() const
{
  return m_thread->maxSpeedByMedia();
}
