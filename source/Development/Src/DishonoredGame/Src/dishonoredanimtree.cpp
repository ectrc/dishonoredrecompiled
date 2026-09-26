// DishonoredGame/src/dishonoredanimtree.cpp
// The Arkane anim tree. Beyond UAnimTree it carries the anim-state bookkeeping (m_AnimStateInfo): which picker drove
// this tree in, which object fired the state and which native state the state is attached to.
// PDB functions attributed to this file (11), 2012 rvas:
//   0x6b1c20  UDishonoredAnimTree::InitializePrivateStaticClass   (generated)
//   0x6b1c40  UDishonoredAnimTree::EndAnimState
//   0x6b1c60  UDishonoredAnimTree::InitAnim
//   0x6b1c90  UDishonoredAnimTree::OnCeaseRelevant
//   0x6b1cc0  UDishonoredAnimTree::GetAnimStateInfo
//   0x6bdcc0  UDishonoredAnimTree::IsAnimStateActive
//   0x6c0dd0  UDishonoredAnimTree::TransformSingleBone             (not ported: only GetChildBoneAtoms uses it)
//   0x6c5750  UDishonoredAnimTree::GetAnimStateFireInterface
//   0x6c8c60  UDishonoredAnimTree::GetAnimStateEquipUsage          (not ported: needs UDisItemContext / UStatePlayerAction)

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x67d6c0 (2012 0x6b1c40)
void UDishonoredAnimTree::EndAnimState()
{
	m_AnimStateInfo.m_bIsAnimStateActive = FALSE;
	m_AnimStateInfo.m_AnimStatePicker = ANIMSTATE_MAX;
	m_AnimStateInfo.m_pAnimState_FiredFromObj = NULL;
	m_AnimStateInfo.m_pAnimState_AttachToState = NULL;
}

// DISHONORED(written): 2013 rva 0x67d6e0 (2012 0x6b1c60): a re-initialised tree is no longer inside an anim state
void UDishonoredAnimTree::InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent )
{
	EndAnimState();
	Super::InitAnim( MeshComp, Parent );
}

// DISHONORED(written): 2013 rva 0x67d710 (2012 0x6b1c90): retail goes straight to UAnimNode, skipping the blend base
void UDishonoredAnimTree::OnCeaseRelevant()
{
	EndAnimState();
	UAnimNode::OnCeaseRelevant();
}

// DISHONORED(written): 2013 rva 0x67d740 (2012 0x6b1cc0)
void UDishonoredAnimTree::GetAnimStateInfo( FDisAnimStateTreeInfo* Out ) const
{
	if( Out )
	{
		*Out = m_AnimStateInfo;
	}
}

// DISHONORED(written): 2013 rva 0x688a90 (2012 0x6bdcc0): an NPC's tree is always treated as inside an anim state
UBOOL UDishonoredAnimTree::IsAnimStateActive() const
{
	if( SkelComponent && Cast<ADishonoredNPCPawn>( SkelComponent->GetOwner() ) )
	{
		return TRUE;
	}
	return m_AnimStateInfo.m_bIsAnimStateActive ? TRUE : FALSE;
}

// DISHONORED(written): 2013 rva 0x6918c0 (2012 0x6c5750)
IDisAnimStateFiringInterface* UDishonoredAnimTree::GetAnimStateFireInterface() const
{
	UObject* FiredFrom = (UObject*)m_AnimStateInfo.m_pAnimState_FiredFromObj;
	if( !FiredFrom )
	{
		return NULL;
	}
	return (IDisAnimStateFiringInterface*)FiredFrom->GetInterfaceAddress( UDisAnimStateFiringInterface::StaticClass() );
}
