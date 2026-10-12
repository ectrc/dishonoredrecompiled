// Engine/src/interptrackstretchanimcontrol.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (52):
//   0x53be20  public: static void __cdecl UInterpTrackStretchAnimControl::InitializePrivateStaticClassUInterpTrackStretchAnimControl(void)
//   0x53be40  public: static void __cdecl UAnimNotify_SoireeAccent::InitializePrivateStaticClassUAnimNotify_SoireeAccent(void)
//   0x53be60  public: static void __cdecl UAnimNotify_SoireeApex::InitializePrivateStaticClassUAnimNotify_SoireeApex(void)
//   0x53be80  public: static void __cdecl UAnimNotify_SoireeLoop::InitializePrivateStaticClassUAnimNotify_SoireeLoop(void)
//   0x53bea0  public: static void __cdecl UAnimNotify_SoireeEnd::InitializePrivateStaticClassUAnimNotify_SoireeEnd(void)
//   0x53bec0  public: static void __cdecl UAnimNotify_SoireeIn::InitializePrivateStaticClassUAnimNotify_SoireeIn(void)
//   0x53bee0  public: static void __cdecl UAnimNotify_SoireeOut::InitializePrivateStaticClassUAnimNotify_SoireeOut(void)
//   0x53bf20  public: static void __cdecl UInterpTrackInstStretchAnimControl::InitializePrivateStaticClassUInterpTrackInstStretchAnimControl(void)
//   0x53d110  public: static class UClass * __cdecl UInterpTrackStretchAnimKeyProperties::GetPrivateStaticClassUInterpTrackStretchAnimKeyProperties(wchar_t const *)
//   0x53d1a0  public: static class UClass * __cdecl UAnimNotify_SoireeAccent::GetPrivateStaticClassUAnimNotify_SoireeAccent(wchar_t const *)
//   0x53d230  public: static class UClass * __cdecl UAnimNotify_SoireeApex::GetPrivateStaticClassUAnimNotify_SoireeApex(wchar_t const *)
//   0x53d2c0  public: static class UClass * __cdecl UAnimNotify_SoireeLoop::GetPrivateStaticClassUAnimNotify_SoireeLoop(wchar_t const *)
//   0x53d350  public: static class UClass * __cdecl UAnimNotify_SoireeEnd::GetPrivateStaticClassUAnimNotify_SoireeEnd(wchar_t const *)
//   0x53d3e0  public: static class UClass * __cdecl UAnimNotify_SoireeIn::GetPrivateStaticClassUAnimNotify_SoireeIn(wchar_t const *)
//   0x53d470  public: static class UClass * __cdecl UAnimNotify_SoireeOut::GetPrivateStaticClassUAnimNotify_SoireeOut(wchar_t const *)
//   0x53d500  public: static class UClass * __cdecl UInterpTrackInstStretchAnimControl::GetPrivateStaticClassUInterpTrackInstStretchAnimControl(wchar_t const *)
//   0x540030  public: static void __cdecl UInterpTrackStretchAnimKeyProperties::InitializePrivateStaticClassUInterpTrackStretchAnimKeyProperties(void)
//   0x540050  public: static class UClass * __cdecl UAnimNotify_SoireeAccent::StaticClassNoInline(void)
//   0x540080  public: static class UClass * __cdecl UAnimNotify_SoireeApex::StaticClassNoInline(void)
//   0x5400b0  public: static class UClass * __cdecl UAnimNotify_SoireeLoop::StaticClassNoInline(void)
//   0x5400e0  public: static class UClass * __cdecl UAnimNotify_SoireeEnd::StaticClassNoInline(void)
//   0x540110  public: static class UClass * __cdecl UAnimNotify_SoireeIn::StaticClassNoInline(void)
//   0x540140  public: static class UClass * __cdecl UAnimNotify_SoireeOut::StaticClassNoInline(void)
//   0x540170  public: virtual void __thiscall UInterpTrackStretchAnimControl::GetTimeRange(float &, float &)const
//   0x540230  public: virtual int __thiscall UInterpTrackStretchAnimControl::DuplicateKeyframe(int, float)
//   0x5403a0  public: virtual float __thiscall UInterpTrackStretchAnimControl::GetKeyframeTime(int)const
//   0x540410  public: class UAnimSequence * __thiscall UInterpTrackStretchAnimControl::FindAnimSequenceFromName(class UInterpGroupInst const *, class FName)const
//   0x540620  public: class FName __thiscall UInterpTrackStretchAnimControl::GetAnimName(int)const
//   0x5406a0  public: float __thiscall UInterpTrackStretchAnimControl::GetAnimStartOffset(int)const
//   0x540710  public: unsigned int __thiscall UInterpTrackStretchAnimControl::GetAnimCollisionStatus(int)const
//   0x540780  public: enum EMatRootMotionMode __thiscall UInterpTrackStretchAnimControl::GetAnimRootMotionMode(int)const
//   0x5407f0  public: enum EMatMeshTranslationMode __thiscall UInterpTrackStretchAnimControl::GetAnimMeshTranslationMode(int)const
//   0x540860  protected: float __thiscall UInterpTrackInstStretchAnimControl::ConditionallyReversePosition(struct FStretchAnimControlTrackKey const &, class UAnimSequence const *, float)const
//   0x5408d0  public: static class UClass * __cdecl UInterpTrackInstStretchAnimControl::StaticClassNoInline(void)
//   0x543080  public: static class UClass * __cdecl UInterpTrackStretchAnimKeyProperties::StaticClassNoInline(void)
//   0x5430b0  public: enum ESoireeMarkers __thiscall UInterpTrackStretchAnimControl::GetMarkerFromNotify(class UAnimNotify const *)
//   0x543170  public: virtual void __thiscall UInterpTrackStretchAnimControl::RemoveKeyframe(int)
//   0x5431a0  public: virtual float __thiscall UInterpTrackStretchAnimControl::GetKeyframeLength(int)const
//   0x543300  public: virtual int __thiscall UInterpTrackStretchAnimControl::SetKeyframeTime(int, float, unsigned int)
//   0x5434c0  public: virtual unsigned int __thiscall UInterpTrackStretchAnimControl::GetClosestSnapPosition(float, class TArray<int, class FDefaultAllocator> &, float &)
//   0x543930  public: virtual float __thiscall UInterpTrackStretchAnimControl::GetTrackEndTime(void)const
//   0x543a00  protected: unsigned int __thiscall UInterpTrackInstStretchAnimControl::GetAnimForTime(float, int &, float &, float &, float &)const
//   0x543f10  public: float __thiscall UInterpTrackStretchAnimControl::GetWeightForTime(float)
//   0x543f40  public: virtual void __thiscall UInterpTrackInstStretchAnimControl::UpdateTrackInst(float, unsigned int)
//   0x5451f0  public: virtual void __thiscall UInterpTrackStretchAnimControl::PostLoad(void)
//   0x545390  public: virtual int __thiscall UInterpTrackStretchAnimControl::AddKeyframe(float, class UInterpTrackInst *, enum EInterpCurveMode)
//   0x5454f0  public: virtual class UObject * __thiscall UInterpTrackStretchAnimControl::GetKeyProperties(int)
//   0x549120  public: static class UClass * __cdecl UInterpTrackStretchAnimControl::GetPrivateStaticClassUInterpTrackStretchAnimControl(wchar_t const *)
//   0x549390  public: static class UClass * __cdecl UInterpTrackStretchAnimControl::StaticClassNoInline(void)
//   0x549690  public: int __thiscall UInterpTrackStretchAnimControl::CalcChannelIndex(void)
//   0x549ac0  public: virtual void __thiscall UInterpTrackInstStretchAnimControl::InitTrackInst(class UInterpTrack *)
//   0xb9ed70  _dynamic_initializer_for__s_StretchAnimDragOperationMatchingPoints__

