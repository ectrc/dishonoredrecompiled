// UDisGFxMoviePlayerGlobal cpptext: included inside the generated class body.

// ---- agent EI (PHASE12 EI): the one movie the message box lives in ----
public:
	// DISHONORED(port): agent EI, 2013 rva 0x7aa0c0. Inserts by priority, hands back the new id and raises the
	// box when it lands at the head of the queue. Bodies in disgfxmovieplayerglobal.cpp.
	void AddMessageBox( const struct FDisMsgBoxInfo& _rInfo, INT& _rOutID, UINT _Priority );
	// DISHONORED(port): agent EI, 2013 rva 0x7aa230
	void RemoveMessageBox( INT _ID );
	// DISHONORED(port): agent EI, 2013 rva 0x794400
	void AddMessageBoxTimer( INT _ID, FLOAT _fDuration );
	// DISHONORED(port): agent EI, 2013 rva 0x7abbd0, reached from exec 0x5f7300. The content's answer: pop the
	// queue, and unless the id is one of the manager's own, raise DisGameEventType_MessageBoxResult.
	virtual void OnMessageBoxConfirm( INT _SelectedIndex );

protected:
	// DISHONORED(port): agent EI, 2013 rva 0x7946b0 - four GFxValue strings and one Invoke on this movie's view
	void ShowMessageBox( const struct FDisMsgBoxInfo& _rInfo );
	// DISHONORED(port): agent EI, 2013 rva 0x78c920
	void HideCurrentMessageBox();
	// DISHONORED(port): agent EI, 2013 rva 0x7a5040 - the movie attributes a visible box implies
	void UpdateMessageBoxAttributes( UBOOL _bMessageBoxUp );
	// DISHONORED(port): agent EI, 2013 rva 0x79ec60 - the focus and input this movie needs while a box is up
	void RefreshMessageBoxFocus();

	// ---- agent FP (PHASE16 FP): UI_Global's own letterbox ----
	// DISHONORED(port): the stripes arm of UDisGFxMoviePlayerGlobal::PreAdvance (2013 rva 0x7aa610, retail vtable
	// +476), which arrives through the GFxUI PreAdvance seam like the HUD's. Body in disgfxmovieplayerglobal.cpp.
	friend void DisGFxMoviePlayerGlobalPreAdvance( class UDisGFxMoviePlayerGlobal* Global, FLOAT DeltaTime );

// ---- agent FU (PHASE17 FU): the mouse cursor, which is this movie's `mouseCursor` symbol ----
public:
	// DISHONORED(port): 2013 rva 0x7aa430 (2012 0x7f4640), retail vtable PostStart: m_pMouseCursor is a fresh GFxValue,
	// the online delegates are registered, the view advanced once and a queued message box re-shown. Reached through
	// the GFxUI Started seam.
	void PostStart();
	// DISHONORED(port): 2013 rva 0x7aa4d0 (2012 0x802e60): the stripes latch cleared, the cursor clip removed and the
	// GFxValue deleted while the view is alive, the online delegates unregistered, the priority-0 message boxes dropped.
	// Reached through the GFxUI PreClose seam.
	void PreClose( UBOOL _bUnload );
	// DISHONORED(port): 2013 rva 0x79eb90 (2012 0x802c60), from UDishonoredViewportClient::Draw: hide the cursor
	// unless m_bMouseCursorAllowed and not closing, else attach it and move it to the mouse in movie space.
	void PreRender( class FViewport* _pViewport );
	// DISHONORED(port): 2013 rva 0x79eb30 (2012 0x802c50): the cursor hidden and the message-box queue shrunk
	void PreCommitMapChange();
protected:
	// DISHONORED(port): 2013 rva 0x78c790 (2012 0x7f6070): _root.mouseCursor_mc.attachMovie("mouseCursor",
	// "mouseCursorInst", getNextHighestDepth()) once, and the viewport client's bDisplayingUIMouseCursor set
	void ConditionalShowMouseCursor();
	// DISHONORED(port): 2013 rva 0x7945b0 (2012 0x7fa780): removeMovieClip on the attached clip, the handle released
	// and the viewport client's bit cleared
	void ConditionalHideMouseCursor();
	// DISHONORED(port): 2013 rva 0x78c8c0 (2012 0x7f61a0): _x and _y through SetDisplayInfo, nothing else
	void MoveMouseCursor( FVector2D _Position );
