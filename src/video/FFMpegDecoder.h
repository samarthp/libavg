//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#ifndef _FFMpegDecoder_H_
#define _FFMpegDecoder_H_

#include "IVideoDecoder.h"
#include "IDemuxer.h"

#include "../base/ProfilingZone.h"
#include "../avgconfigwrapper.h"

#ifndef _WIN32
// This is probably GCC-specific.
#define INT64_C(c)    c ## L
#endif
extern "C" {
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#ifdef AVG_ENABLE_SWSCALE
#include <ffmpeg/swscale.h>
#endif
}
#include <boost/thread/mutex.hpp>

namespace avg {

class FFMpegDecoder: public IVideoDecoder
{
    public:
        FFMpegDecoder();
        virtual ~FFMpegDecoder();
        virtual void open(const std::string& sFilename, YCbCrMode ycbcrMode,
                bool bThreadedDemuxer);
        virtual void close();
        virtual void seek(int DestFrame);
        virtual IntPoint getSize();
        virtual int getNumFrames();
        virtual double getFPS();
        virtual void setFPS(double FPS);
        virtual PixelFormat getPixelFormat();

        virtual FrameAvailableCode renderToBmp(BitmapPtr pBmp, long long TimeWanted);
        virtual FrameAvailableCode renderToYCbCr420p(BitmapPtr pBmpY, BitmapPtr pBmpCb, 
                BitmapPtr pBmpCr, long long TimeWanted);
        virtual long long getCurFrameTime();
        virtual bool isEOF();
        
        virtual void setAudioEnabled(bool bEnabled);
        virtual void fillAudioFrame(unsigned char* audioBuffer, int audioBufferSize, 
                int channels, int rate);

    private:
        void initVideoSupport();
        FrameAvailableCode readFrameForTime(AVFrame& Frame, long long TimeWanted);
        void readFrame(AVFrame& Frame, long long& FrameTime);
        PixelFormat calcPixelFormat(YCbCrMode ycbcrMode);
        void convertFrameToBmp(AVFrame& Frame, BitmapPtr pBmp);
        long long getFrameTime(AVPacket* pPacket);
        double calcStreamFPS();
        int copyRawAudio(unsigned char* buf, int size);
        int copyResampledAudio(unsigned char* buf, int size, int outputChannels, int outputRate);
        void resampleAudio(int channels, int rate);
        int decodeAudio();

        IDemuxer * m_pDemuxer;
        AVFormatContext * m_pFormatContext;
        int m_VStreamIndex;
        AVStream * m_pVStream;
        int m_AStreamIndex;
        AVStream * m_pAStream;
        bool m_bEOFPending;
        bool m_bEOF;
        bool m_bAudioEnabled;
        PixelFormat m_PF;
#ifdef AVG_ENABLE_SWSCALE
        SwsContext * m_pSwsContext;
#endif

        AVPacket * m_AudioPacket;
        unsigned char * m_AudioPacketData;
        int m_AudioPacketSize;
        char * m_pSampleBuffer;
        int m_SampleBufferStart;
        int m_SampleBufferEnd;
        int m_SampleBufferSize;
        char * m_pResampleBuffer;
        int m_ResampleBufferEnd;
        int m_ResampleBufferStart;
        int m_ResampleBufferSize;
        ReSampleContext * m_pAudioResampleContext;
        boost::mutex m_AudioMutex;
        
        long long m_AudioClock;
        long long m_AudioClockStart;
        
        unsigned char * m_pPacketData;
        AVPacket * m_pPacket;
        int m_PacketLenLeft;
        bool m_bFirstPacket;
        std::string m_sFilename;
        IntPoint m_Size;

        double m_TimeUnitsPerSecond;
        long long m_StartTimestamp;
        long long m_LastFrameTime;
        bool m_bUseStreamFPS;
        double m_FPS;
        long long m_StreamTimeOffset;

        ProfilingZone * m_pRenderToBmpProfilingZone;
        ProfilingZone * m_pConvertImageProfilingZone;

        static bool m_bInitialized;
        // Prevents different decoder instances from executing open/close simultaneously
        static boost::mutex s_OpenMutex;   
};

}
#endif 

