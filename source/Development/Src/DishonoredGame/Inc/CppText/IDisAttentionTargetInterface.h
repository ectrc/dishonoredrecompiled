// IDisAttentionTargetInterface cpptext: included inside the generated interface body.
// DISHONORED(written): agent CG (PHASE9 CG). Anything an NPC can pay attention to implements this: pawns, corpses,
// bodies of water, wall of light pylons. The proxy reaches a target only through these. Bodies in
// Src/disattentiontargetinterface.cpp.
// DISHONORED(port): agent GI (PHASE20 GI). The whole 2012 PDB IDisAttentionTargetInterface_vtbl (24 slots, all_types.h
// struct 23580), in retail's order: +0 destructor, +4 GetUObjectInterfaceDisAttentionTargetInterface, +8
// CollectAttnTargetParams, +12 GetAttnTargetTweaks, +16 GetObservableComponent, +20 GetAttnTargetLocation, +24
// GetAttnTargetVelocity, +28 GetAttnTargetExtent, +32 GetAttnTargetIncomingRelationship, +36 ToRelationshipInterface, +40
// GetAttnTargetFocalPoint, +44 GetAttnTargetFocalHeight, +48 GetAttnTargetBrain, +52 IsAttnTargetDead, +56
// IsAttnTargetCombatable, +60 IsAttnTargetInCombat, +64 CanAttnTargetFight, +68 IsAttnTargetPlayer, +72 AllowAttentionGain,
// +76 CanAttnTargetBeAttacked, +80 IsAttnTargetBeingMurdered, +84 IsAttnTargetCarryingACorpse, +88 OnAttentionReceived,
// +92 OnAttentionLost. The base bodies are the identical-code folds of the PDB (empty, FALSE, zero vectors); CG's
// GetAttnTargetActor is this tree's name for the UObject accessor cast to an actor and is kept. ADishonoredPawn overrides
// them in Src/dishonoredpawn_attentiontarget.cpp.
public:
	virtual class AActor* GetAttnTargetActor() { return NULL; }
	virtual void CollectAttnTargetParams( struct FDisAttnTargetParams& _rOut_Params );
	virtual const class UDisTweaks_AttentionTarget* GetAttnTargetTweaks() const { return NULL; }
	virtual const class FDisComponentObservable* GetObservableComponent() const { return NULL; }
	virtual FVector GetAttnTargetLocation() const { return FVector( 0.f, 0.f, 0.f ); }
	virtual FVector GetAttnTargetVelocity() const { return FVector( 0.f, 0.f, 0.f ); }
	virtual FVector GetAttnTargetExtent() const { return FVector( 0.f, 0.f, 0.f ); }
	virtual BYTE GetAttnTargetIncomingRelationship( const class IDisRelationshipInterface* _pRelator ) const;
	virtual class IDisRelationshipInterface* ToRelationshipInterface() const { return NULL; }
	virtual FVector GetAttnTargetFocalPoint() const;
	virtual FLOAT GetAttnTargetFocalHeight() const;
	virtual const class UDishonoredAIBrain* GetAttnTargetBrain() const { return NULL; }
	virtual UBOOL IsAttnTargetDead() const { return FALSE; }
	virtual UBOOL IsAttnTargetCombatable() const { return FALSE; }
	virtual UBOOL IsAttnTargetInCombat() const { return FALSE; }
	virtual UBOOL CanAttnTargetFight() const { return FALSE; }
	virtual UBOOL IsAttnTargetPlayer() const { return FALSE; }
	virtual UBOOL AllowAttentionGain() const { return TRUE; }
	virtual UBOOL CanAttnTargetBeAttacked() const { return FALSE; }
	virtual UBOOL IsAttnTargetBeingMurdered( class IDisAttentionTargetInterface** _ppMurderer ) const { if( _ppMurderer ) *_ppMurderer = NULL; return FALSE; }
	virtual UBOOL IsAttnTargetCarryingACorpse() const { return FALSE; }
	virtual void OnAttentionReceived( class UDishonoredAIBrain* _pBrain ) {}
	virtual void OnAttentionLost( class UDishonoredAIBrain* _pBrain ) {}
