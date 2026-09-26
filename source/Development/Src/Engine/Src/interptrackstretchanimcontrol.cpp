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

#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"      // USeqVar_Character, used by the matinee group instances
#include "EngineInterpolationClasses.h"

IMPLEMENT_CLASS(UInterpTrackStretchAnimControl);
IMPLEMENT_CLASS(UInterpTrackStretchAnimKeyProperties);
IMPLEMENT_CLASS(UInterpTrackInstStretchAnimControl);

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
