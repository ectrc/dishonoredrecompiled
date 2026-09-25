/*=============================================================================
	EdgeAnimEvaluate.cpp: the Sony Edge animation evaluator as Dishonored ships it, in plain C++.
	DISHONORED(port): retail 2013 links Sony's x86 SSE Edge build (2012 PDB module edge, sources
	edge/src/sse/edgeanim_evaluate_*_int.cpp); every function below names its 2013 rva (2012 rva) and
	keeps the retail operation order so the results match the retail evaluator bit for bit on the same CPU
	(verified by source/Tests/EdgeAnimSmoke against the retail code itself). Data layout: resources/docs/edgeanim.md.
=============================================================================*/

#include "Core.h"
#include "EdgeAnim.h"

#include <xmmintrin.h>

namespace
{

const BYTE* SelfRelative(const DWORD& Field)
{
	return Field ? (const BYTE*)&Field + Field : NULL;
}

DWORD AlignUp(DWORD Value, DWORD Alignment)
{
	return (Value + Alignment - 1) & ~(Alignment - 1);
}

WORD LoadBE16(const BYTE* Data)
{
	return (WORD)((Data[0] << 8) | Data[1]);
}

DWORD LoadBE32(const BYTE* Data)
{
	return ((DWORD)Data[0] << 24) | ((DWORD)Data[1] << 16) | ((DWORD)Data[2] << 8) | Data[3];
}

FLOAT LoadLEFloat(const BYTE* Data)
{
	FLOAT Value;
	appMemcpy(&Value, Data, sizeof(Value));
	return Value;
}

FLOAT FloatFromBits(DWORD Bits)
{
	FLOAT Value;
	appMemcpy(&Value, &Bits, sizeof(Value));
	return Value;
}

// SSE shifts by a register count give 0 for counts of 32 and above (the retail code relies on it)
DWORD ShiftLeft(DWORD Value, DWORD Count)
{
	return Count >= 32 ? 0 : Value << Count;
}

DWORD ShiftRight(DWORD Value, DWORD Count)
{
	return Count >= 32 ? 0 : Value >> Count;
}

FLOAT SseSqrt(FLOAT Value)
{
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(Value)));
}

FLOAT SseRsqrt(FLOAT Value)
{
	return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(Value)));
}

// rcpps + one Newton-Raphson step, as the retail code writes it: (1 - r*d)*r + r
FLOAT SseReciprocal(FLOAT Value)
{
	const FLOAT Estimate = _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(Value)));
	return (1.0f - Estimate * Value) * Estimate + Estimate;
}

FLOAT SseMin(FLOAT A, FLOAT B)
{
	return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(A), _mm_set_ss(B)));
}

FLOAT SseMax(FLOAT A, FLOAT B)
{
	return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(A), _mm_set_ss(B)));
}

/** Big-endian bit stream, most significant bit first (the SPU data order the x86 build byte swaps on load). */
struct FEdgeBitReader
{
	const BYTE* Data;
	DWORD BitOffset;

	FEdgeBitReader(const BYTE* InData, DWORD InBitOffset)
		: Data(InData)
		, BitOffset(InBitOffset)
	{
	}

	DWORD Read(DWORD NumBits)
	{
		if (NumBits == 0)
		{
			return 0;
		}
		const BYTE* Byte = Data + (BitOffset >> 3);
		const DWORD FirstBit = BitOffset & 7;
		const DWORD NumBytes = (FirstBit + NumBits + 7) >> 3;
		QWORD Window = 0;
		for (DWORD Index = 0; Index < NumBytes; Index++)
		{
			Window |= (QWORD)Byte[Index] << (56 - 8 * Index);
		}
		BitOffset += NumBits;
		return (DWORD)((Window << FirstBit) >> (64 - NumBits));
	}

	UBOOL Test(DWORD Bit) const
	{
		const DWORD Position = BitOffset + Bit;
		return (Data[Position >> 3] >> (7 - (Position & 7))) & 1;
	}
};

/** One packing spec DWORD: per component sign (1), exponent (4) and mantissa (5) bit counts, low 2 bits = omitted quaternion component. */
struct FEdgePackingSpec
{
	DWORD SignBits[3];
	DWORD ExponentBits[3];
	DWORD MantissaBits[3];
	FLOAT InvScale[3];
	DWORD OmittedComponent;
	DWORD KeyBits;

