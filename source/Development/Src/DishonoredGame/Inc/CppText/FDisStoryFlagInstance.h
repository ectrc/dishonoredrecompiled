// FDisStoryFlagInstance cpptext: included inside the generated struct body.

	// DISHONORED(port): agent EL, 2013 rvas 0x7febc0, 0x801ab0 and 0x801b00. Bodies in disstoryflagset.cpp,
	// which is the unit the 2012 PDB attributes all three to.
	void BuildStoryFlagInstance( const FName& _rStoryFlagSetPath, const FGuid& _rGUID );
	UBOOL MatchesStoryFlagInstance( const FName& _rStoryFlagSetPath, const FGuid& _rGUID ) const;
	UBOOL CheckStoryFlagValue( const FName& _rStoryFlagSetPath, const FGuid& _rGUID ) const;