/*-----------------------------------------------------------------------------
	DISHONORED(port): agent FS (PHASE17 package FS). The StretchAnimControl track: 267 of l_tower_p's
	694 tracks and on every one of its six player groups (agent FK's census). A key names an animation,
	a start/end offset and a play rate, and six "soirée offsets" that re-time the animation's
	SoireeAccent/Apex/Loop/End/In/Out notifies so a scene can stretch an animation to its beats; the
	instance maps the matinee's position onto the animation's time through those marks each frame and
	hands it to the actor's MAT_SetAnimPosition (APawn -> UArkAnimNodeStack). The editor side
	(AddKeyframe, DuplicateKeyframe, RemoveKeyframe, SetKeyframeTime, GetClosestSnapPosition,
	GetKeyProperties, GetKeyframeColor, the drag operation) is not ported.
-----------------------------------------------------------------------------*/

#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"      // USeqVar_Character, used by the matinee group instances
#include "EngineInterpolationClasses.h"
#include "EngineAnimClasses.h"       // UAnimSequence, UAnimSet, UAnimNotify

IMPLEMENT_CLASS(UInterpTrackStretchAnimControl);
IMPLEMENT_CLASS(UInterpTrackStretchAnimKeyProperties);
IMPLEMENT_CLASS(UInterpTrackInstStretchAnimControl);

