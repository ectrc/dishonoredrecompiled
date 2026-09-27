// UDisTweaks_Attributes cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent BF. Bodies in distweaks_attributes.cpp: ConstructAttributes 2013 rva 0x88aca0
// (2012 0x8f85f0), RefreshAttributesFromSource (2012 rva 0x8f8130, 496 bytes; unmatched in the 2013 symbol set,
// reached from ADishonoredPawn::PreBeginPlay_Attributes 0x762190 and OnDifficultyChange).
//
// DISHONORED(layout): the class adds NO members of its own - retail sizeof 140 is UDisTweaksBase's own 140, and the
// 2012 PDB type agrees. Agent AU's hand-over warned that the refresh "reads native members the generated class does not
// have"; it does not. RefreshAttributesFromSource is entirely reflection-driven: it walks its own Class with a
// TFieldIterator<UStructProperty> and collects every property whose struct is DisAttribute or DisAttribute_RangeLimits
// out of the *subclass* (UDisTweaks_Pawn_Attributes has 70-odd of them, all generated), so nothing native is read.
public:
	/** 2013 rva 0x88aca0: construct a UDisAttributes outered to Outer and fill it from this tweak object. */
	class UDisAttributes* ConstructAttributes( UObject* Outer ) const;

	/** 2012 rva 0x8f8130: refresh Attributes from this object's own reflected FDisAttribute properties. */
	void RefreshAttributesFromSource( class UDisAttributes* Attributes, UObject* const Outer ) const;
