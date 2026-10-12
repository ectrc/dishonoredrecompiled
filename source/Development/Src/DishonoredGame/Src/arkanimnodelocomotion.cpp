// DishonoredGame/src/arkanimnodelocomotion.cpp
// DISHONORED(port): agent GC (PHASE19 GC). UArkAnimNodeLocomotion, the NPC anim tree's walk blender, from the 2013 binary
// with the 2012 names checked at the rvas (2013 rvas are VA - 0x400000; the 2012 database has image base 0). Retail's unit
// is Engine/Src/arkanimnodelocomotion.cpp (52 functions); it is here for the reason Inc/arkcomponentlocomotion.h gives: the
// class and UArkComponentLocomotionConfig are DishonoredGameEngineShims.h classes. The four UArkComponentLocomotionConfig
// table readers the node needs are here too (retail: arkcomponentlocomotionconfig.cpp).
//
// What is ported: TickAnim and its eight phases, Initialize, the setters FArkComponentLocomotion / FArkComponentFaceTo
// drive, the three specific-animation requests and the turn-anim duration, OnBecomeRelevant / OnCeaseRelevant,
// GetFootProperties. Not ported: the foot-placement readers (GetNextFootOnGroundProperties 2013 0x515a00,
// GetDefaultLocalRootFootDelta 0x515d80, GetFootIndexFromBoneIndex 0x5117c0), and the AnimTree-editor functions (Render
// 0x509c40, GetSliderPosition 0x515f70, HandleSliderMove 0x516170, GetSliderDrawValue 0x5163f0, UpdateGridMove 0x511910).
// The skeleton is evaluated by the stock blend-base GetBoneAtoms through FT's UArkAnimNodeStack stand-in: retail PC goes
// through EdgeAnim and nothing of that path is here (STATUS.md, Known pitfalls).

#include "DishonoredGame.h"
#include "arkcomponentlocomotion.h"

// DISHONORED(retail): 2012 dynamic initialisers 0xb9f990 / 0xb9f9b0, the strings at 2012 .rdata 0xca8c8c / 0xca8c98.
const FName UArkAnimNodeLocomotion::s_AnimNodeLocoName( TEXT("LocoBlender") );
const FName UArkAnimNodeLocomotion::s_AnimNodeLocoGroupName( TEXT("LocoGroup") );

/** Retail's FArkCpntLocoDirAndWeight (Engine/Inc/arkcomponentlocomotionconfig.h): one direction animation and its weight. */
struct FArkCpntLocoDirAndWeight
{
	INT		m_Index;
	FLOAT	m_fWeight;
};

/** A unit taken off the exclude list whose dispatch is all virtual needs an external anchor (gen_classes_header.py, link_anchors). */
void DishonoredLinkArkAnimNodeLocomotion()
{
}

static UBOOL DisGCDiagEnabled()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		Enabled = ( appStrfind( appCmdLine(), TEXT("-gadiag") ) != NULL || appStrfind( appCmdLine(), TEXT("-gcdiag") ) != NULL ) ? 1 : 0;
	}
	return Enabled != 0;
}