/** -fkdiag (agent FK's switch) also prints what this track drives; read once. */
static UBOOL DisFSStretchDiagEnabled()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		Enabled = ( appStrfind( appCmdLine(), TEXT("-fkdiag") ) != NULL ) ? 1 : 0;
	}
	return Enabled != 0;
}

// DISHONORED(port): 2013 rva 0x504b10 (2012 0x5451f0, interptrackstretchanimcontrol.cpp:100): a key with a degenerate play rate is
// reset to 1 and a key without its properties object gets a transactional one
void UInterpTrackStretchAnimControl::PostLoad()
{
	Super::PostLoad();
	for( INT KeyIndex = 0; KeyIndex < AnimSeqs.Num(); KeyIndex++ )
	{
		FStretchAnimControlTrackKey& Key = AnimSeqs(KeyIndex);
		if( Key.AnimPlayRate < 0.001f )
		{
			Key.AnimPlayRate = 1.0f;
		}
		if( !Key.Properties )
		{
			Key.Properties = ConstructObject<UInterpTrackStretchAnimKeyProperties>( UInterpTrackStretchAnimKeyProperties::StaticClass(), this, NAME_None, RF_Transactional );
		}
	}
}

// 2012 rva 0x540170 (177 B)
void UInterpTrackStretchAnimControl::GetTimeRange( FLOAT& StartTime, FLOAT& EndTime ) const
{
	if( AnimSeqs.Num() == 0 )
	{
		StartTime = 0.f;
		EndTime = 0.f;
		return;
	}
	StartTime = AnimSeqs(0).StartTime;
	EndTime = AnimSeqs(AnimSeqs.Num() - 1).StartTime;
}

// 2012 rva 0x5403a0 (100 B)
FLOAT UInterpTrackStretchAnimControl::GetKeyframeTime( INT KeyIndex ) const
{
	if( KeyIndex < 0 || KeyIndex >= AnimSeqs.Num() )
	{
		return 0.f;
	}
	return AnimSeqs(KeyIndex).StartTime;
}

// 2012 rva 0x5431a0 (343 B): the animation's playable length at the key's rate
FLOAT UInterpTrackStretchAnimControl::GetKeyframeLength( INT KeyIndex ) const
{
	const FStretchAnimControlTrackKey& Key = AnimSeqs(KeyIndex);
	const UAnimSequence* Seq = FindAnimSequenceFromName( NULL, Key.AnimSeqName );
	if( Seq == NULL )
	{
		return 0.f;
	}
	return ( Seq->SequenceLength - ( Key.AnimEndOffset + Key.AnimStartOffset ) ) / Key.AnimPlayRate;
}

// 2012 rva 0x543930 (194 B)
FLOAT UInterpTrackStretchAnimControl::GetTrackEndTime() const
{
	if( AnimSeqs.Num() == 0 )
	{
		return 0.f;
	}
	const FStretchAnimControlTrackKey& Key = AnimSeqs(AnimSeqs.Num() - 1);
	const UAnimSequence* Seq = FindAnimSequenceFromName( NULL, Key.AnimSeqName );
	if( Seq == NULL )
	{
		return Key.StartTime;
	}
	const FLOAT Length = Max( ( Seq->SequenceLength - ( Key.AnimEndOffset + Key.AnimStartOffset ) ) / Key.AnimPlayRate, 0.01f );
	return Key.StartTime + Length;
}

