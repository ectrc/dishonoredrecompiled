// FDisLookAtInfluence cpptext: included inside the generated struct body (dishonoredgameclasses.h) through the struct
// cpptext hook agent CG added to gen_classes_header.py.
// DISHONORED(written): agent DF. How much of a look-at is carried by the head and how much by the torso. The four named
// combinations are process-wide constants; retail stores their float pair statically and only the bitfield through a
// dynamic initialiser (2012 rvas 0xbaae80 Eyes, 0xbaae90 Head, 0xbaaea0 Torso, 0xbaaeb0 TorsoSpeedIndependent), which is
// why the values below are read off the .data image rather than out of code:
//   Eyes                   { 0.0, 0.0, TRUE  }  (2012 rva 0x1010c74, uninitialised .data, so both influences are zero)
//   Head                   { 1.0, 0.0, TRUE  }  (0xe369a4)
//   Torso                  { 1.0, 1.0, TRUE  }  (0xe369b0)
//   TorsoSpeedIndependent  { 1.0, 1.0, FALSE }  (0xe369bc)
public:
	// DISHONORED(port): 2013 rva 0x8a7d20 (2012 0x8f8da0)
	FDisLookAtInfluence( FLOAT _fHeadInfluence, FLOAT _fTorsoInfluence, UBOOL _bTorsoInfIsMoveSpeedDependant );
	// DISHONORED(port): 2013 rva 0x8a7d50 (2012 0x8f8dd0)
	UBOOL operator!=( const FDisLookAtInfluence& _rLookAtInfluence ) const;

	static const FDisLookAtInfluence Eyes;
	static const FDisLookAtInfluence Head;
	static const FDisLookAtInfluence Torso;
	static const FDisLookAtInfluence TorsoSpeedIndependent;
