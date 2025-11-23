/*
 *
 *
 * Copyright (C) 2004-2008 Sebastian Trueg <trueg@k3b.org>
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
#include "k3bffmpegwrapper.h"

#include <config-k3b.h>

extern "C" {
/*
 Recent versions of FFmepg uses C99 constant macros which are not presebt in C++ standard.
 The macro __STDC_CONSTANT_MACROS allow C++ to use these macros. Altough it's not defined by C++ standard
 it's supported by many implementations.
 See bug 236036 and discussion: http://lists.mplayerhq.hu/pipermail/ffmpeg-devel/2010-May/088074.html
 */
#define __STDC_CONSTANT_MACROS
#ifdef NEWFFMPEGAVCODECPATH
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#else
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#endif
}

#include <cstring>

#include <klocale.h>


#ifndef HAVE_FFMPEG_CODEC_MP3
#  define CODEC_ID_MP3 CODEC_ID_MP3LAME
#endif

// Has been deprecated for ages. Now removed. For now do what ffmpeg does
// http://ffmpeg.org/pipermail/ffmpeg-cvslog/2012-August/053785.html
// 192000 = 1 second of 48khz 32bit audio
#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
#endif

K3bFFMpegWrapper* K3bFFMpegWrapper::s_instance = nullptr;


class K3bFFMpegFile::Private
{
public:
    ::AVFormatContext* formatContext;
    const ::AVCodec* codec;
    ::AVCodecContext* codecContext;
    ::AVStream *audio_stream;

    K3b::Msf length;

    // for decoding. ffmpeg requires 16-byte alignment.
    ::AVFrame* frame = nullptr;
    ::AVPacket* packet = nullptr;
    char* outputBufferPos = nullptr;
    int outputBufferSize = 0;
    ::AVSampleFormat sampleFormat;
    bool isSpacious;
};


K3bFFMpegFile::K3bFFMpegFile( const QString& filename )
    : m_filename(filename)
{
    d = new Private;
    d->formatContext = nullptr;
    d->codec = nullptr;
    d->audio_stream = nullptr;
    d->frame = av_frame_alloc();
}


K3bFFMpegFile::~K3bFFMpegFile()
{
    close();
#ifdef HAVE_FFMPEG_AVCODEC_DECODE_AUDIO4
#  if LIBAVCODEC_BUILD < AV_VERSION_INT(55,28,0)
    av_free(d->frame);
#  else
    av_frame_free(&d->frame);
#  endif
#endif
    delete d;
}


bool K3bFFMpegFile::open()
{
    close();

    // open the file
    int err = ::avformat_open_input( &d->formatContext, m_filename.toLocal8Bit().constData(), 0, 0 );
    if( err < 0 ) {
        kDebug() << "(K3bFFMpegFile) unable to open " << m_filename << " with error " << err;
        return false;
    }

    // analyze the streams
    ::avformat_find_stream_info( d->formatContext, 0 );

    // we only handle files containing one audio stream
    if( d->formatContext->nb_streams != 1 ) {
        kDebug() << "(K3bFFMpegFile) more than one stream in " << m_filename;
        return false;
    }

    // get the codec
    d->codec = ::avcodec_find_decoder(
        d->audio_stream->codecpar->codec_id
    );
    if( !d->codec ) {
        kDebug() << "(K3bFFMpegFile) no codec found for " << m_filename;
        return false;
    }

    // get the codec context
    d->codecContext = ::avcodec_alloc_context3(d->codec);
    ::avcodec_parameters_to_context(d->codecContext, d->audio_stream->codecpar);
    if(d->codecContext->codec_type != AVMEDIA_TYPE_AUDIO) {
        qDebug() << "(K3bFFMpegFile) not a simple audio stream: " << m_filename;
        return false;
    }

    // open the codec on our context
    kDebug() << "(K3bFFMpegFile) found codec for " << m_filename;
    if( ::avcodec_open2( d->codecContext, d->codec, 0 ) < 0 ) {
        kDebug() << "(K3bFFMpegDecoderFactory) could not open codec.";
        return false;
    }

    // determine the length of the stream
    d->length = K3b::Msf::fromSeconds(
        static_cast<double>(d->formatContext->duration) / static_cast<double>(AV_TIME_BASE));

    if( d->length == 0 ) {
        kDebug() << "(K3bFFMpegDecoderFactory) invalid length.";
        return false;
    }

    d->sampleFormat = d->codecContext->sample_fmt;
#if LIBAVCODEC_VERSION_MAJOR < 61
    d->isSpacious = ::av_sample_fmt_is_planar(d->sampleFormat) &&
        d->codecContext->channels > 1;
#else
#pragma Unimplemented
    d->isSpacious = ::av_sample_fmt_is_planar(d->sampleFormat);
#endif
    d->packet = ::av_packet_alloc();

    // dump some debugging info
    ::av_dump_format( d->formatContext, 0, m_filename.toLocal8Bit().constData(), 0 );

    return true;
}


