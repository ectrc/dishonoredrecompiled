// UGFxInteraction cpptext: included inside the generated class body (GFxUIClasses.h). Bodies in
// Src/gfxuinatives.cpp; retail has them in gfxui/src/gfxuiinteraction.cpp, whose input pipeline
// (Init, InputKey, InputAxis, InputChar, Exec, Tick) belongs to the runtime-glue package.
public:
	virtual void CloseAllMoviePlayers();                                                      // 2012 0x5d22b0
	virtual void NotifyGameSessionEnded();                                                    // 2012 0x5d4f30
	virtual void NotifyPlayerAdded( INT PlayerIndex, class ULocalPlayer* AddedPlayer );        // 2012 gfxuiinteraction.cpp
	virtual void NotifyPlayerRemoved( INT PlayerIndex, class ULocalPlayer* RemovedPlayer );    // 2012 gfxuiinteraction.cpp
	virtual class UGFxMoviePlayer* GetFocusMovie( INT ControllerId );                          // 2012 0x5caf40