/*-----------------------------------------------------------------------------
	UArkComponentLocomotionConfig: the speed-mode table readers
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x5331d0 (2012 0x574b20), 217 bytes.
void UArkComponentLocomotionConfig::ComputeAnimDirAndWeightFromAngleAndDirCount( INT _Angle, INT _AnimNumber, FArkCpntLocoDirAndWeight* const _DirAndWeight, FLOAT _fSnapOnOneAngleThreshold ) const
{
	if( _AnimNumber == 1 )
	{
		_DirAndWeight[0].m_Index = 4;
		_DirAndWeight[0].m_fWeight = 1.f;
		_DirAndWeight[1].m_Index = -1;
		_DirAndWeight[1].m_fWeight = 0.f;
		return;
	}
	const INT Step = 8 / _AnimNumber;
	const FLOAT fSlot = (FLOAT)( _Angle + 0x7FFF ) / (FLOAT)( 0x10000 / _AnimNumber );
	const INT Slot = appTrunc( fSlot );
	const FLOAT fNextWeight = fSlot - (FLOAT)Slot;
	_DirAndWeight[0].m_Index = Step * Slot;
	_DirAndWeight[0].m_fWeight = 1.f - fNextWeight;
	_DirAndWeight[1].m_Index = Step * ( ( Slot + 1 ) % _AnimNumber );
	_DirAndWeight[1].m_fWeight = fNextWeight;
	if( _DirAndWeight[0].m_fWeight >= _fSnapOnOneAngleThreshold )
	{
		_DirAndWeight[0].m_fWeight = 1.f;
		_DirAndWeight[1].m_fWeight = 0.f;
		_DirAndWeight[1].m_Index = -1;
	}
	else if( fNextWeight >= _fSnapOnOneAngleThreshold )
	{
		_DirAndWeight[1].m_fWeight = 1.f;
		_DirAndWeight[0].m_fWeight = 0.f;
		_DirAndWeight[0].m_Index = -1;
	}
}

// DISHONORED(port): 2013 rva 0x5343a0 (2012 0x575c30), 224 bytes.
void UArkComponentLocomotionConfig::GetSpeedModeFromForwardSpeed( FLOAT _fSpeed, INT _ModifierIdx, INT& _SpeedModeIdx0, INT& _SpeedModeIdx1, FLOAT& _fSpeedMode0Weight ) const
{
	_SpeedModeIdx0 = -1;
	_SpeedModeIdx1 = -1;
	const INT SpeedNames = m_SpeedNames.Num();
	const INT Base = _ModifierIdx * SpeedNames;
	FLOAT fSpeed0 = 0.f;
	FLOAT fSpeed1 = 0.f;
	for( INT SpeedIdx = 0; SpeedIdx < SpeedNames; ++SpeedIdx )
	{
		const FLOAT fModeSpeed = GetForwardSpeedOfSpeedMode( Base + SpeedIdx );
		if( _fSpeed < fModeSpeed )
		{
			_SpeedModeIdx1 = SpeedIdx;
			fSpeed1 = fModeSpeed;
			break;
		}
		_SpeedModeIdx0 = SpeedIdx;
		fSpeed0 = fModeSpeed;
	}
	if( _SpeedModeIdx1 == -1 )
	{
		_fSpeedMode0Weight = 1.f;
		_SpeedModeIdx0 += Base;
		return;
	}
	_fSpeedMode0Weight = ( fSpeed1 - _fSpeed ) / ( fSpeed1 - fSpeed0 );
	_SpeedModeIdx0 += Base;
	_SpeedModeIdx1 += Base;
}

// DISHONORED(port): 2013 rva 0x5342a0 (2012 0x575b30), 212 bytes: the forward direction's animation is slot 4 of the
// eight-slot ring, so move animation 4 / (8 / count).
FLOAT UArkComponentLocomotionConfig::GetForwardSpeedOfSpeedMode( INT _SpeedModeIdx ) const
{
	const FArkCpntLocoSpeedModeProp& rMode = m_SpeedModePropDictionary( m_SpeedModes( _SpeedModeIdx ) );
	const INT Count = rMode.m_MoveAnimProps.Num();
	if( Count <= 0 )
	{
		return 0.f;
	}
	return rMode.m_MoveAnimProps( 4 / ( 8 / Count ) ).m_fAnimSpeed;
}

// DISHONORED(port): 2013 rva 0x534380 (2012 0x575c10).
FLOAT UArkComponentLocomotionConfig::GetForwardSpeedOfSpeedMode( INT _SpeedIdx, INT _ModifierIdx ) const
{
	return GetForwardSpeedOfSpeedMode( _SpeedIdx + _ModifierIdx * m_SpeedNames.Num() );
}

/*-----------------------------------------------------------------------------
	UArkAnimNodeLocomotion: lookup, initialisation, the setters
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x515930 (2012 0x54c370, 63 bytes; 2013 adds the Cast, 72 bytes). Retail's
// UAnimTree::FindAnimNodeFast table is not ported, FindAnimNode walks the tree.
UArkAnimNodeLocomotion* UArkAnimNodeLocomotion::GetAnimNodeLocomotion( const APawn* const _pPawnOwner )
{
	if( !_pPawnOwner || !_pPawnOwner->Mesh || !_pPawnOwner->Mesh->Animations )
	{
		return NULL;
	}
	UAnimTree* AnimTree = Cast<UAnimTree>( _pPawnOwner->Mesh->Animations );
	return AnimTree ? Cast<UArkAnimNodeLocomotion>( AnimTree->FindAnimNode( s_AnimNodeLocoName ) ) : NULL;
}

const FArkCpntLocoSpeedModeProp& UArkAnimNodeLocomotion::GetSpeedModeProp( INT _SpeedModeIdx ) const
{
	const UArkComponentLocomotionConfig* Config = GetConfig();
	return Config->m_SpeedModePropDictionary( Config->m_SpeedModes( _SpeedModeIdx ) );
}

// DISHONORED(port): 2013 rva 0x51d4c0 (2012 0x55e050), 550 bytes. The two modifier slots are reset, every child must be a
// UAnimNodeSequence (m_bHaveAllAnimNodeSequences, also cleared when the node has no parent tree), the foot flags are sized
// to the config's feet, and the node is initialised with no relevant child.
void UArkAnimNodeLocomotion::Initialize( const UArkComponentLocomotionConfig* const _pConfig )
{
	m_pConfig = (FPointer)_pConfig;
	for( INT ModifierSlotIdx = 0; ModifierSlotIdx < 2; ++ModifierSlotIdx )
	{
		ResetModifierSlot( ModifierSlotIdx, TRUE );
	}
	m_bHaveAllAnimNodeSequences = TRUE;
	m_fModeWeights[0] = 1.f;
	m_fModeWeights[1] = 0.f;
	if( !m_pParentAnimTree )
	{
		m_bHaveAllAnimNodeSequences = FALSE;
	}
	for( INT ChildSlotIdx = 0; ChildSlotIdx < 9; ++ChildSlotIdx )
	{
		UAnimNodeSequence* pSequence = NULL;
		if( ChildSlotIdx < Children.Num() )
		{
			pSequence = Cast<UAnimNodeSequence>( Children( ChildSlotIdx ).Anim );
		}
		if( !pSequence )
		{
			m_bHaveAllAnimNodeSequences = FALSE;
		}
		m_ChildSlots[ChildSlotIdx].m_pAnimNode = pSequence;
		m_ChildSlots[ChildSlotIdx].m_SpeedModeIdx = -1;
		m_ChildSlots[ChildSlotIdx].m_DirIndex = -1;
	}
	if( _pConfig->m_Feet.Num() > 0 )
	{
		m_bFootOnGround.Empty();
		m_bFootOnGround.AddZeroed( _pConfig->m_Feet.Num() );
	}
	m_bInitialized = TRUE;
	m_MostRelevantWeightIndex = -1;
}

// DISHONORED(port): 2013 rva 0x509a40 (2012 0x54a2d0): the angle wrapped into -32768..32767.
void UArkAnimNodeLocomotion::SetAngle( INT _Angle )
{
	m_Angle = (INT)(WORD)( _Angle + 0x7FFF ) - 0x7FFF;
}

// DISHONORED(port): 2013 rva 0x509a60 (2012 0x54a2f0).
void UArkAnimNodeLocomotion::SetSpeed( FLOAT _fSpeed, UBOOL _bNoSmooth )
{
	m_fSpeed = _fSpeed;
	if( _bNoSmooth )
	{
		m_fSmoothedSpeed = _fSpeed;
	}
}

// DISHONORED(port): 2013 rva 0x509aa0 (2012 0x54a330).
void UArkAnimNodeLocomotion::SetSpeedMultiplier( FLOAT _fSpeedMultiplier )
{
	m_fSpeedMultiplier = _fSpeedMultiplier;
}

// DISHONORED(port): 2013 rva 0x509ac0 (2012 0x54a350).
void UArkAnimNodeLocomotion::SetModifier( INT _Modifier )
{
	m_Modifier = _Modifier;
}

// DISHONORED(port): 2013 rva 0x509ad0 (2012 0x54a360).
void UArkAnimNodeLocomotion::GetMovementAnimPrevAndCurProgress( FLOAT& _fPrevProgress, FLOAT& _fCurProgress ) const
{
	_fPrevProgress = m_fMovePrevProgress;
	_fCurProgress = m_fMoveCurProgress;
}

// DISHONORED(port): 2013 rva 0x509b40 (2012 0x54a3d0).
UBOOL UArkAnimNodeLocomotion::IsPlayingOnlySpecificAnim() const
{
	return m_bPlayingSpecificAnim && Abs( m_fModeWeights[0] ) < 0.00000001f;
}

// DISHONORED(port): 2013 rva 0x509b70 (2012 0x54a400).
void UArkAnimNodeLocomotion::AbortSpecificAnimRequest( UBOOL _bResetEvenIfSpecificAnimPlaying )
{
	if( m_bPlayingSpecificAnim && _bResetEvenIfSpecificAnimPlaying )
	{
		m_fModeWeights[0] = 1.f;
		m_fModeWeights[1] = 0.f;
		m_bPlayingSpecificAnim = FALSE;
		m_bStartSpecificAnimBlendOut = FALSE;
	}
	m_bSpecificAnimRequested = FALSE;
}

// DISHONORED(port): 2013 rva 0x509bc0 (2012 0x54a450): the slot with more than half the weight, NULL while not relevant.
const FAnimNodeLocoModifierSlot* UArkAnimNodeLocomotion::GetMostRelevantModifierSlot() const
{
	if( !bRelevant )
	{
		return NULL;
	}
	return ( m_ModifierSlots[0].m_fWeight > 0.5f ) ? &m_ModifierSlots[0] : &m_ModifierSlots[1];
}

// DISHONORED(port): 2013 rva 0x515ec0 (2012 0x556390), 170 bytes: a node that becomes relevant with a stale modifier
// blend snaps the main slot to the current modifier and the smoothed speed to the asked one.
void UArkAnimNodeLocomotion::OnBecomeRelevant()
{
	if( !m_bInitialized || !m_bHaveAllAnimNodeSequences )
	{
		return;
	}
	FAnimNodeLocoModifierSlot& rMain = m_ModifierSlots[m_MainModifierSlotIdx];
	if( rMain.m_Modifier != m_Modifier || Abs( rMain.m_fWeight ) >= 0.00000001f || Abs( m_ModifierSlots[m_OldModifierSlotIdx].m_fWeight ) >= 0.00000001f )
	{
		rMain.m_fWeight = 1.f;
		rMain.m_Modifier = m_Modifier;
		ResetModifierSlot( m_OldModifierSlotIdx, TRUE );
		m_fSmoothedSpeed = m_fSpeed;
	}
}

// DISHONORED(port): 2013 rva 0x509c00 (2012 0x54a490): the specific-animation state is dropped (bits 2..5 of the bitfield).
void UArkAnimNodeLocomotion::OnCeaseRelevant()
{
	m_bSpecificAnimRequested = FALSE;
	m_bPlayingSpecificAnim = FALSE;
	m_bStartSpecificAnimBlendOut = FALSE;
	m_bTransitionRequestedLastTick = FALSE;
	m_fModeWeights[0] = 1.f;
	m_fModeWeights[1] = 0.f;
}

/*-----------------------------------------------------------------------------
	the specific-animation requests
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x510060 (2012 0x550750), 355 bytes: how long the most relevant move animation needs to
// reach the turn animation's move-in point, at the group's rate.
FLOAT UArkAnimNodeLocomotion::GetDurationBeforePlayingTurnAnim( INT _SpeedModeIdx, INT _TurnAnimIdx ) const
{
	if( m_MostRelevantWeightIndex == -1 || !bRelevant )
	{
		return 0.f;
	}
	const UAnimNodeSequence* pSequence = m_ChildSlots[m_MostRelevantWeightIndex].m_pAnimNode;
	if( !pSequence->AnimSeq )
	{
		return 0.f;
	}
	const FLOAT fMoveAnimIn = GetSpeedModeProp( _SpeedModeIdx ).m_TurnAnimProps( _TurnAnimIdx ).m_fMoveAnimIn;
	const FLOAT fSequenceLength = pSequence->AnimSeq->SequenceLength;
	const FLOAT fProgress = pSequence->CurrentTime / fSequenceLength;
	const FLOAT fProgressLeft = ( fMoveAnimIn < fProgress ) ? ( 1.f - fProgress ) + fMoveAnimIn : fMoveAnimIn - fProgress;
	return m_fMostRelevantRateScale * fProgressLeft * fSequenceLength;
}

// DISHONORED(port): 2013 rva 0x5101d0 (2012 0x5508c0), 353 bytes.
void UArkAnimNodeLocomotion::RequestTurnAnim( INT _SpeedModeIdx, INT _TurnAnimIdx, UBOOL _bPlayNow, UBOOL _bInMovement, UBOOL _bFastBlend )
{
	const FArkCpntLocoTurnAnimProp& rTurn = GetSpeedModeProp( _SpeedModeIdx ).m_TurnAnimProps( _TurnAnimIdx );
	m_SpecificAnimProps.m_AnimName = rTurn.m_AnimName;
	m_SpecificAnimProps.m_fMoveAnimIn = rTurn.m_fMoveAnimIn;
	m_SpecificAnimProps.m_fMoveAnimOut = rTurn.m_fMoveAnimOut;
	m_SpecificAnimProps.m_fCutBeforeEnd = rTurn.m_fCutIfMoving;
	m_SpecificAnimProps.m_fBlendDuration = _bFastBlend ? m_fTurnFastBlendDuration : m_fTurnSlowBlendDuration;
	m_SpecificAnimProps.m_Type = ANIM_NODE_LOCO_SPECIFIC_ANIM_TURN;
	m_SpecificAnimProps.m_bUseAnimIn = _bPlayNow ? FALSE : TRUE;
	m_SpecificAnimProps.m_bUseCutBeforeEnd = _bInMovement ? TRUE : FALSE;
	m_SpecificAnimProps.m_EndModifier = _SpeedModeIdx / GetConfig()->m_SpeedNames.Num();
	m_bSpecificAnimRequested = TRUE;
}

// DISHONORED(port): 2013 rva 0x510340 (2012 0x550a30), 298 bytes: the speed mode's first start animation.
void UArkAnimNodeLocomotion::RequestStartAnim( INT _SpeedModeIdx )
{
	const FArkCpntLocoStartAnimProp& rStart = GetSpeedModeProp( _SpeedModeIdx ).m_StartAnimProps( 0 );
	m_SpecificAnimProps.m_AnimName = rStart.m_AnimName;
	m_SpecificAnimProps.m_fMoveAnimIn = 0.f;
	m_SpecificAnimProps.m_fMoveAnimOut = rStart.m_fMoveAnimOut;
	m_SpecificAnimProps.m_bUseAnimIn = FALSE;
	m_SpecificAnimProps.m_bUseCutBeforeEnd = FALSE;
	m_SpecificAnimProps.m_fBlendDuration = m_fStartBlendDuration;
	m_SpecificAnimProps.m_fCutBeforeEnd = 1.f;
	m_SpecificAnimProps.m_Type = ANIM_NODE_LOCO_SPECIFIC_ANIM_START;
	m_SpecificAnimProps.m_EndModifier = _SpeedModeIdx / GetConfig()->m_SpeedNames.Num();
	m_bSpecificAnimRequested = TRUE;
}

// DISHONORED(port): 2013 rva 0x510470 (2012 0x550b60), 313 bytes.
void UArkAnimNodeLocomotion::RequestStopAnim( INT _SpeedModeIdx, INT _StopAnimIdx )
{
	const FArkCpntLocoStopAnimProp& rStop = GetSpeedModeProp( _SpeedModeIdx ).m_StopAnimProps( _StopAnimIdx );
	m_SpecificAnimProps.m_AnimName = rStop.m_AnimName;
	m_SpecificAnimProps.m_fMoveAnimIn = rStop.m_fMoveAnimIn;
	m_SpecificAnimProps.m_bUseAnimIn = FALSE;
	m_SpecificAnimProps.m_bUseCutBeforeEnd = FALSE;
	m_SpecificAnimProps.m_fMoveAnimOut = 0.f;
	m_SpecificAnimProps.m_fBlendDuration = m_fStopBlendDuration;
	m_SpecificAnimProps.m_fCutBeforeEnd = 1.f;
	m_SpecificAnimProps.m_Type = ANIM_NODE_LOCO_SPECIFIC_ANIM_STOP;
	m_SpecificAnimProps.m_EndModifier = _SpeedModeIdx / GetConfig()->m_SpeedNames.Num();
	m_bSpecificAnimRequested = TRUE;
}

// DISHONORED(port): 2013 rva 0x515980 (2012 0x555e50). GetFootIndexFromBoneIndex (0x5117c0) is not ported, so no bone
// resolves to a foot yet; the copy is retail's once it is.
INT UArkAnimNodeLocomotion::GetFootProperties( INT _BoneIdx, FArkCpntLocoFootProp& _FootProperties ) const
{
	const INT FootIdx = -1;
	if( FootIdx != -1 )
	{
		_FootProperties = GetConfig()->m_Feet( FootIdx );
	}
	return FootIdx;
}

/*-----------------------------------------------------------------------------
	TickAnim and its phases
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x51b280 (2012 0x55b910), 413 bytes. The eight phases run only on an initialised node whose
// children are all sequences; otherwise child 0 is the whole pose (the "Empty_Idle" a locomotion-driven NPC slid in before
// this port, agentGA.md section 6). UpdateGridMove (0x511910) is the AnimTree editor's preview and is not ported.
void UArkAnimNodeLocomotion::TickAnim( FLOAT DeltaSeconds )
{
	if( m_bInitialized && m_bHaveAllAnimNodeSequences )
	{
		BlendSpeedAndAngleProperties( DeltaSeconds );
		HandleTransition();
		UpdateSpecificAnim( DeltaSeconds );
		UpdateMovementAnims( DeltaSeconds );
		UpdateMovementChildren();
		UpdateSpecificChild();
		UpdateFeetPropeties();
		UpdateProgressVariables( DeltaSeconds );
		m_fCurDeltaTime = DeltaSeconds;
	}
	else
	{
		for( INT ChildIdx = 0; ChildIdx < Children.Num(); ++ChildIdx )
		{
			Children( ChildIdx ).Weight = ( ChildIdx == 0 ) ? 1.f : 0.f;
		}
	}
	Super::TickAnim( DeltaSeconds );
}

// DISHONORED(port): 2013 rva 0x50bc00 (2012 0x54c3b0), 427 bytes. The (speed, angle) pair is a planar velocity; the smoothed
// one is moved towards the asked one at m_fVelocityMaxBlendSpeed scaled by how far apart they are (m_fVelocityMaxDif,
// floor m_fVelocityMinBlendSpeedRatio). 0.000095873802 is 2 pi / 65536; 10430.219 is its inverse.
void UArkAnimNodeLocomotion::BlendSpeedAndAngleProperties( FLOAT _fDeltaSeconds )
{
	if( GIsEditor && !GIsGame )
	{
		m_fSmoothedSpeed = m_fSpeed;
		m_SmoothedAngle = m_Angle;
		return;
	}
	const DOUBLE SmoothedAngle = (DOUBLE)m_SmoothedAngle * 0.000095873802;
	const DOUBLE Angle = (DOUBLE)m_Angle * 0.000095873802;
	const FLOAT SmoothedX = (FLOAT)cos( SmoothedAngle ) * m_fSmoothedSpeed;
	const FLOAT SmoothedY = (FLOAT)sin( SmoothedAngle ) * m_fSmoothedSpeed;
	const FLOAT TargetX = (FLOAT)cos( Angle ) * m_fSpeed;
	const FLOAT TargetY = (FLOAT)sin( Angle ) * m_fSpeed;
	const FLOAT fDif = appSqrt( ( SmoothedY - TargetY ) * ( SmoothedY - TargetY ) + ( SmoothedX - TargetX ) * ( SmoothedX - TargetX ) );
	FLOAT fRatio = fDif / m_fVelocityMaxDif;
	if( fRatio < m_fVelocityMinBlendSpeedRatio )
	{
		fRatio = m_fVelocityMinBlendSpeedRatio;
	}
	else if( fRatio >= 1.f )
	{
		fRatio = 1.f;
	}
	if( fDif > 0.f )
	{
		FLOAT fStep = ( m_fVelocityMaxBlendSpeed * fRatio * _fDeltaSeconds ) / fDif;
		if( fStep >= 1.f )
		{
			fStep = 1.f;
		}
		const FLOAT NewY = ( TargetY - SmoothedY ) * fStep + SmoothedY;
		const FLOAT NewX = ( TargetX - SmoothedX ) * fStep + SmoothedX;
		const DOUBLE NewAngle = atan2( (DOUBLE)NewY, (DOUBLE)NewX );
		m_SmoothedAngle = appTrunc( (FLOAT)( NewAngle * 10430.219 + NewAngle * 10430.219 + 0.5 ) ) >> 1;
		m_fSmoothedSpeed = appSqrt( NewY * NewY + NewX * NewX );
	}
}

// DISHONORED(port): 2013 rva 0x511230 (2012 0x551920), 355 bytes. A pawn that has (nearly) stopped with a new modifier
// pending plays the config's transition animation between the two modifiers as a specific animation.
void UArkAnimNodeLocomotion::HandleTransition()
{
	if( m_bTransitionRequestedLastTick )
	{
		m_bTransitionRequestedLastTick = FALSE;
		return;
	}
	const UArkComponentLocomotionConfig* Config = GetConfig();
	if( Config->m_fVelocityToStopMoving <= m_fSmoothedSpeed || m_bSpecificAnimRequested || m_bPlayingSpecificAnim )
	{
		return;
	}
	const FAnimNodeLocoModifierSlot& rMain = m_ModifierSlots[m_MainModifierSlotIdx];
	if( rMain.m_Modifier == m_Modifier || Abs( rMain.m_fWeight - 1.f ) >= 0.00001f )
	{
		return;
	}
	for( INT TransitionIdx = 0; TransitionIdx < Config->m_Transitions.Num(); ++TransitionIdx )
	{
		const FArkCpntLocoTransitionProp& rTransition = Config->m_Transitions( TransitionIdx );
		if( rTransition.m_StartModifier == rMain.m_Modifier && rTransition.m_EndModifier == m_Modifier )
		{
			m_SpecificAnimProps.m_AnimName = rTransition.m_AnimName;
			m_SpecificAnimProps.m_fBlendDuration = m_fTransitionBlendDuration;
			m_SpecificAnimProps.m_bUseAnimIn = FALSE;
			m_SpecificAnimProps.m_bUseCutBeforeEnd = FALSE;
			m_SpecificAnimProps.m_fMoveAnimIn = 0.f;
			m_SpecificAnimProps.m_fMoveAnimOut = 0.f;
			m_SpecificAnimProps.m_fCutBeforeEnd = 1.f;
			m_SpecificAnimProps.m_Type = ANIM_NODE_LOCO_SPECIFIC_ANIM_TRANSITION;
			m_SpecificAnimProps.m_EndModifier = m_Modifier;
			m_bSpecificAnimRequested = TRUE;
			m_bTransitionRequestedLastTick = TRUE;
			return;
		}
	}
}

// DISHONORED(port): 2013 rva 0x50bdb0 (2012 0x54c560), 669 bytes. The mode weights ([0] movement, [1] specific) follow the
// specific animation: blend in over m_fBlendDuration from its start, blend out over the same before its (cut) end; a
// requested animation with a move-in point waits until the most relevant move animation crosses that point.
void UArkAnimNodeLocomotion::UpdateSpecificAnim( FLOAT _fDeltaSeconds )
{
	if( m_bPlayingSpecificAnim )
	{
		const UAnimNodeSequence* pSpecific = m_ChildSlots[8].m_pAnimNode;
		const FLOAT fRate = pSpecific->Rate;
		FLOAT fLength = pSpecific->AnimSeq->SequenceLength;
		if( m_SpecificAnimProps.m_bUseCutBeforeEnd )
		{
			fLength *= m_SpecificAnimProps.m_fCutBeforeEnd;
		}
		const FLOAT fCurrentTime = pSpecific->CurrentTime;
		const FLOAT fProgress = fCurrentTime / fLength;
		const FLOAT fOldMovementWeight = m_fModeWeights[0];
		FLOAT fRemaining = 0.f;
		if( fProgress >= m_fSpecificPrevProgress )
		{
			fRemaining = Max( ( fLength - fCurrentTime ) * fRate, 0.f );
		}
		const FLOAT fBlendDuration = m_SpecificAnimProps.m_fBlendDuration;
		if( fBlendDuration <= fRemaining )
		{
			const FLOAT fElapsed = fCurrentTime * fRate;
			if( fBlendDuration <= fElapsed )
			{
				m_fModeWeights[1] = 1.f;
			}
			else
			{
				m_fModeWeights[1] = Max( fElapsed / fBlendDuration, 0.01f );
			}
		}
		else
		{
			m_fModeWeights[1] = fRemaining * ( 1.f / fBlendDuration );
			if( Abs( m_fModeWeights[1] ) < 0.0001f )
			{
				m_fModeWeights[0] = 1.f;
				m_fModeWeights[1] = 0.f;
				m_bPlayingSpecificAnim = FALSE;
			}
		}
		m_fModeWeights[0] = 1.f - m_fModeWeights[1];
		if( Abs( m_fModeWeights[0] ) >= 0.00000001f && Abs( fOldMovementWeight ) < 0.00000001f )
		{
			m_bStartSpecificAnimBlendOut = TRUE;
		}
		m_fSpecificPrevProgress = fProgress;
	}

	if( m_MostRelevantWeightIndex == -1 || !m_bSpecificAnimRequested )
	{
		return;
	}
	const UAnimNodeSequence* pMostRelevant = m_ChildSlots[m_MostRelevantWeightIndex].m_pAnimNode;
	if( !pMostRelevant->AnimSeq )
	{
		return;
	}
	m_bPlayingSpecificAnim = m_SpecificAnimProps.m_bUseAnimIn ? FALSE : TRUE;
	if( !m_bPlayingSpecificAnim )
	{
		const FLOAT fInvLength = 1.f / pMostRelevant->AnimSeq->SequenceLength;
		const FLOAT fCurrentTime = pMostRelevant->CurrentTime;
		const FLOAT fNewProgress = ( m_fMostRelevantRateScale * _fDeltaSeconds + fCurrentTime ) * fInvLength;
		const FLOAT fMoveAnimIn = m_SpecificAnimProps.m_fMoveAnimIn;
		const FLOAT fCurProgress = appFmod( fInvLength * fCurrentTime + 1.f, 1.f );
		UBOOL bPlay;
		if( fCurProgress <= fNewProgress )
		{
			bPlay = !( fMoveAnimIn < fCurProgress || fNewProgress < fMoveAnimIn );
		}
		else
		{
			bPlay = !( fMoveAnimIn < fCurProgress && fNewProgress < fMoveAnimIn );
		}
		m_bPlayingSpecificAnim = bPlay;
	}
	if( m_bPlayingSpecificAnim )
	{
		m_fModeWeights[1] = 0.01f;
		m_fModeWeights[0] = 0.99f;
	}
}

// DISHONORED(port): 2013 rva 0x516770 (2012 0x556c40), 472 bytes. The two modifier slots crossfade over
// m_fModifierBlendDuration when the modifier changes (a slot under m_fModifierBlendOverwriteMaxPct is overwritten in place
// instead); a specific animation that blends out hands its end modifier to the main slot; then each weighted slot picks
// its speed modes for the smoothed speed scaled by the out-of-shape ratio.
void UArkAnimNodeLocomotion::UpdateMovementAnims( FLOAT _fDeltaSeconds )
{
	FAnimNodeLocoModifierSlot* pMain = &m_ModifierSlots[m_MainModifierSlotIdx];
	if( pMain->m_Modifier != m_Modifier && !m_bPlayingSpecificAnim )
	{
		if( pMain->m_fWeight < m_fModifierBlendOverwriteMaxPct )
		{
			pMain->m_Modifier = m_Modifier;
		}
		else if( Abs( pMain->m_fWeight - 1.f ) < 0.00001f )
		{
			const INT PreviousMain = m_MainModifierSlotIdx;
			m_MainModifierSlotIdx = m_OldModifierSlotIdx;
			m_OldModifierSlotIdx = PreviousMain;
			m_ModifierSlots[m_MainModifierSlotIdx].m_Modifier = m_Modifier;
		}
	}
	pMain = &m_ModifierSlots[m_MainModifierSlotIdx];
	FAnimNodeLocoModifierSlot* pOld = &m_ModifierSlots[m_OldModifierSlotIdx];
	if( m_bStartSpecificAnimBlendOut )
	{
		pMain->m_Modifier = m_SpecificAnimProps.m_EndModifier;
		pMain->m_fWeight = 1.f;
		ResetModifierSlot( m_OldModifierSlotIdx, TRUE );
	}

	const FLOAT fMainWeight = pMain->m_fWeight;
	if( Abs( fMainWeight - 1.f ) >= 0.00001f )
	{
		if( Abs( fMainWeight ) < 0.00001f && Abs( pOld->m_fWeight ) < 0.00001f )
		{
			pMain->m_fWeight = 1.f;
		}
		else
		{
			const FLOAT fStep = _fDeltaSeconds / m_fModifierBlendDuration;
			pMain->m_fWeight = Min( fMainWeight + fStep, 1.f );
			if( Abs( pOld->m_fWeight ) >= 0.00001f )
			{
				if( pOld->m_fWeight <= fStep )
				{
					ResetModifierSlot( m_OldModifierSlotIdx, TRUE );
				}
				else
				{
					pOld->m_fWeight -= fStep;
				}
			}
		}
	}

	if( Abs( m_fModeWeights[0] ) >= 0.00001f )
	{
		m_fOutOfShapeRatio = ComputeBlendingOutOfShapeRatio();
		m_fSmoothedSpeedWithBlend = m_fOutOfShapeRatio * m_fSmoothedSpeed;
		for( INT ModifierSlotIdx = 0; ModifierSlotIdx < 2; ++ModifierSlotIdx )
		{
			if( Abs( m_ModifierSlots[ModifierSlotIdx].m_fWeight ) >= 0.00001f )
			{
				UpdateModifierSlot( ModifierSlotIdx, 0.98f );
			}
		}
	}
}

// DISHONORED(port): 2013 rva 0x5105b0 (2012 0x550ca0), 946 bytes. Walks the slot's modifier's speed modes from the slowest
// up: each mode's speed at the smoothed angle is its direction animations' foot speeds weighted by the angle blend; the
// last mode not faster than the blended speed and the first faster one become the two speed slots, weighted linearly
// between their speeds and snapped to one past _fSnapOnOneSpeedThreshold. Speed index 0 is the static (idle) mode.
void UArkAnimNodeLocomotion::UpdateModifierSlot( INT _ModifierSlotIdx, FLOAT _fSnapOnOneSpeedThreshold )
{
	struct FAnimNodeLocoSpeedProps
	{
		FAnimNodeLocoAngleSlot	m_AngleSlots[2];
		INT						m_SpeedModeIdx;
		FLOAT					m_fSpeed;
		UBOOL					m_bStatic;
	};
	const UArkComponentLocomotionConfig* Config = GetConfig();
	FAnimNodeLocoModifierSlot& rSlot = m_ModifierSlots[_ModifierSlotIdx];
	FAnimNodeLocoSpeedProps SpeedProps[2];
	SpeedProps[0].m_SpeedModeIdx = -1;
	SpeedProps[1].m_SpeedModeIdx = -1;
	const INT SpeedNames = Config->m_SpeedNames.Num();
	for( INT SpeedIdx = 0; SpeedIdx < SpeedNames; ++SpeedIdx )
	{
		const INT SpeedModeIdx = rSlot.m_Modifier * SpeedNames + SpeedIdx;
		const FArkCpntLocoSpeedModeProp& rMode = GetSpeedModeProp( SpeedModeIdx );
		const INT AnimCount = rMode.m_MoveAnimProps.Num();
		const INT Step = 8 / AnimCount;
		FArkCpntLocoDirAndWeight DirAndWeights[2];
		Config->ComputeAnimDirAndWeightFromAngleAndDirCount( m_SmoothedAngle, AnimCount, DirAndWeights, 0.98f );
		FLOAT fSpeed = 0.f;
		for( INT DirIdx = 0; DirIdx < 2; ++DirIdx )
		{
			if( DirAndWeights[DirIdx].m_Index != -1 )
			{
				fSpeed += rMode.m_MoveAnimProps( DirAndWeights[DirIdx].m_Index / Step ).m_fAnimSpeed * DirAndWeights[DirIdx].m_fWeight;
			}
		}
		const UBOOL bFaster = fSpeed > m_fSmoothedSpeedWithBlend;
		FAnimNodeLocoSpeedProps& rProps = SpeedProps[bFaster ? 1 : 0];
		rProps.m_SpeedModeIdx = SpeedModeIdx;
		rProps.m_fSpeed = fSpeed;
		rProps.m_AngleSlots[0].m_Index = DirAndWeights[0].m_Index;
		rProps.m_AngleSlots[0].m_fWeight = DirAndWeights[0].m_fWeight;
		rProps.m_AngleSlots[1].m_Index = DirAndWeights[1].m_Index;
		rProps.m_AngleSlots[1].m_fWeight = DirAndWeights[1].m_fWeight;
		rProps.m_bStatic = ( SpeedIdx == 0 );
		if( bFaster )
		{
			break;
		}
	}

	if( SpeedProps[1].m_SpeedModeIdx == -1 )
	{
		rSlot.m_SpeedSlots[0].m_fWeight = 1.f;
		rSlot.m_SpeedSlots[1].m_fWeight = 0.f;
	}
	else
	{
		const FLOAT fFasterWeight = ( 1.f / ( SpeedProps[1].m_fSpeed - SpeedProps[0].m_fSpeed ) ) * ( m_fSmoothedSpeedWithBlend - SpeedProps[0].m_fSpeed );
		rSlot.m_SpeedSlots[1].m_fWeight = fFasterWeight;
		rSlot.m_SpeedSlots[0].m_fWeight = 1.f - fFasterWeight;
		if( rSlot.m_SpeedSlots[0].m_fWeight >= _fSnapOnOneSpeedThreshold )
		{
			rSlot.m_SpeedSlots[0].m_fWeight = 1.f;
			rSlot.m_SpeedSlots[1].m_fWeight = 0.f;
		}
		else if( rSlot.m_SpeedSlots[1].m_fWeight >= _fSnapOnOneSpeedThreshold )
		{
			rSlot.m_SpeedSlots[0].m_fWeight = 0.f;
			rSlot.m_SpeedSlots[1].m_fWeight = 1.f;
		}
	}

	for( INT SpeedSlotIdx = 0; SpeedSlotIdx < 2; ++SpeedSlotIdx )
	{
		FAnimNodeLocoSpeedSlot& rSpeedSlot = rSlot.m_SpeedSlots[SpeedSlotIdx];
		const FAnimNodeLocoSpeedProps& rProps = SpeedProps[SpeedSlotIdx];
		if( Abs( rSpeedSlot.m_fWeight ) < 0.00001f )
		{
			rSpeedSlot.m_AngleSlots[0].m_Index = -1;
			rSpeedSlot.m_AngleSlots[0].m_fWeight = 0.f;
			rSpeedSlot.m_AngleSlots[1].m_Index = -1;
			rSpeedSlot.m_AngleSlots[1].m_fWeight = 0.f;
			rSpeedSlot.m_bStatic = FALSE;
			rSpeedSlot.m_SpeedModeIdx = -1;
			rSpeedSlot.m_fWeight = 0.f;
			continue;
		}
		rSpeedSlot.m_SpeedModeIdx = rProps.m_SpeedModeIdx;
		rSpeedSlot.m_bStatic = rProps.m_bStatic ? TRUE : FALSE;
		for( INT AngleSlotIdx = 0; AngleSlotIdx < 2; ++AngleSlotIdx )
		{
			if( rProps.m_AngleSlots[AngleSlotIdx].m_Index == -1 )
			{
				rSpeedSlot.m_AngleSlots[AngleSlotIdx].m_Index = -1;
				rSpeedSlot.m_AngleSlots[AngleSlotIdx].m_fWeight = 0.f;
			}
			else
			{
				rSpeedSlot.m_AngleSlots[AngleSlotIdx] = rProps.m_AngleSlots[AngleSlotIdx];
			}
		}
	}
}

// DISHONORED(port): 2013 rva 0x510c90 (2012 0x551380), 91 bytes.
void UArkAnimNodeLocomotion::ResetModifierSlot( INT _ModifierSlotIdx, UBOOL _bResetWeight )
{
	FAnimNodeLocoModifierSlot& rSlot = m_ModifierSlots[_ModifierSlotIdx];
	for( INT SpeedSlotIdx = 0; SpeedSlotIdx < 2; ++SpeedSlotIdx )
	{
		FAnimNodeLocoSpeedSlot& rSpeedSlot = rSlot.m_SpeedSlots[SpeedSlotIdx];
		rSpeedSlot.m_AngleSlots[0].m_Index = -1;
		rSpeedSlot.m_AngleSlots[0].m_fWeight = 0.f;
		rSpeedSlot.m_AngleSlots[1].m_Index = -1;
		rSpeedSlot.m_AngleSlots[1].m_fWeight = 0.f;
		rSpeedSlot.m_bStatic = FALSE;
		rSpeedSlot.m_SpeedModeIdx = -1;
		rSpeedSlot.m_fWeight = 0.f;
	}
	rSlot.m_Modifier = -1;
	if( _bResetWeight )
	{
		rSlot.m_fWeight = 0.f;
	}
}

// DISHONORED(port): 2013 rva 0x510cf0 (2012 0x5513e0), 308 bytes: the fastest foot speed of any move animation.
FLOAT UArkAnimNodeLocomotion::FindFastestSpeed() const
{
	const UArkComponentLocomotionConfig* Config = GetConfig();
	const INT SpeedModeNumber = Config->m_SpeedNames.Num() * Config->m_Modifiers.Num();
	FLOAT fMaxSpeed = 0.f;
	for( INT SpeedModeIdx = 0; SpeedModeIdx < SpeedModeNumber; ++SpeedModeIdx )
	{
		const FArkCpntLocoSpeedModeProp& rMode = GetSpeedModeProp( SpeedModeIdx );
		for( INT AnimIdx = 0; AnimIdx < rMode.m_MoveAnimProps.Num(); ++AnimIdx )
		{
			fMaxSpeed = Max( fMaxSpeed, rMode.m_MoveAnimProps( AnimIdx ).m_fAnimSpeed );
		}
	}
	return fMaxSpeed;
}

// DISHONORED(port): 2013 rva 0x510e30 (2012 0x551520), 1,017 bytes. Two direction animations blended by angle move the
// body along the chord between their two points on the unit circle, not along the circle: the ratio is 1 over the chord's
// distance from the centre along the smoothed angle, and it scales the speed the speed modes are chosen for. The eight
// chord points are the function-local static (2012 .data 0xf4a730, SegmentPoints) at 45 degree steps from +X.
FLOAT UArkAnimNodeLocomotion::ComputeBlendingOutOfShapeRatio() const
{
	static const FVector2D SegmentPoints[8] =
	{
		FVector2D( 1.f, 0.f ), FVector2D( 0.70710677f, 0.70710677f ), FVector2D( 0.f, 1.f ), FVector2D( -0.70710677f, 0.70710677f ),
		FVector2D( -1.f, 0.f ), FVector2D( -0.70710677f, -0.70710677f ), FVector2D( 0.f, -1.f ), FVector2D( 0.70710677f, -0.70710677f )
	};
	const UArkComponentLocomotionConfig* Config = GetConfig();
	INT SpeedModeIdx0 = -1;
	INT SpeedModeIdx1 = -1;
	FLOAT fSpeedMode0Weight = 0.f;
	Config->GetSpeedModeFromForwardSpeed( m_fSmoothedSpeed, m_Modifier, SpeedModeIdx0, SpeedModeIdx1, fSpeedMode0Weight );
	INT SpeedModeIdx = SpeedModeIdx1;
	if( fSpeedMode0Weight >= 0.5f )
	{
		if( SpeedModeIdx0 == -1 )
		{
			return 1.f;
		}
		const FArkCpntLocoSpeedModeProp& rMode0 = GetSpeedModeProp( SpeedModeIdx0 );
		SpeedModeIdx = ( rMode0.m_MoveAnimProps.Num() > 0 && Abs( rMode0.m_MoveAnimProps( 0 ).m_fAnimSpeed ) >= 0.1f ) ? SpeedModeIdx0 : SpeedModeIdx1;
	}
	if( SpeedModeIdx == -1 )
	{
		return 1.f;
	}
	const INT AnimCount = GetSpeedModeProp( SpeedModeIdx ).m_MoveAnimProps.Num();
	if( AnimCount <= 1 )
	{
		return 1.f;
	}
	FArkCpntLocoDirAndWeight DirAndWeights[2];
	Config->ComputeAnimDirAndWeightFromAngleAndDirCount( m_SmoothedAngle, AnimCount, DirAndWeights, 0.98f );
	if( DirAndWeights[0].m_Index == -1 || DirAndWeights[1].m_Index == -1 )
	{
		return 1.f;
	}
	const FLOAT fAngle = (FLOAT)m_SmoothedAngle * 0.000095873802f;
	const INT Step = 8 / AnimCount;
	INT Segment = appTrunc( (FLOAT)appTrunc( fAngle / (FLOAT)( ( 1.0 / (DOUBLE)AnimCount ) * 6.283185307179586 ) ) * (FLOAT)Step );
	if( fAngle < 0.f )
	{
		Segment += 8 - Step;
	}
	const INT NextSegment = ( Segment + Step ) % 8;
	const FVector2D& P0 = SegmentPoints[Segment & 7];
	const FVector2D& P1 = SegmentPoints[NextSegment];
	const FLOAT fCos = appCos( fAngle );
	const FLOAT fSin = appSin( fAngle );
	const FLOAT fDenominator = ( P0.X - P1.X ) * fSin - ( P0.Y - P1.Y ) * fCos;
	if( Abs( fDenominator ) < 0.00000001f )
	{
		return 1.f;
	}
	const FLOAT fCross = P1.Y * P0.X - P0.Y * P1.X;
	const FLOAT fInvDenominator = 1.f / fDenominator;
	const FLOAT fY = fCross * fSin * fInvDenominator;
	const FLOAT fX = fCross * fCos * fInvDenominator;
	return 1.f / appSqrt( fY * fY + fX * fX );
}

// DISHONORED(port): 2013 rva 0x5118b0 (2012 0x551fa0), 89 bytes: the modifier's speed multiplier times the look-at one.
FLOAT UArkAnimNodeLocomotion::GetFinalSpeedMultiplier() const
{
	return GetConfig()->m_Modifiers( m_Modifier ).m_fSpeedMultiplier * m_fSpeedMultiplier;
}

// DISHONORED(port): 2013 rva 0x510970 (2012 0x551060), 800 bytes. One movement child: its weight is modifier x speed x
// angle weight times the movement mode weight; a non-static speed slot synchronises it in the group; a direction left of
// forward in a multi-direction mode plays its animation reversed (bReverseSync, Rate -1); a static slot that is not nearly
// still is stopped rather than played. Returns the child's contribution to the group's foot speed, weight x anim speed x
// sequence length (the sum is divided out in UpdateMovementChildren's rate scale).
FLOAT UArkAnimNodeLocomotion::UpdateMovementChild( const FAnimNodeLocoModifierSlot& _ModifierSlot, const FAnimNodeLocoSpeedSlot& _SpeedSlot, const FAnimNodeLocoAngleSlot& _AngleSlot, INT _ChildSlotIdx, UBOOL _bSetAnim )
{
	const UBOOL bSynchronize = !_SpeedSlot.m_bStatic;
	const UBOOL bPlay = bSynchronize || Abs( m_fSmoothedSpeedWithBlend ) < 0.1f;
	const FLOAT fWeight = _ModifierSlot.m_fWeight * _SpeedSlot.m_fWeight * _AngleSlot.m_fWeight;
	const FArkCpntLocoSpeedModeProp& rMode = GetSpeedModeProp( _SpeedSlot.m_SpeedModeIdx );
	const INT AnimCount = rMode.m_MoveAnimProps.Num();
	const INT ModeAnimIdx = _AngleSlot.m_Index / ( 8 / AnimCount );
	FAnimNodeLocoChildSlot& rChildSlot = m_ChildSlots[_ChildSlotIdx];
	UAnimNodeSequence* pSequence = rChildSlot.m_pAnimNode;

	pSequence->bSynchronize = bSynchronize ? TRUE : FALSE;
	pSequence->bReverseSync = ( AnimCount > 1 && _AngleSlot.m_Index < 4 ) ? TRUE : FALSE;
	pSequence->Rate = pSequence->bReverseSync ? -1.f : 1.f;
	Children( _ChildSlotIdx ).Weight = Min( m_fModeWeights[0] * fWeight, 1.f );
	if( bPlay )
	{
		pSequence->bPlaying = TRUE;
	}
	else
	{
		pSequence->StopAnim();
	}
	rChildSlot.m_SpeedModeIdx = _SpeedSlot.m_SpeedModeIdx;
	rChildSlot.m_DirIndex = _AngleSlot.m_Index;
	if( _bSetAnim )
	{
		pSequence->SetAnim( rMode.m_MoveAnimProps( ModeAnimIdx ).m_AnimName );
		pSequence->CurrentTime = 0.f;
		pSequence->PreviousTime = 0.f;
	}
	if( fWeight > m_fMostRelevantWeight )
	{
		m_fMostRelevantWeight = fWeight;
		m_MostRelevantWeightIndex = _ChildSlotIdx;
	}
	if( !pSequence->AnimSeq )
	{
		return 0.f;
	}
	return rMode.m_MoveAnimProps( ModeAnimIdx ).m_fAnimSpeed * pSequence->AnimSeq->SequenceLength * fWeight;
}

// DISHONORED(port): 2013 rva 0x516950 (2012 0x556e20), 2,799 bytes. The eight movement children against the 2 x 2 x 2
// (modifier, speed, angle) slots: a child already playing a slot's animation keeps its slot and is updated in place; a
// child whose slot is gone is cleared; every weighted slot nobody plays takes the first free child and starts its
// animation. A new set with no synchronised child yet (all static) or a specific animation blending out forces every
// synchronised child to the group's position (or the specific animation's move-out point). Finally the group's rate scale
// makes the weighted foot speed of the chosen animations match the pawn's speed.
void UArkAnimNodeLocomotion::UpdateMovementChildren()
{
	UBOOL bAngleSlotFound[2][2][2];
	appMemzero( bAngleSlotFound, sizeof( bAngleSlotFound ) );
	INT FirstFreeChildIdx = -1;
	FLOAT fAnimSpeedSum = 0.f;
	UBOOL bHasSynchronizedChild = FALSE;
	m_fMostRelevantWeight = 0.f;
	m_MostRelevantWeightIndex = -1;

	for( INT ChildSlotIdx = 0; ChildSlotIdx < 8; ++ChildSlotIdx )
	{
		FAnimNodeLocoChildSlot& rChildSlot = m_ChildSlots[ChildSlotIdx];
		UBOOL bKept = FALSE;
		if( rChildSlot.m_SpeedModeIdx != -1 )
		{
			for( INT ModifierSlotIdx = 0; ModifierSlotIdx < 2 && !bKept; ++ModifierSlotIdx )
			{
				const FAnimNodeLocoModifierSlot& rModifierSlot = m_ModifierSlots[ModifierSlotIdx];
				for( INT SpeedSlotIdx = 0; SpeedSlotIdx < 2 && !bKept; ++SpeedSlotIdx )
				{
					const FAnimNodeLocoSpeedSlot& rSpeedSlot = rModifierSlot.m_SpeedSlots[SpeedSlotIdx];
					for( INT AngleSlotIdx = 0; AngleSlotIdx < 2 && !bKept; ++AngleSlotIdx )
					{
						const FAnimNodeLocoAngleSlot& rAngleSlot = rSpeedSlot.m_AngleSlots[AngleSlotIdx];
						if( bAngleSlotFound[ModifierSlotIdx][SpeedSlotIdx][AngleSlotIdx] || rSpeedSlot.m_SpeedModeIdx == -1 || rAngleSlot.m_Index == -1 )
						{
							continue;
						}
						const FArkCpntLocoSpeedModeProp& rMode = GetSpeedModeProp( rSpeedSlot.m_SpeedModeIdx );
						const INT ModeAnimIdx = rAngleSlot.m_Index / ( 8 / rMode.m_MoveAnimProps.Num() );
						if( rChildSlot.m_pAnimNode->AnimSeqName == rMode.m_MoveAnimProps( ModeAnimIdx ).m_AnimName )
						{
							fAnimSpeedSum += UpdateMovementChild( rModifierSlot, rSpeedSlot, rAngleSlot, ChildSlotIdx, FALSE );
							bHasSynchronizedChild |= !rSpeedSlot.m_bStatic;
							bAngleSlotFound[ModifierSlotIdx][SpeedSlotIdx][AngleSlotIdx] = TRUE;
							bKept = TRUE;
						}
					}
				}
			}
		}
		if( !bKept )
		{
			rChildSlot.m_SpeedModeIdx = -1;
			rChildSlot.m_DirIndex = -1;
			Children( ChildSlotIdx ).Weight = 0.f;
			if( rChildSlot.m_pAnimNode->AnimSeqName != NAME_None )
			{
				rChildSlot.m_pAnimNode->SetAnim( NAME_None );
			}
		}
		if( FirstFreeChildIdx == -1 && rChildSlot.m_SpeedModeIdx == -1 )
		{
			FirstFreeChildIdx = ChildSlotIdx;
		}
	}

	FLOAT fGroupProgress = 0.f;
	if( !bHasSynchronizedChild )
	{
		const INT GroupIndex = m_pParentAnimTree->GetGroupIndex( s_AnimNodeLocoGroupName );
		if( GroupIndex != -1 )
		{
			const FAnimGroup& rGroup = m_pParentAnimTree->AnimGroups( GroupIndex );
			if( rGroup.SynchMaster )
			{
				fGroupProgress = rGroup.SynchMaster->bReverseSync ? 1.f - rGroup.SynchPctPosition : rGroup.SynchPctPosition;
			}
		}
	}

	INT NextFreeChildIdx = FirstFreeChildIdx;
	for( INT ModifierSlotIdx = 0; ModifierSlotIdx < 2; ++ModifierSlotIdx )
	{
		const FAnimNodeLocoModifierSlot& rModifierSlot = m_ModifierSlots[ModifierSlotIdx];
		for( INT SpeedSlotIdx = 0; SpeedSlotIdx < 2; ++SpeedSlotIdx )
		{
			const FAnimNodeLocoSpeedSlot& rSpeedSlot = rModifierSlot.m_SpeedSlots[SpeedSlotIdx];
			for( INT AngleSlotIdx = 0; AngleSlotIdx < 2; ++AngleSlotIdx )
			{
				const FAnimNodeLocoAngleSlot& rAngleSlot = rSpeedSlot.m_AngleSlots[AngleSlotIdx];
				if( bAngleSlotFound[ModifierSlotIdx][SpeedSlotIdx][AngleSlotIdx] || rModifierSlot.m_fWeight <= 0.f || rSpeedSlot.m_fWeight <= 0.f || rAngleSlot.m_Index == -1 )
				{
					continue;
				}
				while( NextFreeChildIdx < 8 && m_ChildSlots[NextFreeChildIdx].m_SpeedModeIdx != -1 )
				{
					++NextFreeChildIdx;
				}
				if( NextFreeChildIdx >= 8 )
				{
					continue;
				}
				fAnimSpeedSum += UpdateMovementChild( rModifierSlot, rSpeedSlot, rAngleSlot, NextFreeChildIdx, TRUE );
				++NextFreeChildIdx;
			}
		}
	}

	if( ( m_bStartSpecificAnimBlendOut || !bHasSynchronizedChild ) && m_MostRelevantWeightIndex != -1 )
	{
		UAnimNodeSequence* pMostRelevant = m_ChildSlots[m_MostRelevantWeightIndex].m_pAnimNode;
		if( !pMostRelevant->bSynchronize )
		{
			INT BestIdx = -1;
			FLOAT fBestWeight = 0.f;
			for( INT ChildSlotIdx = 0; ChildSlotIdx < 8; ++ChildSlotIdx )
			{
				if( m_ChildSlots[ChildSlotIdx].m_SpeedModeIdx != -1 && m_ChildSlots[ChildSlotIdx].m_pAnimNode->bSynchronize && Children( ChildSlotIdx ).Weight > fBestWeight )
				{
					BestIdx = ChildSlotIdx;
					fBestWeight = Children( ChildSlotIdx ).Weight;
				}
			}
			if( BestIdx != -1 )
			{
				pMostRelevant = m_ChildSlots[BestIdx].m_pAnimNode;
			}
		}
		if( pMostRelevant->AnimSeq )
		{
			FLOAT fProgress = fGroupProgress;
			if( m_bStartSpecificAnimBlendOut && pMostRelevant->bSynchronize )
			{
				fProgress = appFmod( m_SpecificAnimProps.m_fMoveAnimOut - m_SpecificAnimProps.m_fBlendDuration / pMostRelevant->AnimSeq->SequenceLength, 1.f );
				if( fProgress < 0.f )
				{
					fProgress += 1.f;
				}
				m_pParentAnimTree->ForceGroupRelativePosition( s_AnimNodeLocoGroupName, fProgress );
			}
			for( INT ChildSlotIdx = 0; ChildSlotIdx < 8; ++ChildSlotIdx )
			{
				if( m_ChildSlots[ChildSlotIdx].m_SpeedModeIdx == -1 )
				{
					continue;
				}
				UAnimNodeSequence* pSequence = m_ChildSlots[ChildSlotIdx].m_pAnimNode;
				if( !pSequence->AnimSeq )
				{
					continue;
				}
				if( pSequence->bSynchronize )
				{
					pSequence->CurrentTime = pSequence->AnimSeq->SequenceLength * fProgress;
					pSequence->PreviousTime = pSequence->CurrentTime;
				}
				else if( m_bStartSpecificAnimBlendOut )
				{
					pSequence->CurrentTime = pSequence->AnimSeq->SequenceLength - m_SpecificAnimProps.m_fBlendDuration;
					pSequence->PreviousTime = pMostRelevant->CurrentTime;
				}
			}
		}
	}

	m_fMostRelevantRateScale = 1.f;
	if( m_MostRelevantWeightIndex != -1 )
	{
		const FAnimNodeLocoChildSlot& rMostRelevant = m_ChildSlots[m_MostRelevantWeightIndex];
		const UAnimSequence* pAnimSequence = rMostRelevant.m_pAnimNode->AnimSeq;
		if( pAnimSequence && Abs( m_fSmoothedSpeedWithBlend ) >= 0.1f && Abs( fAnimSpeedSum ) >= 0.1f )
		{
			const FArkCpntLocoSpeedModeProp& rMode = GetSpeedModeProp( rMostRelevant.m_SpeedModeIdx );
			if( rMode.m_MoveAnimProps.Num() > 0 && Abs( rMode.m_MoveAnimProps( 0 ).m_fAnimSpeed ) >= 0.1f )
			{
				m_fMostRelevantRateScale = pAnimSequence->SequenceLength / ( fAnimSpeedSum / m_fSmoothedSpeedWithBlend );
				m_fMostRelevantRateScale = GetConfig()->m_Modifiers( m_Modifier ).m_fSpeedMultiplier * m_fSpeedMultiplier * m_fMostRelevantRateScale;
			}
		}
	}
	m_pParentAnimTree->SetGroupRateScale( s_AnimNodeLocoGroupName, m_fMostRelevantRateScale );
}

// DISHONORED(port): 2013 rva 0x517440 (2012 0x557910), 319 bytes. Child 8 plays the specific animation: a fresh request
// sets it, rates it by the final speed multiplier and rewinds it (and is dropped if the sequence does not exist); a child
// no longer playing is cleared. Its weight is the specific mode weight.
void UArkAnimNodeLocomotion::UpdateSpecificChild()
{
	m_bStartSpecificAnimBlendOut = FALSE;
	FLOAT fWeight = 0.f;
	UAnimNodeSequence* pSpecific = m_ChildSlots[8].m_pAnimNode;
	if( m_bPlayingSpecificAnim )
	{
		if( m_bSpecificAnimRequested )
		{
			pSpecific->SetAnim( m_SpecificAnimProps.m_AnimName );
			pSpecific->Rate = GetFinalSpeedMultiplier();
			pSpecific->bPlaying = TRUE;
			pSpecific->CurrentTime = 0.f;
			m_fSpecificPrevProgress = 0.f;
			m_bSpecificAnimRequested = FALSE;
			m_bPlayingSpecificAnim = ( pSpecific->AnimSeq != NULL );
		}
		fWeight = m_fModeWeights[1];
		if( !m_bPlayingSpecificAnim )
		{
			m_fModeWeights[0] = 1.f;
			m_fModeWeights[1] = 0.f;
		}
	}
	else if( pSpecific->AnimSeqName != NAME_None )
	{
		pSpecific->SetAnim( NAME_None );
	}
	Children( 8 ).Weight = fWeight;
}

// DISHONORED(port): 2013 rva 0x5114a0 (2012 0x551b90), 793 bytes. Each configured foot is on the ground
// when the group's position is within half the weighted foot-on-ground range of the foot's own on-ground point.
// DISHONORED(bringup): retail divides by the weight sum whenever it is under 1 and gets infinity for a sum of 0 (which
// the multiplication by a 0 range turns into NaN and the comparison into FALSE); the sum is tested here instead.
void UArkAnimNodeLocomotion::UpdateFeetPropeties()
{
	const INT FeetCount = m_bFootOnGround.Num();
	if( FeetCount <= 0 )
	{
		return;
	}
	if( m_MostRelevantWeightIndex == -1 )
	{
		for( INT FootIdx = 0; FootIdx < FeetCount; ++FootIdx )
		{
			m_bFootOnGround( FootIdx ) = 0;
		}
		return;
	}
	const FLOAT fProgress = m_pParentAnimTree->GetGroupRelativePosition( s_AnimNodeLocoGroupName );
	FLOAT fHalfRange = 0.f;
	FLOAT fWeightSum = 0.f;
	for( INT ChildSlotIdx = 0; ChildSlotIdx < 8; ++ChildSlotIdx )
	{
		if( m_ChildSlots[ChildSlotIdx].m_SpeedModeIdx == -1 )
		{
			continue;
		}
		const FArkCpntLocoSpeedModeProp& rMode = GetSpeedModeProp( m_ChildSlots[ChildSlotIdx].m_SpeedModeIdx );
		if( rMode.m_MoveAnimProps.Num() > 0 && Abs( rMode.m_MoveAnimProps( 0 ).m_fAnimSpeed ) >= 0.1f )
		{
			const FLOAT fWeight = Children( ChildSlotIdx ).Weight;
			fHalfRange += rMode.m_fFootOnGroundPct * fWeight;
			fWeightSum += fWeight;
		}
	}
	if( fWeightSum < 1.f && fWeightSum > 0.00001f )
	{
		fHalfRange *= 1.f / fWeightSum;
	}
	const UArkComponentLocomotionConfig* Config = GetConfig();
	for( INT FootIdx = 0; FootIdx < FeetCount; ++FootIdx )
	{
		FLOAT fDistance = Abs( Config->m_Feet( FootIdx ).m_fOnGroundPct - fProgress );
		if( fDistance > 0.5f )
		{
			fDistance = 1.f - fDistance;
		}
		m_bFootOnGround( FootIdx ) = ( fHalfRange * 0.5f >= fDistance ) ? 1 : 0;
	}
}

// DISHONORED(port): 2013 rva 0x5113a0 (2012 0x551a90), 246 bytes: the group's position this tick and the one it will have
// next tick at the group's rate, for the turn / stop state machines' move-in tests.
void UArkAnimNodeLocomotion::UpdateProgressVariables( FLOAT _fDeltaSeconds )
{
	if( m_MostRelevantWeightIndex != -1 && bRelevant )
	{
		const INT GroupIndex = m_pParentAnimTree->GetGroupIndex( s_AnimNodeLocoGroupName );
		const UAnimSequence* pAnimSequence = m_ChildSlots[m_MostRelevantWeightIndex].m_pAnimNode->AnimSeq;
		if( GroupIndex != -1 && pAnimSequence )
		{
			const FAnimGroup& rGroup = m_pParentAnimTree->AnimGroups( GroupIndex );
			m_fMovePrevProgress = rGroup.SynchPctPosition;
			m_fMoveCurProgress = appFmod( _fDeltaSeconds / pAnimSequence->SequenceLength * rGroup.RateScale + rGroup.SynchPctPosition, 1.f );
			return;
		}
	}
	m_fMovePrevProgress = 0.f;
	m_fMoveCurProgress = 0.f;
}

/*-----------------------------------------------------------------------------
	-gadiag
-----------------------------------------------------------------------------*/