void K3bFFMpegFile::close()
{
    d->outputBufferSize = 0;
    ::av_packet_free(&d->packet);

    if( d->codec ) {
#if LIBAVCODEC_VERSION_MAJOR >= 60
        ::avcodec_free_context(&d->codecContext);
#else
        ::avcodec_close(d->codecContext);
#endif
        d->codec = nullptr;
        ::avcodec_free_context(&d->codecContext);
        d->codecContext = nullptr;
    }

    if( d->formatContext ) {
        ::avformat_close_input( &d->formatContext );
        d->formatContext = nullptr;
    }

    d->audio_stream = nullptr;
}


K3b::Msf K3bFFMpegFile::length() const
{
    return d->length;
}


int K3bFFMpegFile::sampleRate() const
{
    return d->codecContext->sample_rate;
}


int K3bFFMpegFile::channels() const
{
#if LIBAVCODEC_VERSION_MAJOR < 61
    return d->codecContext->channels;
#else
#pragma Unimplemented
    return 0;
#endif
}


int K3bFFMpegFile::type() const
{
    return d->codecContext->codec_id;
}


QString K3bFFMpegFile::typeComment() const
{
#if LIBAVCODEC_BUILD < AV_VERSION_INT(54,25,0)
    #define AV_CODEC_ID_WMAV1  CODEC_ID_WMAV1
    #define AV_CODEC_ID_WMAV2  CODEC_ID_WMAV2
    #define AV_CODEC_ID_MP3    CODEC_ID_MP3
    #define AV_CODEC_ID_AAC    CODEC_ID_AAC
#endif
    switch( type() ) {
    case AV_CODEC_ID_WAVPACK:
        return i18n("WavPack");
    case AV_CODEC_ID_APE:
        return i18n("Monkey's Audio (APE)");
    case AV_CODEC_ID_AAC:
        return i18n("Advanced Audio Coding (AAC)");
    default:
        return QString::fromLocal8Bit( d->codec->name );
    }
}


QString K3bFFMpegFile::title() const
{
    // FIXME: is this UTF8 or something??
    AVDictionaryEntry *ade = av_dict_get( d->formatContext->metadata, "TITLE", nullptr, 0 );
    return ade && ade->value[0] != '\0' ? QString::fromLocal8Bit( ade->value ) : QString();
}


QString K3bFFMpegFile::author() const
{
    // FIXME: is this UTF8 or something??
    AVDictionaryEntry *ade = av_dict_get( d->formatContext->metadata, "ARTIST", nullptr, 0 );
    return ade && ade->value[0] != '\0' ? QString::fromLocal8Bit( ade->value ) : QString();
}


QString K3bFFMpegFile::comment() const
{
    // FIXME: is this UTF8 or something??
    AVDictionaryEntry *ade = av_dict_get( d->formatContext->metadata, "COMMENT", nullptr, 0 );
    return ade && ade->value[0] != '\0' ? QString::fromLocal8Bit( ade->value ) : QString();
}


int K3bFFMpegFile::read( char* buf, int bufLen )
{
    if (!buf ) {
        return -1;
    }

    if(d->outputBufferSize <= 0)
        d->outputBufferPos = new char[bufLen];

    int ret = fillOutputBuffer();
    if (ret <= 0) {
        return ret;
    }

    int len = std::min(bufLen, ret);
    ::memcpy( buf, d->outputBufferPos, len );

    // TODO: only swap if needed
    for(int i = 0; i < len-1; i += 2)
        std::swap(buf[i], buf[i+1]); // BE -> LE

    d->outputBufferPos += len;
    if(d->outputBufferSize > 0)
        d->outputBufferSize -= len;
    else
        delete[] d->outputBufferPos;
    return len;
}


int K3bFFMpegFile::readPacket()
{
    if( ::av_read_frame( d->formatContext, d->packet ) < 0 ) {
        return 0;
    }

    return d->packet->size;
}


