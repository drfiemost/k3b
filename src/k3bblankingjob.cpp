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

#include "k3bblankingjob.h"

#include "k3b.h"
#include "tools/k3bglobals.h"
#include "device/k3bdevice.h"
#include "tools/k3bexternalbinmanager.h"

#include <kprocess.h>
#include <kconfig.h>
#include <klocale.h>
#include <kio/global.h>
#include <kio/job.h>

#include <qstring.h>
#include <kdebug.h>



K3bBlankingJob::K3bBlankingJob( QObject* parent )
  : K3bJob( parent )
{
  m_process = new KProcess();
  m_device = 0;
  m_mode = Fast;
  m_speed = 1;

  // connect to the cdrecord slots
  connect( m_process, SIGNAL(processExited(KProcess*)),
	   this, SLOT(slotCdrecordFinished()) );
  connect( m_process, SIGNAL(receivedStderr(KProcess*, char*, int)),
	   this, SLOT(slotParseCdrecordOutput(KProcess*, char*, int)) );
  connect( m_process, SIGNAL(receivedStdout(KProcess*, char*, int)),
	   this, SLOT(slotParseCdrecordOutput(KProcess*, char*, int)) );
}


K3bBlankingJob::~K3bBlankingJob()
{
  delete m_process;
}


void K3bBlankingJob::setDevice( K3bDevice* dev )
{
  m_device = dev;
}


void K3bBlankingJob::start()
{
  if( m_process->isRunning() )
    return;

  if( m_device == 0 )
    return;

  if( !KIO::findDeviceMountPoint( m_device->mountDevice() ).isEmpty() ) {
    // TODO: enable me after message freeze
    // emit infoMessage( i18n("Unmounting disk"), INFO );
    // unmount the cd
    connect( KIO::unmount( m_device->mountPoint(), false ), SIGNAL(result(KIO::Job*)),
	     this, SLOT(slotStartErasing()) );
  }
  else {
    slotStartErasing();
  }
}


void K3bBlankingJob::slotStartErasing()
{
  m_process->clearArguments();

  if( !k3bMain()->externalBinManager()->foundBin( "cdrecord" ) ) {
    kdDebug() << "(K3bBlankingJob) could not find cdrecord executable" << endl;
    emit infoMessage( i18n("cdrecord executable not found."), K3bJob::ERROR );

    emit finished( false );
    return;
  }

  *m_process << k3bMain()->externalBinManager()->binPath( "cdrecord" );
  *m_process << QString("dev=%1").arg( m_device->busTargetLun() );
  *m_process << QString("speed=%1").arg(m_speed);
  if( m_force )
    *m_process << "-force";
  if( k3bMain()->eject() )
    *m_process << "-eject";

  if( k3bMain()->externalBinManager()->binObject( "cdrecord" )->hasFeature( "gracetime") )
    *m_process << "gracetime=2";  // 2 is the lowest allowed value (Joerg, why do you do this to us?)


  QString mode;
  switch( m_mode ) {
  case Fast:
    mode = "fast";
    break;
  case Complete:
    mode = "all";
    break;
  case Track:
    mode = "track";
    break;
  case Unclose:
    mode = "unclose";
    break;
  case Session:
    mode = "session";
    break;
  }

  *m_process << "blank=" + mode;


  // debugging output
  kdDebug() << "***** cdrecord parameters:\n";
  for( QValueList<QCString>::const_iterator it = m_process->args().begin();
       it != m_process->args().end(); ++it ) {  
    kdDebug() << *it << " ";
  }
  kdDebug() << endl << flush;



  // now we can start the process
  if( !m_process->start( KProcess::NotifyOnExit, KProcess::AllOutput ) )
    {
      // something went wrong when starting the program
      // it "should" be the executable
      kdDebug() << "(K3bBlankingJob) could not start cdrecord" << endl;
      emit infoMessage( i18n("Could not start cdrecord!"), K3bJob::ERROR );
      emit finished( false );
    }
  else
    {
      emit infoMessage( i18n("Start erasing disk at speed %1...").arg(m_speed), K3bJob::STATUS );
    }
}


void K3bBlankingJob::cancel()
{
  if( m_process->isRunning() ) {
    m_process->kill();

    // we need to unlock the writer because cdrecord locked it while writing
    bool block = m_device->block( false );
    if( !block )
      emit infoMessage( i18n("Could not unlock CD drive."), K3bJob::ERROR );
    //    else if( k3bMain()->eject() )
    // m_device->eject();

    emit canceled();
    emit finished( false );
  }
}


void K3bBlankingJob::slotParseCdrecordOutput( KProcess*, char* data, int len )
{
  kdDebug() << QString::fromLatin1( data, len) << endl;
}


void K3bBlankingJob::slotCdrecordFinished()
{
  if( m_process->normalExit() )
    {
      // TODO: check the process' exitStatus()
      switch( m_process->exitStatus() )
	{
	case 0:
	  emit infoMessage( i18n("Process completed successfully"), K3bJob::STATUS );
	  emit finished( true );
	  break;
				
	default:
	  // no recording device and also other errors!! :-(
	  emit infoMessage( i18n("cdrecord returned an error!"), K3bJob::ERROR );
	  emit infoMessage( i18n("Sorry, no error handling yet! :-(("), K3bJob::ERROR );
	  emit finished( false );
	  break;
	}
    }
  else
    {
      emit infoMessage( i18n("cdrecord did not exit cleanly."), K3bJob::ERROR );
      emit finished( false );
    }
}


bool K3bBlankingJob::active() const
{
  return m_process->isRunning();
}


#include "k3bblankingjob.moc"