// 2012 rva 0x540410 (527 B): the group's own anim sets, last added first; then in game the AI group
// instance's pawn anim sets (GroupAnimSetsPawn, filled by UInterpGroupInstAI::InitPawn), in the editor
// the group's. The instance may be NULL (the editor-side callers above pass none).
UAnimSequence* UInterpTrackStretchAnimControl::FindAnimSequenceFromName( const UInterpGroupInst* GroupInst, FName InName ) const
{
	if( InName == NAME_None )
	{
		return NULL;
	}
	const UInterpGroup* Group = CastChecked<UInterpGroup>( GetOuter() );
	for( INT SetIndex = Group->GroupAnimSets.Num() - 1; SetIndex >= 0; SetIndex-- )
	{
		UAnimSet* Set = Group->GroupAnimSets(SetIndex);
		if( Set != NULL )
		{
			UAnimSequence* Seq = Set->FindAnimSequence( InName );
			if( Seq != NULL )
			{
				return Seq;
			}
		}
	}
	if( GIsGame )
	{
		const UInterpGroupInstAI* AIInst = Cast<UInterpGroupInstAI>( const_cast<UInterpGroupInst*>( GroupInst ) );
		if( AIInst != NULL )
		{
			for( INT SetIndex = AIInst->GroupAnimSetsPawn.Num() - 1; SetIndex >= 0; SetIndex-- )
			{
				UAnimSet* Set = AIInst->GroupAnimSetsPawn(SetIndex);
				if( Set != NULL )
				{
					UAnimSequence* Seq = Set->FindAnimSequence( InName );
					if( Seq != NULL )
					{
						return Seq;
					}
				}
			}
		}
	}
	else
	{
		for( INT SetIndex = Group->GroupAnimSetsPawn.Num() - 1; SetIndex >= 0; SetIndex-- )
		{
			UAnimSet* Set = Group->GroupAnimSetsPawn(SetIndex);
			if( Set != NULL )
			{
				UAnimSequence* Seq = Set->FindAnimSequence( InName );
				if( Seq != NULL )
				{
					return Seq;
				}
			}
		}
	}
	return NULL;
}

// 2012 rvas 0x540620, 0x5406a0, 0x540710, 0x540780, 0x5407f0: key-array reads, -1 is "no key"
FName UInterpTrackStretchAnimControl::GetAnimName( INT KeyIndex ) const
{
	return KeyIndex < 0 ? FName(NAME_None) : AnimSeqs(KeyIndex).AnimSeqName;
}

FLOAT UInterpTrackStretchAnimControl::GetAnimStartOffset( INT KeyIndex ) const
{
	return KeyIndex < 0 ? 0.f : AnimSeqs(KeyIndex).AnimStartOffset;
}

UBOOL UInterpTrackStretchAnimControl::GetAnimCollisionStatus( INT KeyIndex ) const
{
	return KeyIndex < 0 ? FALSE : ( AnimSeqs(KeyIndex).Properties != NULL && AnimSeqs(KeyIndex).Properties->m_bDisableCollision );
}

BYTE UInterpTrackStretchAnimControl::GetAnimRootMotionMode( INT KeyIndex ) const
{
	return ( KeyIndex < 0 || AnimSeqs(KeyIndex).Properties == NULL ) ? (BYTE)MRMM_Disable : AnimSeqs(KeyIndex).Properties->m_MatineeRootMotionMode;
}

BYTE UInterpTrackStretchAnimControl::GetAnimMeshTranslationMode( INT KeyIndex ) const
{
	return ( KeyIndex < 0 || AnimSeqs(KeyIndex).Properties == NULL ) ? (BYTE)EMMTM_NoChange : AnimSeqs(KeyIndex).Properties->m_MeshTranslationMode;
}

