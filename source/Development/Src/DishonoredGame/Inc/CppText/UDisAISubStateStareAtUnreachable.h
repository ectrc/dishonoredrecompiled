// UDisAISubStateStareAtUnreachable cpptext: included inside the generated class body (DishonoredGameSearchClasses.h).
// DISHONORED(written): agent DF. What a guard does when it knows where you are and cannot get to you: two desires, both
// stated once, and nothing else. Bodies in Src/disaisubstatestareatunreachable.cpp.
public:
	virtual void BeginSubState_Derived();
	virtual struct FDisFaceToRequest* GetDesiresFaceToRequest() { return &m_FaceToRequest; }
	virtual struct FDisLookAtRequest* GetDesiresLookAtRequest() { return &m_LookAtRequest; }
