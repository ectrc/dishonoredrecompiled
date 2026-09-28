// Engine/src/interptrackinstsoireecontrol.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (7):
//   0x53bd10  public: static void __cdecl UInterpTrackInstSoireeControl::InitializePrivateStaticClassUInterpTrackInstSoireeControl(void)
//   0x53bd30  public: virtual void __thiscall UInterpTrackInstSoireeControl::SaveData(class FArchive &)
//   0x53f830  public: unsigned int __thiscall UInterpTrackInstSoireeControl::SoireeShouldLoop(float, float, unsigned int, float &, float &, class FName &, int &)
//   0x53f990  public: unsigned int __thiscall UInterpTrackInstSoireeControl::SoireeStartLoop(float, float, float &)
//   0x544ac0  public: virtual void __thiscall UInterpTrackInstSoireeControl::InitTrackInst(class UInterpTrack *)
//   0x546f80  public: static class UClass * __cdecl UInterpTrackInstSoireeControl::GetPrivateStaticClassUInterpTrackInstSoireeControl(wchar_t const *)
//   0x548270  public: static class UClass * __cdecl UInterpTrackInstSoireeControl::StaticClassNoInline(void)

#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"

IMPLEMENT_CLASS(UInterpTrackSoireeControl);
IMPLEMENT_CLASS(UInterpTrackSoireeControlKeyProperties);
IMPLEMENT_CLASS(UInterpTrackInstSoireeControl);

// DISHONORED(port): the matinee's Kismet input link for a SoireeControl key is named after the key's
// pin with this in front of it - key pin 'Loop' is input link 'SCT_Loop'. 2013: a file-scope FString
// initialised from the literal at 0xb825a0, read at 0x506c83.
static const FString GSoireeControlPinPrefix( TEXT("SCT_") );

// DISHONORED(port): 2013 rva 0x215100 (2012 0x22d220). The Outer of a track instance is its group
// instance, whose Outer is the matinee.
USeqAct_Interp* UInterpTrackInst::GetMatinee() const
{
	UObject* GroupInst = GetOuter();
	return GroupInst != NULL ? Cast<USeqAct_Interp>( GroupInst->GetOuter() ) : NULL;
}

// DISHONORED(port): 2013 rva 0x506b30 (2012 0x544ac0). One status entry per SoireeControl key, each
// carrying the index of the matinee input link whose name matches the key's pin. That index is how
// NeedsSynchronizing learns the pin has been impulsed, which is what breaks the loop.
void UInterpTrackInstSoireeControl::InitTrackInst( UInterpTrack* InTrack )
{
	Track = InTrack;
	m_iCurrentKeyIndex = INDEX_NONE;
	m_iCurrentLoopCount = 0;
	m_fCurrentPauseDuration = 0.f;

	USeqAct_Interp* Matinee = GetMatinee();
	UInterpTrackSoireeControl* SoireeTrack = CastChecked<UInterpTrackSoireeControl>( InTrack );

	m_lKeysStatus.Empty();
	m_lKeysStatus.AddZeroed( SoireeTrack->SoireeControlKeys.Num() );
	for( INT KeyIndex = 0; KeyIndex < SoireeTrack->SoireeControlKeys.Num(); KeyIndex++ )
	{
		FSoireeControlKeyStatus& Status = m_lKeysStatus( KeyIndex );
		Status.m_bIsBroken = FALSE;
		Status.m_InputIndex = INDEX_NONE;

		const FString PinName = GSoireeControlPinPrefix
			+ SoireeTrack->SoireeControlKeys( KeyIndex ).Properties->m_PinName.ToString();
		if( Matinee != NULL )
		{
			for( INT LinkIndex = 0; LinkIndex < Matinee->InputLinks.Num(); LinkIndex++ )
			{
				if( Matinee->InputLinks( LinkIndex ).LinkDesc == PinName )
				{
					Status.m_InputIndex = LinkIndex;
				}
			}
		}
	}

	FLOAT LoopStart = 0.f;
	SoireeStartLoop( 0.f, Matinee != NULL ? Matinee->Position : 0.f, LoopStart );
}

// DISHONORED(port): 2013 rva 0x5005e0 (2012 0x53f990). The last key the matinee has just reached
// becomes the active one and its loop count starts again from zero.
UBOOL UInterpTrackInstSoireeControl::SoireeStartLoop( FLOAT CurPosition, FLOAT NewPosition, FLOAT& OutLoopStart )
{
	UInterpTrackSoireeControl* SoireeTrack = CastChecked<UInterpTrackSoireeControl>( Track );
	for( INT KeyIndex = SoireeTrack->SoireeControlKeys.Num() - 1; KeyIndex >= 0; KeyIndex-- )
	{
		const FLOAT KeyTime = SoireeTrack->SoireeControlKeys( KeyIndex ).StartTime;
		if( KeyTime >= CurPosition && NewPosition > KeyTime )
		{
			m_iCurrentKeyIndex = KeyIndex;
			m_iCurrentLoopCount = 0;
			OutLoopStart = KeyTime;
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x500480 (2012 0x53f830). The active key defines the segment
// [StartTime, StartTime + KeyLength]; once the matinee's new position is past its end the segment is
// played again, m_LoopCount times, with 0 meaning until something breaks it. It is broken by the
// key's own status flag - which NeedsSynchronizing copies from the matinee's ActivatedLinks, i.e. by
// the Kismet impulse on the pin the key names - or by a smaller count from
// GetDistractionLoopOverride.
UBOOL UInterpTrackInstSoireeControl::SoireeShouldLoop( FLOAT CurPosition, FLOAT NewPosition, UBOOL bPreview,
                                                      FLOAT& OutLoopStart, FLOAT& OutLoopEnd,
                                                      FName& OutPinName, INT& OutLoopCount )
{
	UInterpTrackSoireeControl* SoireeTrack = CastChecked<UInterpTrackSoireeControl>( Track );
	USeqAct_Interp* Matinee = GetMatinee();
	if( m_iCurrentKeyIndex < 0 )
	{
		return FALSE;
	}

	const FSoireeControlTrackKey& Key = SoireeTrack->SoireeControlKeys( m_iCurrentKeyIndex );
	UInterpTrackSoireeControlKeyProperties* Properties = Key.Properties;
	if( Properties->m_SoireeControlType != ESCT_Loop )
	{
		return FALSE;
	}

	INT LoopCount = Properties->m_LoopCount;
	const INT Override = Matinee != NULL ? Matinee->GetDistractionLoopOverride( Properties->m_PinName ) : INDEX_NONE;
	if( Override >= 0 )
	{
		LoopCount = Override;
	}

	const FLOAT LoopStart = Key.StartTime;
	const FLOAT LoopEnd = Key.StartTime + Key.KeyLength;
	if( NewPosition <= LoopEnd || m_lKeysStatus( m_iCurrentKeyIndex ).m_bIsBroken )
	{
		return FALSE;
	}

	if( LoopCount != 0 )
	{
		if( m_iCurrentLoopCount >= LoopCount )
		{
			return FALSE;
		}
		if( ++m_iCurrentLoopCount >= LoopCount )
		{
			m_iCurrentKeyIndex = INDEX_NONE;
			m_iCurrentLoopCount = 0;
			return FALSE;
		}
	}
	else
	{
		m_iCurrentLoopCount++;
	}

	OutLoopStart = LoopStart;
	OutLoopEnd = LoopEnd;
	OutPinName = Properties->m_PinName;
	OutLoopCount = m_iCurrentLoopCount;
	return TRUE;
}