	explicit FEdgePackingSpec(DWORD Spec)
	{
		SignBits[0] = Spec >> 31;
		ExponentBits[0] = (Spec >> 27) & 15;
		MantissaBits[0] = (Spec >> 22) & 31;
		SignBits[1] = (Spec >> 21) & 1;
		ExponentBits[1] = (Spec >> 17) & 15;
		MantissaBits[1] = (Spec >> 12) & 31;
		SignBits[2] = (Spec >> 11) & 1;
		ExponentBits[2] = (Spec >> 7) & 15;
		MantissaBits[2] = (Spec >> 2) & 31;
		OmittedComponent = Spec & 3;
		KeyBits = 0;
		for (INT Component = 0; Component < 3; Component++)
		{
			KeyBits += SignBits[Component] + ExponentBits[Component] + MantissaBits[Component];
			InvScale[Component] = MantissaBits[Component] ? SseReciprocal((FLOAT)(INT)ShiftRight(0xFFFFFFFF, 32 - MantissaBits[Component])) : 0.0f;
		}
	}

	FLOAT DecodeComponent(INT Component, FEdgeBitReader& Reader) const
	{
		const DWORD Sign = Reader.Read(SignBits[Component]);
		const DWORD Exponent = Reader.Read(ExponentBits[Component]);
		const DWORD Mantissa = Reader.Read(MantissaBits[Component]);
		if (ExponentBits[Component] == 0)
		{
			const INT Fixed = (INT)(Mantissa | (Sign ? ShiftLeft(0xFFFFFFFF, MantissaBits[Component]) : 0));
			return (FLOAT)Fixed * InvScale[Component];
		}
		const DWORD Bias = 128 - (1 << (ExponentBits[Component] - 1));
		return FloatFromBits((Sign << 31) | ((Exponent + Bias) << 23) | ShiftLeft(Mantissa, 23 - MantissaBits[Component]));
	}

	void DecodeKey(const BYTE* Data, DWORD BitOffset, FLOAT Out[3]) const
	{
		FEdgeBitReader Reader(Data, BitOffset);
		for (INT Component = 0; Component < 3; Component++)
		{
			Out[Component] = DecodeComponent(Component, Reader);
		}
	}
};

/** The stored three components fill the non-omitted slots in order, the omitted one is rebuilt from the unit length. */
void PlaceQuaternion(DWORD OmittedComponent, const FLOAT Stored[3], FLOAT Rebuilt, FLOAT Out[4])
{
	INT StoredIndex = 0;
	for (DWORD Component = 0; Component < 4; Component++)
	{
		Out[Component] = Component == OmittedComponent ? Rebuilt : Stored[StoredIndex++];
	}
}

FLOAT RebuildClamped(const FLOAT Stored[3])
{
	return SseSqrt(SseMin(SseMax(((1.0f - Stored[0] * Stored[0]) - Stored[1] * Stored[1]) - Stored[2] * Stored[2], 0.0f), 1.0f));
}

void WriteVector(FEdgeAnimVec4& Out, FLOAT X, FLOAT Y, FLOAT Z, FLOAT W)
{
	Out.X = X;
	Out.Y = Y;
	Out.Z = Z;
	Out.W = W;
}

FEdgeAnimVec4& OutputSlot(EdgeAnimJointTransform* Joints, WORD Joint, INT Slot)
{
	return (&Joints[Joint].rotation)[Slot];
}

FLOAT SinPolynomial(FLOAT Angle)
{
	const FLOAT Square = Angle * Angle;
	return ((((((Square * 2.7526000e-06f + -1.9840900e-04f) * Square) + 8.3333319e-03f) * Square + -1.6666667e-01f) * Square) + 1.0f) * Angle;
}

/**
 * The slerp of __edgeAnimEvaluateBitPacked (2013 rva 0x5ce380): acos by the Abramowitz-Stegun polynomial on rsqrtps,
 * sin by a degree-9 polynomial, 1/sin by rcpps + Newton, linear weights above a cosine of 0.999.
 */
void SlerpQuaternion(const FLOAT Left[4], const FLOAT Right[4], FLOAT T, FLOAT Out[4])
{
	const FLOAT Dot = ((Right[1] * Left[1] + Right[0] * Left[0]) + Right[2] * Left[2]) + Right[3] * Left[3];
	const FLOAT Sign = Dot < 0.0f ? -1.0f : 1.0f;
	const FLOAT Cosine = Sign * Dot;
	const FLOAT OneMinusT = 1.0f - T;
	const FLOAT InvRoot = SseRsqrt(1.0f - Cosine);
	const FLOAT Angle = (1.5707963f - (0.2145988f - (0.088978991f - (Cosine * 0.050174303f) * Cosine) * Cosine) * Cosine) * (InvRoot - InvRoot * Cosine);
	const FLOAT InvSin = SseReciprocal(SinPolynomial(Angle));
	FLOAT LeftWeight;
	FLOAT RightWeight;
	if (0.99900001f < Cosine)
	{
		LeftWeight = OneMinusT;
		RightWeight = T;
	}
	else
	{
		LeftWeight = SinPolynomial(OneMinusT * Angle) * InvSin;
		RightWeight = SinPolynomial(T * Angle) * InvSin;
	}
	LeftWeight = LeftWeight * Sign;
	for (INT Component = 0; Component < 4; Component++)
	{
		Out[Component] = RightWeight * Right[Component] + LeftWeight * Left[Component];
	}
}

