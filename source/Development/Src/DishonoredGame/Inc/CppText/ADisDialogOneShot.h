// ADisDialogOneShot cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The level actor that carries a one-shot dialog tree and names its speakers
// (spawners, the player). PostLoad 2013 rva 0x64c730, PostBeginPlay 0x659da0, NotifyOneShotSpeakers 2012 0x69d4f0,
// InitSpeaker 0x647600, InitDialogTreeInternal 2012 0x698850 (editor), Spawned 2012 0x691c40, GetOwnerForSpeaker 2012
// 0x698920, GetOwnerActorForSpeaker(_Derived) 2012 0x691c90 / 0x691ca0. Bodies in disdialogoneshot.cpp.
public:
	virtual void PostLoad();
	virtual void PostBeginPlay();
	virtual void Spawned();
	void NotifyOneShotSpeakers();
	class IDisConvOneShotOwnerInterface* GetOwnerForSpeaker( struct FDisOneOffSpeaker& _rSpeakerStruct );
	AActor* GetOwnerActorForSpeaker( struct FDisOneOffSpeaker& _rSpeakerStruct ) { return GetOwnerActorForSpeaker_Derived( _rSpeakerStruct ); }
protected:
	virtual AActor* GetOwnerActorForSpeaker_Derived( struct FDisOneOffSpeaker& _rSpeakerStruct );
	virtual void InitDialogTreeInternal();
	void InitSpeaker( struct FDisOneOffSpeaker& _rSpeaker );
