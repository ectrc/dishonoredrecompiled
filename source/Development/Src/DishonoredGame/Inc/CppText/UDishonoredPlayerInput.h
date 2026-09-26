// UDishonoredPlayerInput cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): retail vtable slots +352 GetFOVScale, +356 Dis_SneakOrSlide, +360 Dis_Jump_ButtonDown,
// +364 Dis_PlayerChoice_RequestSkip, +368 Dis_PlayerChoice_RequestSkip_Released (2013 rvas in dishonoredplayerinput.cpp).
public:
	virtual void Dis_Jump_ButtonDown();
	virtual void PlayerInput( FLOAT DeltaTime );
	void OnInit( ADishonoredPlayerController* Owner );
	// DISHONORED(written): the binding half of ApplyGameSettings (2013 rva 0x6bd610): AddBindingSet 0x6b84f0,
	// TranslateBindingSet 0x6afe50
	void AddBindingSet( const TArrayNoInit<FKeyBind>& Set );
	void BuildBindings();
