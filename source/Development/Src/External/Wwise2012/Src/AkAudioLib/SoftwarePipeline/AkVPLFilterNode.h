/*=============================================================================
	AkVPLFilterNode.h - a voice's insert effect nodes: CAkVPLFilterNodeBase (akvplfilternodebase.cpp), CAkVPLFilterNode
	(in-place, akvplfilternode.cpp) and CAkVPLFilterNodeOutOfPlace (akvplfilternodeoutofplace.cpp).

	DISHONORED(port): agent IH4 fork F1 (wave 26). Layouts are the 2012 PDB's (CAkVPLFilterNode 0x28 bytes, the
	out-of-place node 0x94: AddPipeline's allocations); virtual order is CAkVPLFilterNodeBase_vtbl: the CAkVPLNode eight,
	then Init, Term, ReleaseMemory, ReleaseInputBuffer, GetBuffer, ConsumeBuffer.
=============================================================================*/
#pragma once

#include "AkVPLNode.h"
#include "AkFxContext.h"

struct AkFXDesc;

class CAkVPLFilterNodeBase : public CAkVPLNode
{
public:
	virtual void VirtualOn( AkVirtualQueueBehavior eBehavior );
	virtual AKRESULT VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset );
	virtual AKRESULT Init( AK::IAkPlugin * in_pPlugin, const AkFXDesc & in_fxDesc, AkUInt32 in_uFXIndex, CAkPBI * in_pCtx, AkAudioFormat & io_format );
	virtual void Term();
	virtual void ReleaseMemory() = 0;
	virtual bool ReleaseInputBuffer() { return false; }
	virtual void GetBuffer( AkVPLState & io_state ) = 0;
	virtual void ConsumeBuffer( AkVPLState & io_state ) = 0;

	CAkPBI *				m_pCtx;
	CAkInsertFXContext *	m_pInsertFXContext;
	AK::IAkPluginParam *	m_pParam;
	AkPluginID				m_FXID;
	bool					m_bLast;
	bool					m_bBypassed;
	bool					m_LastBypassed;
	AkUInt32				m_uFXIndex;
};

class CAkVPLFilterNode : public CAkVPLFilterNodeBase
{
public:
	CAkVPLFilterNode() : m_pEffect( NULL ), m_pAllocatedBuffer( NULL ) {}

	virtual void ReleaseBuffer();
	virtual AKRESULT TimeSkip( AkUInt32 & io_uFrames );
	virtual void VirtualOn( AkVirtualQueueBehavior eBehavior );
	virtual AKRESULT Seek();
	virtual AKRESULT Init( AK::IAkPlugin * in_pPlugin, const AkFXDesc & in_fxDesc, AkUInt32 in_uFXIndex, CAkPBI * in_pCtx, AkAudioFormat & io_format );
	virtual void Term();
	virtual void ReleaseMemory();
	virtual void GetBuffer( AkVPLState & io_state );
	virtual void ConsumeBuffer( AkVPLState & io_state );

	AK::IAkInPlaceEffectPlugin *	m_pEffect;
	AkUInt8 *						m_pAllocatedBuffer;
};

class CAkVPLFilterNodeOutOfPlace : public CAkVPLFilterNodeBase
{
public:
	CAkVPLFilterNodeOutOfPlace() : m_pEffect( NULL ) {}

	virtual void ProcessDone( AkVPLState & io_state );
	virtual void ReleaseBuffer();
	virtual AKRESULT TimeSkip( AkUInt32 & io_uFrames );
	virtual void VirtualOn( AkVirtualQueueBehavior eBehavior );
	virtual AKRESULT Seek();
	virtual AKRESULT Init( AK::IAkPlugin * in_pPlugin, const AkFXDesc & in_fxDesc, AkUInt32 in_uFXIndex, CAkPBI * in_pCtx, AkAudioFormat & io_format );
	virtual void Term();
	virtual void ReleaseMemory();
	virtual bool ReleaseInputBuffer();
	virtual void GetBuffer( AkVPLState & io_state );
	virtual void ConsumeBuffer( AkVPLState & io_state );

	AK::IAkOutOfPlaceEffectPlugin *	m_pEffect;
	AkUInt16						m_usRequestedFrames;
	AkPipelineBuffer				m_BufferIn;
	AkPipelineBuffer				m_BufferOut;
	AkUInt32						m_uInOffset;
	AkUInt32						m_InputFramesBeforeExec;
	AkUInt32						m_uConsumedInputFrames;
	AkUInt32						m_uRequestedInputFrames;
	AkReal32						m_fAveragedInput;
	AkReal32						m_fAveragedOutput;
	AkUInt32						m_uConsumedSinceLastOutput;
};

static_assert( sizeof( CAkVPLFilterNode ) == 0x28, "CAkVPLFilterNode: 2012 sizeof 0x28 (AddPipeline)" );
static_assert( sizeof( CAkVPLFilterNodeOutOfPlace ) == 0x94, "CAkVPLFilterNodeOutOfPlace: 2012 sizeof 0x94 (AddPipeline)" );
