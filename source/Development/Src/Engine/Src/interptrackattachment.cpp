// Engine/src/interptrackattachment.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (16):
//   0x53b9f0  public: static void __cdecl UInterpTrackAttachment::InitializePrivateStaticClassUInterpTrackAttachment(void)
//   0x53ca60  public: static class UClass * __cdecl UInterpTrackAttachmentKeyProperties::GetPrivateStaticClassUInterpTrackAttachmentKeyProperties(wchar_t const *)
//   0x53e2a0  public: virtual void __thiscall UInterpTrackAttachment::GetTimeRange(float &, float &)const
//   0x53e350  public: virtual float __thiscall UInterpTrackAttachment::GetTrackEndTime(void)const
//   0x53e3c0  public: virtual float __thiscall UInterpTrackAttachment::GetKeyframeTime(int)const
//   0x53e420  public: virtual int __thiscall UInterpTrackAttachment::DuplicateKeyframe(int, float)
//   0x53e560  public: virtual void __thiscall UInterpTrackAttachment::UpdateTrack(float, class UInterpTrackInst *, unsigned int)
//   0x541af0  public: static void __cdecl UInterpTrackAttachmentKeyProperties::InitializePrivateStaticClassUInterpTrackAttachmentKeyProperties(void)
//   0x541b10  public: virtual int __thiscall UInterpTrackAttachment::SetKeyframeTime(int, float, unsigned int)
//   0x541ca0  public: virtual void __thiscall UInterpTrackAttachment::RemoveKeyframe(int)
//   0x541cd0  public: virtual unsigned int __thiscall UInterpTrackAttachment::GetClosestSnapPosition(float, class TArray<int, class FDefaultAllocator> &, float &)
//   0x544770  public: static class UClass * __cdecl UInterpTrackAttachmentKeyProperties::StaticClassNoInline(void)
//   0x546cb0  public: virtual class UObject * __thiscall UInterpTrackAttachment::GetKeyProperties(int)
//   0x546de0  public: virtual int __thiscall UInterpTrackAttachment::AddKeyframe(float, class UInterpTrackInst *, enum EInterpCurveMode)
//   0x548e50  public: static class UClass * __cdecl UInterpTrackAttachment::GetPrivateStaticClassUInterpTrackAttachment(wchar_t const *)
//   0x5492a0  public: static class UClass * __cdecl UInterpTrackAttachment::StaticClassNoInline(void)
//
// DISHONORED(port): agent GB (PHASE19 GB). The matinee Attachment track: each key names, through its
// UInterpTrackAttachmentKeyProperties, an attach (m_KeyType 0: the TARGET group's actor is attached to this group's actor
// through AActor::MAT_Attach -> DoKismetAttachment with the key's FAttachmentInfos) or a detach (1: MAT_Detach, then
// MAT_SetPhysics( TRUE, the group instance's BackupCollisionType ) when the target group is bBecomeDynamic). The Boat ride's
// 'Boat' group carries four of them: the player to bone 'Player', the guards to 'Captain' / 'Driver', the Boat_Mover
// InterpActor to socket 'Boat_Socket', all attached at the first key and the pawns detached at the last. 2012 bodies
// decompiled in build/agentGB/decomp2012/attachment_2012.c; the 2013 UpdateTrack (0x8ff2b0, vtable slot 79, unnamed) is
// the same loop plus the 2013-only transform backup (sub_61FCB0, FS's m_bShouldBackupTransform) after each attach/detach,
// not ported. DuplicateKeyframe, SetKeyframeTime and GetClosestSnapPosition are editor paths written in the shape of
// UInterpTrackEvent's, not from their decompiles.
#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineAnimClasses.h"

IMPLEMENT_CLASS(UInterpTrackAttachment);
IMPLEMENT_CLASS(UInterpTrackAttachmentKeyProperties);

