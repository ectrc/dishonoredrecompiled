// DishonoredGame/src/disbehaviorpatrol.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (31):
//   0x723c80  public: static void __cdecl UDisTweaks_AIBehavior_Patrol::InitializePrivateStaticClassUDisTweaks_AIBehavior_Patrol(void)
//   0x723ca0  public: virtual void __thiscall UDisBehaviorPatrol::OnBehaviorPause(unsigned int)
//   0x723ce0  public: virtual unsigned char const * __thiscall UDisBehaviorPatrol::BuildEvaluateStimMask(void)const
//   0x723d20  public: virtual unsigned char const * __thiscall UDisBehaviorPatrol::BuildFilterStimMask(void)const
//   0x723e70  private: void __thiscall UDisBehaviorPatrol::SetupFromPatrolRequest(struct FAIStimStruct_PatrolRequest const &)
//   0x728a40  public: virtual unsigned char const * __thiscall UDisBehaviorPatrol::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x72be30  public: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorPatrol::GetSetupFromStimDelegate(enum EAIStimID)
//   0x72be60  public: virtual void __thiscall UDisBehaviorPatrol::OnEnterCallback_Stand(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x72bed0  protected: virtual void __thiscall UDisBehaviorPatrol::OnNewRouteChosen(void)
//   0x72bf00  private: unsigned int __thiscall UDisBehaviorPatrolSearch::EvaluatePatrolSearchRequest(struct FAIStimStruct_PatrolSearchRequest const &)const
//   0x72e280  public: virtual void __thiscall UDisBehaviorPatrol::InitBehavior(class UDishonoredAIBrain * const)
//   0x730e00  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorPatrol::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x736850  public: static class UClass * __cdecl UDisBehaviorPatrol::GetPrivateStaticClassUDisBehaviorPatrol(wchar_t const *)
//   0x738ae0  public: static void __cdecl UDisBehaviorPatrol::InitializePrivateStaticClassUDisBehaviorPatrol(void)
//   0x73a350  public: static class UClass * __cdecl UDisBehaviorPatrol::StaticClassNoInline(void)
//   0x73a380  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Patrol::GetPrivateStaticClassUDisTweaks_AIBehavior_Patrol(wchar_t const *)
//   0x73ae90  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Patrol::StaticClassNoInline(void)
//   0x73e560  public: virtual void __thiscall UDisBehaviorPatrol::OnBehaviorResume(void)
//   0x73e5f0  private: unsigned int __thiscall UDisBehaviorPatrol::ChooseNewRoute(void)
//   0x73e710  protected: void __thiscall UDisBehaviorPatrol::SetGuardPoint(class ADishonoredNavPoint *)
//   0x73e800  protected: void __thiscall UDisBehaviorPatrol::ResetPatrol(void)
//   0x73e8f0  public: virtual void __thiscall UDisBehaviorPatrol::TickCallback_TakeActorPosition(class UDishonoredNativeState *, float)
//   0x73e990  private: virtual void __thiscall UDisBehaviorPatrol::OnPostGameLoad(unsigned int, unsigned int)
//   0x741850  public: virtual void __thiscall UDisBehaviorPatrol::OnBehaviorStart(void)
//   0x741860  public: virtual void __thiscall UDisBehaviorPatrol::OnBehaviorStop(unsigned int)
//   0x7418a0  private: void __thiscall UDisBehaviorPatrol::FindNextPoint(void)
//   0x741a50  private: void __thiscall UDisBehaviorPatrol::TickGuarding(float)
//   0x742c70  private: void __thiscall UDisBehaviorPatrol::OnReachedDestination(class AActor * const, unsigned int)
//   0x742d50  public: virtual void __thiscall UDisBehaviorPatrol::RequestStateExitCallback_TakeActorPosition(class UDishonoredNativeState *)
//   0x742e20  public: virtual void __thiscall UDisBehaviorPatrol::TickCallback_Stand(class UDishonoredNativeState *, float)
//   0x743a70  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorPatrol::GetFilterStimDelegate(enum EAIStimID)
