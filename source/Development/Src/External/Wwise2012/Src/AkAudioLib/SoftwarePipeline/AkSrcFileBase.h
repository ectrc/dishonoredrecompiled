/*=============================================================================
	AkSrcFileBase.h - CAkSrcFileBase (aksrcfilebase.cpp/.h): a source streamed from a file through an
	AK::IAkAutoStream - stream creation, prefetched data, stream looping ahead of the decoder, seeking.

	DISHONORED(port): agent IH2 fork S (wave 26). Layout is the 2012 PDB's (84 bytes); ParseHeader / GetThroughput /
	FindClosestFileOffset are the three virtuals it adds after CAkSrcBaseEx's.
=============================================================================*/
#pragma once

#include "AkSrcBase.h"
#include "AkSrcFileServices.h"

class CAkSrcFileBase : public CAkSrcBaseEx
{
public:
	explicit CAkSrcFileBase( CAkPBI * in_pCtx );
	virtual ~CAkSrcFileBase();

	virtual void VirtualOn( AkVirtualQueueBehavior eBehavior );
	virtual AKRESULT VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset );
	virtual AKRESULT StartStream();
	virtual void StopStream();
	virtual AKRESULT StopLooping();
	virtual AKRESULT ChangeSourcePosition();

	AK::IAkAutoStream * m_pStream;
	AkUInt8 * m_pNextAddress;
	AkUInt32 m_ulSizeLeft;
	AkUInt32 m_ulFileOffset;
	AkUInt32 m_uiCorrection;
	AkUInt32 m_ulLoopStart;
	AkUInt32 m_ulLoopEnd;
	AkUInt16 m_uStreamLoopCntAhead;
	AkUInt8 m_bIsLastStmBuffer : 1;
	AkUInt8 m_bSkipBufferRelease : 1;
	AkUInt8 m_bFormatReady : 1;

protected:
	virtual AKRESULT OnLoopComplete( bool in_bEndOfFile );
	virtual AKRESULT ParseHeader( AkUInt8 * in_pBuffer ) = 0;
	virtual AkReal32 GetThroughput( const AkAudioFormat & in_rFormat ) = 0;
	virtual AKRESULT FindClosestFileOffset( AkUInt32 in_uDesiredSample, AkUInt32 & out_uSeekedSample, AkUInt32 & out_uFileOffset ) = 0;

	AKRESULT CreateStream( AkAutoStmBufSettings & in_bufSettings, AkUInt8 in_uMinNumBuffers );
	AKRESULT HandlePrefetch( bool & out_bUsePrefetchedData );
	AKRESULT ProcessFirstBuffer();
	AKRESULT ProcessStreamBuffer( AkUInt8 * in_pBuffer );
	AKRESULT FetchStreamBuffer();
	void ReleaseStreamBuffer();
	AKRESULT IsInitialPrebufferingReady();
	AKRESULT SetStreamPosition( AkUInt32 in_uPosition );
	void GetStreamLoopHeuristic( bool in_bIsLooping, AkAutoStmHeuristics & io_heuristics );
	AKRESULT SeekStream( AkUInt32 in_uDesiredSample, AkUInt32 & out_uSeekedSample );
	AKRESULT SeekToSourceOffset();
	void ResetStreamingAfterSeek();
};
static_assert( sizeof( CAkSrcFileBase ) == 84, "CAkSrcFileBase: 2012 PDB sizeof 84" );
