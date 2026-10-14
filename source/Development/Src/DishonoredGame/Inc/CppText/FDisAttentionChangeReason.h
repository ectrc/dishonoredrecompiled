// FDisAttentionChangeReason cpptext: included inside the generated struct body (dishonoredgameclasses.h) through the struct
// cpptext hook agent CG added to gen_classes_header.py.
// DISHONORED(port): agent GI (PHASE20 GI). Why an attention value moved: the reason type and the (actor, two ints) that
// qualify it. 2012 rvas 0x745d90 (default: DACRT_Drain, no actor), 0x745da0 (a type), 0x745dc0 (a type and its info); 2013
// 0x700c00 is the one-argument form (the other two are inlined there). Bodies in Src/disaibrainprocessattention.cpp.
public:
	explicit FDisAttentionChangeReason( EDisAttentionChangeReasonType _ReasonType );
	FDisAttentionChangeReason( EDisAttentionChangeReasonType _ReasonType, const struct FDisAttentionChangeReasonInfo& _rReasonInfo );
	static FDisAttentionChangeReason Drain();