// 2012 rva 0x5430b0 (189 B). Retail compares the notify's class against the six UAnimNotify_Soiree*
// classes; those are DishonoredGame shims here (DishonoredGameEngineShims.h), so the comparison is by
// class name, which is the same test on a cooked asset.
BYTE UInterpTrackStretchAnimControl::GetMarkerFromNotify( const UAnimNotify* Notify ) const
{
	static const FName MarkerClassNames[SMK_MAX] =
	{
		FName(TEXT("AnimNotify_SoireeAccent")),
		FName(TEXT("AnimNotify_SoireeApex")),
		FName(TEXT("AnimNotify_SoireeLoop")),
		FName(TEXT("AnimNotify_SoireeEnd")),
		FName(TEXT("AnimNotify_SoireeIn")),
		FName(TEXT("AnimNotify_SoireeOut")),
	};
	if( Notify == NULL )
	{
		return SMK_MAX;
	}
	const FName ClassName = Notify->GetClass()->GetFName();
	for( INT Marker = 0; Marker < SMK_MAX; Marker++ )
	{
		if( ClassName == MarkerClassNames[Marker] )
		{
			return (BYTE)Marker;
		}
	}
	return SMK_MAX;
}

// 2012 rva 0x543f10 (39 B)
FLOAT UInterpTrackStretchAnimControl::GetWeightForTime( FLOAT InTime )
{
	return FloatTrack.Eval( InTime, 0.f );
}

// 2012 rva 0x549690 (1070 B): this track's channel on its slot is the number of enabled AnimControl and
// StretchAnimControl tracks with the same slot name that come before it in the group.
INT UInterpTrackStretchAnimControl::CalcChannelIndex()
{
	const UInterpGroup* Group = CastChecked<UInterpGroup>( GetOuter() );
	INT ChannelIndex = 0;
	for( INT TrackIndex = 0; TrackIndex < Group->InterpTracks.Num(); TrackIndex++ )
	{
		UInterpTrack* Track = Group->InterpTracks(TrackIndex);
		if( Track == this )
		{
			return ChannelIndex;
		}
		const UInterpTrackAnimControl* AnimTrack = Cast<UInterpTrackAnimControl>( Track );
		const UInterpTrackStretchAnimControl* StretchTrack = Cast<UInterpTrackStretchAnimControl>( Track );
		if( ( AnimTrack != NULL && !AnimTrack->IsDisabled() && AnimTrack->SlotName == SlotName )
			|| ( StretchTrack != NULL && !StretchTrack->IsDisabled() && StretchTrack->SlotName == SlotName ) )
		{
			ChannelIndex++;
		}
	}
	return 0;
}

/*-----------------------------------------------------------------------------
	UInterpTrackInstStretchAnimControl
-----------------------------------------------------------------------------*/

// 2012 rva 0x549ac0 (71 B). Retail does not chain to UInterpTrackInst::InitTrackInst (the post-process
// backup for camera actors); it stores the track and resets the loop state.
void UInterpTrackInstStretchAnimControl::InitTrackInst( UInterpTrack* InTrack )
{
	Track = InTrack;
	const USeqAct_Interp* Matinee = GetMatinee();
	LastUpdatePosition = ( Matinee != NULL ) ? Matinee->Position : 0.f;
	bAnimIsLooping = 0;
	OffsetForLoopWhileSyncing = 0.f;
	LoopStart = 0.f;
	LoopEnd = 0.f;
	m_iPreviousKeyIndex = INDEX_NONE;
	m_ChannelIndex = CastChecked<UInterpTrackStretchAnimControl>( Track )->CalcChannelIndex();
}

// 2012 rva 0x540860 (112 B)
FLOAT UInterpTrackInstStretchAnimControl::ConditionallyReversePosition( const FStretchAnimControlTrackKey& SeqKey, const UAnimSequence* Seq, FLOAT InPosition ) const
{
	if( !SeqKey.bReverse )
	{
		return InPosition;
	}
	if( Seq == NULL )
	{
		Seq = CastChecked<UInterpTrackStretchAnimControl>( Track )->FindAnimSequenceFromName( Cast<UInterpGroupInst>( GetOuter() ), SeqKey.AnimSeqName );
		if( Seq == NULL )
		{
			return InPosition;
		}
	}
	const FLOAT PlayLength = Seq->SequenceLength - ( SeqKey.AnimEndOffset + SeqKey.AnimStartOffset );
	return ( PlayLength - ( InPosition - SeqKey.AnimStartOffset ) ) + SeqKey.AnimStartOffset;
}

