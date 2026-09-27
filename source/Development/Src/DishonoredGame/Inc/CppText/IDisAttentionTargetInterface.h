// IDisAttentionTargetInterface cpptext: included inside the generated interface body.
// DISHONORED(written): agent CG (PHASE9 CG). Anything an NPC can pay attention to implements this: pawns, corpses,
// bodies of water, wall of light pylons. The proxy reaches a target only through these. Bodies in
// Src/disattentiontargetinterface.cpp.
public:
	virtual class AActor* GetAttnTargetActor() { return NULL; }
	virtual FVector GetAttnTargetLocation() const { return FVector( 0.f, 0.f, 0.f ); }
	virtual FVector GetAttnTargetExtent() const { return FVector( 0.f, 0.f, 0.f ); }
	virtual FVector GetAttnTargetFocalPoint() const;
	virtual FLOAT GetAttnTargetFocalHeight() const;
	virtual UBOOL CanAttnTargetBeAttacked() const { return FALSE; }
	virtual class IDisRelationshipInterface* ToRelationshipInterface() const { return NULL; }
