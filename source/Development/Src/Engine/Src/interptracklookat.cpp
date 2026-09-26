// Engine/src/interptracklookat.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (13):
//   0x53bd90  public: static void __cdecl UInterpTrackLookAt::InitializePrivateStaticClassUInterpTrackLookAt(void)
//   0x53bdb0  public: static int __cdecl UInterpTrackLookAt::GetGameLookatPriority(enum ESoireeLookatRequestsPriority)
//   0x53bde0  public: static void __cdecl UInterpTrackLookAt::SetLookatPriority(int, int, int)
//   0x53cff0  public: static class UClass * __cdecl UInterpTrackLookAtKeyProperties::GetPrivateStaticClassUInterpTrackLookAtKeyProperties(wchar_t const *)
//   0x53fbb0  public: static void __cdecl UInterpTrackLookAtKeyProperties::InitializePrivateStaticClassUInterpTrackLookAtKeyProperties(void)
//   0x542ab0  public: static class UClass * __cdecl UInterpTrackLookAtKeyProperties::StaticClassNoInline(void)
//   0x542ae0  public: virtual void __thiscall UInterpTrackLookAt::PostLoad(void)
//   0x542c10  public: unsigned int __thiscall UInterpTrackLookAt::GetLookAtInfos(float, class UInterpTrackInstLookAt const *, class FVector &, float &, float &, float &, float &)const
//   0x544e50  public: virtual class UObject * __thiscall UInterpTrackLookAt::GetKeyProperties(int)
//   0x547010  public: static void __cdecl UInterpTrackLookAt::StaticUpdateTracks(float, class UInterpGroupInstAI *, unsigned int, unsigned int)
//   0x549000  public: static class UClass * __cdecl UInterpTrackLookAt::GetPrivateStaticClassUInterpTrackLookAt(wchar_t const *)
//   0x549330  public: static class UClass * __cdecl UInterpTrackLookAt::StaticClassNoInline(void)
//   0xb9ed60  _dynamic_initializer_for__s_LookAtDragOperationMatchingPoints__

#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"      // USeqVar_Character, used by the matinee group instances
#include "EngineInterpolationClasses.h"

IMPLEMENT_CLASS(UInterpTrackLookAt);
IMPLEMENT_CLASS(UInterpTrackLookAtKeyProperties);
IMPLEMENT_CLASS(UInterpTrackInstLookAt);

// DISHONORED(port): 2013 rva 0x4fcc90 (2012 0x53bde0): the three look-at priorities the game sets once at startup
INT UInterpTrackLookAt::ms_InterpTrackLookAtLowPriority = 0;
INT UInterpTrackLookAt::ms_InterpTrackLookAtMediumPriority = 0;
INT UInterpTrackLookAt::ms_InterpTrackLookAtHighPriority = 0;

void UInterpTrackLookAt::SetLookatPriority( INT LookatLowPriority, INT LookatMediumPriority, INT LookatHighPriority )
{
	ms_InterpTrackLookAtLowPriority = LookatLowPriority;
	ms_InterpTrackLookAtMediumPriority = LookatMediumPriority;
	ms_InterpTrackLookAtHighPriority = LookatHighPriority;
}

// DISHONORED(port): 2013 rva 0x504120 (2012 0x542ae0, interptracklookat.cpp:132): outside a package (i.e. inside a UMatineeData) every key's target
// group is looked up by name in the owning matinee data; a key without a target name loses its group pointer.
void UInterpTrackLookAt::PostLoad()
{
	Super::PostLoad();
	if( GetOuter()->IsA(UPackage::StaticClass()) )
	{
		return;
	}
	UInterpGroup* Group = GetOwningGroup();
	if( !Group || Group->GetOuter()->IsA(UPackage::StaticClass()) )
	{
		return;
	}
	UMatineeData* Data = CastChecked<UMatineeData>( Group->GetOuter() );
	for( INT KeyIndex = 0; KeyIndex < LookAtKeys.Num(); KeyIndex++ )
	{
		UInterpTrackLookAtKeyProperties* Properties = LookAtKeys(KeyIndex).Properties;
		if( !Properties )
		{
			continue;
		}
		if( Properties->m_TargetName != NAME_None )
		{
			const INT GroupIndex = Data->FindGroupByName( Properties->m_TargetName );
			if( GroupIndex != INDEX_NONE )
			{
				Properties->m_Target = Data->GetInterpGroup( GroupIndex );
			}
		}
		else
		{
			Properties->m_Target = NULL;
		}
	}
}
