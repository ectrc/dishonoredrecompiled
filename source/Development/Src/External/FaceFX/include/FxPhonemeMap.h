/*=============================================================================
	FxPhonemeMap.h - FaceFX 1.7.3.1 phonemes (fxphonemeenum.h) and the phoneme to target mapping (fxphonememap.h).

	DISHONORED(port): agent FX3 (wave 26). FxPhonToNameMap 12 bytes {phoneme, name, amount}, FxPhonemeMap 28 bytes
	(FxObject + _mapping + _targetNames), class version 1. Every cooked head stores one (FxActor::Serialize, version >= 1);
	nothing on the runtime path reads it (it drives FaceFX Studio's curve generation).
=============================================================================*/
#ifndef FxPhonemeMap_H__
#define FxPhonemeMap_H__

#include "FxFaceGraphPrerequisites.h"

namespace OC3Ent
{
namespace Face
{

enum FxPhoneme
{
	PHON_INVALID = -1,
	PHON_FIRST = 0,
	PHON_SIL = PHON_FIRST,
	PHON_P, PHON_B, PHON_T, PHON_D, PHON_K, PHON_G, PHON_M, PHON_N, PHON_NG, PHON_RA, PHON_RU, PHON_FLAP, PHON_PH, PHON_F,
	PHON_V, PHON_TH, PHON_DH, PHON_S, PHON_Z, PHON_SH, PHON_ZH, PHON_CX, PHON_X, PHON_GH, PHON_HH, PHON_R, PHON_Y, PHON_L,
	PHON_W, PHON_H, PHON_TS, PHON_CH, PHON_JH, PHON_IY, PHON_E, PHON_EN, PHON_EH, PHON_A, PHON_AA, PHON_AAN, PHON_AO,
	PHON_AON, PHON_O, PHON_ON, PHON_UW, PHON_UY, PHON_EU, PHON_OE, PHON_OEN, PHON_AH, PHON_IH, PHON_UU, PHON_UH, PHON_AX,
	PHON_UX, PHON_AE, PHON_ER, PHON_AXR, PHON_EXR, PHON_EY, PHON_AW, PHON_AY, PHON_OY, PHON_OW,
	PHON_LAST = PHON_OW,
	NUM_PHONS
};

class FxPhonemeUtility
{
public:
	static FxPhoneme UpdatePhoneme(FxInt32 oldPhoneme);
	static FxPhoneme UpdatePhoneme(FxPhoneme oldPhoneme);
};

class FxPhonToNameMap
{
public:
	/** 2012 0xb1e480 (phoneme and amount left as is, like retail). */
	FxPhonToNameMap()
	{
	}

	/** 2012 0xb1e410. */
	FxBool operator==(const FxPhonToNameMap& other) const
	{
		return name == other.name && phoneme == other.phoneme && FxRealEquality(amount, other.amount);
	}

	/** 2012 0xb1e3e0. */
	FxBool operator!=(const FxPhonToNameMap& other) const
	{
		return !(*this == other);
	}

	FxPhoneme phoneme;
	FxName name;
	FxReal amount;
};

FxArchive& operator<<(FxArchive& arc, FxPhonToNameMap& obj);

class FxPhonemeMap : public FxObject
{
	FX_DECLARE_CLASS(FxPhonemeMap, FxObject)
public:
	FxPhonemeMap();
	virtual ~FxPhonemeMap();

	FxBool operator==(const FxPhonemeMap& other) const;

	FxReal GetMappingAmount(const FxPhoneme& phon, const FxName& name) const;
	FxReal GetMappingAmount(const FxPhoneme& phon, FxSize targetIndex) const;
	void SetMappingAmount(const FxPhoneme& phon, const FxName& name, FxReal amount);
	void SetMappingAmount(const FxPhoneme& phon, const FxChar* name, FxReal amount);
	void SetMappingAmount(const FxPhoneme& phon, FxSize targetIndex, FxReal amount);
	void RemovePhonemeMappingEntries(const FxPhoneme& phon);

	FxSize GetNumMappingEntries() const;
	const FxPhonToNameMap& GetMappingEntry(FxSize index) const;

	void AddTarget(const FxName& targetName);
	void RemoveTarget(const FxName& targetName);
	FxSize GetNumTargets() const;
	const FxName& GetTargetName(FxSize n) const;
	FxSize GetTargetIndex(const FxName& name) const;

	void Clear();
	void MakeDefaultMapping();

	virtual void Serialize(FxArchive& arc);

private:
	FxArray<FxPhonToNameMap> _mapping;
	FxArray<FxName> _targetNames;
};

} // namespace Face
} // namespace OC3Ent

#endif