/** Intra-frame key selection shared by the animated decoders (the popcount / bit-scan part of R, ST and BitPacked). */
struct FEdgeKeySelection
{
	UBOOL bLeftIsInitial;
	UBOOL bRightIsFinal;
	DWORD LeftIntraKey;
	DWORD RightIntraKey;
	FLOAT T;
};

struct FEdgeChannelStream
{
	const BYTE* IntraBits;
	DWORD IntraBitsOffset;
	DWORD IntraFrameCount;
	DWORD FrameInteger;
	FLOAT FrameFraction;

	DWORD CountKeys(DWORD Channel) const
	{
		FEdgeBitReader Bits(IntraBits, IntraBitsOffset + Channel * IntraFrameCount);
		DWORD Count = 0;
		for (DWORD Frame = 0; Frame < IntraFrameCount; Frame++)
		{
			Count += Bits.Test(Frame);
		}
		return Count;
	}

	FEdgeKeySelection Select(DWORD Channel, DWORD FirstIntraKey) const
	{
		FEdgeBitReader Bits(IntraBits, IntraBitsOffset + Channel * IntraFrameCount);
		FEdgeKeySelection Selection;
		DWORD KeysBefore = 0;
		INT LeftFrame = 0;
		for (DWORD Frame = 0; Frame < FrameInteger && Frame < IntraFrameCount; Frame++)
		{
			if (Bits.Test(Frame))
			{
				KeysBefore++;
				LeftFrame = Frame + 1;
			}
		}
		INT RightFrame = IntraFrameCount + 1;
		for (DWORD Frame = FrameInteger; Frame < IntraFrameCount; Frame++)
		{
			if (Bits.Test(Frame))
			{
				RightFrame = Frame + 1;
				break;
			}
		}
		Selection.bLeftIsInitial = KeysBefore == 0;
		Selection.bRightIsFinal = RightFrame == (INT)IntraFrameCount + 1;
		Selection.LeftIntraKey = FirstIntraKey + KeysBefore - 1;
		Selection.RightIntraKey = FirstIntraKey + KeysBefore;
		const FLOAT FromLeft = (FLOAT)((INT)FrameInteger - LeftFrame);
		const FLOAT ToRight = (FLOAT)(RightFrame - (INT)FrameInteger);
		Selection.T = (FromLeft + FrameFraction) / (ToRight + FromLeft);
		return Selection;
	}
};

/** __edgeAnimEvaluateBitPackedConst, 2013 rva 0x5cd3a0 (2012 0x615050). One spec for every constant channel. */
void EvaluateBitPackedConst(DWORD ConstCount, const BYTE* ConstData, const WORD* ConstTable, DWORD Spec, EdgeAnimJointTransform* Out, INT Slot, UBOOL bIsRotation)
{
	const FEdgePackingSpec Packing(Spec);
	for (DWORD Channel = 0; Channel < ConstCount; Channel++)
	{
		FLOAT Stored[3];
		Packing.DecodeKey(ConstData, Channel * Packing.KeyBits, Stored);
		FEdgeAnimVec4& Target = OutputSlot(Out, ConstTable[Channel], Slot);
		if (bIsRotation)
		{
			FLOAT Quat[4];
			PlaceQuaternion(Packing.OmittedComponent, Stored, RebuildClamped(Stored), Quat);
			WriteVector(Target, Quat[0], Quat[1], Quat[2], Quat[3]);
		}
		else
		{
			WriteVector(Target, Stored[0], Stored[1], Stored[2], 1.0f);
		}
	}
}

