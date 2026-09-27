// UDisBlindedPpController cpptext: included inside the generated class body (DishonoredGamePostProcessClasses.h).
// DISHONORED(port): Tick 2013 rva 0x7e7b00; Update (2013 0x7ea620) is not ported - see
// dispostprocesscontrollers.cpp. This class does not override IsShown, so retail shows its node whenever the node
// itself is shown, and the node the content gives it (TEST_PPG_Blinded_INST) is m_bShowInGame 0.
public:
	virtual UBOOL Tick(FLOAT DeltaTime,enum ELevelTick TickType);
