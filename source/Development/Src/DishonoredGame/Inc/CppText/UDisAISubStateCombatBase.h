// UDisAISubStateCombatBase cpptext: included inside the generated class body (DishonoredGameAICombatClasses.h).
// DISHONORED(written): agent DF. The shared half of the melee and wolfhound combat sub-states: the enemy proxy and the
// stance to hold while fighting it. Bodies in Src/disaisubstatecombatbase.cpp.
public:
	/** DISHONORED(port): 2012 rva 0x766e00: asks the brain's attack manager for the pattern to use against this enemy,
	    which is how two guards attacking one target take turns instead of both lunging.
	    DISHONORED(bringup): UDisAISubProcessManageAttacks is one of the four small subsystems agent CG left (agentCG.md
	    blocker table, ~20 natives), so there is no attack manager to ask and the pattern stays the default. */
	BYTE CombatEngageAttackPattern() const;
