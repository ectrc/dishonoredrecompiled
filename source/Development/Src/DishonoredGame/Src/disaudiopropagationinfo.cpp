// DishonoredGame/src/disaudiopropagationinfo.cpp
// PDB functions attributed to this file (8):
//   0x7f3d10  public: static void __cdecl UDisAudioPropagationInfo::InitializePrivateStaticClassUDisAudioPropagationInfo(void)
//   0x7f3d30  DisAudioPropagationFindPathInfo
//   0x7f7ea0  class UDisAudioPropagationInfo * __cdecl DisGetAudioPropagationInfo(void)
//   0x7f7f30  public: unsigned int __thiscall UDisAudioPropagationInfo::FindShortestPath(struct FDisAudioPathEndPair const &, float, unsigned int, struct FDisAudioPortalPath &)
//   0x801dd0  public: unsigned int __thiscall UDisAudioPropagationInfo::FinalizeAudioPath(struct FDisAudioPropagationCache &, struct FDisAudioPortalPath const &, int, int, class FVector const &, class FVector const &, float, unsigned int, class TArray<struct FDisAudioDebugPath, class FDefaultAllocator> *)
//   0x807670  public: virtual void __thiscall UDisAudioPropagationInfo::PostLoad(void)
//   0x814980  public: static class UClass * __cdecl UDisAudioPropagationInfo::GetPrivateStaticClassUDisAudioPropagationInfo(wchar_t const *)
//   0x816cb0  public: static class UClass * __cdecl UDisAudioPropagationInfo::StaticClassNoInline(void)
//
// DISHONORED(port): agent IE (PHASE23 IE). 2013 rvas (2012 in parentheses): DisGetAudioPropagationInfo 0x792290 (0x7f7ea0, 143 B),
// PostLoad 0x7a3070 (0x807670, 147 B). The level's audio cells and portals: the first streaming level whose world info carries
// one wins, else the persistent world info's. PostLoad drops a build whose tag is not 2, or one with portals and no path data.
// Agent IH3 (fork G): the portal path search - DisAudioPropagationFindPathInfo 0x786d50 (0x7f3d30), FindShortestPath 0x792320
// (0x7f7f30), FinalizeAudioPath 0x79d5e0 (0x801dd0). m_PathData is baked by the editor: m_NumPaths sorted 32-bit keys
// (FDisAudioPathEndPairCompact), m_NumPaths offsets (low byte = portal count - 2, high 24 bits = byte offset of the path from the
// start of m_PathData), then per path its 16-bit minimum length (times m_fDistCoeff) and the inner portals' indices.
// Deviation (named): FinalizeAudioPath's three alloca scratch arrays are TArrays.
// Retail reads UWorld::m_pWorldInfoCheckStreamingPersistent (the cached GetWorldInfo(TRUE)) unchecked; GetWorldInfo(TRUE) here, as
// DishonoredGetMapInfo does, and no world answers no propagation info.

#include "DishonoredGame.h"
#include "disaudiopropagation.h"

UDisAudioPropagationInfo* DisGetAudioPropagationInfo()
{
	AWorldInfo* WorldInfo = GWorld ? GWorld->GetWorldInfo( TRUE ) : NULL;
	if( !WorldInfo )
	{
		return NULL;
	}
	for( INT i = 0; i < WorldInfo->StreamingLevels.Num(); i++ )
	{
		ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(i);
		if( StreamingLevel && StreamingLevel->LoadedLevel )
		{
			AWorldInfo* LevelWorldInfo = StreamingLevel->LoadedLevel->GetWorldInfo();
			if( LevelWorldInfo->m_pAudioPropagationInfo )
			{
				return (UDisAudioPropagationInfo*)LevelWorldInfo->m_pAudioPropagationInfo;
			}
		}
	}
	return (UDisAudioPropagationInfo*)WorldInfo->m_pAudioPropagationInfo;
}

void UDisAudioPropagationInfo::PostLoad()
{
	Super::PostLoad();
	if( m_BuildTag != 2 || ( m_Portals.Num() && m_PathData.Num() <= 0 ) )
	{
		m_fDistCoeff = 0.0f;
		m_NumPaths = 0;
		m_PathData.Empty();
		m_Cells.Empty();
		m_Portals.Empty();
		m_BuildTag = -1;
	}
}

/** A binary search of the baked keys; the path's data, and its portal count including both ends. */
FDisAudioPathInfo* DisAudioPropagationFindPathInfo( INT _NumPaths, FDisAudioPathEndPairCompact _Key, FDisAudioPathEndPairCompact* _pPathKeys,
	FDisAudioOffsetAndNumPortals* _pPathOffsets, BYTE* _pPathData, INT& _rOutNumPortals )
{
	INT Low = 0;
	INT High = _NumPaths - 1;
	while( Low <= High )
	{
		const INT Middle = Low + ( High - Low ) / 2;
		if( _pPathKeys[Middle].m_U32 > _Key.m_U32 )
		{
			High = Middle - 1;
		}
		else if( _pPathKeys[Middle].m_U32 < _Key.m_U32 )
		{
			Low = Middle + 1;
		}
		else
		{
			_rOutNumPortals = _pPathOffsets[Middle].m_NumPortals + 2;
			return (FDisAudioPathInfo*)&_pPathData[_pPathOffsets[Middle].m_Offset];
		}
	}
	return NULL;
}

