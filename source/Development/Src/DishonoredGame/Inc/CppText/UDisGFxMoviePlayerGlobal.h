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
