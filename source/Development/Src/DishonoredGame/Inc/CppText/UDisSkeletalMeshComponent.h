// UDisSkeletalMeshComponent cpptext: included inside the generated class body (dishonoredgameclasses.h).
// agent GF (PHASE19 GF): retail's component tick-end phase. UDisSkeletalMeshComponent::TickEnd 2012 0x799d10 (71 B) is the
// override of USkeletalMeshComponent::TickEnd the retail engine calls after the component ticked; it runs UpdateChildForTickEnd
// 0x794e50 (2013 0x750f20, 179 B): the _Derived hook (0x57ef10, a one-byte ret in the base) then the same on every attached
// UDishonoredPlayerSkeletalComponent. The reference engine has no tick-end phase; USkeletalMeshComponent::TickEnd is a
// hand-added empty virtual called at the end of USkeletalMeshComponent::Tick (UnSkeletalMesh.h / UnSkeletalComponent.cpp).
// Bodies in dishonoredpawn_body.cpp.
public:
	virtual void TickEnd( FLOAT DeltaTime );
	void UpdateChildForTickEnd();
protected:
	virtual void UpdateChildForTickEnd_Derived() {}
public:
