// UDisAISubProcessHeadTrackPause cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent GE (PHASE19 GE). A walking NPC whose head-track target is behind him stops and turns toward it
// (a loco desire to his own position, a face-to desire past his back), and resumes once the target is within the front
// half-angle of where he was going. Bodies in Src/disaisubprocessheadtrackpause.cpp.
private:
	virtual void BeginSubProcess_Derived();
	virtual void TickSubProcess_Derived( const FLOAT _fDeltaTime );
