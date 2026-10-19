/*=============================================================================
	AkVPLNode.h - CAkVPLNode (akvplnode.cpp/.h), IAkSoftwareCodec, CAkVPLSrcNode (akvplsrcnode.cpp): the base of every
	node of a voice pipeline and of every source (codec) node.

	DISHONORED(port): agent IH2 (wave 26). Virtual order is the 2012 PDB's CAkVPLNode_vtbl / CAkVPLSrcNode_vtbl:
	dtor, ProcessDone, ReleaseBuffer, TimeSkip, VirtualOn, VirtualOff, Seek, Connect, then StartStream, StopStream,
	GetBuffer, GetDuration, StopLooping, GetPluginParam, NotifySourceStarvation, ChangeSourcePosition.
=============================================================================*/
#pragma once

#include "AkPBI.h"

class CAkVPLNode
{
public:
	CAkVPLNode() : m_pInput( NULL ) {}
	virtual ~CAkVPLNode() {}
	virtual void ProcessDone( AkVPLState & io_state ) {}
	virtual void ReleaseBuffer() = 0;
	virtual AKRESULT TimeSkip( AkUInt32 & io_uFrames ) = 0;
	virtual void VirtualOn( AkVirtualQueueBehavior eBehavior ) {}
	virtual AKRESULT VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset ) { return AK_Success; }
	virtual AKRESULT Seek() { return m_pInput ? m_pInput->Seek() : AK_Success; }
	virtual void Connect( CAkVPLNode * in_pInput ) { m_pInput = in_pInput; }

	CAkVPLNode * m_pInput;

protected:
	static void CopyRelevantMarkers( AkPipelineBuffer * in_pInputBuffer, AkPipelineBuffer * io_pBuffer, AkUInt32 in_ulBufferStartOffset, AkUInt32 in_ulNumFrames );
};

class IAkSoftwareCodec : public CAkVPLNode
{
};

class CAkVPLSrcNode : public IAkSoftwareCodec
{
public:
	static CAkVPLSrcNode * Create( CAkPBI * in_pCtx );

	virtual AKRESULT TimeSkip( AkUInt32 & io_uFrames );
	virtual AKRESULT Seek();
	virtual AKRESULT StartStream() = 0;
	virtual void StopStream() = 0;
	virtual void GetBuffer( AkVPLState & io_state ) = 0;
	virtual AkReal32 GetDuration() const = 0;
	virtual AKRESULT StopLooping() = 0;
	virtual void * GetPluginParam() { return NULL; }
	virtual void NotifySourceStarvation() {}
	virtual AKRESULT ChangeSourcePosition() = 0;

	void Term( AkCtxDestroyReason in_eReason );
	void Start();
	void Stop();
	void Pause();
	void Resume( AkReal32 in_fOutputRate );
	AKRESULT FetchStreamedData();
	CAkPBI * GetContext() const { return m_pCtx; }
	bool IsIOReady() const { return m_bIOReady != 0; }

	CAkPBI *	m_pCtx;
	AkUInt8		m_bIOReady : 1;
	AkUInt8		m_bWaitForBuffering : 1;

protected:
	explicit CAkVPLSrcNode( CAkPBI * in_pCtx );
};

/** IAkSoftwareCodec factories a codec registers (AK::SoundEngine::RegisterCodec). */
typedef IAkSoftwareCodec * ( *AkCreateFileSourceCallback )( void * in_pCtx );
typedef IAkSoftwareCodec * ( *AkCreateBankSourceCallback )( void * in_pCtx );

/** CAkEffectsMgr (akeffectsmgr.cpp): the registered codecs and plug-ins; only the codec half runs before IH4. */
class CAkEffectsMgr
{
public:
	struct CodecTypeRecord
	{
		AkCreateFileSourceCallback	pFileCreateFunc;
		AkCreateBankSourceCallback	pBankCreateFunc;
	};

	struct EffectTypeRecord
	{
		void *	pCreateFunc;
		void *	pCreateParamFunc;
	};

	static AKRESULT Term();
	static AKRESULT RegisterPlugin( AkPluginType in_eType, AkUInt32 in_ulCompanyID, AkUInt32 in_ulPluginID, void * in_pCreateFunc, void * in_pCreateParamFunc );
	static AKRESULT RegisterCodec( AkUInt32 in_ulCompanyID, AkUInt32 in_ulPluginID, AkCreateFileSourceCallback in_pFileCreateFunc, AkCreateBankSourceCallback in_pBankCreateFunc );
	static IAkSoftwareCodec * AllocCodec( CAkPBI * in_pCtx, AkUInt32 in_uSrcType, AkCodecID in_CodecID );

	static CAkKeyArray<AkPluginID, EffectTypeRecord, 5>	m_RegisteredFXList;
	static CAkKeyArray<AkCodecID, CodecTypeRecord, 5>	m_RegisteredCodecList;
};