/** __edgeAnimEvaluateBitPacked, 2013 rva 0x5ce380 (2012 0x616030). One spec per animated channel. */
void EvaluateBitPacked(DWORD AnimCount, const WORD* AnimTable, const DWORD* Specs, const BYTE* Initial, const BYTE* Intra, const BYTE* Final, const FEdgeChannelStream& Stream, EdgeAnimJointTransform* Out, INT Slot, UBOOL bIsRotation)
{
	DWORD KeyBitOffset = 0;
	DWORD IntraBitOffset = 0;
	for (DWORD Channel = 0; Channel < AnimCount; Channel++)
	{
		const FEdgePackingSpec Packing(Specs[Channel]);
		const FEdgeKeySelection Selection = Stream.Select(Channel, 0);
		FLOAT Left[3];
		FLOAT Right[3];
		if (Selection.bLeftIsInitial)
		{
			Packing.DecodeKey(Initial, KeyBitOffset, Left);
		}
		else
		{
			Packing.DecodeKey(Intra, IntraBitOffset + Selection.LeftIntraKey * Packing.KeyBits, Left);
		}
		if (Selection.bRightIsFinal)
		{
			Packing.DecodeKey(Final, KeyBitOffset, Right);
		}
		else
		{
			Packing.DecodeKey(Intra, IntraBitOffset + Selection.RightIntraKey * Packing.KeyBits, Right);
		}
		FEdgeAnimVec4& Target = OutputSlot(Out, AnimTable[Channel], Slot);
		if (bIsRotation)
		{
			FLOAT LeftQuat[4];
			FLOAT RightQuat[4];
			FLOAT Quat[4];
			PlaceQuaternion(Packing.OmittedComponent, Left, RebuildClamped(Left), LeftQuat);
			PlaceQuaternion(Packing.OmittedComponent, Right, RebuildClamped(Right), RightQuat);
			SlerpQuaternion(LeftQuat, RightQuat, Selection.T, Quat);
			WriteVector(Target, Quat[0], Quat[1], Quat[2], Quat[3]);
		}
		else
		{
			const FLOAT OneMinusT = 1.0f - Selection.T;
			WriteVector(Target,
				Selection.T * Right[0] + OneMinusT * Left[0],
				Selection.T * Right[1] + OneMinusT * Left[1],
				Selection.T * Right[2] + OneMinusT * Left[2],
				1.0f);
		}
		KeyBitOffset += Packing.KeyBits;
		IntraBitOffset += Stream.CountKeys(Channel) * Packing.KeyBits;
	}
}

/** 48-bit rotation: BE16 a (15 bits), BE32 b:15 c:15 omitted:2; components map [0, 32767] onto [-1/sqrt2, 1/sqrt2]. */
void DecodeSmallestThree(const BYTE* Key, FLOAT Stored[3], DWORD& OmittedComponent)
{
	const DWORD Low = LoadBE32(Key + 2);
	Stored[0] = (FLOAT)(INT)LoadBE16(Key) * 4.3159689e-05f + -0.70710677f;
	Stored[1] = (FLOAT)(INT)(Low >> 17) * 4.3159689e-05f + -0.70710677f;
	Stored[2] = (FLOAT)(INT)((Low >> 2) & 0x7FFF) * 4.3159689e-05f + -0.70710677f;
	OmittedComponent = Low & 3;
}

void DecodeRotation48(const BYTE* Key, FLOAT Quat[4])
{
	FLOAT Stored[3];
	DWORD OmittedComponent;
	DecodeSmallestThree(Key, Stored, OmittedComponent);
	PlaceQuaternion(OmittedComponent, Stored, SseSqrt(((1.0f - Stored[0] * Stored[0]) - Stored[1] * Stored[1]) - Stored[2] * Stored[2]), Quat);
}

/** __edgeAnimEvaluateRConst, 2013 rva 0x5d4d90 (2012 0x61ca40). */
void EvaluateRConst(DWORD ConstCount, const BYTE* ConstData, const WORD* ConstTable, EdgeAnimJointTransform* Out)
{
	for (DWORD Channel = 0; Channel < ConstCount; Channel++)
	{
		FLOAT Quat[4];
		DecodeRotation48(ConstData + 6 * Channel, Quat);
		WriteVector(Out[ConstTable[Channel]].rotation, Quat[0], Quat[1], Quat[2], Quat[3]);
	}
}

/** __edgeAnimEvaluateR, 2013 rva 0x5d5390 (2012 0x61d040). */
void EvaluateR(DWORD AnimCount, const WORD* AnimTable, const BYTE* Initial, const BYTE* Intra, const BYTE* Final, const FEdgeChannelStream& Stream, EdgeAnimJointTransform* Out)
{
	DWORD IntraKey = 0;
	for (DWORD Channel = 0; Channel < AnimCount; Channel++)
	{
		const FEdgeKeySelection Selection = Stream.Select(Channel, IntraKey);
		FLOAT Left[4];
		FLOAT Right[4];
		FLOAT Quat[4];
		DecodeRotation48(Selection.bLeftIsInitial ? Initial + 6 * Channel : Intra + 6 * Selection.LeftIntraKey, Left);
		DecodeRotation48(Selection.bRightIsFinal ? Final + 6 * Channel : Intra + 6 * Selection.RightIntraKey, Right);
		SlerpQuaternion(Left, Right, Selection.T, Quat);
		WriteVector(Out[AnimTable[Channel]].rotation, Quat[0], Quat[1], Quat[2], Quat[3]);
		IntraKey += Stream.CountKeys(Channel);
	}
}

