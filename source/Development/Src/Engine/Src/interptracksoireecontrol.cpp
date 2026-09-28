// Engine/src/interptracksoireecontrol.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (12):
//   0x53be00  public: static void __cdecl UInterpTrackSoireeControl::InitializePrivateStaticClassUInterpTrackSoireeControl(void)
//   0x53d080  public: static class UClass * __cdecl UInterpTrackSoireeControlKeyProperties::GetPrivateStaticClassUInterpTrackSoireeControlKeyProperties(wchar_t const *)
//   0x53fbd0  public: static void __cdecl UInterpTrackSoireeControlKeyProperties::InitializePrivateStaticClassUInterpTrackSoireeControlKeyProperties(void)
//   0x53fbf0  public: virtual float __thiscall UInterpTrackSoireeControl::GetKeyframeLength(int)const
//   0x53fca0  public: virtual float __thiscall UInterpTrackSoireeControl::GetTrackEndTime(void)const
//   0x53fd10  public: virtual unsigned int __thiscall UInterpTrackInstSoireeControl::NeedsSynchronizing(float, float, float &, unsigned int)
//   0x542e90  public: static class UClass * __cdecl UInterpTrackSoireeControlKeyProperties::StaticClassNoInline(void)
//   0x542ec0  public: virtual unsigned int __thiscall UInterpTrackSoireeControl::GetClosestSnapPosition(float, class TArray<int, class FDefaultAllocator> &, float &)
//   0x544f90  public: virtual class UObject * __thiscall UInterpTrackSoireeControl::GetKeyProperties(int)
//   0x5450c0  public: virtual int __thiscall UInterpTrackSoireeControl::AddKeyframe(float, class UInterpTrackInst *, enum EInterpCurveMode)
//   0x549090  public: static class UClass * __cdecl UInterpTrackSoireeControl::GetPrivateStaticClassUInterpTrackSoireeControl(wchar_t const *)
//   0x549360  public: static class UClass * __cdecl UInterpTrackSoireeControl::StaticClassNoInline(void)

#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"

// DISHONORED(port): 2013 rva 0x500920 (2012 0x53fbf0). A Pause key occupies a point in time, not a
// span, so it has no length; a Loop key's length is the segment the matinee is rewound over.
FLOAT UInterpTrackSoireeControl::GetKeyframeLength( INT KeyIndex ) const
{
	if( KeyIndex < 0 || KeyIndex >= SoireeControlKeys.Num() )
	{
		return 0.f;
	}
	if( SoireeControlKeys( KeyIndex ).Properties->m_SoireeControlType == ESCT_Pause )
	{
		return 0.f;
	}
	return SoireeControlKeys( KeyIndex ).KeyLength;
}

// DISHONORED(port): 2013 rva 0x500870 (2012 0x2340c0)
void UInterpTrackSoireeControl::GetTimeRange( FLOAT& StartTime, FLOAT& EndTime ) const
{
	if( SoireeControlKeys.Num() == 0 )
	{
		StartTime = 0.f;
		EndTime = 0.f;
		return;
	}
	StartTime = SoireeControlKeys( 0 ).StartTime;
	EndTime = SoireeControlKeys( SoireeControlKeys.Num() - 1 ).StartTime;
}

// DISHONORED(port): 2013 rva 0x500a40 (2012 0x53fd10). Two jobs in one virtual, and the first is what
// releases a loop: every key's status flag is refreshed from the matinee's ActivatedLinks, the sticky
// record UpdateOp keeps of which input links have been impulsed. The second answers the question the
// virtual is named for - a Pause key holds the matinee at the key's time until the pin is impulsed
// (or, in preview, for the key's preview duration), and a broken Loop key whose break mode is
// immediate jumps past the segment and hands the matinee the key's own blend-out time.
UBOOL UInterpTrackInstSoireeControl::NeedsSynchronizing( FLOAT CurPosition, FLOAT NewPosition, FLOAT& OutPosition, UBOOL bPreview )
{
	UInterpTrackSoireeControl* SoireeTrack = CastChecked<UInterpTrackSoireeControl>( Track );
	USeqAct_Interp* Matinee = GetMatinee();

	for( INT KeyIndex = 0; KeyIndex < m_lKeysStatus.Num(); KeyIndex++ )
	{
		FSoireeControlKeyStatus& Status = m_lKeysStatus( KeyIndex );
		if( Status.m_InputIndex == INDEX_NONE || Matinee == NULL
			|| Status.m_InputIndex >= Matinee->ActivatedLinks.Num() )
		{
			continue;
		}
		Status.m_bIsBroken = Matinee->ActivatedLinks( Status.m_InputIndex ) != 0;
	}

	for( INT KeyIndex = 0; KeyIndex < SoireeTrack->SoireeControlKeys.Num(); KeyIndex++ )
	{
		const FSoireeControlTrackKey& Key = SoireeTrack->SoireeControlKeys( KeyIndex );
		if( Key.StartTime + SoireeTrack->GetKeyframeLength( KeyIndex ) < CurPosition || NewPosition <= Key.StartTime )
		{
			continue;
		}

		UInterpTrackSoireeControlKeyProperties* Properties = Key.Properties;
		if( Properties->m_SoireeControlType != ESCT_Loop )
		{
			if( Properties->m_SoireeControlType == ESCT_Pause && !m_lKeysStatus( KeyIndex ).m_bIsBroken )
			{
				const FLOAT MaxDuration = bPreview ? Properties->m_PreviewPauseDuration : BIG_NUMBER;
				m_fCurrentPauseDuration += NewPosition - CurPosition;
				if( m_fCurrentPauseDuration <= MaxDuration )
				{
					OutPosition = Key.StartTime;
					return TRUE;
				}
				m_fCurrentPauseDuration = 0.f;
			}
			return FALSE;
		}

		if( NewPosition > Key.StartTime + Key.KeyLength
			|| !m_lKeysStatus( KeyIndex ).m_bIsBroken
			|| Properties->m_BreakMode != ESBM_BreakImmediately )
		{
			return FALSE;
		}
		OutPosition = Key.StartTime + Key.KeyLength + 0.0001f;
		if( Matinee != NULL )
		{
			Matinee->BlendOutTimeOverride = Properties->m_BreakImmediatelyBlendOut;
		}
		return TRUE;
	}
	return FALSE;
}
