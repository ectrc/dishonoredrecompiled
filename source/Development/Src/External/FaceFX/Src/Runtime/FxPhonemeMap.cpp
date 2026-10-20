/*=============================================================================
	FxPhonemeMap.cpp - FaceFX 1.7.3.1 FxPhonemeUtility (fxphonemeenum.cpp) and FxPhonemeMap (fxphonememap.cpp).

	DISHONORED(port): agent FX3 (wave 26), from the 2012 decompile (FxPhonemeEnum.obj, FxPhonemeMap.obj).
	Shape change: MakeDefaultMapping (0xb1c610, 6,068 B) is retail's 146 SetMappingAmount(phon, "target", amount)
	calls; here the same calls in the same order, read from one table.
=============================================================================*/
#include "FxPhonemeMap.h"

namespace OC3Ent
{
namespace Face
{

static_assert(sizeof(FxPhonToNameMap) == 12, "FxPhonToNameMap: 2012 PDB sizeof 12");
static_assert(sizeof(FxPhonemeMap) == 28, "FxPhonemeMap: 2012 PDB sizeof 28");

/** The pre-1.7 phoneme numbering (43 phonemes, IY .. FLAP) to the current one; 2012 data EnumUpdateMapVersion0To1. */
static const FxPhoneme EnumUpdateMapVersion0To1[] =
{
	PHON_IY, PHON_IH, PHON_EH, PHON_EY, PHON_AE, PHON_AA, PHON_AW, PHON_AY, PHON_AH, PHON_AO, PHON_OY, PHON_OW, PHON_UH,
	PHON_UW, PHON_ER, PHON_AX, PHON_S, PHON_SH, PHON_Z, PHON_ZH, PHON_F, PHON_TH, PHON_V, PHON_DH, PHON_M, PHON_N, PHON_NG,
	PHON_L, PHON_R, PHON_W, PHON_Y, PHON_HH, PHON_B, PHON_D, PHON_JH, PHON_G, PHON_P, PHON_T, PHON_K, PHON_CH, PHON_SIL,
	PHON_SIL, PHON_FLAP
};

/** 2012 0xb2af40. */
FxPhoneme FxPhonemeUtility::UpdatePhoneme(FxInt32 oldPhoneme)
{
	return EnumUpdateMapVersion0To1[oldPhoneme];
}

/** 2012 0xb2af50. */
FxPhoneme FxPhonemeUtility::UpdatePhoneme(FxPhoneme oldPhoneme)
{
	return UpdatePhoneme(static_cast<FxInt32>(oldPhoneme));
}

/** 2012 0xb1be00 (AllocateStaticClassFxPhonemeMap). */
FX_IMPLEMENT_CLASS(FxPhonemeMap, 1, FxObject)

/** 2012 0xb1be90. */
FxPhonemeMap::FxPhonemeMap()
{
}

/** 2012 0xb1bf10. */
FxPhonemeMap::~FxPhonemeMap()
{
}

/** 2012 0xb1bf80. */
FxBool FxPhonemeMap::operator==(const FxPhonemeMap& other) const
{
	if( _targetNames.Length() != other._targetNames.Length() || _mapping.Length() != other._mapping.Length() )
	{
		return FxFalse;
	}
	FxSize numTargetNames = GetNumTargets();
	for( FxSize i = 0; i < numTargetNames; ++i )
	{
		if( GetTargetName(i) != other.GetTargetName(i) )
		{
			return FxFalse;
		}
	}
	FxSize numMappingEntries = GetNumMappingEntries();
	for( FxSize i = 0; i < numMappingEntries; ++i )
	{
		if( GetMappingEntry(i) != other.GetMappingEntry(i) )
		{
			return FxFalse;
		}
	}
	return FxTrue;
}

/** 2012 0xb1c080. */
FxReal FxPhonemeMap::GetMappingAmount(const FxPhoneme& phon, const FxName& name) const
{
	FxSize numMappingEntries = _mapping.Length();
	for( FxSize i = 0; i < numMappingEntries; ++i )
	{
		if( _mapping[i].name == name && _mapping[i].phoneme == phon )
		{
			return _mapping[i].amount;
		}
	}
	return 0.0f;
}

/** 2012 0xb1c110. */
FxReal FxPhonemeMap::GetMappingAmount(const FxPhoneme& phon, FxSize targetIndex) const
{
	return GetMappingAmount(phon, GetTargetName(targetIndex));
}

/** 2012 0xb1c140. */
void FxPhonemeMap::SetMappingAmount(const FxPhoneme& phon, const FxName& name, FxReal amount)
{
	FxSize numMappingEntries = _mapping.Length();
	for( FxSize i = 0; i < numMappingEntries; ++i )
	{
		if( _mapping[i].name == name && _mapping[i].phoneme == phon )
		{
			if( FxRealEquality(amount, 0.0f) )
			{
				_mapping.Remove(i);
			}
			else
			{
				_mapping[i].amount = amount;
			}
			return;
		}
	}
	FxBool newName = FxTrue;
	FxSize numTargetNames = _targetNames.Length();
	for( FxSize i = 0; i < numTargetNames; ++i )
	{
		newName &= _targetNames[i] != name;
	}
	if( newName )
	{
		_targetNames.PushBack(name);
	}
	if( !FxRealEquality(amount, 0.0f) )
	{
		FxPhonToNameMap newMappingInfo;
		newMappingInfo.phoneme = phon;
		newMappingInfo.name = name;
		newMappingInfo.amount = amount;
		_mapping.PushBack(newMappingInfo);
	}
}

/** 2012 0xb1c300. */
void FxPhonemeMap::SetMappingAmount(const FxPhoneme& phon, const FxChar* name, FxReal amount)
{
	SetMappingAmount(phon, FxName(name), amount);
}

/** 2012 0xb1c380. */
void FxPhonemeMap::SetMappingAmount(const FxPhoneme& phon, FxSize targetIndex, FxReal amount)
{
	if( targetIndex < _targetNames.Length() )
	{
		SetMappingAmount(phon, GetTargetName(targetIndex), amount);
	}
}

/** 2012 0xb1c3c0. */
void FxPhonemeMap::RemovePhonemeMappingEntries(const FxPhoneme& phon)
{
	for( FxSize i = 0; i < _mapping.Length(); ++i )
	{
		if( _mapping[i].phoneme == phon )
		{
			_mapping.Remove(i--);
		}
	}
}

/** 2012 0xb1c430. */
FxSize FxPhonemeMap::GetNumMappingEntries() const
{
	return _mapping.Length();
}

/** 2012 0xb1c450. */
const FxPhonToNameMap& FxPhonemeMap::GetMappingEntry(FxSize index) const
{
	return _mapping[index];
}

/** 2012 0xb1c470. */
void FxPhonemeMap::AddTarget(const FxName& targetName)
{
	_targetNames.PushBack(targetName);
}

/** 2012 0xb1c490. */
void FxPhonemeMap::RemoveTarget(const FxName& targetName)
{
	for( FxSize i = 0; i < _mapping.Length(); ++i )
	{
		if( _mapping[i].name == targetName )
		{
			_mapping.Remove(i--);
		}
	}
	FxSize targetIndex = GetTargetIndex(targetName);
	if( targetIndex != FxInvalidIndex )
	{
		_targetNames.Remove(targetIndex);
	}
}

/** 2012 0xb1c530. */
FxSize FxPhonemeMap::GetNumTargets() const
{
	return _targetNames.Length();
}

/** 2012 0xb1c550. */
const FxName& FxPhonemeMap::GetTargetName(FxSize n) const
{
	return _targetNames[n];
}

/** 2012 0xb1c570. */
FxSize FxPhonemeMap::GetTargetIndex(const FxName& name) const
{
	FxSize numTargetNames = _targetNames.Length();
	for( FxSize i = 0; i < numTargetNames; ++i )
	{
		if( _targetNames[i] == name )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

/** 2012 0xb1c5e0. */
void FxPhonemeMap::Clear()
{
	_targetNames.Clear();
	_mapping.Clear();
}

/** 2012 0xb1c610. */
void FxPhonemeMap::MakeDefaultMapping()
{
	struct FxDefaultMappingEntry
	{
		FxPhoneme phoneme;
		const FxChar* target;
		FxReal amount;
	};
	static const FxDefaultMappingEntry defaultMapping[] =
	{
		{ PHON_P, "open", 0.0f }, { PHON_B, "open", 0.0f }, { PHON_T, "open", 0.4f }, { PHON_D, "open", 0.4f }, { PHON_K, "open", 0.25f },
		{ PHON_G, "open", 0.25f }, { PHON_M, "open", 0.0f }, { PHON_N, "open", 0.4f }, { PHON_NG, "open", 0.4f }, { PHON_RA, "open", 0.4f },
		{ PHON_RU, "open", 0.25f }, { PHON_FLAP, "open", 0.3f }, { PHON_PH, "open", 0.1f }, { PHON_F, "open", 0.0f }, { PHON_V, "open", 0.0f },
		{ PHON_TH, "open", 0.45f }, { PHON_DH, "open", 0.45f }, { PHON_S, "open", 0.15f }, { PHON_Z, "open", 0.15f }, { PHON_SH, "open", 0.0f },
		{ PHON_ZH, "open", 0.0f }, { PHON_CX, "open", 0.25f }, { PHON_X, "open", 0.25f }, { PHON_GH, "open", 0.25f }, { PHON_HH, "open", 0.3f },
		{ PHON_R, "open", 0.1f }, { PHON_Y, "open", 0.0f }, { PHON_L, "open", 0.4f }, { PHON_W, "open", 0.0f }, { PHON_H, "open", 0.2f },
		{ PHON_TS, "open", 0.4f }, { PHON_CH, "open", 0.0f }, { PHON_JH, "open", 0.0f }, { PHON_IY, "open", 0.2f }, { PHON_E, "open", 0.35f },
		{ PHON_EN, "open", 0.35f }, { PHON_EH, "open", 0.5f }, { PHON_A, "open", 0.55f }, { PHON_AA, "open", 0.55f }, { PHON_AAN, "open", 0.55f },
		{ PHON_AO, "open", 0.4f }, { PHON_AON, "open", 0.4f }, { PHON_O, "open", 0.4f }, { PHON_ON, "open", 0.4f }, { PHON_UW, "open", 0.4f },
		{ PHON_UY, "open", 0.0f }, { PHON_EU, "open", 0.4f }, { PHON_OE, "open", 0.4f }, { PHON_OEN, "open", 0.4f }, { PHON_AH, "open", 0.5f },
		{ PHON_IH, "open", 0.5f }, { PHON_UU, "open", 0.4f }, { PHON_UH, "open", 0.4f }, { PHON_AX, "open", 0.5f }, { PHON_UX, "open", 0.5f },
		{ PHON_AE, "open", 0.5f }, { PHON_ER, "open", 0.4f }, { PHON_AXR, "open", 0.4f }, { PHON_EXR, "open", 0.4f }, { PHON_EY, "open", 0.5f },
		{ PHON_AW, "open", 0.5f }, { PHON_AY, "open", 0.5f }, { PHON_OY, "open", 0.4f }, { PHON_OW, "open", 0.4f },
		{ PHON_R, "W", 0.7f }, { PHON_Y, "W", 0.5f }, { PHON_W, "W", 0.85f }, { PHON_AO, "W", 0.55f }, { PHON_AON, "W", 0.55f },
		{ PHON_O, "W", 0.55f }, { PHON_ON, "W", 0.55f }, { PHON_UW, "W", 0.55f }, { PHON_UY, "W", 0.85f }, { PHON_EU, "W", 0.55f },
		{ PHON_OE, "W", 0.55f }, { PHON_OEN, "W", 0.55f }, { PHON_UU, "W", 0.55f }, { PHON_UH, "W", 0.55f }, { PHON_OY, "W", 0.55f },
		{ PHON_OW, "W", 0.55f },
		{ PHON_SH, "ShCh", 0.85f }, { PHON_ZH, "ShCh", 0.85f }, { PHON_Y, "ShCh", 0.3f }, { PHON_CH, "ShCh", 0.85f }, { PHON_JH, "ShCh", 0.85f },
		{ PHON_ER, "ShCh", 0.5f }, { PHON_AXR, "ShCh", 0.5f }, { PHON_EXR, "ShCh", 0.5f },
		{ PHON_P, "PBM", 0.9f }, { PHON_B, "PBM", 0.9f }, { PHON_M, "PBM", 0.9f },
		{ PHON_PH, "FV", 0.4f }, { PHON_F, "FV", 0.75f }, { PHON_V, "FV", 0.75f },
		{ PHON_S, "wide", 0.5f }, { PHON_Z, "wide", 0.5f }, { PHON_IY, "wide", 0.8f }, { PHON_E, "wide", 0.25f }, { PHON_EN, "wide", 0.25f },
		{ PHON_EH, "wide", 0.6f }, { PHON_AH, "wide", 0.6f }, { PHON_IH, "wide", 0.6f }, { PHON_AX, "wide", 0.6f }, { PHON_UX, "wide", 0.6f },
		{ PHON_AE, "wide", 0.6f }, { PHON_EY, "wide", 0.6f }, { PHON_AW, "wide", 0.6f }, { PHON_AY, "wide", 0.6f },
		{ PHON_K, "tBack", 0.8f }, { PHON_G, "tBack", 0.8f }, { PHON_RU, "tBack", 0.8f }, { PHON_CX, "tBack", 0.8f }, { PHON_X, "tBack", 0.8f },
		{ PHON_GH, "tBack", 0.8f }, { PHON_EH, "tBack", 0.4f }, { PHON_AH, "tBack", 0.4f }, { PHON_IH, "tBack", 0.4f }, { PHON_AX, "tBack", 0.4f },
		{ PHON_UX, "tBack", 0.4f }, { PHON_AE, "tBack", 0.4f }, { PHON_EY, "tBack", 0.4f }, { PHON_AW, "tBack", 0.4f }, { PHON_AY, "tBack", 0.4f },
		{ PHON_T, "tRoof", 0.8f }, { PHON_D, "tRoof", 0.8f }, { PHON_N, "tRoof", 0.8f }, { PHON_NG, "tRoof", 0.8f }, { PHON_RA, "tRoof", 0.5f },
		{ PHON_FLAP, "tRoof", 0.6f }, { PHON_S, "tRoof", 0.4f }, { PHON_Z, "tRoof", 0.4f }, { PHON_SH, "tRoof", 0.4f }, { PHON_ZH, "tRoof", 0.4f },
		{ PHON_Y, "tRoof", 0.4f }, { PHON_L, "tRoof", 0.8f }, { PHON_TS, "tRoof", 0.8f }, { PHON_CH, "tRoof", 0.4f }, { PHON_JH, "tRoof", 0.4f },
		{ PHON_IY, "tRoof", 0.2f }, { PHON_E, "tRoof", 0.2f }, { PHON_EN, "tRoof", 0.2f }, { PHON_ER, "tRoof", 0.5f }, { PHON_AXR, "tRoof", 0.5f },
		{ PHON_EXR, "tRoof", 0.5f },
		{ PHON_TH, "tTeeth", 0.9f }, { PHON_DH, "tTeeth", 0.9f }
	};
	Clear();
	for( FxSize i = 0; i < sizeof(defaultMapping) / sizeof(defaultMapping[0]); ++i )
	{
		SetMappingAmount(defaultMapping[i].phoneme, defaultMapping[i].target, defaultMapping[i].amount);
	}
}

/** 2012 0xb1ddd0. */
FxArchive& operator<<(FxArchive& arc, FxPhonToNameMap& obj)
{
	FxInt32 tempPhon = obj.phoneme;
	arc << tempPhon << obj.name << obj.amount;
	obj.phoneme = static_cast<FxPhoneme>(tempPhon);
	return arc;
}

/** 2012 0xb1de20. Version 0 archives use the pre-1.7 numbering: the old SHORTSIL entries (value 41) are dropped, every
	phoneme is renumbered and each phoneme 1.7 split off copies the mapping of the old phoneme it came from. */
void FxPhonemeMap::Serialize(FxArchive& arc)
{
	Super::Serialize(arc);
	FxUInt16 version = arc.SerializeClassVersion("FxPhonemeMap");
	arc << _mapping << _targetNames;
	if( arc.IsLoading() && version == 0 )
	{
		FxPhoneme phon = PHON_AO;
		RemovePhonemeMappingEntries(phon);
		for( FxSize i = 0; i < _mapping.Length(); ++i )
		{
			_mapping[i].phoneme = FxPhonemeUtility::UpdatePhoneme(_mapping[i].phoneme);
		}
		struct FxPhonLookup
		{
			FxPhoneme newPhon;
			FxPhoneme oldPhon;
		};
		const FxPhonLookup phonLookup[] =
		{
			{ PHON_RA, PHON_R }, { PHON_RU, PHON_R }, { PHON_PH, PHON_F }, { PHON_CX, PHON_K }, { PHON_X, PHON_K },
			{ PHON_GH, PHON_G }, { PHON_H, PHON_HH }, { PHON_TS, PHON_T }, { PHON_E, PHON_AA }, { PHON_EN, PHON_AA },
			{ PHON_A, PHON_AA }, { PHON_AAN, PHON_AA }, { PHON_AON, PHON_AO }, { PHON_O, PHON_AO }, { PHON_ON, PHON_AO },
			{ PHON_UY, PHON_W }, { PHON_EU, PHON_AO }, { PHON_OE, PHON_AO }, { PHON_OEN, PHON_AO }, { PHON_UU, PHON_W },
			{ PHON_UX, PHON_AX }, { PHON_AXR, PHON_ER }, { PHON_EXR, PHON_ER }
		};
		const FxSize numLookupEntries = 23;
		FxArray<FxPhonToNameMap> tempMappingEntries(numLookupEntries);
		FxSize numMappingEntries = _mapping.Length();
		for( FxSize k = 0; k < numLookupEntries; ++k )
		{
			for( FxSize j = 0; j < numMappingEntries; ++j )
			{
				if( _mapping[j].phoneme == phonLookup[k].oldPhon )
				{
					FxPhonToNameMap entry(_mapping[j]);
					entry.phoneme = phonLookup[k].newPhon;
					tempMappingEntries.PushBack(entry);
				}
			}
		}
		for( FxSize i = 0; i < tempMappingEntries.Length(); ++i )
		{
			_mapping.PushBack(tempMappingEntries[i]);
		}
	}
}

} // namespace Face
} // namespace OC3Ent
