// UDishonoredViewportClient cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): retail vtables 2013 rva 0xcd8278 (UObject), 0xcd8200 (FViewportClient), 0xcd81fc (FExec).
// ApplyListenerLocationModifier is a UGameViewportClient virtual in retail (+344, empty base 2013 rva 0x1cb0c0) that our
// UGameViewportClient lacks, so the audio listener code cannot reach it yet; PostRender_Native (+348) is new here.
public:
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );
	virtual void Draw( FViewport* Viewport, FCanvas* Canvas );
	virtual void ApplyListenerLocationModifier( FVector& ListenerLocation );
	virtual void PostRender_Native( UCanvas* Canvas );
	void SetListenerLocationOverride( FVector Location );
	void EnableListenerLocationOverride( UBOOL bEnable );