/** __edgeAnimEvaluateSTConst, 2013 rva 0x5d6ca0 (2012 0x61e950): three floats per channel, W = 1. */
void EvaluateSTConst(DWORD ConstCount, const BYTE* ConstData, const WORD* ConstTable, EdgeAnimJointTransform* Out, INT Slot)
{
	for (DWORD Channel = 0; Channel < ConstCount; Channel++)
	{
		const BYTE* Key = ConstData + 12 * Channel;
		WriteVector(OutputSlot(Out, ConstTable[Channel], Slot), LoadLEFloat(Key), LoadLEFloat(Key + 4), LoadLEFloat(Key + 8), 1.0f);
	}
}

/** __edgeAnimEvaluateST, 2013 rva 0x5d6d80 (2012 0x61ea30). */
void EvaluateST(DWORD AnimCount, const WORD* AnimTable, const BYTE* Initial, const BYTE* Intra, const BYTE* Final, const FEdgeChannelStream& Stream, EdgeAnimJointTransform* Out, INT Slot)
{
	DWORD IntraKey = 0;
	for (DWORD Channel = 0; Channel < AnimCount; Channel++)
	{
		const FEdgeKeySelection Selection = Stream.Select(Channel, IntraKey);
		const BYTE* Left = Selection.bLeftIsInitial ? Initial + 12 * Channel : Intra + 12 * Selection.LeftIntraKey;
		const BYTE* Right = Selection.bRightIsFinal ? Final + 12 * Channel : Intra + 12 * Selection.RightIntraKey;
		const FLOAT OneMinusT = 1.0f - Selection.T;
		WriteVector(OutputSlot(Out, AnimTable[Channel], Slot),
			Selection.T * LoadLEFloat(Right) + OneMinusT * LoadLEFloat(Left),
			Selection.T * LoadLEFloat(Right + 4) + OneMinusT * LoadLEFloat(Left + 4),
			Selection.T * LoadLEFloat(Right + 8) + OneMinusT * LoadLEFloat(Left + 8),
			1.0f);
		IntraKey += Stream.CountKeys(Channel);
	}
}

} // namespace

/** Joint lookup by name hash with a rolling hint (retail search order: from the hint to the end, then from 0 to the hint); AnimSkeletonGetJointIndexByHash, 2013 rva 0x54de20 (2012 0x58e9f0). */
INT EdgeAnimSkeletonGetJointIndexByHash(const EdgeAnimSkeleton* Skel, DWORD JointHash, INT& IndexHint)
{
	const DWORD* Hashes = (const DWORD*)SelfRelative(Skel->offsetJointNameHashArray);
	const INT NumJoints = Skel->numJoints;
	INT Index = IndexHint;
	if (Index < NumJoints)
	{
		for (; Index < NumJoints; Index++)
		{
			if (Hashes[Index] == JointHash)
			{
				IndexHint = Index + 1;
				return Index;
			}
		}
	}
	for (Index = 0; Index < IndexHint; Index++)
	{
		if (Hashes[Index] == JointHash)
		{
			IndexHint = Index + 1;
			return Index;
		}
	}
	return INDEX_NONE;
}

/** Time → frame set, frame and fraction; the fetch and evaluate stages of _edgeAnimProcessCommandList, 2013 rva of 2012 0x6238c0. */
UBOOL EdgeAnimLocateFrame(const EdgeAnimAnimation* Anim, FLOAT EvalTime, FEdgeAnimFrameSample& OutSample)
{
	const EdgeAnimFrameSetInfo* Infos = (const EdgeAnimFrameSetInfo*)SelfRelative(Anim->offsetFrameSetInfoArray);
	const BYTE* DmaArray = SelfRelative(Anim->offsetFrameSetDmaArray);
	if (Anim->numFrameSets == 0 || Infos == NULL || DmaArray == NULL)
	{
		return FALSE;
	}
	FLOAT Frame = EvalTime * Anim->sampleFrequency;
	if (Frame < 0.0f)
	{
		Frame = 0.0f;
	}
	const WORD FrameIndex = (WORD)(INT)Frame;
	DWORD Low = 0;
	DWORD High = Anim->numFrameSets - 1;
	if (High > 1)
	{
		do
		{
			const DWORD Middle = (Low + High) >> 1;
			if (FrameIndex >= Infos[Middle].baseFrame)
			{
				Low = Middle;
			}
			else
			{
				High = Middle;
			}
		}
		while (Low + 1 < High);
	}
	const EdgeAnimFrameSetInfo& Info = Infos[Low];
	OutSample.FrameSetData = (const BYTE*)Anim + *(const DWORD*)(DmaArray + 8 * Low + 4);
	OutSample.FrameSetSize = *(const WORD*)(DmaArray + 8 * Low + 2);
	OutSample.IntraFrameCount = Info.numIntraFrames;
	const DOUBLE Relative = (DOUBLE)Frame - (DOUBLE)Info.baseFrame;
	DWORD FrameInteger = (DWORD)(SQWORD)Relative;
	FLOAT FrameFraction = (FLOAT)(Relative - (DOUBLE)(SQWORD)Relative);
	if (FrameInteger > Info.numIntraFrames)
	{
		FrameInteger = Info.numIntraFrames;
		FrameFraction = 1.0f;
	}
	OutSample.FrameInteger = FrameInteger;
	OutSample.FrameFraction = FrameFraction;
	return TRUE;
}