FString UArkAnimNodeLocomotion::DescribeForDiag() const
{
	FString Out = FString::Printf( TEXT("init %d allseq %d relevant %d speed %.1f smoothed %.1f blend %.1f angle %d/%d modifier %d slots %d/%d ratio %.3f mode %.2f/%.2f rate %.3f mri %d specific req %d play %d"),
		(INT)m_bInitialized, (INT)m_bHaveAllAnimNodeSequences, (INT)bRelevant, m_fSpeed, m_fSmoothedSpeed, m_fSmoothedSpeedWithBlend, m_Angle, m_SmoothedAngle,
		m_Modifier, m_MainModifierSlotIdx, m_OldModifierSlotIdx, m_fOutOfShapeRatio, m_fModeWeights[0], m_fModeWeights[1], m_fMostRelevantRateScale,
		m_MostRelevantWeightIndex, (INT)m_bSpecificAnimRequested, (INT)m_bPlayingSpecificAnim );
	for( INT ModifierSlotIdx = 0; ModifierSlotIdx < 2; ++ModifierSlotIdx )
	{
		const FAnimNodeLocoModifierSlot& rSlot = m_ModifierSlots[ModifierSlotIdx];
		Out += FString::Printf( TEXT(" | M%d mod %d w %.2f"), ModifierSlotIdx, rSlot.m_Modifier, rSlot.m_fWeight );
		for( INT SpeedSlotIdx = 0; SpeedSlotIdx < 2; ++SpeedSlotIdx )
		{
			const FAnimNodeLocoSpeedSlot& rSpeed = rSlot.m_SpeedSlots[SpeedSlotIdx];
			if( rSpeed.m_SpeedModeIdx != -1 )
			{
				Out += FString::Printf( TEXT(" S%d mode %d w %.2f%s dirs %d:%.2f %d:%.2f"), SpeedSlotIdx, rSpeed.m_SpeedModeIdx, rSpeed.m_fWeight, rSpeed.m_bStatic ? TEXT(" static") : TEXT(""),
					rSpeed.m_AngleSlots[0].m_Index, rSpeed.m_AngleSlots[0].m_fWeight, rSpeed.m_AngleSlots[1].m_Index, rSpeed.m_AngleSlots[1].m_fWeight );
			}
		}
	}
	Out += TEXT(" | children");
	for( INT ChildSlotIdx = 0; ChildSlotIdx < 9 && ChildSlotIdx < Children.Num(); ++ChildSlotIdx )
	{
		const UAnimNodeSequence* pSequence = m_ChildSlots[ChildSlotIdx].m_pAnimNode;
		if( Children( ChildSlotIdx ).Weight > 0.f || ( pSequence && pSequence->AnimSeqName != NAME_None ) )
		{
			Out += FString::Printf( TEXT(" %d:%s w %.2f%s"), ChildSlotIdx, pSequence ? *pSequence->AnimSeqName.ToString() : TEXT("none"), Children( ChildSlotIdx ).Weight,
				( pSequence && pSequence->bPlaying ) ? TEXT("") : TEXT(" stopped") );
			if( pSequence )
			{
				Out += FString::Printf( TEXT(" t %.2f%s"), pSequence->CurrentTime, pSequence->bSynchronize ? TEXT(" sync") : TEXT("") );
			}
		}
	}
	return Out;
}

UBOOL DisGCLocoDiagEnabled()
{
	return DisGCDiagEnabled();
}
