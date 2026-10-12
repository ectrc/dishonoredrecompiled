// DishonoredGame/src/arkanimnodelookat.cpp
// DISHONORED(port): agent FT (PHASE17 FT). The half of UArkAnimNodeLookAt that reads the look-at config: the triangle
// mapping from a look-at angle to the two 3x3 aim grids (BlendInfos), the head/torso split (ComputeHeadAndTorsoAim), the
// crossfade between look-at modes (HandleBlendingBetweenLookatNodes), the per-mode anim names (CheckAnimsUpToDate) and the
// setters FArkComponentLookat drives. Agent FL's eight (InitAnim, TickAnim, both GetBoneAtoms, GetAnimInfoTotalWeight,
// SetAim, UpdateNodeWeights, AutoInitConfigInAnimTreeEd) stay in dishonoredplayercontroller.cpp under agentFL:lookatnode.
// Retail's unit is Engine/Src/arkanimnodelookat.cpp (43 functions); it is here for the reason Inc/arkcomponentlookat.h
// gives. UArkComponentLookatConfig's two procedural functions are here too (retail: arkcomponentlookatconfig.cpp).

#include "DishonoredGame.h"
#include "arkcomponentlookat.h"

// DISHONORED(retail): 2012 dynamic initialiser 0xb9f9d0 (2013 0xb83250), the string at 2012 .rdata 0xca8dcc.
const FName UArkAnimNodeLookAt::s_AnimNodeLookAtName( TEXT("LookatBlender") );

UBOOL BlendInfos::ms_Init = FALSE;
FVector2D BlendInfos::ms_SquarePositions[9];

/*-----------------------------------------------------------------------------
	BlendInfos
-----------------------------------------------------------------------------*/

// DISHONORED(port): the inline part of UArkAnimNodeLookAt's constructor (2013 rva 0x523e10): appMalloc(88) then staticInit.
BlendInfos::BlendInfos()
	: m_MaxRange( 0.f, 0.f )
	, m_P1Index( INDEX_NONE )
	, m_P2Index( INDEX_NONE )
{
	for( INT Corner = 0; Corner < 9; Corner++ )
	{
		m_CustomRanges[Corner] = FVector2D( 0.f, 0.f );
	}
	staticInit();
}

// DISHONORED(port): 2013 rva 0x50a2b0 (2012 0x54ab40), 185 bytes. Eight directions round the circle from +X, each
// pushed out to the unit square by dividing by its own |cos| and |sin|: (1,0) (1,1) (0,1) (-1,1) (-1,0) (-1,-1) (0,-1)
// (1,-1); [0] is the centre.
void BlendInfos::staticInit()
{
	if( ms_Init )
	{
		return;
	}
	ms_Init = TRUE;
	ms_SquarePositions[0] = FVector2D( 0.f, 0.f );
	for( INT Index = 0; Index < 8; Index++ )
	{
		const DOUBLE Angle = (DOUBLE)Index * 0.125 * 6.283185307179586;
		FLOAT X = (FLOAT)cos( Angle );
		FLOAT Y = (FLOAT)sin( Angle );
		if( Abs( X ) > 0.0001f )
		{
			X /= Abs( X );
		}
		if( Abs( Y ) > 0.0001f )
		{
			Y /= Abs( Y );
		}
		ms_SquarePositions[Index + 1] = FVector2D( X, Y );
	}
}

// DISHONORED(port): 2013 rva 0x50a140 (2012 0x54a9d0), 362 bytes. The grid arrives row-major (index = row*3 + column,
// row 0 up); it is stored relative to the centre and in circle order.
void BlendInfos::UpdateRanges( const FVector2D* const _CustomRanges )
{
	const FVector2D& Centre = _CustomRanges[4];
	m_CustomRanges[0] = FVector2D( 0.f, 0.f );
	m_CustomRanges[1] = _CustomRanges[5] - Centre;
	m_CustomRanges[2] = _CustomRanges[2] - Centre;
	m_CustomRanges[3] = _CustomRanges[1] - Centre;
	m_CustomRanges[4] = _CustomRanges[0] - Centre;
	m_CustomRanges[5] = _CustomRanges[3] - Centre;
	m_CustomRanges[6] = _CustomRanges[6] - Centre;
	m_CustomRanges[7] = _CustomRanges[7] - Centre;
	m_CustomRanges[8] = _CustomRanges[8] - Centre;
}

