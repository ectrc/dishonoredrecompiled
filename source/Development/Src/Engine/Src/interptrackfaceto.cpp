// Engine/src/interptrackfaceto.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (23):
//   0x53ba20  public: static void __cdecl UInterpTrackFaceTo::InitializePrivateStaticClassUInterpTrackFaceTo(void)
//   0x53ba40  public: static void __cdecl UInterpTrackKeyProperties::InitializePrivateStaticClassUInterpTrackKeyProperties(void)
//   0x53ba60  public: static void __cdecl UInterpTrackFaceTo::SetFaceToPriority(int)
//   0x53ba70  public: virtual class FColor __thiscall UInterpTrackStretchAnimControl::GetKeyframeColor(int)const
//   0x53caf0  public: static class UClass * __cdecl UInterpTrackKeyProperties::GetPrivateStaticClassUInterpTrackKeyProperties(wchar_t const *)
//   0x53cb80  public: static class UClass * __cdecl UInterpTrackFaceToKeyProperties::GetPrivateStaticClassUInterpTrackFaceToKeyProperties(wchar_t const *)
//   0x53e810  public: static class UClass * __cdecl UInterpTrackKeyProperties::StaticClassNoInline(void)
//   0x53e840  public: static void __cdecl UInterpTrackFaceToKeyProperties::InitializePrivateStaticClassUInterpTrackFaceToKeyProperties(void)
//   0x53e860  public: virtual void __thiscall UInterpTrackFaceTo::GetTimeRange(float &, float &)const
//   0x53e910  public: virtual float __thiscall UInterpTrackFaceTo::GetKeyframeTime(int)const
//   0x53e970  public: virtual float __thiscall UInterpTrackFaceTo::GetKeyframeLength(int)const
//   0x53e9d0  public: virtual int __thiscall UInterpTrackFaceTo::DuplicateKeyframe(int, float)
//   0x53eb20  public: virtual float __thiscall UInterpTrackFaceTo::GetTrackEndTime(void)const
//   0x53eb90  public: virtual void __thiscall UInterpTrackInstFaceTo::UpdateTrackInst(float, unsigned int)
//   0x541e60  public: static class UClass * __cdecl UInterpTrackFaceToKeyProperties::StaticClassNoInline(void)
//   0x541e90  public: virtual int __thiscall UInterpTrackFaceTo::SetKeyframeTime(int, float, unsigned int)
//   0x542030  public: virtual void __thiscall UInterpTrackFaceTo::RemoveKeyframe(int)
//   0x542060  public: virtual void __thiscall UInterpTrackFaceTo::PostLoad(void)
//   0x542180  public: virtual unsigned int __thiscall UInterpTrackFaceTo::GetClosestSnapPosition(float, class TArray<int, class FDefaultAllocator> &, float &)
//   0x544850  public: virtual class UObject * __thiscall UInterpTrackFaceTo::GetKeyProperties(int)
//   0x544990  public: virtual int __thiscall UInterpTrackFaceTo::AddKeyframe(float, class UInterpTrackInst *, enum EInterpCurveMode)
//   0x548ee0  public: static class UClass * __cdecl UInterpTrackFaceTo::GetPrivateStaticClassUInterpTrackFaceTo(wchar_t const *)
//   0x5492d0  public: static class UClass * __cdecl UInterpTrackFaceTo::StaticClassNoInline(void)

#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"      // USeqVar_Character, used by the matinee group instances
#include "EngineInterpolationClasses.h"

IMPLEMENT_CLASS(UInterpTrackFaceTo);
IMPLEMENT_CLASS(UInterpTrackFaceToKeyProperties);
IMPLEMENT_CLASS(UInterpTrackInstFaceTo);

// DISHONORED(port): 2013 rva 0x4fc8a0 (2012 0x53ba60): the request priority the game sets once at startup
INT UInterpTrackFaceTo::s_InterpTrackFaceToPriority = 0;

void UInterpTrackFaceTo::SetFaceToPriority( INT FaceToPriority )
{
	s_InterpTrackFaceToPriority = FaceToPriority;
}

// DISHONORED(port): 2013 rva 0x503200 (2012 0x542060, interptrackfaceto.cpp:86): outside a package (i.e. inside a UMatineeData) every key's target
// group is looked up by name in the owning matinee data; a key without a target name loses its group pointer.
void UInterpTrackFaceTo::PostLoad()
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
	for( INT KeyIndex = 0; KeyIndex < FaceToKeys.Num(); KeyIndex++ )
	{
		UInterpTrackFaceToKeyProperties* Properties = FaceToKeys(KeyIndex).Properties;
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
