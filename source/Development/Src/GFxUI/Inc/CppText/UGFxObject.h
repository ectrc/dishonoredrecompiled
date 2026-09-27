// UGFxObject cpptext: included inside the generated class body (GFxUIClasses.h). The bodies are in
// Src/gfxuimovie.cpp, which is where the 2012 PDB attributes them
// (v:\dishonored\unrealengine3qatest\development\src\gfxui\src\gfxuimovie.cpp).
// UGFxObject wraps one GFxValue, kept inline in the reflected INT Value[12]; every method below is a
// guarded forwarder onto GFxValue::ObjectInterface. Names and signatures are the 2012 PDB's.
//
// GetObject is spelled as retail spells it: on a UNICODE Win32 build windows.h rewrites it to GetObjectW,
// which is exactly the symbol the PDB carries (2012 rva 0x5e6e30), and the declaration and every call site
// are rewritten together.
//
// The elaborated "class GFxValue" below is deliberate: this header is included inside the class body, and an
// elaborated-type-specifier in a member declaration declares the name in the enclosing NAMESPACE scope, not
// as a nested class - which is how the layer names a GFx type the generated GFxUI.h has not included yet.
public:
	/** the GFxValue this object wraps, living in the reflected Value[12] */
	class GFxValue* GetASValue() const { return (class GFxValue*)Value; }
	/** take ownership of a value the movie produced (adds the managed reference) */
	void SetValue( const void* InValue );

	virtual void BeginDestroy();                                                                    // 2012 0x5b71b0
	virtual void Clear();                                                                           // 2012 0x5b9e80

	FASValue Invoke( const FString& Member, const TArray<FASValue>& Args );                         // 2012 0x5c1d80
	void GotoAndPlay( const FString& Frame );                                                       // 2012 0x5c2210
	void GotoAndStop( const FString& Frame );                                                       // 2012 0x5c22e0
	void GotoAndPlayI( INT Frame );                                                                 // 2012 inlined in execGotoAndPlayI 0x5b93f0
	void GotoAndStopI( INT Frame );                                                                 // 2012 inlined in execGotoAndStopI 0x5b9460

	FASValue Get( const FString& Member );                                                          // 2012 0x5c23b0
	UBOOL GetBool( const FString& Member );                                                         // 2012 0x5c25c0
	FLOAT GetFloat( const FString& Member );                                                        // 2012 0x5c2700
	FString GetString( const FString& Member );                                                     // 2012 0x5c2840
	UGFxObject* GetObject( const FString& Member, UClass* Type );                                   // 2012 0x5e6e30 (GetObjectW)
	void Set( const FString& Member, FASValue Arg );                                                // 2012 0x5c2bb0
	void SetBool( const FString& Member, UBOOL B );                                                 // 2012 0x5c2f40
	void SetFloat( const FString& Member, FLOAT F );                                                // 2012 0x5c3070
	void SetString( const FString& Member, const FString& S, UTranslationContext* InContext );      // 2012 0x5d51f0
	void SetObject( const FString& Member, UGFxObject* Value );                                     // 2012 0x5c2d50
	void SetFunction( const FString& Member, UObject* Context, FName FunctionName );                // 2012 0x5cb8e0

	FString GetText();                                                                              // 2012 0x5c2a50
	void SetText( const FString& S, UTranslationContext* InContext );                               // 2012 0x5d5380

	FASDisplayInfo GetDisplayInfo();                                                                // 2012 0x5b7440
	void SetDisplayInfo( FASDisplayInfo D );                                                        // 2012 0x5b75f0
	FMatrix GetDisplayMatrix();                                                                     // 2012 0x5b74b0
	void SetDisplayMatrix( const FMatrix& M );                                                      // 2012 inlined in execSetDisplayMatrix 0x5b7650
	void SetDisplayMatrix3D( const FMatrix& M );                                                    // 2012 inlined in execSetDisplayMatrix3D 0x5b9f20
	FASColorTransform GetColorTransform();                                                          // 2012 inlined in execGetColorTransform 0x5b91a0
	void SetColorTransform( FASColorTransform Cxform );                                             // 2012 inlined in execSetColorTransform 0x5b9320
	UBOOL GetPosition( FLOAT& X, FLOAT& Y );                                                        // 2012 0x5b7580
	void SetPosition( FLOAT X, FLOAT Y );                                                           // 2012 0x5b9eb0
	void SetVisible( UBOOL bVisible );                                                              // 2012 0x5b9fc0

	UGFxObject* CreateEmptyMovieClip( const FString& InstanceName, INT Depth, UClass* Type );       // 2012 0x5e6fd0
	UGFxObject* AttachMovie( const FString& SymbolName, const FString& InstanceName, INT Depth, UClass* Type ); // 2012 0x5e7150

	FASValue GetElement( INT Index );                                                               // 2012 0x5c31a0
	UBOOL GetElementBool( INT Index );                                                              // 2012 0x5ba0d0
	FLOAT GetElementFloat( INT Index );                                                             // 2012 0x5ba1a0
	FString GetElementString( INT Index );                                                          // 2012 0x5c32f0
	UGFxObject* GetElementObject( INT Index, UClass* Type );                                        // 2012 0x5e7370
	void SetElement( INT Index, FASValue Arg );                                                     // 2012 0x5c3460
	void SetElementBool( INT Index, UBOOL B );                                                      // 2012 0x5c3570
	void SetElementFloat( INT Index, FLOAT F );                                                     // 2012 0x5ba260
	void SetElementString( INT Index, const FString& S );                                           // 2012 0x5ba2f0
	void SetElementObject( INT Index, UGFxObject* Value );                                          // 2012 0x5ba020
	void SetElementVisible( INT Index, UBOOL bVisible );                                            // 2012 0x5ba3c0
	void SetElementPosition( INT Index, FLOAT X, FLOAT Y );                                         // 2012 0x5ba4a0
	void SetElementColorTransform( INT Index, FASColorTransform Cxform );                           // 2012 0x5ba580
	FASDisplayInfo GetElementDisplayInfo( INT Index );                                              // 2012 0x5ba680
	void SetElementDisplayInfo( INT Index, FASDisplayInfo D );                                      // 2012 0x5ba8c0
	FMatrix GetElementDisplayMatrix( INT Index );                                                   // 2012 0x5ba770
	void SetElementDisplayMatrix( INT Index, const FMatrix& M );                                    // 2012 0x5ba990

	FASValue GetElementMember( INT Index, const FString& Member );                                  // 2012 0x5c3600
	UBOOL GetElementMemberBool( INT Index, const FString& Member );                                 // 2012 0x5c3830
	FLOAT GetElementMemberFloat( INT Index, const FString& Member );                                // 2012 0x5c39c0
	FString GetElementMemberString( INT Index, const FString& Member );                             // 2012 0x5c3b50
	UGFxObject* GetElementMemberObject( INT Index, const FString& Member, UClass* Type );           // 2012 0x5e7470
	void SetElementMember( INT Index, const FString& Member, FASValue Arg );                        // 2012 0x5c3d20
	void SetElementMemberBool( INT Index, const FString& Member, UBOOL B );                         // 2012 0x5c4030
	void SetElementMemberFloat( INT Index, const FString& Member, FLOAT F );                        // 2012 0x5c4190
	void SetElementMemberString( INT Index, const FString& Member, const FString& S );              // 2012 0x5c42e0
	void SetElementMemberObject( INT Index, const FString& Member, UGFxObject* Value );             // 2012 0x5c3ee0
