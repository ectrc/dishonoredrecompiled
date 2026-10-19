// DishonoredGame/src/disanimnodeblendbystepupmantle.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (6):
//   0x6cfd40  public: static void __cdecl UDisAnimNodeBlendByStepUpMantle::InitializePrivateStaticClassUDisAnimNodeBlendByStepUpMantle(void)
//   0x6d8690  private: class UAnimNodeSequence * __thiscall UDisAnimNodeBlendByStepUpMantle::GetMantleSeqNode(void)const
//   0x6e4520  public: virtual void __thiscall UDisAnimNodeBlendByStepUpMantle::TickAnim(float)
//   0x6e4590  public: void __thiscall UDisAnimNodeBlendByStepUpMantle::OnStepUpMantle(void)
//   0x6f20f0  public: static class UClass * __cdecl UDisAnimNodeBlendByStepUpMantle::GetPrivateStaticClassUDisAnimNodeBlendByStepUpMantle(wchar_t const *)
//   0x6f4960  public: static class UClass * __cdecl UDisAnimNodeBlendByStepUpMantle::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent IG (PHASE23 IG): the arms' step-up mantle blend. DoMantle_StepUp finds it by the mantle tweaks' node name
	(StepUpMantle); OnStepUpMantle blends the additive step-up sequence (child 1) in and plays it from the start, TickAnim blends it
	back out over m_fBlendTime once that much of the sequence is left. 2012 / 2013 rvas in Inc/CppText/UDisAnimNodeBlendByStepUpMantle.h.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

void DishonoredLinkIG_DisAnimNodeBlendByStepUpMantle()
{
}

// 2012 0x6d8690 / 2013 0x688ff0 (73 B)
UAnimNodeSequence* UDisAnimNodeBlendByStepUpMantle::GetMantleSeqNode() const
{
	return Children(1).Anim ? Cast<UAnimNodeSequence>( Children(1).Anim ) : NULL;
}

// 2012 0x6e4520 / 2013 0x6920c0 (112 B)
void UDisAnimNodeBlendByStepUpMantle::TickAnim( FLOAT DeltaSeconds )
{
	if( Child2WeightTarget > 0.f )
	{
		UAnimNodeSequence* MantleSeq = GetMantleSeqNode();
		if( MantleSeq && m_fBlendTime >= MantleSeq->GetTimeLeft() )
		{
			SetBlendTarget( 0.f, m_fBlendTime );
		}
	}
	Super::TickAnim( DeltaSeconds );
}

// 2012 0x6e4590 / 2013 0x692130 (130 B)
void UDisAnimNodeBlendByStepUpMantle::OnStepUpMantle()
{
	UAnimNodeSequence* MantleSeq = Children(1).Anim ? Cast<UAnimNodeSequence>( Children(1).Anim ) : NULL;
	if( MantleSeq )
	{
		SetBlendTarget( 1.f, m_fBlendTime );
		MantleSeq->PlayAnim( FALSE, 1.f, 0.f );
	}
}
