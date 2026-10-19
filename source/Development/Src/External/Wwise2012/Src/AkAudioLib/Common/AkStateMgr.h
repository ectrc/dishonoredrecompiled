/*=============================================================================
	AkStateMgr.h - CAkStateMgr (akstatemgr.cpp): the state groups (current state, default and per-pair transition
	times from STMG), the nodes' state chunks listening to each group, the switch containers following a state group,
	the trigger listeners, and the game-sync preparation (PrepareGameSyncs); CAkPreparationAware (akpreparationaware.cpp).

	DISHONORED(port): agent IH3 (wave 26), fork R; replaces IH1's AkGameSyncs data store. Layouts are the 2012 PDB's
	(AkStateGroupInfo 0x18, CAkStateMgr::PreparationStateItem 0x20, CAkPreparationAware 0x0C).
=============================================================================*/
#pragma once

#include "AkAudioLibGlobals.h"

class AkStateGroupChunk;
class CAkSwitchAware;
class CAkRegisteredObj;

typedef bool ( *AkExternalStateHandlerCallback )( AkStateGroupID in_stateGroupID, AkStateID in_stateID );
extern AkExternalStateHandlerCallback g_pExternalStateHandler;

struct AkStateTransition
{
	AkStateID	StateFrom;
	AkStateID	StateTo;

	bool operator==( const AkStateTransition & in_rOther ) const { return StateFrom == in_rOther.StateFrom && StateTo == in_rOther.StateTo; }
};

struct AkListBareLightNextInGroup
{
	static AkStateGroupChunk *& Get( AkStateGroupChunk * in_pItem );
};

struct AkStateGroupInfo
{
	AkStateGroupInfo() : lDefaultTransitionTime( 0 ), ActualState( 0 ) {}

	void Term();

	AkTimeMs												lDefaultTransitionTime;
	AkStateID												ActualState;
	AkListBareLight<AkStateGroupChunk, AkListBareLightNextInGroup>	members;
	CAkKeyArray<AkStateTransition, AkTimeMs, 5>				mapTransitions;
};

class IAkTriggerAware
{
public:
	virtual void Trigger( AkTriggerID in_triggerID ) = 0;
};

class CAkPreparedContent
{
public:
	AkArray<AkUInt32, AkUInt32, ArrayPoolDefault, 4> m_PreparableContentList;
};

class CAkPreparationAware
{
public:
	CAkPreparationAware() : pNextItemPrepare( NULL ), m_uPreparationCount( 0 ) {}

	virtual AKRESULT ModifyActiveState( AkUInt32 in_GameSyncID, bool in_bSupported ) = 0;

	CAkPreparedContent * GetPreparedContent( AkUInt32 in_ulGroup, AkGroupType in_eGroupType );

	CAkPreparationAware *	pNextItemPrepare;
	AkUInt32				m_uPreparationCount;

protected:
	AKRESULT SubscribePrepare( AkUInt32 in_ulGroup, AkGroupType in_eGroupType );
	void UnsubscribePrepare( AkUInt32 in_ulGroup, AkGroupType in_eGroupType );
};

class CAkStateMgr
{
public:
	struct RegisteredSwitch
	{
		CAkSwitchAware *	pSwitch;
		AkStateGroupID		ulStateGroup;
	};

	struct RegisteredTrigger
	{
		IAkTriggerAware *	pTriggerAware;
		AkTriggerID			triggerID;
		CAkRegisteredObj *	gameObj;
	};

	class PreparationStateItem
	{
	public:
		struct AkListBareNextItemPrepare
		{
			static CAkPreparationAware *& Get( CAkPreparationAware * in_pItem ) { return in_pItem->pNextItemPrepare; }
		};

		explicit PreparationStateItem( AkUInt32 in_GroupID ) : pNextItem( NULL ), m_GroupID( in_GroupID ) {}

		AKRESULT Notify( AkUInt32 in_GameSyncID, bool in_bIsActive );

		PreparationStateItem *	pNextItem;
		CAkPreparedContent		m_PreparedContent;
		AkUInt32				m_GroupID;
		AkListBare<CAkPreparationAware, AkListBareNextItemPrepare>	m_PreparationList;
	};

	typedef AkListBare<PreparationStateItem> PreparationList;

	CAkStateMgr();
	~CAkStateMgr();

	AKRESULT Init();
	void Term();

	AKRESULT AddStateGroup( AkStateGroupID in_ulStateGroupID );
	AKRESULT AddStateGroupMember( AkStateGroupID in_ulStateGroupID, AkStateGroupChunk * in_pMember );
	AKRESULT SetdefaultTransitionTime( AkStateGroupID in_ulStateGroupID, AkTimeMs lTransitionTime );
	AKRESULT AddStateTransition( AkStateGroupID in_ulStateGroupID, AkStateID in_ulStateID1, AkStateID in_ulStateID2, AkTimeMs lTransitionTime, bool in_bIsShared = false );
	AKRESULT RemoveStateGroupMember( AkStateGroupID in_ulStateGroupID, AkStateGroupChunk * in_pMember );
	AKRESULT RemoveAllStateGroups( bool in_bIsFromClearBanks );

	void SetStateInternal( AkStateGroupID in_ulStateGroupID, AkStateID in_ulStateID, bool in_bSkipTransitionTime = false, bool in_bSkipExtension = false );
	AkStateID GetState( AkStateGroupID in_ulStateGroupID );

	AKRESULT RegisterSwitch( CAkSwitchAware * in_pSwitchCntr, AkStateGroupID in_ulStateGroup );
	AKRESULT UnregisterSwitch( CAkSwitchAware * in_pSwitchCntr );

	AKRESULT RegisterTrigger( IAkTriggerAware * in_pTrigerAware, CAkRegisteredObj * in_GameObj );
	AKRESULT UnregisterTrigger( IAkTriggerAware * in_pTrigerAware );
	void Trigger( AkTriggerID in_Trigger, CAkRegisteredObj * in_GameObj );

	PreparationStateItem * GetPreparationItem( AkUInt32 in_ulGroup, AkGroupType in_eGroupType );
	AKRESULT PrepareGameSync( AkGroupType in_eGroupType, AkUInt32 in_uGroupID, AkUInt32 in_uGameSyncID, bool in_bIsActive );

	AkUInt32 NumStateGroups() const { return m_StateGroups.Length(); }

	CAkKeyArray<AkStateGroupID, AkStateGroupInfo *, 8>				m_StateGroups;
	CAkList2<RegisteredSwitch, const RegisteredSwitch &, 1>			m_listRegisteredSwitch;
	CAkList2<RegisteredTrigger, const RegisteredTrigger &, 1>		m_listRegisteredTrigger;
	CAkLock															m_PrepareGameSyncLock;
	PreparationList													m_PreparationGroupsStates;
	PreparationList													m_PreparationGroupsSwitches;

private:
	AKRESULT UpdateSwitches( AkStateGroupID in_ulStateGroup, AkStateID in_StateFrom, AkStateID in_StateTo );
	void TermPreparationGroup( PreparationList & in_rGroup );
};