int K3bFFMpegFile::fillOutputBuffer()
{
    static int ret = -1;

    // decode if the output buffer is empty
    while( d->outputBufferSize <= 0 ) {
        d->outputBufferSize = 0;

    if ( ret >= 0 ) {
            ret = ::avcodec_receive_frame( d->codecContext, d->frame );

            if ( ret == AVERROR(EAGAIN) || ret == AVERROR_EOF ) {
                ret = -1;
                continue;
            }
            else if ( ret < 0 ) {
                qDebug() << "(K3bFFMpegFile) decoding failed for " << m_filename;
                return -1;
            }

            int nb_s = d->frame->nb_samples;
            const int nb_ch = 2;  // copy only 2 channels even if there're more
            d->outputBufferSize = nb_s * nb_ch * 2;  // 2 means 2 bytes (16bit)

            if( d->isSpacious ) {
                if( d->sampleFormat == AV_SAMPLE_FMT_FLTP ) {
                    constexpr int width = sizeof(float);  // sample width of float audio
                    for( int sample = 0; sample < nb_s; sample++ ) {
                        for( int ch = 0; ch < nb_ch; ch++ ) {
                            float val = *(reinterpret_cast<float*>(
                                d->frame->extended_data[ch] + sample * width));
                            val = ::abs(val) > 1 ? ::copysign(1.0, val) : val;
                            int16_t result = static_cast<int16_t>(
                                val * 32767.0 + 32768.5) - 32768;
                            ::memcpy(
                                d->outputBufferPos + (sample * nb_ch + ch) * 2,
                                &result,
                                2  // 2 is sample width of 16 bit audio
                            );
                        }
                    }
                } else if ( d->sampleFormat == AV_SAMPLE_FMT_S16P ) {
                    for(int sample = 0; sample < nb_s; sample++) {
                        for(int ch = 0; ch < nb_ch; ch++) {
                            ::memcpy(
                                d->outputBufferPos + (sample * nb_ch + ch) * 2,
                                d->frame->extended_data[ch] + sample * 2,
                                2 // 16 bit here as well
                            );
                        }
                    }
                } else {
                    qDebug() << "(K3bFFMpegFile) some planar formats are not supported yet";
                    return -1;
                }
            } else {
                qDebug() << "(K3bFFMpegFile) non planar and monophonic audio support is not implemented";
                return -1;
            }
        } else {
            // make sure we have data to decode
            if( readPacket() == 0 ) {
                return 0;
            }
            ret = ::avcodec_send_packet( d->codecContext, d->packet );
            if( ret < 0 ) {
                qDebug() << "(K3bFFMpegFile) error submitting packet to the decoder";
                return -1;
            }
            continue;
        }
    }

    return d->outputBufferSize;
}


bool K3bFFMpegFile::seek( const K3b::Msf& msf )
{
    d->outputBufferSize = 0;

    double seconds = (double)msf.totalFrames()/75.0;
    quint64 timestamp = (quint64)(seconds * (double)AV_TIME_BASE);

    // FIXME: do we really need the start_time and why?
#if LIBAVFORMAT_BUILD >= 4619
    return ( ::av_seek_frame( d->formatContext, -1, timestamp + d->formatContext->start_time, 0 ) >= 0 );
#else
    return ( ::av_seek_frame( d->formatContext, -1, timestamp + d->formatContext->start_time ) >= 0 );
#endif
}






K3bFFMpegWrapper::K3bFFMpegWrapper()
{
    //::av_register_all();
}


K3bFFMpegWrapper::~K3bFFMpegWrapper()
{
    s_instance = nullptr;
}


K3bFFMpegWrapper* K3bFFMpegWrapper::instance()
{
    if( !s_instance ) {
        s_instance = new K3bFFMpegWrapper();
    }

    return s_instance;
}


K3bFFMpegFile* K3bFFMpegWrapper::open( const QString& filename ) const
{
    K3bFFMpegFile* file = new K3bFFMpegFile( filename );
    if( file->open() ) {
#ifndef K3B_FFMPEG_ALL_CODECS
        //
        // only allow tested formats. ffmpeg seems not to be too reliable with every format.
        // mp3 being one of them sadly. Most importantly: allow the libsndfile decoder to do
        // its thing.
        //
        if( file->type() == AV_CODEC_ID_AAC ||
            file->type() == AV_CODEC_ID_APE ||
            file->type() == AV_CODEC_ID_WAVPACK )
#endif
            return file;
    }

    delete file;
    return nullptr;
}
