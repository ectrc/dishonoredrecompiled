// Engine/src/interptrackinstattachment.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (7):
//   0x53bad0  public: static void __cdecl UInterpTrackInstAttachment::InitializePrivateStaticClassUInterpTrackInstAttachment(void)
//   0x53baf0  public: virtual void __thiscall UInterpTrackInstAttachment::InitTrackInst(class UInterpTrack *)
//   0x53bf00  public: virtual void __thiscall UInterpTrackInstAttachment::LoadData(class FArchive &)
//   0x53cca0  public: static class UClass * __cdecl UInterpTrackInstAttachment::GetPrivateStaticClassUInterpTrackInstAttachment(wchar_t const *)
//   0x53ee10  public: static class UClass * __cdecl UInterpTrackInstAttachment::StaticClassNoInline(void)
//   0x53ee40  public: virtual void __thiscall UInterpTrackInstAttachment::TermTrackInst(class UInterpTrack *)
//   0x53ee90  public: virtual void __thiscall UInterpTrackInstAttachment::SaveData(class FArchive &)
//
// DISHONORED(port): agent GB (PHASE19 GB). InitTrackInst (2012 0x53baf0 / 2013 0x8fc950) and TermTrackInst (0x53ee40 /
// 0x8ffb40). LoadData / SaveData (the save layer's backup of the attached actor's transform) are not ported.
#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineAnimClasses.h"

IMPLEMENT_CLASS(UInterpTrackInstAttachment);

void UInterpTrackInstAttachment::InitTrackInst(UInterpTrack* InTrack)
{
	m_bActorBackuped = FALSE;
	Track = InTrack;
	m_LastUpdatePosition = -1.f;
}

void UInterpTrackInstAttachment::TermTrackInst(UInterpTrack* InTrack)
{
	UInterpTrackAttachment* AttachTrack = Cast<UInterpTrackAttachment>( Track );
	if( AttachTrack == NULL || !AttachTrack->m_bDetachActorOnInteruption )
	{
		return;
	}
	UInterpGroupInst* TargetInst = GetMatinee()->FindFirstGroupInstByName( AttachTrack->m_TargetName );
	if( TargetInst != NULL && TargetInst->GetGroupActor() != NULL )
	{
		TargetInst->GetGroupActor()->MAT_Detach();
	}
}
