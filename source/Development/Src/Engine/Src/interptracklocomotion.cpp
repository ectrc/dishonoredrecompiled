// Engine/src/interptracklocomotion.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (13):
//   0x53bd60  public: static void __cdecl UInterpTrackLocomotion::InitializePrivateStaticClassUInterpTrackLocomotion(void)
//   0x53bd80  public: static void __cdecl UInterpTrackLocomotion::SetLocomotionPriority(int)
//   0x53cf10  public: static class UClass * __cdecl UInterpTrackLocomotionKeyProperties::GetPrivateStaticClassUInterpTrackLocomotionKeyProperties(wchar_t const *)
//   0x53cfa0  public: class AActor * __thiscall UInterpTrackLocomotionKeyProperties::FindKeyTargetActor(class USeqAct_Interp const * const, class APlayerController const * const)const
//   0x53fa30  public: static void __cdecl UInterpTrackLocomotionKeyProperties::InitializePrivateStaticClassUInterpTrackLocomotionKeyProperties(void)
//   0x53fa50  public: virtual unsigned int __thiscall UInterpTrackInstLocomotion::NeedsSynchronizing(float, float, float &, unsigned int)
//   0x5424e0  public: static class UClass * __cdecl UInterpTrackLocomotionKeyProperties::StaticClassNoInline(void)
//   0x542510  public: virtual void __thiscall UInterpTrackLocomotion::PostLoad(void)
//   0x542630  public: virtual void __thiscall UInterpTrackInstLocomotion::UpdateTrackInst(float, unsigned int)
//   0x544d10  public: virtual class UObject * __thiscall UInterpTrackLocomotion::GetKeyProperties(int)
//   0x548f70  public: static class UClass * __cdecl UInterpTrackLocomotion::GetPrivateStaticClassUInterpTrackLocomotion(wchar_t const *)
//   0x549300  public: static class UClass * __cdecl UInterpTrackLocomotion::StaticClassNoInline(void)
//   0xb9ed40  _dynamic_initializer_for__UInterpTrackLocomotion::s_TrackLocoPlayerTargerName__

#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"      // USeqVar_Character, used by the matinee group instances
#include "EngineInterpolationClasses.h"

IMPLEMENT_CLASS(UInterpTrackLocomotion);
IMPLEMENT_CLASS(UInterpTrackLocomotionKeyProperties);
IMPLEMENT_CLASS(UInterpTrackInstLocomotion);

// DISHONORED(port): 2013 rva 0x4fcc30 (2012 0x53bd80)
INT UInterpTrackLocomotion::s_iInterpTrackLocomotionPriority = 0;

void UInterpTrackLocomotion::SetLocomotionPriority( INT LocomotionPriority )
{
	s_iInterpTrackLocomotionPriority = LocomotionPriority;
}

// DISHONORED(port): 2013 rva 0x5039d0 (2012 0x542510, interptracklocomotion.cpp:100): outside a package (i.e. inside a UMatineeData) every key's target
// group is looked up by name in the owning matinee data; a key without a target name loses its group pointer.
void UInterpTrackLocomotion::PostLoad()
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
	for( INT KeyIndex = 0; KeyIndex < LocoKeys.Num(); KeyIndex++ )
	{
		UInterpTrackLocomotionKeyProperties* Properties = LocoKeys(KeyIndex).Properties;
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
