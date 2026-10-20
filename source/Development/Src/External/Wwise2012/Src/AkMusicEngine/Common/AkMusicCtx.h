/*=============================================================================
	AkMusicCtx.h - CAkMusicCtx (akmusicctx.cpp): the base of every music context. It is a child of its parent context,
	a transport-aware object the playing manager counts for the playing id, and a transition user for its
	play/stop and pause/resume fades, which it pushes down to its children as PBI fades. It is reference counted;
	the last release unlinks it from its parent (or from the music renderer at the top) and frees it.

	DISHONORED(port): agent IH4 (wave 26). Layout (2012 PDB, 60 bytes): CAkChildCtx (0), CAkTransportAware (+12),
	ITransitionable (+20), m_listChildren (+24), m_PBTrans (+28), m_uRefCount (+40), m_fPlayStopFadeRatio (+44),
	m_fPauseResumeFadeRatio (+48), m_uNumLastSamples (+52), m_uPauseCount (+56), status bits (+58: m_eStatus bits 0-3,
	m_bIsPaused bit 4).
=============================================================================*/
#pragma once

#include "AkChildCtx.h"
#include "AkMusicStructs.h"
#include <AkPBI.h>
#include <AkTransitionManager.h>

class CAkRegisteredObj;
class UserParams;

/** m_eStatus (akmusicctx.h). */
enum AkMusicCtxStatus
{
	CtxStatusIdle		= 0,
	CtxStatusPlaying	= 1,
	CtxStatusLastFrame	= 2,
	CtxStatusStopped	= 4
};

class CAkMusicCtx : public CAkChildCtx, public CAkTransportAware, public ITransitionable
{
public:
	explicit CAkMusicCtx( CAkMusicCtx * in_parent );
	virtual ~CAkMusicCtx();

	void Init( CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams );

	void _Play( AkMusicFade & in_fadeParams );
	void _Stop( TransParams & in_transParams, AkUInt32 in_uNumLastSamples );
	void _Pause( TransParams & in_transParams );
	void _Resume( TransParams & in_transParams, bool in_bIsMasterResume );
	void _Cancel();

	void AddChild( CAkChildCtx * in_pChildCtx );
	void RemoveChild( CAkChildCtx * in_pChildCtx );
	void AddRef() { ++m_uRefCount; }
	void Release();

	bool IsPlaying() const { return ( m_eStatus & ( CtxStatusPlaying | CtxStatusLastFrame ) ) != 0; }
	bool IsPaused() const { return m_bIsPaused != 0; }
	bool IsStopping() const { return ( m_eStatus & CtxStatusLastFrame ) != 0; }
	bool IsIdle() const { return m_eStatus == CtxStatusIdle; }
	AkUInt32 GetRegisteredNotif() const { return m_uRegisteredNotif; }
	void SetRegisteredNotif( AkUInt32 in_uRegisteredNotif ) { m_uRegisteredNotif = in_uRegisteredNotif; }

	virtual void TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated );

	AkListBareLight<CAkChildCtx>	m_listChildren;
	PlaybackTransition				m_PBTrans;
	AkUInt32						m_uRefCount;
	AkReal32						m_fPlayStopFadeRatio;
	AkReal32						m_fPauseResumeFadeRatio;
	AkUInt32						m_uNumLastSamples;
	AkUInt16						m_uPauseCount;
	AkUInt8							m_eStatus : 4;
	AkUInt8							m_bIsPaused : 1;

	virtual void OnLastFrame( AkUInt32 in_uNumSamples );
	virtual void OnPaused();
	virtual void OnResumed();
	virtual void SetPBIFade( void * in_pOwner, AkReal32 in_fFadeRatio );
	virtual void VirtualAddRef();
	virtual void VirtualRelease();
	virtual void OnPlayed();
	virtual void OnStopped();

	/** CAkTransportAware's stop: a music context is stopped through _Stop( TransParams ) only (retail's slot is empty). */
	virtual void _Stop( AkPBIStopMode, bool, bool ) {}
};

static_assert( sizeof( CAkMusicCtx ) == 60, "CAkMusicCtx: 2012 PDB 60" );
