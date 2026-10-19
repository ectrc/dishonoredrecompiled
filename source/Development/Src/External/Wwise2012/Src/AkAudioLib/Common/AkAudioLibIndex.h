/*=============================================================================
	AkAudioLibIndex.h - CAkCommonBase / CAkIndexable and the object index (g_pIndex) every bank object lives in.

	DISHONORED(port): agent IH1 (wave 26). CAkAudioLibIndex is 0x1F40 bytes in the 2012 PDB (AK::SoundEngine::
	PreInit mallocs 0x1F40): ten CAkIndexItem<T> of 800 bytes = { CAkLock, AkHashListBare<AkUniqueID,
	CAkIndexable, 193> }, in this order: audio nodes (0), custom states (800), events (1600), actions (2400),
	layers (3200), attenuations (4000), dynamic sequences (4800), dialogue events (5600), FX share sets (6400),
	FX custom (7200) - the offsets the bank manager's decompile uses (g_pIndex + 800 for states, ...).
	CAkIndexItem::GetPtrAndAddRef 2012 0x92b370 (one instantiation folded for every T), CAkIndexable ctor
	0x94be90, CAkAudioLibIndex::Init 0x9347e0, ReleaseTempObjects 0x934710.
=============================================================================*/
#pragma once

#include <AkPrivateTypes.h>

enum AkObjectCategory
{
	ObjCategory_Undefined	= 0,
	ObjCategory_ActorMixer	= 1,
	ObjCategory_Event		= 2,
	ObjCategory_Action		= 3,
	ObjCategory_State		= 4,
	ObjCategory_Bus			= 5,
	ObjCategory_Layer		= 6,
	ObjCategory_Attenuation	= 7,
	ObjCategory_DialogueEvent	= 8,
	ObjCategory_FxShareSet	= 9,
	ObjCategory_FxCustom	= 10
};

/** CAkCommonBase: vptr + m_lRef (8 bytes). */
class CAkCommonBase
{
public:
	CAkCommonBase() : m_lRef( 1 ) {}
	virtual ~CAkCommonBase() {}
	virtual AkUInt32 AddRef() = 0;
	virtual AkUInt32 Release() = 0;

	AkInt32 m_lRef;
};

/** CAkIndexable: + pNextItem / key, the AkHashListBare links (16 bytes). */
class CAkIndexable : public CAkCommonBase
{
public:
	explicit CAkIndexable( AkUniqueID in_ulID ) : pNextItem( NULL ), key( in_ulID ) {}
	virtual ~CAkIndexable() {}
	virtual AkObjectCategory Category() = 0;

	AkUniqueID ID() const { return key; }

	CAkIndexable *	pNextItem;
	AkUniqueID		key;
};

/** CAkIndexItemBase: one id -> object map with its own lock (the PDB's CAkIndexItem<T> data, 800 bytes). */
class CAkIndexItemBase
{
public:
	/** 2012 0x92b370 (one instantiation folded for every T). */
	CAkIndexable * GetPtrAndAddRefBase( AkUniqueID in_ID )
	{
		AkAutoLock<CAkLock> IndexLock( m_IndexLock );
		CAkIndexable * pItem = m_mapIDToPtr.Exists( in_ID );
		if( pItem )
		{
			++pItem->m_lRef;
		}
		return pItem;
	}

	/** The AddToIndex body every indexable inlines (e.g. CAkParameterNodeBase::AddToIndex 0x944a40). */
	void SetIDToPtr( CAkIndexable * in_pObject )
	{
		AkAutoLock<CAkLock> IndexLock( m_IndexLock );
		m_mapIDToPtr.Set( in_pObject );
	}

	void RemoveID( AkUniqueID in_ID )
	{
		AkAutoLock<CAkLock> IndexLock( m_IndexLock );
		m_mapIDToPtr.Unset( in_ID );
	}

	CAkLock m_IndexLock;
	AkHashListBare<AkUniqueID, CAkIndexable, 193> m_mapIDToPtr;
};

template <class T>
class CAkIndexItem : public CAkIndexItemBase
{
public:
	T GetPtrAndAddRef( AkUniqueID in_ID ) { return static_cast<T>( GetPtrAndAddRefBase( in_ID ) ); }
};

class CAkParameterNodeBase;
class CAkState;
class CAkEvent;
class CAkAction;
class CAkLayer;
class CAkAttenuation;
class CAkDynamicSequence;
class CAkDialogueEvent;
class CAkFxShareSet;
class CAkFxCustom;

class CAkAudioLibIndex
{
public:
	void Init();
	void Term();
	void ReleaseTempObjects();

	CAkIndexItem<CAkParameterNodeBase *>	m_idxAudioNode;
	CAkIndexItem<CAkState *>				m_idxCustomStates;
	CAkIndexItem<CAkEvent *>				m_idxEvents;
	CAkIndexItem<CAkAction *>				m_idxActions;
	CAkIndexItem<CAkLayer *>				m_idxLayers;
	CAkIndexItem<CAkAttenuation *>			m_idxAttenuations;
	CAkIndexItem<CAkDynamicSequence *>		m_idxDynamicSequences;
	CAkIndexItem<CAkDialogueEvent *>		m_idxDialogueEvents;
	CAkIndexItem<CAkFxShareSet *>			m_idxFxShareSets;
	CAkIndexItem<CAkFxCustom *>				m_idxFxCustom;
};

static_assert( sizeof( CAkIndexItem<CAkParameterNodeBase *> ) == 800, "CAkIndexItem: 2012 PDB sizeof 800" );
static_assert( sizeof( CAkAudioLibIndex ) == 0x1F40, "CAkAudioLibIndex: 2012 PDB sizeof 0x1F40" );

extern CAkAudioLibIndex * g_pIndex;