// DISHONORED(port): 2013 rva 0x50a370 (2012 0x54ac00), 188 bytes. The two consecutive corners the direction lies
// between, by the sign of the cross products; a zero direction answers the first pair.
UBOOL BlendInfos::GetP1P2( const FVector2D* const _CustomRanges, INT& _rP1Index, INT& _rP2Index, const FVector2D& _rRequestedAngleDirection )
{
	_rP1Index = 0;
	_rP2Index = 1;
	if( _rRequestedAngleDirection.X == 0.f && _rRequestedAngleDirection.Y == 0.f )
	{
		return TRUE;
	}
	for( INT Index = 0; Index < 8; Index++ )
	{
		_rP1Index = Index + 1;
		_rP2Index = Index == 7 ? 1 : Index + 2;
		const FVector2D& P1 = _CustomRanges[_rP1Index];
		const FVector2D& P2 = _CustomRanges[_rP2Index];
		if( _rRequestedAngleDirection.Y * P1.X - P1.Y * _rRequestedAngleDirection.X > 0.f
			&& P2.X * _rRequestedAngleDirection.Y - P2.Y * _rRequestedAngleDirection.X <= 0.f )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x50a430 (2012 0x54acc0), 228 bytes: is the angle outside the triangle (centre, P1, P2)?
UBOOL BlendInfos::IsOutside( const FVector2D& _rRequestedAngle, const FVector2D* const _CustomRanges, INT _P1Index, INT _P2Index )
{
	const FVector2D EdgesStart[3] = { _CustomRanges[0], _CustomRanges[_P1Index], _CustomRanges[_P2Index] };
	const FVector2D EdgesEnd[3] = { _CustomRanges[_P1Index], _CustomRanges[_P2Index], _CustomRanges[0] };
	for( INT Edge = 0; Edge < 3; Edge++ )
	{
		const FVector2D ToAngle = _rRequestedAngle - EdgesStart[Edge];
		const FVector2D EdgeDir = EdgesEnd[Edge] - EdgesStart[Edge];
		if( EdgeDir.X * ToAngle.Y - ToAngle.X * EdgeDir.Y < -0.0001f )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x50c4c0 (2012 0x54cc70), 234 bytes. m_MaxRange is where the direction leaves the edge P1-P2.
void BlendInfos::UpdateDirection( const FVector2D& _rRequestedAngleDirection )
{
	if( !GetP1P2( m_CustomRanges, m_P1Index, m_P2Index, _rRequestedAngleDirection ) )
	{
		m_P1Index = INDEX_NONE;
		m_P2Index = INDEX_NONE;
		m_MaxRange = FVector2D( 0.f, 0.f );
		return;
	}
	const FVector2D& P1 = m_CustomRanges[m_P1Index];
	const FVector2D& P2 = m_CustomRanges[m_P2Index];
	const FLOAT Denominator = -( ( P1.X - P2.X ) * _rRequestedAngleDirection.Y ) - ( -( ( P1.Y - P2.Y ) * _rRequestedAngleDirection.X ) );
	FLOAT Scale = P2.Y * P1.X - P1.Y * P2.X;
	if( Denominator != 0.f )
	{
		Scale /= Denominator;
	}
	m_MaxRange = FVector2D( -( _rRequestedAngleDirection.X * Scale ), -( _rRequestedAngleDirection.Y * Scale ) );
}

// DISHONORED(port): 2013 rva 0x5175c0 (2012 0x557a50), 296 bytes.
FVector2D BlendInfos::Map( FVector2D& _rRequestedAngle ) const
{
	if( m_P1Index < 0 || m_P2Index < 0 )
	{
		return FVector2D( 0.f, 0.f );
	}
	if( _rRequestedAngle.SizeSquared() < 0.00000001f )
	{
		_rRequestedAngle = FVector2D( 0.f, 0.f );
	}
	if( IsOutside( _rRequestedAngle, m_CustomRanges, m_P1Index, m_P2Index ) )
	{
		_rRequestedAngle = m_MaxRange;
	}
	const FVector2D Vertices[3] = { FVector2D( 0.f, 0.f ), m_CustomRanges[m_P1Index], m_CustomRanges[m_P2Index] };
	const FVector2D ValuesToInterpolate[3] = { FVector2D( 0.f, 0.f ), ms_SquarePositions[m_P1Index], ms_SquarePositions[m_P2Index] };
	return MapIntoNormalizedSpace( _rRequestedAngle, Vertices, ValuesToInterpolate );
}

/**
 * DISHONORED(port): 2013 rva 0x511bc0 (2012 0x5522b0), 2,479 bytes. Barycentric-by-scanline: the three vertices are
 * sorted by Y into A (lowest), B (middle), C (highest); the horizontal through the point meets two edges, the values at
 * the two crossings are interpolated along those edges, and the point's value is interpolated between the crossings.
 * Retail interpolates each edge by the crossing's projection onto the edge rather than by its Y, which this keeps.
 */
FVector2D BlendInfos::MapIntoNormalizedSpace( const FVector2D& _rRequestedAngle, const FVector2D* const _Vertices, const FVector2D* const _ValuesToInterpolate )
{
	INT IndexA;
	INT IndexB;
	INT IndexC;
	const FLOAT Y0 = _Vertices[0].Y;
	const FLOAT Y1 = _Vertices[1].Y;
	const FLOAT Y2 = _Vertices[2].Y;
	if( Y0 <= Y1 )
	{
		if( Y1 <= Y2 )
		{
			IndexC = 2;
			if( Y0 <= Y1 ) { IndexA = 0; IndexB = 1; }
			else { IndexA = 1; IndexB = 0; }
		}
		else
		{
			IndexC = 1;
			if( Y0 <= Y2 ) { IndexA = 0; IndexB = 2; }
			else { IndexA = 2; IndexB = 0; }
		}
	}
	else if( Y0 <= Y2 )
	{
		IndexC = 2;
		IndexA = 1;
		IndexB = 0;
	}
	else
	{
		IndexC = 0;
		if( Y1 > Y2 ) { IndexA = 2; IndexB = 1; }
		else { IndexA = 1; IndexB = 2; }
	}

	const FVector2D& A = _Vertices[IndexC];
	const FVector2D& B = _Vertices[IndexB];
	const FVector2D& C = _Vertices[IndexA];
	const FVector2D& VA = _ValuesToInterpolate[IndexC];
	const FVector2D& VB = _ValuesToInterpolate[IndexB];
	const FVector2D& VC = _ValuesToInterpolate[IndexA];
	if( _rRequestedAngle == A ) { return VA; }
	if( _rRequestedAngle == B ) { return VB; }
	if( _rRequestedAngle == C ) { return VC; }

	const FVector2D AB = B - A;
	const FLOAT ABLength = AB.Size();
	const FVector2D ABNormalized = AB * ( 1.f / ABLength );
	const FVector2D AC = C - A;
	const FLOAT ACLength = AC.Size();
	const FVector2D ACNormalized = AC * ( 1.f / ACLength );
	const FVector2D BC = C - B;
	const FLOAT BCLength = BC.Size();
	const FVector2D BCNormalized = BC * ( 1.f / BCLength );
	const FLOAT PY = _rRequestedAngle.Y;

	FVector2D Crossing1;
	FVector2D Value1;
	FVector2D Crossing2;
	FVector2D Value2;
	if( PY > B.Y && A.Y != B.Y )
	{
		// between A and B in Y: the edges A-B and A-C
		FLOAT TAB = 0.f;
		if( AB.Y != 0.f )
		{
			TAB = Clamp( ( PY - A.Y ) / AB.Y, 0.f, 1.f );
		}
		Crossing1 = A + AB * TAB;
		FLOAT TAC = 0.f;
		if( AC.Y != 0.f )
		{
			TAC = Clamp( ( PY - A.Y ) / AC.Y, 0.f, 1.f );
		}
		Crossing2 = A + AC * TAC;
		const FLOAT Along2 = ( ( Crossing2 - A ) | ACNormalized ) / ACLength;
		Value2 = VC * Along2 + VA * ( 1.f - Along2 );
		const FLOAT Along1 = ( ( Crossing1 - A ) | ABNormalized ) / ABLength;
		Value1 = VB * Along1 + VA * ( 1.f - Along1 );
	}
	else if( B.Y <= PY || B.Y == C.Y )
	{
		// between B and C in Y (or a flat B-C): the edges C-A and C-B
		Crossing1 = B;
		FLOAT TCA = 0.f;
		if( A.Y - C.Y != 0.f )
		{
			TCA = Clamp( ( PY - C.Y ) / ( A.Y - C.Y ), 0.f, 1.f );
		}
		Crossing2 = C + ( A - C ) * TCA;
		const FLOAT Along2 = ( ( Crossing2 - C ) | -ACNormalized ) / ACLength;
		Value2 = VA * Along2 + VC * ( 1.f - Along2 );
		const FLOAT Along1 = ( ( Crossing1 - C ) | -BCNormalized ) / BCLength;
		Value1 = VB * Along1 + VC * ( 1.f - Along1 );
	}
	else
	{
		// between C and B: the edges C-B and C-A
		FLOAT TCB = 0.f;
		if( B.Y - C.Y != 0.f )
		{
			TCB = Clamp( ( PY - C.Y ) / ( B.Y - C.Y ), 0.f, 1.f );
		}
		Crossing1 = C + ( B - C ) * TCB;
		FLOAT TCA = 0.f;
		if( A.Y - C.Y != 0.f )
		{
			TCA = Clamp( ( PY - C.Y ) / ( A.Y - C.Y ), 0.f, 1.f );
		}
		Crossing2 = C + ( A - C ) * TCA;
		const FLOAT Along2 = ( ( Crossing2 - C ) | -ACNormalized ) / ACLength;
		Value2 = VA * Along2 + VC * ( 1.f - Along2 );
		const FLOAT Along1 = ( ( Crossing1 - C ) | -BCNormalized ) / BCLength;
		Value1 = VB * Along1 + VC * ( 1.f - Along1 );
	}

	const FVector2D Span = Crossing1 - Crossing2;
	const FLOAT SpanLength = Span.Size();
	FVector2D SpanDirection = Span;
	if( SpanLength != 0.f )
	{
		SpanDirection = Span * ( 1.f / SpanLength );
	}
	FLOAT T = ( _rRequestedAngle - Crossing2 ) | SpanDirection;
	if( SpanLength != 0.f )
	{
		T /= SpanLength;
	}
	return Value1 * T + Value2 * ( 1.f - T );
}

/*-----------------------------------------------------------------------------
	UArkAnimNodeLookAt - the config half
-----------------------------------------------------------------------------*/

// DISHONORED(port): the two allocations of UArkAnimNodeLookAt's constructor (2013 rva 0x523e10, 163 bytes). This tree's
// generated shim class has no C++ constructor of its own, so they are made on first use; BeginDestroy frees them.
void UArkAnimNodeLookAt::EnsureBlendInfos()
{
	if( !m_HeadBlendInfos )
	{
		m_HeadBlendInfos = (FPointer)new BlendInfos;
	}
	if( !m_TorsoBlendInfos )
	{
		m_TorsoBlendInfos = (FPointer)new BlendInfos;
	}
}

// DISHONORED(port): 2013 rva 0x50a820 (2012 0x54b0b0), 38 bytes.
void UArkAnimNodeLookAt::BeginDestroy()
{
	delete (BlendInfos*)m_TorsoBlendInfos;
	m_TorsoBlendInfos = NULL;
	delete (BlendInfos*)m_HeadBlendInfos;
	m_HeadBlendInfos = NULL;
	Super::BeginDestroy();
}

// DISHONORED(port): 2013 rva 0x517580 (2012 0x54cb90), 53 bytes. Retail's FindAnimNodeFast is a name map on the Ark
// anim tree; the stock FindAnimNode walks the tree.
UArkAnimNodeLookAt* UArkAnimNodeLookAt::GetAnimNodeLookAt( const APawn* const _pPawnOwner )
{
	if( !_pPawnOwner || !_pPawnOwner->Mesh || !_pPawnOwner->Mesh->Animations )
	{
		return NULL;
	}
	UAnimTree* AnimTree = Cast<UAnimTree>( _pPawnOwner->Mesh->Animations );
	return AnimTree ? Cast<UArkAnimNodeLookAt>( AnimTree->FindAnimNode( s_AnimNodeLookAtName ) ) : NULL;
}

// DISHONORED(port): 2013 rva 0x50a090 (2012 0x54a920), 26 bytes: the store, then the virtual CheckAnimsUpToDate.
void UArkAnimNodeLookAt::SetConfig( const UArkComponentLookatConfig* _pConfig )
{
	m_pConfig = (FPointer)_pConfig;
	CheckAnimsUpToDate();
}

// DISHONORED(port): 2013 rva 0x50c410 (2012 0x54cbc0), 48 bytes.
void UArkAnimNodeLookAt::SetLookAtMode( INT _LookAtMode )
{
	const UArkComponentLookatConfig* Config = (const UArkComponentLookatConfig*)m_pConfig;
	if( !Config )
	{
		return;
	}
	const INT Mode = Clamp( _LookAtMode, 0, Config->m_LookAtModes.Num() - 1 );
	if( m_LookAtMode != Mode )
	{
		m_LookAtMode = Mode;
	}
}

// DISHONORED(port): 2013 rva 0x50c440 (2012 0x54cbf0), 61 bytes.
void UArkAnimNodeLookAt::SetUseHeadRange( FLOAT _fUseHeadRange )
{
	const FLOAT Clamped = Clamp( _fUseHeadRange, 0.f, 1.f );
	if( m_fUseHeadRange != Clamped )
	{
		m_fUseHeadRange = Clamped;
	}
}

// DISHONORED(port): 2013 rva 0x50c480 (2012 0x54cc30), 61 bytes.
void UArkAnimNodeLookAt::SetUseTorsoRange( FLOAT _fUseTorsoRange )
{
	const FLOAT Clamped = Clamp( _fUseTorsoRange, 0.f, 1.f );
	if( m_fUseTorsoRange != Clamped )
	{
		m_fUseTorsoRange = Clamped;
	}
}

// DISHONORED(port): 2013 rva 0x50a110 (2012 0x54a9a0), 37 bytes.
void UArkAnimNodeLookAt::SetUseTorsoDeadZone( FLOAT _fUseTorsoDeadZone )
{
	if( m_fUseTorsoDeadZone != _fUseTorsoDeadZone )
	{
		m_fUseTorsoDeadZone = _fUseTorsoDeadZone;
	}
}

// DISHONORED(port): 2013 rva 0x50c5b0 (2012 0x54cd60), 160 bytes: how far, in degrees, the two grids together reach
// along a direction, each scaled by its use range.
FVector2D UArkAnimNodeLookAt::GetMaxAim( const FVector2D& _AimDirection )
{
	EnsureBlendInfos();
	BlendInfos* Head = (BlendInfos*)m_HeadBlendInfos;
	BlendInfos* Torso = (BlendInfos*)m_TorsoBlendInfos;
	Head->UpdateDirection( _AimDirection );
	Torso->UpdateDirection( _AimDirection );
	const FLOAT RadToDeg = 180.f / PI;
	return FVector2D( ( Head->m_MaxRange.X * m_fUseHeadRange + Torso->m_MaxRange.X * m_fUseTorsoRange ) * RadToDeg,
		( Head->m_MaxRange.Y * m_fUseHeadRange + Torso->m_MaxRange.Y * m_fUseTorsoRange ) * RadToDeg );
}

/**
 * DISHONORED(port): 2013 rva 0x5176f0 (2012 0x557b80), 1,111 bytes. m_Aim (degrees) is split into a head angle and a
 * torso angle, in radians, and each is mapped onto its grid:
 *   - the torso starts turning once the aim passes Start_Torso of the head's share of the total reach and is fully
 *     engaged at End_Head, with a smoothstep over the lower half of that band; outside the band it takes whatever the
 *     head cannot;
 *   - the torso dead zone blends the torso angle toward its full reach along the direction (1 - dead zone);
 *   - the head takes the rest, and an aim the head cannot reach falls back to the head's full reach with the torso
 *     taking the remainder.
 * _SafeAimArray[0] is the head grid's normalised aim and [1] the torso grid's, both in [-1,1]; m_CurrentBlendedAim is
 * the sum of the two angles, in radians, which FArkComponentLookat::GetCurrentLookAtDirection reads back.
 */
void UArkAnimNodeLookAt::ComputeHeadAndTorsoAim( FVector2D* const _SafeAimArray )
{
	EnsureBlendInfos();
	BlendInfos* Head = (BlendInfos*)m_HeadBlendInfos;
	BlendInfos* Torso = (BlendInfos*)m_TorsoBlendInfos;
	const UArkComponentLookatConfig* Config = (const UArkComponentLookatConfig*)m_pConfig;
	_SafeAimArray[0] = FVector2D( 0.f, 0.f );
	_SafeAimArray[1] = FVector2D( 0.f, 0.f );
	if( !Config || !Config->m_LookAtModes.IsValidIndex( m_CurrentLookAtMode ) )
	{
		return;
	}
	Head->UpdateDirection( m_Aim );
	Torso->UpdateDirection( m_Aim );

	const FVector2D MaxRangeHead = Head->m_MaxRange * m_fUseHeadRange;
	const FVector2D MaxRangeTorso = Torso->m_MaxRange * m_fUseTorsoRange;
	const FLOAT fMaxRangeHeadSize = MaxRangeHead.Size();
	const FLOAT fMaxRangeTorsoSize = MaxRangeTorso.Size();
	const FLOAT fTotal = fMaxRangeHeadSize + fMaxRangeTorsoSize;
	const FLOAT fOneOverLength = fTotal <= 0.0001f ? 0.f : 1.f / fTotal;
	const FArkCpntLookatMode& rMode = Config->m_LookAtModes( m_CurrentLookAtMode );
	const FLOAT fHeadShare = fOneOverLength * fMaxRangeHeadSize;
	const FLOAT fTorsoShare = fOneOverLength * fMaxRangeTorsoSize;
	const FLOAT fStart = rMode.Start_Torso * fHeadShare;
	const FLOAT fEnd = ( 1.f - fTorsoShare ) * rMode.End_Head + fTorsoShare;
	const FVector2D AimRad( m_Aim.X * ( PI / 180.f ), m_Aim.Y * ( PI / 180.f ) );
	const FLOAT fRatio = AimRad.Size() * fOneOverLength;

	FVector2D TorsoAngle( 0.f, 0.f );
	if( fStart <= fRatio )
	{
		if( fEnd <= fRatio || fRatio <= fStart )
		{
			TorsoAngle = AimRad - MaxRangeHead;
			if( TorsoAngle.Size() > fMaxRangeTorsoSize )
			{
				TorsoAngle = MaxRangeTorso;
			}
		}
		else
		{
			FLOAT T = ( fRatio - fStart ) / ( fEnd - fStart );
			if( T < 0.5f )
			{
				T = ( 3.f - T * 2.f ) * T * T;
			}
			TorsoAngle = ( ( MaxRangeTorso + MaxRangeHead ) * fEnd - MaxRangeHead ) * T;
		}
	}
	const FLOAT fDeadZone = m_fUseTorsoDeadZone;
	const FLOAT fOpen = ( 1.f - fDeadZone ) * fRatio;
	TorsoAngle = TorsoAngle * fDeadZone + MaxRangeTorso * fOpen;
	FVector2D HeadAngle = AimRad - TorsoAngle;
	if( HeadAngle.Size() > fMaxRangeHeadSize + 0.0001f )
	{
		HeadAngle = MaxRangeHead;
		TorsoAngle = AimRad - MaxRangeHead;
		if( TorsoAngle.Size() > fMaxRangeTorsoSize )
		{
			TorsoAngle = MaxRangeTorso;
		}
	}
	_SafeAimArray[0] = Head->Map( HeadAngle );
	_SafeAimArray[1] = Torso->Map( TorsoAngle );
	m_CurrentBlendedAim = HeadAngle + TorsoAngle;
}

/**
 * DISHONORED(port): 2013 rva 0x50c650 (2012 0x54ce00), 897 bytes. Answers whether anything the weights depend on
 * changed since the last tick (aim, ranges, dead zone, mode) and runs the look-at mode crossfade: a mode change starts
 * m_LookAtModeBlendingNode on the new mode for half a second while this node keeps the old one, a second change during
 * the fade is stacked, and when the fade ends this node adopts the target mode and re-resolves its anims. While fading
 * the blending node is kept in step (config, mesh, ranges, aim) and ticked.
 */
UBOOL UArkAnimNodeLookAt::HandleBlendingBetweenLookatNodes( FLOAT _fDeltaSeconds )
{
	const UArkComponentLookatConfig* Config = (const UArkComponentLookatConfig*)m_pConfig;
	UBOOL bChanged = FALSE;
	if( m_Aim.X != m_PreviousAim.X || m_Aim.Y != m_PreviousAim.Y
		|| m_fPreviousUseHeadRange != m_fUseHeadRange
		|| m_fPreviousUseTorsoRange != m_fUseTorsoRange
		|| m_fPreviousUseTorsoDeadZone != m_fUseTorsoDeadZone
		|| m_PreviousLookAtMode != m_LookAtMode )
	{
		bChanged = TRUE;
		ConditionalClearCachedData();
		if( m_PreviousLookAtMode != m_LookAtMode && m_LookAtModeBlendingNode )
		{
			if( m_fLookAtModeBlendingRemainingTime > 0.f )
			{
				m_StackedTargetLookAtMode = m_LookAtMode;
			}
			else
			{
				m_TargetLookAtMode = m_LookAtMode;
				m_fLookAtModeBlendingRemainingTime = 0.5f;
				m_LookAtModeBlendingNode->m_pConfig = m_pConfig;
				m_LookAtModeBlendingNode->SkelComponent = SkelComponent;
				m_LookAtModeBlendingNode->m_CurrentLookAtMode = m_TargetLookAtMode;
				m_LookAtModeBlendingNode->SetLookAtMode( m_TargetLookAtMode );
				m_LookAtModeBlendingNode->CheckAnimsUpToDate();
				m_LookAtModeBlendingNode->ConditionalClearCachedData();
			}
		}
		m_fPreviousUseHeadRange = m_fUseHeadRange;
		m_fPreviousUseTorsoRange = m_fUseTorsoRange;
		m_PreviousAim = m_Aim;
		m_fPreviousUseTorsoDeadZone = m_fUseTorsoDeadZone;
		m_PreviousLookAtMode = m_LookAtMode;
	}

	if( m_fLookAtModeBlendingRemainingTime > 0.f )
	{
		m_fLookAtModeBlendingRemainingTime -= _fDeltaSeconds;
		if( m_fLookAtModeBlendingRemainingTime < 0.f )
		{
			m_CurrentLookAtMode = m_TargetLookAtMode;
			CheckAnimsUpToDate();
			bJustBecameRelevant = TRUE;
		}
	}
	if( m_fLookAtModeBlendingRemainingTime <= 0.f && m_StackedTargetLookAtMode >= 0 && m_LookAtModeBlendingNode && Config )
	{
		m_TargetLookAtMode = m_StackedTargetLookAtMode;
		m_StackedTargetLookAtMode = INDEX_NONE;
		m_fLookAtModeBlendingRemainingTime = 0.5f;
		m_LookAtModeBlendingNode->m_pConfig = m_pConfig;
		m_LookAtModeBlendingNode->SkelComponent = SkelComponent;
		m_LookAtModeBlendingNode->m_CurrentLookAtMode = m_TargetLookAtMode;
		const INT Mode = Clamp( m_TargetLookAtMode, 0, Config->m_LookAtModes.Num() - 1 );
		if( m_LookAtModeBlendingNode->m_LookAtMode != Mode )
		{
			m_LookAtModeBlendingNode->m_LookAtMode = Mode;
		}
		m_LookAtModeBlendingNode->CheckAnimsUpToDate();
		m_LookAtModeBlendingNode->ConditionalClearCachedData();
	}
	if( m_fLookAtModeBlendingRemainingTime > 0.f && m_LookAtModeBlendingNode )
	{
		const FLOAT TorsoRange = Clamp( m_fUseTorsoRange, 0.f, 1.f );
		if( m_LookAtModeBlendingNode->m_fUseTorsoRange != TorsoRange )
		{
			m_LookAtModeBlendingNode->m_fUseTorsoRange = TorsoRange;
		}
		const FLOAT HeadRange = Clamp( m_fUseHeadRange, 0.f, 1.f );
		if( m_LookAtModeBlendingNode->m_fUseHeadRange != HeadRange )
		{
			m_LookAtModeBlendingNode->m_fUseHeadRange = HeadRange;
		}
		if( m_LookAtModeBlendingNode->m_Aim.X != m_Aim.X || m_LookAtModeBlendingNode->m_Aim.Y != m_Aim.Y )
		{
			m_LookAtModeBlendingNode->m_Aim = m_Aim;
		}
		m_LookAtModeBlendingNode->TickAnim( _fDeltaSeconds );
		if( _fDeltaSeconds > 0.f )
		{
			ConditionalClearCachedData();
		}
	}
	return bChanged;
}

/**
 * DISHONORED(port): 2013 rva 0x512570 (2012 0x552c60), 1,653 bytes. The eighteen pose names come from the config's
 * current mode - head LU LC LD CU CC CD RU RC RD into Anims(0..8), torso into Anims(9..17), so index = column*3 + row
 * with column L/C/R and row U/C/D - and the two grids' ranges from the mode's eight range angles (degrees in the config,
 * radians here; left and down are authored negative). Then the stock resolve of each name to its sequence.
 * DISHONORED(retail): this is why the cooked "ADD_Empty_AimHeadIdle_*" names agent FL measured are never what retail
 * plays: they are the AnimTree's preview set, replaced the moment FArkComponentLookat::Starting hands the node its config.
 */
void UArkAnimNodeLookAt::CheckAnimsUpToDate()
{
	const UArkComponentLookatConfig* Config = (const UArkComponentLookatConfig*)m_pConfig;
	if( Config && m_CurrentLookAtMode >= 0 && m_CurrentLookAtMode < Config->m_LookAtModes.Num() && Anims.Num() >= 18 )
	{
		EnsureBlendInfos();
		const FArkCpntLookatMode& rMode = Config->m_LookAtModes( m_CurrentLookAtMode );
		const FName HeadNames[9] = { rMode.Head_AnimName_LU, rMode.Head_AnimName_LC, rMode.Head_AnimName_LD,
			rMode.Head_AnimName_CU, rMode.Head_AnimName_CC, rMode.Head_AnimName_CD,
			rMode.Head_AnimName_RU, rMode.Head_AnimName_RC, rMode.Head_AnimName_RD };
		const FName TorsoNames[9] = { rMode.Torso_AnimName_LU, rMode.Torso_AnimName_LC, rMode.Torso_AnimName_LD,
			rMode.Torso_AnimName_CU, rMode.Torso_AnimName_CC, rMode.Torso_AnimName_CD,
			rMode.Torso_AnimName_RU, rMode.Torso_AnimName_RC, rMode.Torso_AnimName_RD };
		for( INT Cell = 0; Cell < 9; Cell++ )
		{
			Anims(Cell).AnimName = HeadNames[Cell];
			Anims(9 + Cell).AnimName = TorsoNames[Cell];
		}

		const FLOAT DegToRad = PI / 180.f;
		FVector2D CustomRanges[9];
		CustomRanges[0] = FVector2D( rMode.m_TorsoLeftRange * DegToRad, rMode.m_TorsoUpRange * DegToRad );
		CustomRanges[1] = FVector2D( 0.f, rMode.m_TorsoUpRange * DegToRad );
		CustomRanges[2] = FVector2D( rMode.m_TorsoRightRange * DegToRad, rMode.m_TorsoUpRange * DegToRad );
		CustomRanges[3] = FVector2D( rMode.m_TorsoLeftRange * DegToRad, 0.f );
		CustomRanges[4] = FVector2D( 0.f, 0.f );
		CustomRanges[5] = FVector2D( rMode.m_TorsoRightRange * DegToRad, 0.f );
		CustomRanges[6] = FVector2D( rMode.m_TorsoLeftRange * DegToRad, rMode.m_TorsoDownRange * DegToRad );
		CustomRanges[7] = FVector2D( 0.f, rMode.m_TorsoDownRange * DegToRad );
		CustomRanges[8] = FVector2D( rMode.m_TorsoRightRange * DegToRad, rMode.m_TorsoDownRange * DegToRad );
		((BlendInfos*)m_TorsoBlendInfos)->UpdateRanges( CustomRanges );

		CustomRanges[0] = FVector2D( rMode.m_HeadLeftRange * DegToRad, rMode.m_HeadUpRange * DegToRad );
		CustomRanges[1] = FVector2D( 0.f, rMode.m_HeadUpRange * DegToRad );
		CustomRanges[2] = FVector2D( rMode.m_HeadRightRange * DegToRad, rMode.m_HeadUpRange * DegToRad );
		CustomRanges[3] = FVector2D( rMode.m_HeadLeftRange * DegToRad, 0.f );
		CustomRanges[4] = FVector2D( 0.f, 0.f );
		CustomRanges[5] = FVector2D( rMode.m_HeadRightRange * DegToRad, 0.f );
		CustomRanges[6] = FVector2D( rMode.m_HeadLeftRange * DegToRad, rMode.m_HeadDownRange * DegToRad );
		CustomRanges[7] = FVector2D( 0.f, rMode.m_HeadDownRange * DegToRad );
		CustomRanges[8] = FVector2D( rMode.m_HeadRightRange * DegToRad, rMode.m_HeadDownRange * DegToRad );
		((BlendInfos*)m_HeadBlendInfos)->UpdateRanges( CustomRanges );
	}
	Super::CheckAnimsUpToDate();
}

/*-----------------------------------------------------------------------------
	UArkComponentLookatConfig - the procedural look-at patterns
	(retail: Engine/Src/arkcomponentlookatconfig.cpp)
-----------------------------------------------------------------------------*/

static FLOAT DisLookatConfigRand01()
{
	return (FLOAT)rand() * ( 1.f / 32768.f );
}

// DISHONORED(port): 2012 rva 0x57cb30, 2013 by its callers (ProceduralLookAt, UpdateProceduralLookAt): one work entry
// per sequence of the pattern, each with its rules zeroed and a random cooldown inside the sequence's range.
void UArkComponentLookatConfig::InitProceduralLookAt( INT _PatternIdx, TArray<FArkCpntLookatProcSeqWork>& _WorkData ) const
{
	if( !m_ProcPatterns.IsValidIndex( _PatternIdx ) )
	{
		return;
	}
	const FArkCpntLookatProcPattern& rPattern = m_ProcPatterns( _PatternIdx );
	const INT SequencesCount = rPattern.m_Sequences.Num();
	_WorkData.Empty( SequencesCount );
	_WorkData.AddZeroed( SequencesCount );
	for( INT SequenceIdx = 0; SequenceIdx < SequencesCount; SequenceIdx++ )
	{
		FArkCpntLookatProcSeqWork& rWork = _WorkData( SequenceIdx );
		const FArkCpntLookatProcSeq& rSequence = rPattern.m_Sequences( SequenceIdx );
		const INT RulesCount = rSequence.m_Rules.Num();
		rWork.m_fCooldown = rSequence.m_CooldownRange.m_fMinDuration + ( rSequence.m_CooldownRange.m_fMaxDuration - rSequence.m_CooldownRange.m_fMinDuration ) * DisLookatConfigRand01();
		rWork.m_Rules.Empty( RulesCount );
		rWork.m_Rules.AddZeroed( RulesCount );
	}
}

/**
 * DISHONORED(port): 2013 rva 0x53c8a0 (2012 0x57cd10), 1,678 bytes. The pattern state machine, highest sequence first:
 *   - a sequence whose parent is enabled (or has none) counts its cooldown down and, when it expires, rolls its
 *     probability to become enabled; the first enabled sequence found stops the search;
 *   - an enabled sequence's enabled rule runs its duration down; when it expires it repeats, or passes to the next
 *     rule, or ends the sequence; a newly started rule rolls its duration, speed and - if it is a different rule - its
 *     repeat count and centre (absolute from the config, or the current aim), and a target inside its angle range
 *     round that centre, clamped to +/-180 by +/-90 degrees;
 *   - the first enabled sequence with a running rule is reported.
 */
void UArkComponentLookatConfig::UpdateProceduralLookAt( FLOAT _fTimeStep, INT _PatternIdx, const FVector2D& _CurAim, TArray<FArkCpntLookatProcSeqWork>& _WorkData, INT& _SequenceIdx, INT& _RuleIdx ) const
{
	_SequenceIdx = INDEX_NONE;
	_RuleIdx = INDEX_NONE;
	if( !m_ProcPatterns.IsValidIndex( _PatternIdx ) )
	{
		return;
	}
	const FArkCpntLookatProcPattern& rPattern = m_ProcPatterns( _PatternIdx );
	const INT SequencesCount = rPattern.m_Sequences.Num();
	if( _WorkData.Num() == SequencesCount )
	{
		for( INT SequenceIdx = SequencesCount - 1; SequenceIdx >= 0; SequenceIdx-- )
		{
			if( _WorkData(SequenceIdx).m_Rules.Num() != rPattern.m_Sequences(SequenceIdx).m_Rules.Num() )
			{
				InitProceduralLookAt( _PatternIdx, _WorkData );
				break;
			}
		}
	}
	else
	{
		InitProceduralLookAt( _PatternIdx, _WorkData );
	}

	INT SequenceIdx = SequencesCount - 1;
	for( ; SequenceIdx >= 0; SequenceIdx-- )
	{
		FArkCpntLookatProcSeqWork& rWork = _WorkData( SequenceIdx );
		if( rWork.m_bEnabled )
		{
			break;
		}
		const FArkCpntLookatProcSeq& rSequence = rPattern.m_Sequences( SequenceIdx );
		const INT ParentIdx = rSequence.m_ParentSeqIdx;
		if( ParentIdx == INDEX_NONE || ( ParentIdx < SequenceIdx && _WorkData.IsValidIndex( ParentIdx ) && _WorkData(ParentIdx).m_bEnabled ) )
		{
			rWork.m_fCooldown -= _fTimeStep;
			if( rWork.m_fCooldown <= 0.f )
			{
				rWork.m_fCooldown = rSequence.m_CooldownRange.m_fMinDuration + ( rSequence.m_CooldownRange.m_fMaxDuration - rSequence.m_CooldownRange.m_fMinDuration ) * DisLookatConfigRand01();
				if( rWork.m_Rules.Num() > 0 && rSequence.m_fProbability >= DisLookatConfigRand01() )
				{
					rWork.m_bEnabled = TRUE;
					break;
				}
			}
		}
	}

	for( ; SequenceIdx >= 0; SequenceIdx-- )
	{
		FArkCpntLookatProcSeqWork& rWork = _WorkData( SequenceIdx );
		if( !rWork.m_bEnabled )
		{
			continue;
		}
		const FArkCpntLookatProcSeq& rSequence = rPattern.m_Sequences( SequenceIdx );
		const INT RulesCount = rSequence.m_Rules.Num();
		INT EnabledRuleIdx = INDEX_NONE;
		INT NewRuleIdx = INDEX_NONE;
		UBOOL bRuleFound = FALSE;
		for( INT RuleIdx = 0; RuleIdx < RulesCount && RuleIdx < rWork.m_Rules.Num(); RuleIdx++ )
		{
			FArkCpntLookatProcRuleWork& rRuleWork = rWork.m_Rules( RuleIdx );
			if( !rRuleWork.m_bEnabled )
			{
				continue;
			}
			bRuleFound = TRUE;
			EnabledRuleIdx = RuleIdx;
			rRuleWork.m_fDuration -= _fTimeStep;
			if( rRuleWork.m_fDuration <= 0.f )
			{
				if( --rRuleWork.m_RepeatCount < 0 )
				{
					rRuleWork.m_bEnabled = FALSE;
					if( RuleIdx + 1 < RulesCount )
					{
						NewRuleIdx = RuleIdx + 1;
					}
					else
					{
						rWork.m_bEnabled = FALSE;
					}
				}
				else
				{
					NewRuleIdx = RuleIdx;
				}
			}
			break;
		}
		INT ReportedRuleIdx = EnabledRuleIdx;
		if( !bRuleFound )
		{
			NewRuleIdx = 0;
		}
		if( NewRuleIdx != INDEX_NONE && rSequence.m_Rules.IsValidIndex( NewRuleIdx ) && rWork.m_Rules.IsValidIndex( NewRuleIdx ) )
		{
			const FArkCpntLookatProcRule& rRule = rSequence.m_Rules( NewRuleIdx );
			FArkCpntLookatProcRuleWork& rRuleWork = rWork.m_Rules( NewRuleIdx );
			rRuleWork.m_bEnabled = TRUE;
			rRuleWork.m_fDuration = rRule.m_DurationRange.m_fMinDuration + ( rRule.m_DurationRange.m_fMaxDuration - rRule.m_DurationRange.m_fMinDuration ) * DisLookatConfigRand01();
			rRuleWork.m_fSpeed = rRule.m_SpeedRange.m_fMinSpeed + ( rRule.m_SpeedRange.m_fMaxSpeed - rRule.m_SpeedRange.m_fMinSpeed ) * DisLookatConfigRand01();
			if( NewRuleIdx != EnabledRuleIdx )
			{
				rRuleWork.m_RepeatCount = rRule.m_RepeatRange.m_MinRepeatCount + appTrunc( DisLookatConfigRand01() * (FLOAT)( rRule.m_RepeatRange.m_MaxRepeatCount - rRule.m_RepeatRange.m_MinRepeatCount ) );
				rRuleWork.m_Center = rRule.m_AngleCenter.m_bAbsolute
					? FVector2D( rRule.m_AngleCenter.m_Center.m_fHorizontal, rRule.m_AngleCenter.m_Center.m_fVertical )
					: _CurAim;
			}
			const FLOAT fHorizontalRange = rRule.m_AngleRange.m_fRightUpCorner.m_fHorizontal - rRule.m_AngleRange.m_fLeftDownCorner.m_fHorizontal;
			const FLOAT fVerticalRange = rRule.m_AngleRange.m_fRightUpCorner.m_fVertical - rRule.m_AngleRange.m_fLeftDownCorner.m_fVertical;
			rRuleWork.m_Target.m_fHorizontal = Clamp( DisLookatConfigRand01() * fHorizontalRange + rRuleWork.m_Center.X + rRule.m_AngleRange.m_fLeftDownCorner.m_fHorizontal, -180.f, 180.f );
			rRuleWork.m_Target.m_fVertical = Clamp( DisLookatConfigRand01() * fVerticalRange + rRuleWork.m_Center.Y + rRule.m_AngleRange.m_fLeftDownCorner.m_fVertical, -90.f, 90.f );
			ReportedRuleIdx = NewRuleIdx;
		}
		_SequenceIdx = SequenceIdx;
		_RuleIdx = ReportedRuleIdx;
		if( rWork.m_bEnabled )
		{
			return;
		}
	}
}