/**
 * The shortest baked portal path from the end-0 portal of cell 0 to the end-1 portal of cell 1 that is shorter than the audible
 * distance: its portals in listener-to-sound order (reversed when _bSwapEnds) and, per portal, whether it is crossed from cell A.
 * The same portal at both ends is a one-portal path of length 0.
 */
UBOOL UDisAudioPropagationInfo::FindShortestPath( const FDisAudioPathEndPair& _rPathEndPair, FLOAT _fMaxAudibleDistance, UBOOL _bSwapEnds, FDisAudioPortalPath& _rOutPath )
{
	ADishonoredAudioPortal* Portal0 = _rPathEndPair.m_PathEnd0.m_pPortal;
	if( Portal0 == _rPathEndPair.m_PathEnd1.m_pPortal )
	{
		_rOutPath.m_fMinLength = 0.f;
		_rOutPath.m_Nodes.Add( 1 );
		_rOutPath.m_Nodes(0).m_pPortal = Portal0;
		_rOutPath.m_Nodes(0).m_bCellAToCellB = ( _bSwapEnds ? _rPathEndPair.m_PathEnd1.m_pCell : _rPathEndPair.m_PathEnd0.m_pCell ) == Portal0->m_pCellA;
		return TRUE;
	}
	FDisAudioPathEndPairCompact Key;
	Key.m_PathEnd0.m_Cell = _rPathEndPair.m_PathEnd0.m_pCell->m_AudioPropagationIndex;
	Key.m_PathEnd0.m_Portal = Portal0->m_AudioPropagationIndex;
	Key.m_PathEnd1.m_Cell = _rPathEndPair.m_PathEnd1.m_pCell->m_AudioPropagationIndex;
	Key.m_PathEnd1.m_Portal = _rPathEndPair.m_PathEnd1.m_pPortal->m_AudioPropagationIndex;
	BYTE* Data = m_PathData.GetTypedData();
	INT NumPortals = 0;
	const FDisAudioPathInfo* PathInfo = DisAudioPropagationFindPathInfo( m_NumPaths, Key, (FDisAudioPathEndPairCompact*)Data,
		(FDisAudioOffsetAndNumPortals*)&Data[4 * m_NumPaths], Data, NumPortals );
	if( !PathInfo )
	{
		return FALSE;
	}
	const FLOAT fMinLength = (FLOAT)PathInfo->m_MinDist * m_fDistCoeff;
	if( _fMaxAudibleDistance <= fMinLength )
	{
		return FALSE;
	}
	_rOutPath.m_fMinLength = fMinLength;
	_rOutPath.m_Nodes.Add( NumPortals );
	INT NodeIndex = _bSwapEnds ? NumPortals - 1 : 0;
	const INT IndexInc = _bSwapEnds ? -1 : 1;
	_rOutPath.m_Nodes(NodeIndex).m_pPortal = Portal0;
	NodeIndex += IndexInc;
	for( INT i = 0; i < NumPortals - 2; i++ )
	{
		_rOutPath.m_Nodes(NodeIndex).m_pPortal = m_Portals(PathInfo->m_Portals[i]);
		NodeIndex += IndexInc;
	}
	_rOutPath.m_Nodes(NodeIndex).m_pPortal = _rPathEndPair.m_PathEnd1.m_pPortal;
	ADishonoredAudioVolume* Cell = _bSwapEnds ? _rPathEndPair.m_PathEnd1.m_pCell : _rPathEndPair.m_PathEnd0.m_pCell;
	for( INT i = 0; i < NumPortals; i++ )
	{
		ADishonoredAudioPortal* Portal = _rOutPath.m_Nodes(i).m_pPortal;
		_rOutPath.m_Nodes(i).m_bCellAToCellB = Cell == Portal->m_pCellA;
		Cell = Cell == Portal->m_pCellA ? Portal->m_pCellB : Portal->m_pCellA;
	}
	return TRUE;
}

/**
 * Turns a portal path into an FDisAudioPath of the cache: the points are the two ends and, per portal (the skipped ends left
 * out), the middle of its nearest points to both ends, then relaxed up to ten times - each moved to the portal's point nearest
 * to the segment between its neighbours - until the squared moves sum to no more than max(400, portals * raw distance^2 * 0.0016).
 * A path as long as the audible distance is dropped. Occlusion is 1 - the product of the portals' openness as the player hears
 * them, obstruction 1 - the product over the bends of (1 + cos) / 2. For an AI noise only the audibility is answered.
 */
