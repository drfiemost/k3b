/*
 *
 * Copyright (C) 2003-2009 Sebastian Trueg <trueg@k3b.org>
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


#include "k3bwavefilewriter.h"
#include <kdebug.h>

K3b::WaveFileWriter::WaveFileWriter()
    : m_outputStream( &m_outputFile )
{
}


K3b::WaveFileWriter::~WaveFileWriter()
{
    close();
}


bool K3b::WaveFileWriter::open( const QString& filename )
{
    close();

    m_outputFile.setFileName( filename );

    if( m_outputFile.open( QIODevice::ReadWrite ) ) {
        m_filename = filename;

        writeEmptyHeader();

        return true;
    }
    else {
        return false;
    }
}


void K3b::WaveFileWriter::close()
{
    if( isOpen() ) {
        if( m_outputFile.pos() > 0 ) {
            padTo2352();

            // update wave header
            updateHeader();

            m_outputFile.close();
        }
        else {
            m_outputFile.close();
            m_outputFile.remove();
        }
    }

    m_filename = QString();
}


bool K3b::WaveFileWriter::isOpen()
{
    return m_outputFile.isOpen();
}


const QString& K3b::WaveFileWriter::filename() const
{
    return m_filename;
}


void K3b::WaveFileWriter::write( const char* data, int len, Endianess e )
{
    if( isOpen() ) {
        if( e == LittleEndian ) {
            m_outputStream.writeRawData( data, len );
        }
        else {
            if( len % 2 > 0 ) {
                kDebug() << "(K3b::WaveFileWriter) data length ("
                         << len << ") is not a multiple of 2! Cannot swap bytes." << endl;
                return;
            }

            // we need to swap the bytes
            char* buffer = new char[len];
            for( int i = 0; i < len-1; i+=2 ) {
                buffer[i] = data[i+1];
                buffer[i+1] = data[i];
            }
            m_outputStream.writeRawData( buffer, len );

            delete [] buffer;
        }
    }
}


void K3b::WaveFileWriter::writeEmptyHeader()
{
    static const char riffHeader[] =
        {
            (char)0x52, (char)0x49, (char)0x46, (char)0x46, // 0  "RIFF"
            (char)0x00, (char)0x00, (char)0x00, (char)0x00, // 4  wavSize
            (char)0x57, (char)0x41, (char)0x56, (char)0x45, // 8  "WAVE"
            (char)0x66, (char)0x6d, (char)0x74, (char)0x20, // 12 "fmt "
            (char)0x10, (char)0x00, (char)0x00, (char)0x00, // 16
            (char)0x01, (char)0x00, (char)0x02, (char)0x00, // 20
            (char)0x44, (char)0xac, (char)0x00, (char)0x00, // 24
            (char)0x10, (char)0xb1, (char)0x02, (char)0x00, // 28
            (char)0x04, (char)0x00, (char)0x10, (char)0x00, // 32
            (char)0x64, (char)0x61, (char)0x74, (char)0x61, // 36 "data"
            (char)0x00, (char)0x00, (char)0x00, (char)0x00  // 40 byteCount
        };

    m_outputStream.writeRawData( riffHeader, 44 );
}


void K3b::WaveFileWriter::updateHeader()
{
    if( isOpen() ) {

        m_outputFile.flush();

        qint32 dataSize( m_outputFile.pos() - 44 );
        qint32 wavSize(dataSize + 44 - 8);
        char c[4];

        // jump to the wavSize position in the header
        if( m_outputFile.seek( 4 ) ) {
            c[0] = (wavSize   >> 0 ) & 0xff;
            c[1] = (wavSize   >> 8 ) & 0xff;
            c[2] = (wavSize   >> 16) & 0xff;
            c[3] = (wavSize   >> 24) & 0xff;
            m_outputStream.writeRawData( c, 4 );
        }
        else
            kDebug() << "(K3b::WaveFileWriter) unable to seek in file: " << m_outputFile.fileName();

        if( m_outputFile.seek( 40 ) ) {
            c[0] = (dataSize   >> 0 ) & 0xff;
            c[1] = (dataSize   >> 8 ) & 0xff;
            c[2] = (dataSize   >> 16) & 0xff;
            c[3] = (dataSize   >> 24) & 0xff;
            m_outputStream.writeRawData( c, 4 );
        }
        else
            kDebug() << "(K3b::WaveFileWriter) unable to seek in file: " << m_outputFile.fileName();

        // jump back to the end
        m_outputFile.seek( m_outputFile.size() );
    }
}


void K3b::WaveFileWriter::padTo2352()
{
    int bytesToPad = ( m_outputFile.pos() - 44 ) % 2352;
    if( bytesToPad > 0 ) {
        kDebug() << "(K3b::WaveFileWriter) padding wave file with " << bytesToPad << " bytes.";

        char* c = new char[bytesToPad];
        memset( c, 0, bytesToPad );
        m_outputStream.writeRawData( c, bytesToPad );
        delete [] c;
    }
}


int K3b::WaveFileWriter::fd() const
{
    return m_outputFile.handle();
}