// 2012 rva 0x543a00 (1285 B). Maps a matinee time onto the current key's animation time.
//
// The animation's SoireeAccent/Apex/Loop/End/In/Out notifies are the "marks"; the key's SoireeOffsets[mark]
// moves each mark in the stretched time line. The animation is played piecewise-linearly so that each
// mark falls on its moved time, and between a SoireeLoop mark and the SoireeEnd mark it loops a whole
// number of times (the loop count is rounded from the stretched segment over the original one). Returns
// TRUE when InTime is before the first key (the first key's start pose is held), FALSE otherwise; the
// key index is left alone before the first key and set to -1 once the animation has been played out.
UBOOL UInterpTrackInstStretchAnimControl::GetAnimForTime( FLOAT InTime, INT& OutKeyIndex, FLOAT& OutPosition, FLOAT& OutLoopStartPosition, FLOAT& OutLoopDuration ) const
{
	OutPosition = 0.f;
	OutLoopDuration = 0.f;
	OutLoopStartPosition = 0.f;

	const UInterpTrackStretchAnimControl* StretchTrack = CastChecked<UInterpTrackStretchAnimControl>( Track );
	if( StretchTrack->AnimSeqs.Num() == 0 )
	{
		return FALSE;
	}
	const UInterpGroupInst* GroupInst = Cast<UInterpGroupInst>( GetOuter() );

	if( StretchTrack->AnimSeqs(0).StartTime > InTime )
	{
		OutPosition = 0.f;
		const FStretchAnimControlTrackKey& FirstKey = StretchTrack->AnimSeqs(0);
		if( FirstKey.bReverse )
		{
			const UAnimSequence* FirstSeq = StretchTrack->FindAnimSequenceFromName( GroupInst, FirstKey.AnimSeqName );
			if( FirstSeq != NULL )
			{
				OutPosition = ConditionallyReversePosition( FirstKey, FirstSeq, OutPosition );
			}
		}
		return TRUE;
	}

	INT KeyIndex = 0;
	while( KeyIndex < StretchTrack->AnimSeqs.Num() - 1 && InTime >= StretchTrack->AnimSeqs(KeyIndex + 1).StartTime )
	{
		KeyIndex++;
	}
	OutKeyIndex = KeyIndex;
	const FStretchAnimControlTrackKey& Key = StretchTrack->AnimSeqs(KeyIndex);

	const UAnimSequence* Seq = StretchTrack->FindAnimSequenceFromName( GroupInst, Key.AnimSeqName );
	if( Seq == NULL )
	{
		return FALSE;
	}

	const FLOAT SeqEnd = ( Seq->SequenceLength - Key.AnimEndOffset ) + 0.0001f;
	FLOAT LocalInTime = ( InTime - Key.StartTime ) * Key.AnimPlayRate + Key.AnimStartOffset;
	if( LocalInTime < Key.AnimStartOffset )
	{
		LocalInTime = Key.AnimStartOffset;
	}
	else if( LocalInTime >= SeqEnd )
	{
		LocalInTime = SeqEnd;
	}
	if( LocalInTime == SeqEnd )
	{
		// the animation has been played out: the caller keeps the last pose and stops re-sending the key
		OutKeyIndex = INDEX_NONE;
		OutPosition = 0.f;
	}

	// the previous mark, in the animation's own time and in the stretched time
	FLOAT PreviousNotifyTime = 0.f;
	FLOAT PreviousNotifyModifiedTime = Key.AnimStartOffset;
	if( Key.bReverse )
	{
		PreviousNotifyModifiedTime = Key.AnimEndOffset;
		LocalInTime = Seq->SequenceLength - LocalInTime;
	}

	for( INT NotifyIndex = 0; NotifyIndex < Seq->Notifies.Num(); NotifyIndex++ )
	{
		const FAnimNotifyEvent& Notify = Seq->Notifies(NotifyIndex);
		if( Notify.Notify == NULL )
		{
			continue;
		}
		const BYTE Marker = StretchTrack->GetMarkerFromNotify( Notify.Notify );
		if( Marker == SMK_MAX )
		{
			continue;
		}
		const FLOAT NotifyTime = Notify.Time;
		const FLOAT NotifyModifiedTime = Key.SoireeOffsets[Marker] + Notify.Time;
		if( NotifyModifiedTime > LocalInTime )
		{
			if( Marker == SMK_End )
			{
				// loop the segment before the End mark a whole number of times
				const FLOAT SegmentLength = NotifyTime - PreviousNotifyTime;
				FLOAT LoopCount = ( NotifyModifiedTime - PreviousNotifyModifiedTime ) / SegmentLength + 0.5f;
				if( LoopCount <= 1.f )
				{
					LoopCount = 1.f;
				}
				const FLOAT LoopDuration = ( NotifyModifiedTime - PreviousNotifyModifiedTime ) / (FLOAT)(INT)LoopCount;
				const FLOAT SegmentPlayRate = SegmentLength / LoopDuration;
				if( !Key.bReverse )
				{
					OutLoopStartPosition = ( PreviousNotifyModifiedTime - Key.AnimStartOffset ) / Key.AnimPlayRate + Key.StartTime;
				}
				else
				{
					OutLoopStartPosition = ( ( Seq->SequenceLength - NotifyModifiedTime ) - Key.AnimStartOffset ) / Key.AnimPlayRate + Key.StartTime;
				}
				OutLoopDuration = LoopDuration / Key.AnimPlayRate;
				OutLoopStartPosition = (FLOAT)(INT)( ( InTime - OutLoopStartPosition ) / OutLoopDuration ) * OutLoopDuration + OutLoopStartPosition;
				OutPosition = appFmod( LocalInTime - PreviousNotifyModifiedTime, LoopDuration ) * SegmentPlayRate + PreviousNotifyTime;
			}
			else
			{
				OutPosition = ( ( NotifyTime - PreviousNotifyTime ) / ( NotifyModifiedTime - PreviousNotifyModifiedTime ) ) * ( LocalInTime - PreviousNotifyModifiedTime ) + PreviousNotifyTime;
			}
			return FALSE;
		}
		PreviousNotifyTime = NotifyTime;
		PreviousNotifyModifiedTime = NotifyModifiedTime;
	}

	// past the last mark: the rest of the animation is stretched over the rest of the key
	FLOAT Ratio = 1.f;
	const FLOAT RemainingModified = ( Seq->SequenceLength - Key.AnimEndOffset ) - PreviousNotifyModifiedTime;
	if( RemainingModified != 0.f )
	{
		Ratio = ( Seq->SequenceLength - PreviousNotifyTime ) / RemainingModified;
	}
	OutPosition = ( LocalInTime - PreviousNotifyModifiedTime ) * Ratio + PreviousNotifyTime;
	return FALSE;
}

