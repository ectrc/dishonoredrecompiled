// UDisUseState_Base cpptext: included inside the generated class body (DishonoredGamePlayerUseClasses.h).
// DISHONORED(port): agent GD (PHASE19 GD). The base of the player's use-interaction FSM states has no bodies of its own in
// retail (2012 0x722240 / 0x72abb0 / 0x72db00 are the class statics); m_pPlayerController is written by the state parameters'
// OnPending (Inc/disusestate.h). The states' bodies are in disusestate_*.cpp.
public:
	ADishonoredPlayerController* GetPlayerController() const { return m_pPlayerController; }