UBOOL UDisAudioPropagationInfo::FinalizeAudioPath( FDisAudioPropagationCache& _rPropagationCache, const FDisAudioPortalPath& _rPath, INT _SkipFirstPortal, INT _SkipLastPortal,
	const FVector& _rFromPoint, const FVector& _rToPoint, FLOAT _fRawDistSq, UBOOL _bForAINoise, TArray<FDisAudioDebugPath>* _pDebugPaths )
{
	const INT NumNodes = _rPath.m_Nodes.Num() - _SkipFirstPortal - _SkipLastPortal;
	const FLOAT fMaxMoveSq = Max( (FLOAT)NumNodes * _fRawDistSq * 0.0016f, 400.f );
	const INT NumPoints = NumNodes + 2;
	TArray<FVector> Points;
	Points.Add( NumPoints );
	Points(0) = _rFromPoint;
	for( INT i = 0; i < NumNodes; i++ )
	{
		ADishonoredAudioPortal* Portal = _rPath.m_Nodes(i + _SkipFirstPortal).m_pPortal;
		FVector NearestFromPoint, NearestToPoint;
		Portal->GetNearestPoint( _rFromPoint, NearestFromPoint );
		Portal->GetNearestPoint( _rToPoint, NearestToPoint );
		Points(i + 1) = ( NearestToPoint + NearestFromPoint ) * 0.5f;
	}
	Points(NumPoints - 1) = _rToPoint;
	TArray<UBOOL> Dirty;
	Dirty.Add( NumPoints );
	for( INT i = 0; i < NumPoints; i++ )
	{
		Dirty(i) = TRUE;
	}
	INT Iteration = 0;
	FLOAT fMoveSq;
	do
	{
		fMoveSq = 0.f;
		for( INT i = 0; i < NumNodes; i++ )
		{
			if( !Dirty(i + 1) )
			{
				continue;
			}
			Dirty(i + 1) = FALSE;
			const FDisAudioPortalNode& Node = _rPath.m_Nodes(i + _SkipFirstPortal);
			FVector NewPoint;
			if( Node.m_bCellAToCellB )
			{
				Node.m_pPortal->GetNearestPoint( Points(i), Points(i + 2), NewPoint );
			}
			else
			{
				Node.m_pPortal->GetNearestPoint( Points(i + 2), Points(i), NewPoint );
			}
			if( Points(i + 1) != NewPoint )
			{
				fMoveSq += ( Points(i + 1) - NewPoint ).SizeSquared();
				Points(i + 1) = NewPoint;
				Dirty(i) = TRUE;
				Dirty(i + 2) = TRUE;
			}
		}
		Iteration++;
	}
	while( Iteration < 10 && fMoveSq > fMaxMoveSq );
	TArray<FVector> SegDirs;
	SegDirs.Empty( NumPoints - 1 );
	FLOAT fPathLength = 0.f;
	for( INT i = 1; i < NumPoints; i++ )
	{
		const FVector Segment = Points(i) - Points(i - 1);
		const FLOAT fLength = Segment.Size();
		if( fLength > 1e-8f )
		{
			fPathLength += fLength;
			if( fPathLength >= _rPropagationCache.m_fMaxAudibleDistance )
			{
				return FALSE;
			}
			SegDirs.AddItem( Segment * ( 1.f / fLength ) );
		}
	}
	FLOAT fOpeningValue_AI = 1.f;
	FLOAT fOpeningValue_Player = 1.f;
	for( INT i = 0; i < NumNodes; i++ )
	{
		ADishonoredAudioPortal* Portal = _rPath.m_Nodes(i + _SkipFirstPortal).m_pPortal;
		fOpeningValue_AI *= 1.f - Portal->m_fOcclusion_HeardByAI;
		fOpeningValue_Player *= 1.f - Portal->m_fOcclusion_HeardByPlayer;
	}
	if( _bForAINoise )
	{
		const FLOAT fRawDist = appSqrt( _fRawDistSq );
		const FLOAT fOcclusion = 1.f - fOpeningValue_AI;
		const FLOAT fFinalDist = Abs( fOcclusion - 1.f ) >= 0.0001f ? Max( fPathLength / ( 1.f - fOcclusion ), fRawDist ) : 3.4e38f;
		return _rPropagationCache.m_fMaxAudibleDistance > fFinalDist;
	}
	FLOAT fStraightnessValue = 1.f;
	for( INT i = 1; i < SegDirs.Num(); i++ )
	{
		fStraightnessValue *= Clamp( ( ( SegDirs(i - 1) | SegDirs(i) ) + 1.f ) * 0.5f, 0.f, 1.f );
	}
	FDisAudioPath& Path = _rPropagationCache.m_Paths( _rPropagationCache.m_Paths.AddZeroed() );
	Path.m_Direction = SegDirs.Num() > 0 ? SegDirs(0) : FVector( 0.f, 0.f, 0.f );
	Path.m_fDistance = Max( fPathLength, 1e-8f );
	Path.m_fOcclusion = 1.f - fOpeningValue_Player;
	Path.m_fObstruction = 1.f - fStraightnessValue;
	Path.m_Portals.Add( NumNodes );
	for( INT i = 0; i < NumNodes; i++ )
	{
		Path.m_Portals(i) = _rPath.m_Nodes(i + _SkipFirstPortal).m_pPortal;
	}
	if( _pDebugPaths )
	{
		FDisAudioDebugPath& DebugPath = (*_pDebugPaths)( _pDebugPaths->AddZeroed() );
		DebugPath.m_Points = Points;
	}
	return TRUE;
}