// 2012 rva 0x543f40 (512 B). Called from the group's pass loop (UInterpGroup::UpdateGroup) on the
// track's TrackUpdatePass. A matinee that is holding its position (a SoireeControl pause, a dialog
// wait) keeps the loop segment playing through OffsetForLoopWhileSyncing; a matinee that moves maps
// its position through GetAnimForTime. The key's collision / root-motion / mesh-translation choices
// are collected on the group instance for UInterpGroupAI::UpdateGroup to hand to the pawn after all
// tracks have run, and the animation position goes to the actor every frame.
void UInterpTrackInstStretchAnimControl::UpdateTrackInst( FLOAT NewPosition, UBOOL bJump )
{
	AActor* Actor = GetGroupActor();
	if( Actor == NULL )
	{
		return;
	}
	UInterpTrackStretchAnimControl* StretchTrack = CastChecked<UInterpTrackStretchAnimControl>( Track );
	UInterpGroupInst* GroupInst = CastChecked<UInterpGroupInst>( GetOuter() );
	const USeqAct_Interp* Matinee = GetMatinee();
	const INT ChannelIndex = m_ChannelIndex;

	INT KeyIndex = INDEX_NONE;
	FLOAT NewAnimPosition = 0.f;
	FLOAT LoopStartPosition = 0.f;
	FLOAT LoopDuration = 0.f;
	if( NewPosition == LastUpdatePosition )
	{
		FLOAT InTime = NewPosition;
		if( bAnimIsLooping )
		{
			OffsetForLoopWhileSyncing += ( Matinee != NULL ) ? Matinee->DeltaTime : 0.f;
			if( OffsetForLoopWhileSyncing > LoopEnd - NewPosition )
			{
				OffsetForLoopWhileSyncing = LoopStart - NewPosition;
			}
			InTime = NewPosition + OffsetForLoopWhileSyncing;
		}
		GetAnimForTime( InTime, KeyIndex, NewAnimPosition, LoopStartPosition, LoopDuration );
	}
	else
	{
		const FLOAT InTime = OffsetForLoopWhileSyncing + NewPosition;
		LastUpdatePosition = NewPosition;
		GetAnimForTime( InTime, KeyIndex, NewAnimPosition, LoopStartPosition, LoopDuration );
		if( LoopDuration <= 0.f )
		{
			bAnimIsLooping = 0;
		}
		else
		{
			bAnimIsLooping = 1;
			LoopStart = LoopStartPosition;
			LoopEnd = LoopStartPosition + LoopDuration;
		}
	}

	const FName NewAnimSeqName = StretchTrack->GetAnimName( KeyIndex );
	const UBOOL bDisableCollision = StretchTrack->GetAnimCollisionStatus( KeyIndex );
	const BYTE RootMotionMode = StretchTrack->GetAnimRootMotionMode( KeyIndex );
	const BYTE MeshTranslationMode = StretchTrack->GetAnimMeshTranslationMode( KeyIndex );
	const UBOOL bEnableRootMotion = RootMotionMode != MRMM_Disable;

	if( KeyIndex == INDEX_NONE )
	{
		OffsetForLoopWhileSyncing = 0.f;
	}
	else if( m_iPreviousKeyIndex != KeyIndex )
	{
		// a new key: start its animation at its start offset before setting the real position
		const FLOAT AnimStartOffset = StretchTrack->GetAnimStartOffset( KeyIndex );
		Actor->MAT_SetAnimPosition( StretchTrack->SlotName, ChannelIndex, NewAnimSeqName, AnimStartOffset, TRUE, FALSE, bEnableRootMotion );
		m_iPreviousKeyIndex = KeyIndex;
		if( DisFSStretchDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): fkdiag stretch %s group '%s' slot %s ch %d: key %d anim %s start %.2f rate %.2f rootMotion %d meshTrans %d noCollision %d on %s, matinee pos %.2f"),
				( Matinee != NULL ) ? *Matinee->GetName() : TEXT("?"), *GroupInst->Group->GroupName.ToString(), *StretchTrack->SlotName.ToString(), ChannelIndex,
				KeyIndex, *NewAnimSeqName.ToString(), AnimStartOffset, StretchTrack->AnimSeqs(KeyIndex).AnimPlayRate,
				(INT)RootMotionMode, (INT)MeshTranslationMode, (INT)bDisableCollision, *Actor->GetName(), NewPosition );
		}
	}

	GroupInst->SetActorCollision( bDisableCollision );
	GroupInst->SetActorRootMotion( RootMotionMode );
	GroupInst->SetMeshTranslationMode( MeshTranslationMode );
	Actor->MAT_SetAnimPosition( StretchTrack->SlotName, ChannelIndex, NewAnimSeqName, NewAnimPosition, TRUE, FALSE, bEnableRootMotion );
	m_fCurrentAnimPosition = NewAnimPosition;

	if( DisFSStretchDiagEnabled() && KeyIndex != INDEX_NONE )
	{
		// once a second per instance: which animation, where in it, and the weight the curve gives it.
		// The timestamp lives outside the object: the instance's layout is retail's.
		static TMap<const UObject*, FLOAT> LastReportAt;
		const FLOAT Now = GWorld->GetTimeSeconds();
		FLOAT* Last = LastReportAt.Find( this );
		if( Last == NULL || Now - *Last >= 1.f )
		{
			LastReportAt.Set( this, Now );
			debugf( TEXT("DISHONORED(bringup): fkdiag stretch %s group '%s' slot %s ch %d: anim %s time %.2f weight %.2f looping %d, matinee pos %.2f, %s at %.0f/%.0f/%.0f"),
				( Matinee != NULL ) ? *Matinee->GetName() : TEXT("?"), *GroupInst->Group->GroupName.ToString(), *StretchTrack->SlotName.ToString(), ChannelIndex,
				*NewAnimSeqName.ToString(), NewAnimPosition, StretchTrack->GetWeightForTime( NewPosition ), (INT)bAnimIsLooping, NewPosition,
				*Actor->GetName(), Actor->Location.X, Actor->Location.Y, Actor->Location.Z );
		}
	}
}