/** __edgeAnimEvaluate, 2013 rva 0x5dada0 (2012 0x622a50): evaluates the channelled joints of one sample (animation joint order). */
void EdgeAnimEvaluate(EdgeAnimJointTransform* OutJoints, const EdgeAnimAnimation* Anim, const FEdgeAnimFrameSample& Sample)
{
	const BYTE* ConstRData = SelfRelative(Anim->offsetConstRData);
	const BYTE* ConstTData = SelfRelative(Anim->offsetConstTData);
	const BYTE* ConstSData = SelfRelative(Anim->offsetConstSData);
	const DWORD* Specs = (const DWORD*)SelfRelative(Anim->offsetPackingSpecs);
	const DWORD IntraFrameCount = Sample.IntraFrameCount;

	const BYTE* InitialR = NULL;
	const BYTE* InitialT = NULL;
	const BYTE* InitialS = NULL;
	const BYTE* IntraBits = NULL;
	const BYTE* IntraR = NULL;
	const BYTE* IntraT = NULL;
	const BYTE* IntraS = NULL;
	const BYTE* FinalR = NULL;
	const BYTE* FinalT = NULL;
	const BYTE* FinalS = NULL;
	if (Anim->numFrameSets >= 2)
	{
		const BYTE* FrameSet = Sample.FrameSetData;
		const WORD* Sizes = (const WORD*)FrameSet;
		InitialR = FrameSet + 16;
		InitialT = InitialR + Sizes[0];
		InitialS = InitialT + Sizes[1];
		const BYTE* InitialUser = InitialS + Sizes[2];
		IntraBits = InitialUser + Sizes[3];
		IntraR = IntraBits + ((IntraFrameCount * (Anim->numAnimRChannels + Anim->numAnimTChannels + Anim->numAnimUserChannels + Anim->numAnimSChannels) + 7) >> 3);
		IntraT = IntraR + Sizes[4];
		IntraS = IntraT + Sizes[5];
		const BYTE* IntraUser = (const BYTE*)(((PTRINT)IntraS + Sizes[6] + 3) & ~(PTRINT)3);
		const BYTE* FinalSet = (const BYTE*)(((PTRINT)IntraUser + Sizes[7] + 15) & ~(PTRINT)15);
		const WORD* FinalSizes = (const WORD*)FinalSet;
		FinalR = FinalSet + 16;
		FinalT = FinalR + FinalSizes[0];
		FinalS = FinalT + FinalSizes[1];
	}

	const WORD* ConstRTable = (const WORD*)(Anim + 1);
	const WORD* ConstTTable = ConstRTable + AlignUp(Anim->numConstRChannels, 8);
	const WORD* ConstSTable = ConstTTable + AlignUp(Anim->numConstTChannels, 4);
	const WORD* ConstUserTable = ConstSTable + AlignUp(Anim->numConstSChannels, 4);
	const WORD* AnimRTable = ConstUserTable + AlignUp(Anim->numConstUserChannels, 4);
	const WORD* AnimTTable = AnimRTable + AlignUp(Anim->numAnimRChannels, 4);
	const WORD* AnimSTable = AnimTTable + AlignUp(Anim->numAnimTChannels, 4);

	FEdgeChannelStream Stream;
	Stream.IntraBits = IntraBits;
	Stream.IntraFrameCount = IntraFrameCount;
	Stream.FrameInteger = Sample.FrameInteger;
	Stream.FrameFraction = Sample.FrameFraction;

	Stream.IntraBitsOffset = 0;
	if (Anim->flags & 1)
	{
		EvaluateBitPackedConst(Anim->numConstRChannels, ConstRData, ConstRTable, Specs[0], OutJoints, 0, TRUE);
		EvaluateBitPacked(Anim->numAnimRChannels, AnimRTable, Specs + 1, InitialR, IntraR, FinalR, Stream, OutJoints, 0, TRUE);
		Specs += Anim->numAnimRChannels + 1;
	}
	else
	{
		EvaluateRConst(Anim->numConstRChannels, ConstRData, ConstRTable, OutJoints);
		EvaluateR(Anim->numAnimRChannels, AnimRTable, InitialR, IntraR, FinalR, Stream, OutJoints);
	}

	Stream.IntraBitsOffset = IntraFrameCount * Anim->numAnimRChannels;
	if (Anim->flags & 2)
	{
		EvaluateBitPackedConst(Anim->numConstTChannels, ConstTData, ConstTTable, Specs[0], OutJoints, 1, FALSE);
		EvaluateBitPacked(Anim->numAnimTChannels, AnimTTable, Specs + 1, InitialT, IntraT, FinalT, Stream, OutJoints, 1, FALSE);
		Specs += Anim->numAnimTChannels + 1;
	}
	else
	{
		EvaluateSTConst(Anim->numConstTChannels, ConstTData, ConstTTable, OutJoints, 1);
		EvaluateST(Anim->numAnimTChannels, AnimTTable, InitialT, IntraT, FinalT, Stream, OutJoints, 1);
	}

	Stream.IntraBitsOffset = IntraFrameCount * (Anim->numAnimRChannels + Anim->numAnimTChannels);
	if (Anim->flags & 4)
	{
		EvaluateBitPackedConst(Anim->numConstSChannels, ConstSData, ConstSTable, Specs[0], OutJoints, 2, FALSE);
		EvaluateBitPacked(Anim->numAnimSChannels, AnimSTable, Specs + 1, InitialS, IntraS, FinalS, Stream, OutJoints, 2, FALSE);
	}
	else
	{
		EvaluateSTConst(Anim->numConstSChannels, ConstSData, ConstSTable, OutJoints, 2);
		EvaluateST(Anim->numAnimSChannels, AnimSTable, InitialS, IntraS, FinalS, Stream, OutJoints, 2);
	}
}

