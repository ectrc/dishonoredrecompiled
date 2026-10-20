/*=============================================================================
	AkMusicSwitchCntr.h - CAkMusicSwitchCntr (akmusicswitchcntr.cpp): a music switch container - the switch / state
	group it follows, the switch -> child node map with its default, "continue to play" (a switch to the node already
	playing keeps it), and game-sync preparation of its children.

	DISHONORED(port): agent IH4 (wave 26). Layout: CAkMusicTransAware (124) + CAkPreparationAware (+124, 12) +
	m_arSwitchNode (+136) + m_ulGroupID (+148) + m_ulDefaultSwitch (+152) + m_eGroupType (+156) +
	m_bIsContinuePlayback (+160) = 0xA4 (2012 PDB; Create mallocs 0xA4).
=============================================================================*/
#pragma once

#include "AkMusicTransAware.h"
#include <AkStateMgr.h>

class CAkMusicSwitchCntr : public CAkMusicTransAware, public CAkPreparationAware
{
public:
	static CAkMusicSwitchCntr * Create( AkUniqueID in_ulID );

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );

	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_MusicSwitchCntr; }
	virtual AKRESULT Play( AkPBIParams & in_rPBIParams );
	virtual AKRESULT ExecuteAction( ActionParams & in_rAction );
	virtual AKRESULT CanAddChild( CAkParameterNodeBase * in_pAudioNode );
	virtual AKRESULT PrepareData();
	virtual void UnPrepareData();
	virtual CAkMatrixAwareCtx * CreateContext( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams );
	virtual AKRESULT ModifyActiveState( AkUInt32 in_stateID, bool in_bSupported );

	AKRESULT SetSwitchAssocs( AkUInt32 in_uNumAssocs, AkMusicSwitchAssoc * in_pAssocs );
	AKRESULT GetSwitchNode( AkUInt32 in_switchID, AkUniqueID & out_nodeID );

	CAkKeyArray<AkUInt32, AkUniqueID, 8>	m_arSwitchNode;
	AkUInt32								m_ulGroupID;
	AkUInt32								m_ulDefaultSwitch;
	AkGroupType								m_eGroupType;
	bool									m_bIsContinuePlayback;

protected:
	explicit CAkMusicSwitchCntr( AkUniqueID in_ulID );
	virtual ~CAkMusicSwitchCntr();
};

static_assert( sizeof( CAkMusicSwitchCntr ) == 0xA4, "CAkMusicSwitchCntr: 2012 PDB 0xA4 (Create mallocs 0xA4)" );
