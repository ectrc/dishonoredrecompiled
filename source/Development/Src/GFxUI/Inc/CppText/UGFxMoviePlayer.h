// UGFxMoviePlayer cpptext: included inside the generated class body (GFxUIClasses.h). Bodies in
// Src/gfxuimovie.cpp, where the 2012 PDB attributes them
// (v:\dishonored\unrealengine3qatest\development\src\gfxui\src\gfxuimovie.cpp). Signatures are the PDB's;
// which of them are virtual is the PDB's too, and it matters because UDisGFxMoviePlayerBase overrides
// Start / Advance / PostAdvance / Close / PreLoad / SetPause / SetTimingMode / SetPriority /
// FilterButtonInput / FilterInputAxis.
//
// The elaborated "struct FGFxMovie" / "class GFxValue" below are deliberate: this header is included inside
// the class body, and an elaborated-type-specifier in a member declaration declares the name in the
// enclosing NAMESPACE scope rather than as a nested class, which is how the layer names the seam types
// before the generated GFxUI.h has included gfxui_gfx3.h.
public:
	/** the GFx movie handle; NULL until Load succeeds, and always NULL without a GFx runtime */
	struct FGFxMovie* GetMovie() const { return (struct FGFxMovie*)pMovie; }

	virtual void Cleanup();                                                                                 // 2012 0x5b7010
	virtual void FinishDestroy();                                                                           // 2012 0x5b7030

	UBOOL Load( const FString& Filename, UBOOL bInitFirstFrame );                                           // 2012 0x5e35f0
	virtual UBOOL PreLoad();                                                                                // 2012 0x5e3a30
	virtual UBOOL Start( UBOOL bStartPaused );                                                              // 2012 0x5cf5b0
	virtual void Advance( FLOAT DeltaTime );                                                                // 2012 0x5b9e30
	virtual void PostAdvance( FLOAT DeltaTime );                                                            // 2012 0x5b7100
	virtual void Close( UBOOL bUnload );                                                                    // 2012 0x5cf600
	virtual void SetPause( UBOOL bPausePlayback );                                                          // 2012 0x5b7090
	virtual void SetTimingMode( BYTE Mode );                                                                // 2012 0x5b70b0
	virtual void SetPriority( BYTE NewPriority );                                                           // 2012 0x5e4850

	void SetMovieCanReceiveFocus( UBOOL bCanReceiveFocus );                                                 // 2012 0x5b7050
	void SetMovieCanReceiveInput( UBOOL bCanReceiveInput );                                                 // 2012 0x5b7070
	void AddCaptureKey( FName Key );                                                                        // 2012 0x5d9100
	void ClearCaptureKeys();                                                                                // 2012 inlined in execClearCaptureKeys 0x5d9db0
	void AddFocusIgnoreKey( FName Key );                                                                    // 2012 0x5d91d0
	void ClearFocusIgnoreKeys();                                                                            // 2012 0x5d91a0
	void FlushPlayerInput( UBOOL bCaptureKeysOnly );                                                        // 2012 0x5daeb0

	UGameViewportClient* GetGameViewportClient();                                                           // 2012 inlined in execGetGameViewportClient 0x5b8f20
	void SetViewport( INT X, INT Y, INT Width, INT Height );                                                // 2012 0x5b9d80
	void SetViewScaleMode( BYTE ScaleMode );                                                                // 2012 inlined in execSetViewScaleMode 0x5bdd70
	void SetAlignment( BYTE Align );                                                                        // 2012 inlined in execSetAlignment 0x5bdde0
	void GetVisibleFrameRect( FLOAT& MinX, FLOAT& MinY, FLOAT& MaxX, FLOAT& MaxY );                         // 2012 inlined in execGetVisibleFrameRect 0x5bde50
	void SetView3D( const FMatrix& MatView );                                                               // 2012 inlined in execSetView3D 0x5be030
	void SetPerspective3D( const FMatrix& MatPersp );                                                       // 2012 inlined in execSetPerspective3D 0x5be100
	virtual UBOOL SetExternalTexture( const FString& Resource, UTexture* Texture );                         // 2012 0x5cf740

	virtual FASValue Invoke( const FString& Method, const TArray<FASValue>& Args );                         // 2012 0x5c0550
	UGFxObject* CreateValue( const void* InValue, UClass* Type );                                           // 2012 inlined; the AddRef form is 0x5c0930
	UGFxObject* CreateValueAddRef( const void* InValue, UClass* Type );                                     // 2012 0x5c0930
	virtual UGFxObject* CreateObject( const FString& ASClass, UClass* Type );                               // 2012 0x5e6c20
	virtual UGFxObject* CreateArray();                                                                      // 2012 0x5e6d70

	virtual FASValue GetVariable( const FString& Path );                                                    // 2012 0x5c0b70
	virtual UBOOL GetVariableBool( const FString& Path );                                                   // 2012 0x5c0d30
	virtual FLOAT GetVariableNumber( const FString& Path );                                                 // 2012 0x5c0e60
	virtual FString GetVariableString( const FString& Path );                                               // 2012 0x5c0f90
	virtual UGFxObject* GetVariableObject( const FString& Path, UClass* Type );                             // 2012 0x5e6aa0
	virtual void SetVariable( const FString& Path, FASValue Arg );                                          // 2012 0x5c1180
	virtual void SetVariableBool( const FString& Path, UBOOL B );                                           // 2012 0x5c1300
	virtual void SetVariableNumber( const FString& Path, FLOAT F );                                         // 2012 0x5c1420
	virtual void SetVariableString( const FString& Path, const FString& S );                                // 2012 0x5c1540
	virtual void SetVariableObject( const FString& Path, UGFxObject* Value );                               // 2012 0x5c0a00

	virtual UBOOL GetVariableArray( const FString& Path, INT Index, TArray<FASValue>& Out );                // 2012 0x5d3670
	virtual UBOOL GetVariableIntArray( const FString& Path, INT Index, TArray<INT>& Out );                  // 2012 0x5cb2c0
	virtual UBOOL GetVariableFloatArray( const FString& Path, INT Index, TArray<FLOAT>& Out );              // 2012 0x5cb4a0
	virtual UBOOL GetVariableStringArray( const FString& Path, INT Index, TArray<FString>& Out );           // 2012 0x5d3930
	virtual UBOOL SetVariableArray( const FString& Path, INT Index, const TArray<FASValue>& In );           // 2012 0x5c1690
	virtual UBOOL SetVariableIntArray( const FString& Path, INT Index, const TArray<INT>& In );             // 2012 0x5c1970
	virtual UBOOL SetVariableFloatArray( const FString& Path, INT Index, const TArray<FLOAT>& In );         // 2012 0x5c1aa0
	virtual UBOOL SetVariableStringArray( const FString& Path, INT Index, const TArray<FString>& In );      // 2012 0x5c1bd0

	void SetWidgetPathBinding( UGFxObject* WidgetToBind, FName Path );                                      // 2012 inlined in execSetWidgetPathBinding 0x5dd060
	virtual void RefreshDataStoreBindings();                                                                // 2012 0x5dd210
	virtual void PublishDataStoreValues();                                                                  // 2012 0x5b6b30
	void ProcessDataStoreCall( const char* MethodName, const class GFxValue* Args, INT ArgCount );                // 2012 0x5cd5c0

	virtual UBOOL FilterButtonInput( INT ControllerId, FName Key, BYTE Event, UBOOL& bHandled );            // 2012 0x5deb60
	virtual UBOOL FilterInputAxis( INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad, UBOOL& bHandled ); // 2012 0x5deb40
