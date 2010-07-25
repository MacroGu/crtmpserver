/* 
 *  Copyright (c) 2010,
 *  Gavriloaie Eugen-Andrei (shiretu@gmail.com)
 *
 *  This file is part of crtmpserver.
 *  crtmpserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  crtmpserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with crtmpserver.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _BASEINFILESTREAM_H
#define	_BASEINFILESTREAM_H

#include "streaming/baseinstream.h"
#include "buffering/iobuffer.h"
#include "mediaformats/mediaframe.h"
#include "protocols/timer/basetimerprotocol.h"

#ifdef HAS_MMAP
#include "utils/mmapfile.h"
#define FileClass MmapFile
#else
#include "utils/file.h"
#define FileClass File
#endif

class DLLEXP BaseInFileStream
: public BaseInStream {
private:

	class InFileStreamTimer
	: public BaseTimerProtocol {
	private:
		BaseInFileStream *_pInFileStream;
	public:
		InFileStreamTimer(BaseInFileStream *pInFileStream);
		virtual ~InFileStreamTimer();
		void ResetStream();
		virtual bool TimePeriodElapsed();
	};
	friend class InFileStreamTimer;
#ifndef HAS_MMAP
	static map<string, pair<uint32_t, File *> > _fileCache;
#endif /* HAS_MMAP */

	InFileStreamTimer *_pTimer;

	FileClass *_pSeekFile;
	FileClass *_pFile;

	//frame info
	uint32_t _totalFrames;
	uint32_t _currentFrameIndex;
	MediaFrame _currentFrame;

	//timing info
	uint32_t _totalSentTime;
	uint32_t _totalSentTimeBase;
	time_t _startFeedingTime;

	//buffering info
	int32_t _clientSideBufferLength;
	IOBuffer _videoBuffer;
	IOBuffer _audioBuffer;

	//current state info
	bool _paused;
	bool _audioVideoCodecsSent;
public:
	BaseInFileStream(BaseProtocol *pProtocol, StreamsManager *pStreamsManager,
			uint64_t type, string name);
	virtual ~BaseInFileStream();

	/*
	 * Returns the stream capabilities. Specifically, codec and codec related info
	 * */
	virtual StreamCapabilities * GetCapabilities();

	/*
	 * Extracts the complete metadata from partial metadata
	 * metaData - the partial metadata containing at least the media file name
	 * */
	static bool ResolveCompleteMetadata(Variant &metaData);

	/*
	 * This will initialize the stream internally.
	 * clientSideBufferLength - the client side buffer length expressed in seconds
	 * */
	bool Initialize(int32_t clientSideBufferLength);

	/*
	 * Called when a play command was issued
	 * absoluteTimestamp - the timestamp where we want to seek
	 *                     before start the feeding process
	 * */
	virtual bool SignalPlay(double &absoluteTimestamp, double &length);

	/*
	 * Called when a pasue command was issued
	 * */
	virtual bool SignalPause();

	/*
	 * Called when a resume command was issued
	 * */
	virtual bool SignalResume();

	/*
	 * Called when a seek command was issued
	 * absoluteTimestamp - the
	 * */
	virtual bool SignalSeek(double &absoluteTimestamp);

	/*
	 * Called when a stop command was issued
	 * */
	virtual bool SignalStop();

	/*
	 * This is called by the framework. The networking layer signaled the
	 * availability for sending data
	 * */
	virtual void ReadyForSend();

protected:
	virtual bool BuildFrame(FileClass *pFile, MediaFrame &mediaFrame,
			IOBuffer &buffer) = 0;
private:
	/*
	 * This will seek to the specified point in time.
	 * absoluteTimestamp - the timestamp where we want to seek
	 *                     before start the feeding process
	 * */
	bool InternalSeek(double &absoluteTimestamp);

	/*
	 * This is the function that will actually do the feeding.
	 * It is called by the framework and it must deliver one frame at a time
	 * to all subscribers
	 * */
	virtual bool Feed();

	/*
	 * GetFile function will open a file and will cache it if is a regular file.
	 * If the file is mmap based file, it will NOT cache it
	 * ReleaseFile will do the opposite: It will close the file if the references
	 * count will reach 0. This always happens in case of mmap file
	 * */
#ifdef HAS_MMAP
	static MmapFile* GetFile(string filePath, uint32_t windowSize);
	static void ReleaseFile(MmapFile *pFile);
#else
	static File* GetFile(string filePath, uint32_t windowSize);
	static void ReleaseFile(File *pFile);
#endif /* HAS_MMAP */

	/*
	 * This function will ensure that the codec packets are sent.
	 * Also it preserves the current timings and frame index
	 */
	bool SendCodecs();
};

#endif	/* _BASEINFILESTREAM_H */

