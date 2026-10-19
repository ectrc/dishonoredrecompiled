/*=============================================================================
	AkSrcBankVorbis.h - CAkSrcBankVorbis (aksrcbankvorbis.cpp/.h): a Wwise Vorbis source decoded from in-memory media.

	DISHONORED(port): agent IH2 fork S (wave 26). Layout is the 2012 PDB's (236 bytes, CreateVorbisBankPlugin mallocs
	0xEC). The constructor is the header inline CreateVorbisBankPlugin expands.
=============================================================================*/
#pragma once

#include "AkSrcBase.h"
#include "AkVorbisCodec.h"

class CAkSrcBankVorbis : public CAkSrcBaseEx
{
public:
	explicit CAkSrcBankVorbis( CAkPBI * in_pCtx )
		: CAkSrcBaseEx( in_pCtx )
		, m_pucData( NULL )
		, m_pucDataStart( NULL )
	{
		memset( &m_VorbisState, 0, sizeof( m_VorbisState ) );
	}
	virtual ~CAkSrcBankVorbis();

	virtual void ReleaseBuffer();
	virtual AKRESULT VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset );
	virtual AKRESULT StartStream();
	virtual void StopStream();
	virtual void GetBuffer( AkVPLState & io_state );
	virtual AKRESULT StopLooping();
	virtual AKRESULT ChangeSourcePosition();

	AkVorbisSourceState m_VorbisState;
	AkUInt8 * m_pucData;
	AkUInt8 * m_pucDataStart;

protected:
	virtual AKRESULT OnLoopComplete( bool in_bEndOfFile );

	AKRESULT DecodeVorbisHeader();
	AKRESULT SeekToNativeOffset();
	AKRESULT VirtualSeek( AkUInt32 & io_uSeekPosition );

private:
	void VorbisDSPRestart( AkUInt16 in_uSrcOffsetRemainder );
};
static_assert( sizeof( CAkSrcBankVorbis ) == 0xEC, "CAkSrcBankVorbis: CreateVorbisBankPlugin mallocs 0xEC" );
