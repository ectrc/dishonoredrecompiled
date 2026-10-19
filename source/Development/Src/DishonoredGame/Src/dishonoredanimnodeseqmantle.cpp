// DishonoredGame/src/dishonoredanimnodeseqmantle.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (9):
//   0x6d0cd0  private: virtual void __thiscall UDishonoredAnimNodeSeqMantle::BeginDestroy(void)
//   0x6d0d00  private: virtual void __thiscall UDishonoredAnimNodeSeqMantle::PostAnimNodeInstance(class UAnimNode *, class TMap<class UAnimNode *, class UAnimNode *, class FDefaultSetAllocator> *)
//   0x6e0f50  private: float __thiscall UDishonoredAnimNodeSeqMantle::FindTotalAnimZTrans(void)
//   0x6e10d0  private: virtual void __thiscall UDishonoredAnimNodeSeqMantle::GetBoneAtoms(class TArray<class FBoneAtom, class TMemStackAllocator<class FMemStack GMainThreadMemStack, 8>> &, class TArray<unsigned char, class FDefaultAllocator> const &, class FBoneAtom &, int &)
//   0x6e12e0  private: virtual unsigned short __thiscall UDishonoredAnimNodeSeqMantle::BuildEdgeAnimTree(struct UAnimNode::FEdgeAnimTreeContext &)
//   0x6ead00  public: virtual void __thiscall UDishonoredAnimNodeSeqMantle::SetAnim(class FName)
//   0x6f3ce0  public: static class UClass * __cdecl UDishonoredAnimNodeSeqMantle::GetPrivateStaticClassUDishonoredAnimNodeSeqMantle(wchar_t const *)
//   0x6f4ea0  public: static void __cdecl UDishonoredAnimNodeSeqMantle::InitializePrivateStaticClassUDishonoredAnimNodeSeqMantle(void)
//   0x6f52f0  public: static class UClass * __cdecl UDishonoredAnimNodeSeqMantle::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the mantle's sequence (Inc/CppText/UDishonoredAnimNodeSeqMantle.h); 2013 bodies are the same bytes.
	DISHONORED(bringup), named: FindTotalAnimZTrans (2012 0x6e0f50, the root bone's climb measured from the sequence) only runs without
	seek-free loading (the editor); the cooked game uses m_fMantleTravelDistanceZ as retail does. BeginDestroy (0x6d0cd0) frees
	m_pMantleTransform, which nothing in the game allocates; not written.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "imdiag.h"

void DishonoredLinkIM_DishonoredAnimNodeSeqMantle()
{
}

static UStatePlayerMasterMantle* DisMantleStateOf( USkeletalMeshComponent* SkelComponent )
{
	ADishonoredPlayerPawn* Player = SkelComponent ? Cast<ADishonoredPlayerPawn>( SkelComponent->GetOwner() ) : NULL;
	return ( Player && Player->m_pPlayerMasterFSM ) ? Cast<UStatePlayerMasterMantle>( Player->m_pPlayerMasterFSM->GetLogicalState() ) : NULL;
}

// 2012 0x6d0d00 / 2013 0x67fc90 (71 B)
void UDishonoredAnimNodeSeqMantle::PostAnimNodeInstance( UAnimNode* SourceNode, TMap<UAnimNode*,UAnimNode*>& SrcToDestNodeMap )
{
	Super::PostAnimNodeInstance( SourceNode, SrcToDestNodeMap );
	m_pDisParentAnimTree = NULL;
	m_pMantleTransform = NULL;
	if( GIsGame && m_bSequenceMustBeSetInAnimState )
	{
		AnimSeqName = NAME_None;
	}
}

// 2012 0x6ead00 / 2013 0x698000 (91 B)
void UDishonoredAnimNodeSeqMantle::SetAnim( FName Sequence )
{
	Super::SetAnim( Sequence );
	if( AnimSeq && !GUseSeekFreeLoading )
	{
		IMDIAG_ONCE( TEXT("UDishonoredAnimNodeSeqMantle::SetAnim: FindTotalAnimZTrans (editor only) is not ported") );
	}
}

// 2012 0x6e10d0 / 2013 0x68f7a0 (525 B): inside the mantle state, the root motion's world Z is scaled by edge height / the climb
void UDishonoredAnimNodeSeqMantle::GetBoneAtoms( FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys )
{
	Super::GetBoneAtoms( Atoms, DesiredBones, RootMotionDelta, bHasRootMotion, CurveKeys );
	UStatePlayerMasterMantle* Mantle = DisMantleStateOf( SkelComponent );
	if( Mantle == NULL )
	{
		return;
	}
	const FLOAT Scale = m_fMantleTravelDistanceZ == 0.f ? 1.f : Mantle->GetCurrentMantleHeight() / m_fMantleTravelDistanceZ;
	const FMatrix& M = SkelComponent->LocalToWorld;
	const FVector Local = RootMotionDelta.GetTranslation();
	const FVector World( M.M[0][0] * Local.X + M.M[1][0] * Local.Y + M.M[2][0] * Local.Z,
		M.M[0][1] * Local.X + M.M[1][1] * Local.Y + M.M[2][1] * Local.Z,
		( M.M[0][2] * Local.X + M.M[1][2] * Local.Y + M.M[2][2] * Local.Z ) * Scale );
	RootMotionDelta.SetTranslation( FVector( M.M[0][0] * World.X + M.M[0][1] * World.Y + M.M[0][2] * World.Z,
		M.M[1][0] * World.X + M.M[1][1] * World.Y + M.M[1][2] * World.Z,
		M.M[2][0] * World.X + M.M[2][1] * World.Y + M.M[2][2] * World.Z ) );
}
