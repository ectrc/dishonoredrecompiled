/*=============================================================================
	AkFxContext.h - the effect plug-in contexts (akfxcontext.cpp/.h) and the plug-in allocator (akfxmemalloc.cpp).

	DISHONORED(port): agent IH4 fork F1 (wave 26). Layouts are the 2012 PDB's:
	  AkFXMemAlloc 8 (vptr + pool), AkDataReference 16, AkDataReferenceArray = CAkKeyArray<AkUInt32, AkDataReference, 3>,
	  CAkEffectContextBase 0x14 (vptr, m_uFXIndex, m_dataArray), CAkInsertFXContext 0x18 (+ m_pContext),
	  CAkBusFXContext 0x20 (+ m_pBusFX, m_BuxCtx, m_EnvID), CAkSourceFXContext 0x14 (vptr, m_pContext, m_dataArray).
	The two allocator instances are retail's: m_instanceUpper on g_DefaultPoolId (plug-in parameter objects of the
	FX share sets / customs, set by CreateDefaultMemPools 0x918270), m_instanceLower on g_LEngineDefaultPoolId (plug-in
	instances and their per-voice parameter clones, set by CAkLEngine::CreateLEnginePools 0x91dab0).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include "AkAudioLibGlobals.h"
#include "AkPrivateTypes.h"
#include "AkBusCtx.h"

class CAkPBI;
class CAkBusFX;
class CAkUsageSlot;

class AkFXMemAlloc : public AK::IAkPluginMemAlloc
{
public:
	AkFXMemAlloc() : m_poolId( AK_INVALID_POOL_ID ) {}
	virtual ~AkFXMemAlloc() {}

	virtual void * Malloc( size_t in_uSize );
	virtual void Free( void * in_pMemAddress );

	void SetPoolId( AkMemPoolId in_poolId ) { m_poolId = in_poolId; }

	static AkFXMemAlloc m_instanceUpper;
	static AkFXMemAlloc m_instanceLower;

	AkMemPoolId m_poolId;
};

struct AkDataReference
{
	AkUInt8 *		pData;
	AkUInt32		uSize;
	AkUniqueID		uSourceID;
	CAkUsageSlot *	pUsageSlot;
};

class AkDataReferenceArray : public CAkKeyArray<AkUInt32, AkDataReference, 3>
{
public:
	~AkDataReferenceArray();

	AkDataReference * AcquireData( AkUInt32 in_uDataIdx, AkUniqueID in_uSourceID );
	bool IsUsingThisSlot( const CAkUsageSlot * in_pUsageSlot );
};

class CAkEffectContextBase : public AK::IAkEffectPluginContext
{
public:
	virtual AK::IAkStreamMgr * GetStreamMgr() const;
	virtual AK::IAkProcessorFeatures * GetProcessorFeatures();
	virtual AkUInt16 GetMaxBufferLength() const { return LE_MAX_FRAMES_PER_BUFFER_FX; }
	virtual AKRESULT PostMonitorData( void * in_pData, AkUInt32 in_uDataSize ) { return AK_Fail; }
	virtual bool CanPostMonitorData() { return false; }

	enum { LE_MAX_FRAMES_PER_BUFFER_FX = 1024 };

	AkUInt32				m_uFXIndex;
	AkDataReferenceArray	m_dataArray;

protected:
	explicit CAkEffectContextBase( AkUInt32 in_uFXIndex ) : m_uFXIndex( in_uFXIndex ) {}
	virtual ~CAkEffectContextBase() {}
};

class CAkInsertFXContext : public CAkEffectContextBase
{
public:
	CAkInsertFXContext( CAkPBI * in_pCtx, AkUInt32 in_uFXIndex );
	virtual ~CAkInsertFXContext() {}

	virtual bool IsSendModeEffect() const { return false; }
	virtual AkPlayingID GetPlayingID() const;
	virtual void GetPluginMedia( AkUInt32 in_dataIndex, AkUInt8 * & out_rpData, AkUInt32 & out_rDataSize );

	CAkPBI * m_pContext;
};

class CAkBusFXContext : public CAkEffectContextBase
{
public:
	CAkBusFXContext( CAkBusFX * in_pBusFX, AkUInt32 in_uFXIndex, const AK::CAkBusCtx & in_rBusContext, AkUniqueID in_EnvID );
	virtual ~CAkBusFXContext() {}

	virtual bool IsSendModeEffect() const;
	virtual AkPlayingID GetPlayingID() const { return AK_INVALID_PLAYING_ID; }
	virtual void GetPluginMedia( AkUInt32 in_dataIndex, AkUInt8 * & out_rpData, AkUInt32 & out_rDataSize );

	CAkBusFX *		m_pBusFX;
	AK::CAkBusCtx	m_BuxCtx;
	AkUniqueID		m_EnvID;
};

class CAkSourceFXContext : public AK::IAkSourcePluginContext
{
public:
	explicit CAkSourceFXContext( CAkPBI * in_pCtx );
	virtual ~CAkSourceFXContext() {}

	virtual AkUInt16 GetNumLoops() const;
	virtual AK::IAkStreamMgr * GetStreamMgr() const;
	virtual AkPlayingID GetPlayingID() const;
	virtual AkUInt16 GetMaxBufferLength() const { return 1024; }
	virtual void GetPluginMedia( AkUInt32 in_dataIndex, AkUInt8 * & out_rpData, AkUInt32 & out_rDataSize );
	virtual AK::IAkProcessorFeatures * GetProcessorFeatures();

	CAkPBI *				m_pContext;
	AkDataReferenceArray	m_dataArray;
};

static_assert( sizeof( AkFXMemAlloc ) == 8, "AkFXMemAlloc: 2012 PDB sizeof 8" );
static_assert( sizeof( AkDataReference ) == 16, "AkDataReference: 2012 PDB sizeof 16" );
static_assert( sizeof( CAkEffectContextBase ) == 0x14, "CAkEffectContextBase: 2012 PDB sizeof 0x14" );
static_assert( sizeof( CAkInsertFXContext ) == 0x18, "CAkInsertFXContext: 2012 sizeof 0x18 (CAkVPLFilterNodeBase::Init)" );
static_assert( sizeof( CAkBusFXContext ) == 0x20, "CAkBusFXContext: 2012 sizeof 0x20 (CAkBusFX::SetInsertFx)" );
static_assert( sizeof( CAkSourceFXContext ) == 0x14, "CAkSourceFXContext: 2012 PDB sizeof 0x14" );