INT UInterpTrackAttachment::AddKeyframe(FLOAT Time, UInterpTrackInst* TrInst, EInterpCurveMode InitInterpMode)
{
	UInterpTrackAttachmentKeyProperties* NewProps = ConstructObject<UInterpTrackAttachmentKeyProperties>( UInterpTrackAttachmentKeyProperties::StaticClass(), this, NAME_None, RF_Transactional );
	INT KeyIndex = 0;
	while( KeyIndex < m_AttachmentKeys.Num() && Time > m_AttachmentKeys(KeyIndex).Time )
	{
		KeyIndex++;
	}
	m_AttachmentKeys.InsertZeroed( KeyIndex );
	m_AttachmentKeys(KeyIndex).Time = Time;
	m_AttachmentKeys(KeyIndex).Properties = NewProps;
	return KeyIndex;
}

INT UInterpTrackAttachment::DuplicateKeyframe(INT KeyIndex, FLOAT NewKeyTime)
{
	if( KeyIndex < 0 || KeyIndex >= m_AttachmentKeys.Num() )
	{
		return INDEX_NONE;
	}
	FAttachmentControlTrackKey NewKey = m_AttachmentKeys(KeyIndex);
	NewKey.Time = NewKeyTime;
	INT NewKeyIndex = 0;
	while( NewKeyIndex < m_AttachmentKeys.Num() && NewKeyTime > m_AttachmentKeys(NewKeyIndex).Time )
	{
		NewKeyIndex++;
	}
	m_AttachmentKeys.InsertItem( NewKey, NewKeyIndex );
	return NewKeyIndex;
}

INT UInterpTrackAttachment::SetKeyframeTime(INT KeyIndex, FLOAT NewKeyTime, UBOOL bUpdateOrder)
{
	if( KeyIndex < 0 || KeyIndex >= m_AttachmentKeys.Num() )
	{
		return KeyIndex;
	}
	if( !bUpdateOrder )
	{
		m_AttachmentKeys(KeyIndex).Time = NewKeyTime;
		return KeyIndex;
	}
	FAttachmentControlTrackKey MovedKey = m_AttachmentKeys(KeyIndex);
	MovedKey.Time = NewKeyTime;
	m_AttachmentKeys.Remove( KeyIndex );
	INT NewKeyIndex = 0;
	while( NewKeyIndex < m_AttachmentKeys.Num() && NewKeyTime > m_AttachmentKeys(NewKeyIndex).Time )
	{
		NewKeyIndex++;
	}
	m_AttachmentKeys.InsertItem( MovedKey, NewKeyIndex );
	return NewKeyIndex;
}

void UInterpTrackAttachment::RemoveKeyframe(INT KeyIndex)
{
	m_AttachmentKeys.Remove( KeyIndex );
}

FLOAT UInterpTrackAttachment::GetKeyframeTime(INT KeyIndex) const
{
	if( KeyIndex < 0 || KeyIndex >= m_AttachmentKeys.Num() )
	{
		return 0.f;
	}
	return m_AttachmentKeys(KeyIndex).Time;
}

void UInterpTrackAttachment::GetTimeRange(FLOAT& StartTime, FLOAT& EndTime) const
{
	if( m_AttachmentKeys.Num() == 0 )
	{
		StartTime = 0.f;
		EndTime = 0.f;
		return;
	}
	StartTime = m_AttachmentKeys(0).Time;
	EndTime = m_AttachmentKeys( m_AttachmentKeys.Num() - 1 ).Time;
}

FLOAT UInterpTrackAttachment::GetTrackEndTime() const
{
	return m_AttachmentKeys.Num() ? m_AttachmentKeys( m_AttachmentKeys.Num() - 1 ).Time : 0.f;
}

