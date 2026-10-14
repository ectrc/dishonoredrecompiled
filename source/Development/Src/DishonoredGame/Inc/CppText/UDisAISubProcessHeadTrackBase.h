// UDisAISubProcessHeadTrackBase cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent GE (PHASE19 GE). The head-track sub-process keeps the proxy of whoever the brain's attention
// put at DAL_HeadTrack: FAIStimStruct_HeadTrackBegin (raised by UDishonoredAIBrain::OnIncreaseTopAttnTargetAttentionLevel)
// sets it, HeadTrackEnd / NoticeBegin / the end of the sub-process / the target's termination clear it, and the two
// subclasses react to the set (Bark fires DDH_ATTENTION_HEADTRACK, Pause stops walking and turns). It raises NO look-at
// request itself: the head turns because the brain's own FDisLookAtRequest asks for the proxy in
// ChangeTopAttnTargetAttentionLevel (2012 rva 0x75a420) - FV's premise that the request came from these three was wrong.
// Bodies in Src/disaisubprocessheadtrackbase.cpp; 2012 / 2013 rvas on each.
public:
	virtual void BeginSubProcess_Derived();
	virtual void EndSubProcess_Derived( UBOOL _bIsBeingTerminated );
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubProcess( BYTE StimID );
	virtual void BeginDestroy();
	virtual void OnHeadTrackProxySet() {}
	virtual void OnHeadTrackProxyClear() {}

protected:
	struct FDisAttentionProxy GetHeadTrackProxy() const;

private:
	void SetHeadTrackProxy( const struct FDisAttentionProxy& _rProxy );
	void OnOtherActorTerminatedEvent( const class FArkGameEvent& _rEvent );
	UBOOL FilterHeadTrackBegin( const struct FAIStimStruct_HeadTrackBegin& _rHeadTrackBeginStim );
	UBOOL FilterNoticeBegin( const struct FAIStimStruct_NoticeBegin& _rNoticeBeginStim );
