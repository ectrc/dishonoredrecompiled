// FDisAttentionProxy cpptext: included inside the generated struct body (dishonoredgameclasses.h) through the struct
// cpptext hook agent CG added to gen_classes_header.py.
// DISHONORED(written): agent CG. An attention proxy is a brain-plus-target pair: it is how one NPC refers to something
// it is paying attention to without holding the attention state itself. Every query goes back through the brain
// (UDishonoredAIBrain::GetAttentionProxyInfo / GetAttentionLevel), which is why the same target can be at a different
// attention level for two different NPCs. Bodies in Src/disattentionproxy.cpp.
public:
	FDisAttentionProxy( class UDishonoredAIBrain* _pBrain, class IDisAttentionTargetInterface* _pTarget );

	class IDisAttentionTargetInterface* GetProxyAttnTarget() const;
	class IDisConvSpeakerInterface* GetProxySpeaker() const;
	BYTE GetProxyAttnLevel() const;
	INT GetProxyAttnTag() const;
	FVector GetProxyLocation() const;
	FVector GetProxyFeetLocation() const;
	FVector GetBestTargetLocation() const;
	FLOAT GetProxyHeight() const;
	struct FDisAttentionChangeReason GetProxyChangeReason() const;

	UBOOL IsBusted() const;
	UBOOL IsIndeterminate() const;
	UBOOL IsVerifiedAsTarget() const;
	UBOOL IsValid() const;
	UBOOL HasActorReference() const;
	UBOOL IsEqualToActor( const class AActor& _rActor ) const;
	UBOOL CanProxyBeAttacked() const;

	void ClearAttnProxy();
	void SerializeForGC( FArchive& _rAr );