UBOOL UInterpTrackAttachment::GetClosestSnapPosition(FLOAT InPosition, TArray<INT> &IgnoreKeys, FLOAT& OutPosition)
{
	UBOOL bFoundSnap = FALSE;
	FLOAT ClosestSnap = 0.f;
	FLOAT ClosestDist = BIG_NUMBER;
	for( INT KeyIndex = 0; KeyIndex < m_AttachmentKeys.Num(); KeyIndex++ )
	{
		if( !IgnoreKeys.ContainsItem( KeyIndex ) )
		{
			const FLOAT Dist = Abs( m_AttachmentKeys(KeyIndex).Time - InPosition );
			if( Dist < ClosestDist )
			{
				ClosestSnap = m_AttachmentKeys(KeyIndex).Time;
				ClosestDist = Dist;
				bFoundSnap = TRUE;
			}
		}
	}
	OutPosition = ClosestSnap;
	return bFoundSnap;
}

UObject* UInterpTrackAttachment::GetKeyProperties(INT KeyIndex)
{
	if( m_AttachmentKeys(KeyIndex).Properties == NULL )
	{
		m_AttachmentKeys(KeyIndex).Properties = ConstructObject<UInterpTrackAttachmentKeyProperties>( UInterpTrackAttachmentKeyProperties::StaticClass(), this, NAME_None, RF_Transactional );
	}
	return m_AttachmentKeys(KeyIndex).Properties;
}

static void DisApplyAttachmentKey( UInterpTrackAttachmentKeyProperties* Props, AActor* GroupActor, AActor* Target, UInterpGroupInst* TargetInst )
{
	if( Props == NULL )
	{
		return;
	}
	if( Props->m_KeyType == EKAT_Attach )
	{
		Target->MAT_Attach( GroupActor, Props->m_AttachmentInfo );
	}
	else if( Props->m_KeyType == EKAT_Detach )
	{
		Target->MAT_Detach();
		if( TargetInst->Group != NULL && TargetInst->Group->bBecomeDynamic )
		{
			Target->MAT_SetPhysics( TRUE, TargetInst->BackupCollisionType );
		}
	}
}

void UInterpTrackAttachment::UpdateTrack(FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump)
{
	AActor* GroupActor = TrInst->GetGroupActor();
	if( GroupActor == NULL )
	{
		return;
	}
	UInterpTrackInstAttachment* AttachInst = CastChecked<UInterpTrackInstAttachment>( TrInst );
	UInterpGroupInst* TargetInst = TrInst->GetMatinee()->FindFirstGroupInstByName( m_TargetName );
	if( TargetInst == NULL )
	{
		return;
	}
	AActor* Target = TargetInst->GetGroupActor();
	if( Target == NULL )
	{
		return;
	}
	const FLOAT LastPosition = AttachInst->m_LastUpdatePosition;
	if( LastPosition > NewPosition )
	{
		// playing backwards: every key crossed on the way back re-applies the key before it, the first key undoes itself
		for( INT KeyIndex = m_AttachmentKeys.Num() - 1; KeyIndex >= 0; KeyIndex-- )
		{
			const FLOAT KeyTime = m_AttachmentKeys(KeyIndex).Time;
			if( LastPosition >= KeyTime && KeyTime > NewPosition )
			{
				if( KeyIndex == 0 )
				{
					Target->MAT_Detach();
				}
				else
				{
					DisApplyAttachmentKey( m_AttachmentKeys(KeyIndex - 1).Properties, GroupActor, Target, TargetInst );
				}
			}
		}
	}
	else
	{
		for( INT KeyIndex = 0; KeyIndex < m_AttachmentKeys.Num(); KeyIndex++ )
		{
			const FLOAT KeyTime = m_AttachmentKeys(KeyIndex).Time;
			if( ( KeyTime > LastPosition && NewPosition >= KeyTime ) || ( KeyTime == 0.f && LastPosition == 0.f && NewPosition == 0.f ) )
			{
				DisApplyAttachmentKey( m_AttachmentKeys(KeyIndex).Properties, GroupActor, Target, TargetInst );
			}
		}
	}
	AttachInst->m_LastUpdatePosition = NewPosition;
}
