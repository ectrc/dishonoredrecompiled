/*=============================================================================
	disaudiopropagation.h: agent IH3 (fork G) - the native-only types of the audio portal path search (2012 PDB types
	43816-43819 and 49924-49931; none is reflected) and the -ih3diag switch.
=============================================================================*/
#ifndef _INC_DISAUDIOPROPAGATION
#define _INC_DISAUDIOPROPAGATION

struct FDisAudioPathEnd
{
	ADishonoredAudioVolume* m_pCell;
	ADishonoredAudioPortal* m_pPortal;
};

struct FDisAudioPathEndPair
{
	FDisAudioPathEnd m_PathEnd0;
	FDisAudioPathEnd m_PathEnd1;
};

/** The baked key of a path: cell 0 and portal 0 in the high half, cell 1 and portal 1 in the low half. */
union FDisAudioPathEndCompact
{
	WORD m_U16;
	struct
	{
		BYTE m_Portal;
		BYTE m_Cell;
	};
};

union FDisAudioPathEndPairCompact
{
	DWORD m_U32;
	struct
	{
		FDisAudioPathEndCompact m_PathEnd1;
		FDisAudioPathEndCompact m_PathEnd0;
	};
};

union FDisAudioOffsetAndNumPortals
{
	DWORD m_U32;
	struct
	{
		DWORD m_NumPortals : 8;
		DWORD m_Offset : 24;
	};
};

struct FDisAudioPathInfo
{
	WORD m_MinDist;
	BYTE m_Portals[1];
};

struct FDisAudioPortalNode
{
	ADishonoredAudioPortal* m_pPortal;
	UBOOL m_bCellAToCellB;
};

struct FDisAudioPortalPath
{
	FLOAT m_fMinLength;
	TArray<FDisAudioPortalNode> m_Nodes;
};

FDisAudioPathInfo* DisAudioPropagationFindPathInfo( INT _NumPaths, FDisAudioPathEndPairCompact _Key, FDisAudioPathEndPairCompact* _pPathKeys,
	FDisAudioOffsetAndNumPortals* _pPathOffsets, BYTE* _pPathData, INT& _rOutNumPortals );

UBOOL DisIH3DiagEnabled();

#define IH3DIAG( Fmt, ... ) do { if( DisIH3DiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): ih3diag t=%.2f ") TEXT(Fmt), GWorld ? GWorld->GetTimeSeconds() : 0.f, __VA_ARGS__ ); } } while( 0 )

#endif
