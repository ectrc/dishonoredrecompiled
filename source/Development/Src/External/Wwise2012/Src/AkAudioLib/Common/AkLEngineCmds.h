/*=============================================================================
	AkLEngineCmds.h - CAkLEngineCmds (aklenginecmds.cpp): the queue of play / stop / pause / resume / seek commands the
	upper engine posts to the lower engine, applied at the start of every audio frame in sequence-number order.

	DISHONORED(port): agent IH2 (wave 26), pulled from IH3's list (the play path cannot run without it).
=============================================================================*/
#pragma once

#include "AkPBI.h"

class CAkVPLSrcCbxNode;

struct AkLECmd
{
	CAkPBI *	m_pCtx;
	LEState		m_eState;
	AkUInt32	m_ulSequenceNumber;
	bool		m_bSourceConnected;
};

class CAkLEngineCmds
{
public:
	static AKRESULT Init();
	static void Term();
	static AKRESULT EnqueueAction( LEState in_eState, CAkPBI * in_pContext );
	static AKRESULT EnqueueActionStop( CAkPBI * in_pCtx );
	static void DequeuePBI( CAkPBI * in_pPBI );
	static void ProcessAllCommands();
	static void ProcessPlayCommands();
	static void ProcessDisconnectedSources( AkUInt32 in_uFrames );
	static void DestroyDisconnectedSources();
	static void DeleteAllCommandsForSource( CAkVPLSrcCbxNode * in_pCbx );
	static void AddDisconnectedSource( CAkVPLSrcCbxNode * in_pCbx );

	typedef CAkList2<AkLECmd, const AkLECmd &, 1, ArrayPoolLEngineDefault> AkListCmd;

	static AkListCmd						m_listCmd;
	static AkListBare<CAkVPLSrcCbxNode>		m_listSrcsNotConnected;
	static AkUInt32							m_ulPlayEventID;
	static bool								m_bProcessPlayCmdsNeeded;

private:
	static void ProcessPendingCommands();
};
