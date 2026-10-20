/*=============================================================================
	AkMusicRanSeqCntr.h - CAkMusicRanSeqCntr (akmusicranseqcntr.cpp): a music playlist container. Its playlist is a
	tree of CAkRSSub (a group: sequence / random, continuous / step, loop count, weights, avoid-repeat, shuffle) and
	CAkRSSegment leaves (a segment id); the global step selection state lives on each CAkRSSub (akrsiterator.cpp).

	DISHONORED(port): agent IH4 (wave 26). Layouts: CAkRSNode 20, CAkRSSegment 0x18, CAkRSSub 0x30, CAkMusicRanSeqCntr
	0xAC (CAkMusicTransAware 124 + m_playListRoot) - the 2012 PDB's (Create / AddPlaylistChildren malloc these sizes).
	CAkRSSegment's segment id overlays CAkRSSub::m_listChildren.m_pItems (+20), which AkRSIterator reads as
	"pNode[1]" - kept.
=============================================================================*/
#pragma once

#include "AkMusicTransAware.h"
#include <AkContainers.h>

class CAkSequenceCtx;

class CAkRSNode
{
public:
	explicit CAkRSNode( CAkRSNode * in_pParent ) : m_pParent( in_pParent ), m_ID( 0 ), m_Loop( 0 ), m_Weight( 50000 ) {}
	virtual ~CAkRSNode() {}
	virtual bool IsSegment() = 0;

	CAkRSNode *	m_pParent;
	AkUniqueID	m_ID;
	AkInt16		m_Loop;
	AkUInt32	m_Weight;
};

class CAkRSSegment : public CAkRSNode
{
public:
	explicit CAkRSSegment( CAkRSNode * in_pParent ) : CAkRSNode( in_pParent ), m_SegmentID( 0 ) {}
	virtual bool IsSegment() { return true; }

	AkUniqueID m_SegmentID;
};

class CAkRSSub : public CAkRSNode
{
public:
	explicit CAkRSSub( CAkRSNode * in_pParent )
		: CAkRSNode( in_pParent )
		, m_eRSType( RSType_ContinuousSequence )
		, m_pGlobalCntrBaseInfo( NULL )
		, m_bIsUsingWeight( false )
		, m_bIsShuffle( true )
		, m_bHasSegmentLeaves( false )
		, m_wAvoidRepeatCount( 0 )
	{
	}
	virtual ~CAkRSSub();
	virtual bool IsSegment() { return false; }

	void Clear();
	void WasSegmentLeafFound();
	AkUInt32 CalculateTotalWeight();
	CAkContainerBaseInfo * GetGlobalRSInfo();

	AkArray<CAkRSNode *, CAkRSNode *, ArrayPoolDefault, 16>	m_listChildren;
	RSType													m_eRSType;
	CAkContainerBaseInfo *									m_pGlobalCntrBaseInfo;
	bool													m_bIsUsingWeight;
	bool													m_bIsShuffle;
	bool													m_bHasSegmentLeaves;
	AkUInt16												m_wAvoidRepeatCount;
};

class CAkMusicRanSeqCntr : public CAkMusicTransAware
{
public:
	static CAkMusicRanSeqCntr * Create( AkUniqueID in_ulID );

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );

	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_MusicRanSeqCntr; }
	virtual AKRESULT Play( AkPBIParams & in_rPBIParams );
	virtual AKRESULT ExecuteAction( ActionParams & in_rAction );
	virtual AKRESULT CanAddChild( CAkParameterNodeBase * in_pAudioNode );
	virtual CAkMatrixAwareCtx * CreateContext( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams );

	CAkSequenceCtx * CreateSequenceCtx( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams );
	AKRESULT SetPlayList( AkMusicRanSeqPlaylistItem * in_pArrayItems );
	AKRESULT AddPlaylistChildren( CAkRSSub * in_pParent, AkMusicRanSeqPlaylistItem *& io_pArrayItems, AkUInt32 in_ulNumItems );

	CAkRSSub m_playListRoot;

protected:
	explicit CAkMusicRanSeqCntr( AkUniqueID in_ulID );
	virtual ~CAkMusicRanSeqCntr();

	void FlushPlaylist();
};

static_assert( sizeof( CAkRSNode ) == 20, "CAkRSNode: 2012 PDB 20" );
static_assert( sizeof( CAkRSSegment ) == 0x18, "CAkRSSegment: 2012 PDB 0x18 (AddPlaylistChildren mallocs 0x18)" );
static_assert( sizeof( CAkRSSub ) == 0x30, "CAkRSSub: 2012 PDB 0x30 (AddPlaylistChildren mallocs 0x30)" );
static_assert( sizeof( CAkMusicRanSeqCntr ) == 0xAC, "CAkMusicRanSeqCntr: 2012 PDB 0xAC (Create mallocs 0xAC)" );
