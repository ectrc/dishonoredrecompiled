// UDisAISubProcessFleetingGaze cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent GI (PHASE20 GI). The glance a non-hostile NPC gives whoever the brain is paying most attention
// to: the sub-process keeps the brain's top attention proxy (FilterTopAttnProxyReplaced) and, every RefreshSubProcess, when
// the proxy is Busted, within the tweaks' radius and gaze angle, asks a head-only look-at for a random duration. 2012 rvas:
// BeginSubProcess_Derived 0x789a00 / 2013 0x73d8e0 (416 B), RefreshSubProcess_Derived 0x789ba0 / 0x739a00 (299 B),
// FilterTopAttnProxyReplaced 0x778100 / 0x73da80 (135 B), BuildFilterStimMask 0x765f40 / 0x728600 (31 B, one id:
// TopAttnProxyReplaced, 102 in 2012 = 105 here), GetFilterStimDelegate_SubProcess 0x780830 / 0x6fe680 (39 B), BeginDestroy
// 0x7764f0 / 0x739b80 (109 B). Bodies in Src/disaisubprocessfleetinggaze.cpp.
public:
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }
	virtual BYTE GetDesiresLookAtPriority() const { return DisLookAtPriority_AISubProcessFleetingGaze; }
	virtual void BeginDestroy();

protected:
	virtual void BeginSubProcess_Derived();
	virtual void RefreshSubProcess_Derived( const FLOAT _fTimeSinceLastThought );
	virtual const BYTE* BuildFilterStimMask();
	virtual FDisStimPredicateDelegate GetFilterStimDelegate_SubProcess( BYTE _StimID );

private:
	UBOOL FilterTopAttnProxyReplaced( const struct FAIStimStruct_TopAttnProxyReplaced& _rStim );
	void SetTopNotedProxy( const struct FDisAttentionProxy& _rProxy );
	void OnTopNotedActorTerminatedEvent( const class FArkGameEvent& _rEvent );