namespace
{

const WORD NoJoint = 0xFFFF;

/** Game-thread scratch, like the static BoneTrackArrays of the reference batch solver. */
struct FEdgeSkeletonPoseScratch
{
	TArray<EdgeAnimJointTransform> AnimJoints;
	TArray<WORD> AnimToSkel;
	TArray<FEdgeSkelToAnimMapping> SkelToAnim;
	TArray<BYTE> AlignedFrameSet;
};

FEdgeSkeletonPoseScratch GEdgeSkeletonPoseScratch;

void ClearReferenceFlags(const WORD* Table, DWORD Count, const TArray<WORD>& AnimToSkel, TArray<FEdgeSkelToAnimMapping>& SkelToAnim, UBOOL bRotation)
{
	for (DWORD Channel = 0; Channel < Count; Channel++)
	{
		const WORD SkelJoint = AnimToSkel(Table[Channel]);
		if (SkelJoint == NoJoint)
		{
			continue;
		}
		if (bRotation)
		{
			SkelToAnim(SkelJoint).m_bUseRefPoseRotation = 0;
		}
		else
		{
			SkelToAnim(SkelJoint).m_bUseRefPoseTranslation = 0;
		}
	}
}

} // namespace

UBOOL EdgeAnimEvaluateSkeletonPose(const EdgeAnimAnimation* Anim, const EdgeAnimSkeleton* Skel, FLOAT Time, UBOOL bAdditive, TArray<EdgeAnimJointTransform>& OutJoints)
{
	FEdgeSkeletonPoseScratch& Scratch = GEdgeSkeletonPoseScratch;
	FEdgeAnimFrameSample Sample;
	if (!EdgeAnimLocateFrame(Anim, Time, Sample))
	{
		return FALSE;
	}
	// The cooker laid the frame sets out for a 16-byte aligned blob: retail keeps the source address mod 128 when it
	// copies a frame set (_edgeAnimProcessCommandList) and the evaluator rounds absolute addresses.
	if ((PTRINT)Sample.FrameSetData & 15)
	{
		Scratch.AlignedFrameSet.Empty(Sample.FrameSetSize + 16);
		Scratch.AlignedFrameSet.Add(Sample.FrameSetSize + 16);
		BYTE* Aligned = Align(Scratch.AlignedFrameSet.GetData(), 16);
		appMemcpy(Aligned, Sample.FrameSetData, Sample.FrameSetSize);
		Sample.FrameSetData = Aligned;
	}
	if (Scratch.AnimJoints.Num() < Anim->numJoints + 1)
	{
		Scratch.AnimJoints.Empty(Anim->numJoints + 1);
		Scratch.AnimJoints.Add(Anim->numJoints + 1);
	}
	EdgeAnimEvaluate(Scratch.AnimJoints.GetData(), Anim, Sample);

	Scratch.AnimToSkel.Empty(Anim->numJoints);
	Scratch.AnimToSkel.Add(Anim->numJoints);
	Scratch.SkelToAnim.Empty(Skel->numJoints);
	Scratch.SkelToAnim.Add(Skel->numJoints);
	for (INT SkelJoint = 0; SkelJoint < Skel->numJoints; SkelJoint++)
	{
		FEdgeSkelToAnimMapping& Mapping = Scratch.SkelToAnim(SkelJoint);
		Mapping.m_AnimJointIndex = NoJoint;
		Mapping.m_bUseRefPoseRotation = 1;
		Mapping.m_bUseRefPoseTranslation = 1;
	}
	const DWORD* Hashes = (const DWORD*)(SelfRelative(Anim->offsetCustomData) + (Anim->offsetLocomotionDelta ? 32 : 0));
	INT Hint = 0;
	for (WORD AnimJoint = 0; AnimJoint < Anim->numJoints; AnimJoint++)
	{
		const INT SkelJoint = EdgeAnimSkeletonGetJointIndexByHash(Skel, Hashes[AnimJoint], Hint);
		Scratch.AnimToSkel(AnimJoint) = SkelJoint == INDEX_NONE ? NoJoint : (WORD)SkelJoint;
		if (SkelJoint != INDEX_NONE)
		{
			Scratch.SkelToAnim(SkelJoint).m_AnimJointIndex = AnimJoint;
		}
	}
	const WORD* ConstRTable = (const WORD*)(Anim + 1);
	const WORD* ConstTTable = ConstRTable + AlignUp(Anim->numConstRChannels, 8);
	const WORD* ConstSTable = ConstTTable + AlignUp(Anim->numConstTChannels, 4);
	const WORD* ConstUserTable = ConstSTable + AlignUp(Anim->numConstSChannels, 4);
	const WORD* AnimRTable = ConstUserTable + AlignUp(Anim->numConstUserChannels, 4);
	const WORD* AnimTTable = AnimRTable + AlignUp(Anim->numAnimRChannels, 4);
	ClearReferenceFlags(ConstRTable, Anim->numConstRChannels, Scratch.AnimToSkel, Scratch.SkelToAnim, TRUE);
	ClearReferenceFlags(AnimRTable, Anim->numAnimRChannels, Scratch.AnimToSkel, Scratch.SkelToAnim, TRUE);
	ClearReferenceFlags(ConstTTable, Anim->numConstTChannels, Scratch.AnimToSkel, Scratch.SkelToAnim, FALSE);
	ClearReferenceFlags(AnimTTable, Anim->numAnimTChannels, Scratch.AnimToSkel, Scratch.SkelToAnim, FALSE);

	static const FEdgeAnimVec4 IdentityRotation = { 0.0f, 0.0f, 0.0f, 1.0f };
	static const FEdgeAnimVec4 ZeroTranslation = { 0.0f, 0.0f, 0.0f, 1.0f };
	static const FEdgeAnimVec4 UnitScale = { 1.0f, 1.0f, 1.0f, 1.0f };
	const EdgeAnimJointTransform* BasePose = (const EdgeAnimJointTransform*)SelfRelative(Skel->offsetBasePose);
	OutJoints.Empty(Skel->numJoints);
	OutJoints.Add(Skel->numJoints);
	for (INT SkelJoint = 0; SkelJoint < Skel->numJoints; SkelJoint++)
	{
		const FEdgeSkelToAnimMapping& Mapping = Scratch.SkelToAnim(SkelJoint);
		EdgeAnimJointTransform& Joint = OutJoints(SkelJoint);
		Joint.rotation = !Mapping.m_bUseRefPoseRotation ? Scratch.AnimJoints(Mapping.m_AnimJointIndex).rotation : bAdditive ? IdentityRotation : BasePose[SkelJoint].rotation;
		Joint.translation = !Mapping.m_bUseRefPoseTranslation ? Scratch.AnimJoints(Mapping.m_AnimJointIndex).translation : bAdditive ? ZeroTranslation : BasePose[SkelJoint].translation;
		Joint.scale = UnitScale;
	}
	return TRUE;
}
