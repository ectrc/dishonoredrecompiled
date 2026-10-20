// FaceFX 1.7.3.1: FxAnim, FxAnimGroup (fxanim.cpp), FxAnimSet (fxanimset.cpp). Agent FX2, wave 26, ported from the
// 2012 decompile.
// FxAnim class version 8 is Arkane's: versions 1-7 carried the UE3 sound cue path / node wave (read and dropped below
// version 8) and version 3-7 a sound cue index; version 7 added the Wwise event path, wav file and index.
// Deviation: FxLoadAnimSetFromFile / FxSaveAnimSetToFile (0xb159a0 / 0xb15a90) are not ported - the engine reads
// anim sets from package memory only, and the file stores are FX1's.
#include "FxAnim.h"

namespace OC3Ent
{
namespace Face
{

/** 2012 0xb07e70. */
FxArchive& operator<<( FxArchive& arc, FxAnimBoneWeight& animBoneWeight )
{
	arc.SerializeClassVersion("FxAnimBoneWeight", FxTrue);
	arc << animBoneWeight.boneName << animBoneWeight.boneWeight;
	return arc;
}

/** 2012 0xb07eb0 (AllocateStaticClassFxAnim). */
FX_IMPLEMENT_CLASS(FxAnim, 8, FxNamedObject)

/** 2012 0xb07f40. */
FxAnim::FxAnim()
	: _startTime(0.0f)
	, _endTime(1.0f)
	, _blendInTime(0.16f)
	, _blendOutTime(0.22f)
	, _eventIndex(FxInvalidIndex)
	, _eventPointer(0)
{
}

/** 2012 0xb08020. */
FxAnim::FxAnim( const FxAnim& other )
	: FxNamedObject(other)
	, FxDataContainer(other)
	, _animCurves(other._animCurves)
	, _startTime(other._startTime)
	, _endTime(other._endTime)
	, _blendInTime(other._blendInTime)
	, _blendOutTime(other._blendOutTime)
	, _boneWeights(other._boneWeights)
	, _eventPath(other._eventPath)
	, _eventWavFile(other._eventWavFile)
	, _eventIndex(other._eventIndex)
	, _eventPointer(other._eventPointer)
{
}

/** 2012 0xb08150. */
FxAnim& FxAnim::operator=( const FxAnim& other )
{
	if( this == &other )
	{
		return *this;
	}
	FxNamedObject::operator=(other);
	FxDataContainer::operator=(other);
	_animCurves = other._animCurves;
	_startTime = other._startTime;
	_endTime = other._endTime;
	_blendInTime = other._blendInTime;
	_blendOutTime = other._blendOutTime;
	_boneWeights = other._boneWeights;
	_eventPath = other._eventPath;
	_eventWavFile = other._eventWavFile;
	_eventIndex = other._eventIndex;
	_eventPointer = other._eventPointer;
	return *this;
}

/** 2012 0xb08240. */
FxAnim::~FxAnim()
{
}

/** 2012 0xb082d0. */
FxSize FxAnim::FindAnimCurve( const FxName& name ) const
{
	FxSize numAnimCurves = _animCurves.Length();
	for( FxSize i = 0; i < numAnimCurves; ++i )
	{
		if( _animCurves[i].GetName() == name )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

/** 2012 0xb08340. */
FxBool FxAnim::AddAnimCurve( const FxAnimCurve& curve )
{
	if( FindAnimCurve(curve.GetName()) != FxInvalidIndex )
	{
		return FxFalse;
	}
	_animCurves.PushBack(curve);
	FindBounds();
	return FxTrue;
}

/** 2012 0xb08380. */
FxBool FxAnim::InsertAnimCurve( const FxAnimCurve& curve, FxSize index )
{
	if( index == FxInvalidIndex || index >= _animCurves.Length() )
	{
		return AddAnimCurve(curve);
	}
	if( FindAnimCurve(curve.GetName()) != FxInvalidIndex )
	{
		return FxFalse;
	}
	_animCurves.Insert(curve, index);
	FindBounds();
	return FxTrue;
}

/** 2012 0xb083f0. */
FxBool FxAnim::RemoveAnimCurve( const FxName& name )
{
	FxSize index = FindAnimCurve(name);
	if( index == FxInvalidIndex )
	{
		return FxFalse;
	}
	_animCurves.Remove(index);
	FindBounds();
	return FxTrue;
}

/** 2012 0xb08440. */
void FxAnim::SetBlendInTime( FxReal blendInTime )
{
	_blendInTime = blendInTime;
}

/** 2012 0xb08460. */
void FxAnim::SetBlendOutTime( FxReal blendOutTime )
{
	_blendOutTime = blendOutTime;
}

/** 2012 0xb08480. */
FxBool FxAnim::AddBoneWeight( const FxAnimBoneWeight& animBoneWeight )
{
	if( _findAnimBoneWeight(animBoneWeight.boneName) != FxInvalidIndex )
	{
		return FxFalse;
	}
	_boneWeights.PushBack(animBoneWeight);
	return FxTrue;
}

/** 2012 0xb084c0. */
FxBool FxAnim::RemoveBoneWeight( const FxName& boneName )
{
	FxSize index = _findAnimBoneWeight(boneName);
	if( index == FxInvalidIndex )
	{
		return FxFalse;
	}
	_boneWeights.Remove(index);
	return FxTrue;
}

/** 2012 0xb08500. */
void FxAnim::RemoveAllBoneWeights()
{
	_boneWeights.Clear();
}

/** 2012 0xb08520: since version 6 every curve's keys go out as one block of (time, value, slopeIn, slopeOut)
    quadruples followed by the per-curve key counts. */
void FxAnim::Serialize( FxArchive& arc )
{
	FxNamedObject::Serialize(arc);
	FxUInt16 version = arc.SerializeClassVersion("FxAnim");
	if( arc.IsSaving() )
	{
		arc << _animCurves;
		FxSize numCurves = _animCurves.Length();
		FxSize numKeys = 0;
		FxArray<FxSize> curveNumKeys;
		curveNumKeys.Reserve(numCurves);
		for( FxSize i = 0; i < numCurves; ++i )
		{
			FxSize tempNumKeys = _animCurves[i].GetNumKeys();
			curveNumKeys.PushBack(tempNumKeys);
			numKeys += tempNumKeys;
		}
		arc << numKeys;
		if( numKeys )
		{
			FxReal* pKeys = static_cast<FxReal*>(FxAllocate(16 * numKeys));
			FxSize keyIndex = 0;
			for( FxSize index = 0; index < numCurves; ++index )
			{
				for( FxSize k = 0; k < curveNumKeys[index]; ++k )
				{
					const FxAnimKey& key = _animCurves[index].GetKey(k);
					pKeys[keyIndex++] = key.GetTime();
					pKeys[keyIndex++] = key.GetValue();
					pKeys[keyIndex++] = key.GetSlopeIn();
					pKeys[keyIndex++] = key.GetSlopeOut();
				}
			}
			arc.SerializePODArray(pKeys, 4 * numKeys);
			arc << curveNumKeys;
			FxFree(pKeys, 16 * numKeys);
		}
		arc << _blendInTime << _blendOutTime << _boneWeights << _eventPath << _eventWavFile << _eventIndex;
		return;
	}

	if( version < 4 )
	{
		FxSize seekPos = 0;
		arc << seekPos;
	}
	arc << _animCurves;
	if( version >= 6 )
	{
		FxSize numKeys = 0;
		arc << numKeys;
		if( numKeys )
		{
			FxReal* pKeys = static_cast<FxReal*>(FxAllocate(16 * numKeys));
			arc.SerializePODArray(pKeys, 4 * numKeys);
			FxArray<FxSize> curveNumKeys;
			arc << curveNumKeys;
			FxSize keysOffset = 0;
			FxSize numCurves = _animCurves.Length();
			for( FxSize j = 0; j < numCurves; ++j )
			{
				_animCurves[j].SetKeys(pKeys + keysOffset, curveNumKeys[j]);
				keysOffset += 4 * curveNumKeys[j];
			}
			FxFree(pKeys, 16 * numKeys);
		}
	}
	if( version < 5 )
	{
		FxUInt16 dummyVersion = 0;
		FxSize dummySize = 0;
		arc << dummyVersion << dummySize;
	}
	arc << _blendInTime << _blendOutTime;
	if( version >= 2 )
	{
		arc << _boneWeights;
	}
	if( version && version < 8 )
	{
		FxString soundCuePath;
		FxString soundNodeWave;
		arc << soundCuePath << soundNodeWave;
	}
	if( version >= 3 && version < 8 )
	{
		FxSize soundCueIndex = 0;
		arc << soundCueIndex;
	}
	if( version >= 7 )
	{
		arc << _eventPath << _eventWavFile << _eventIndex;
	}
	FindBounds();
}

/** 2012 0xb08a80. */
void FxAnim::SetAkEventPath( const FxString& eventPath )
{
	_eventPath = eventPath;
}

/** 2012 0xb08aa0. */
void FxAnim::SetAkEventWavFile( const FxString& eventWavFile )
{
	_eventWavFile = eventWavFile;
}

/** 2012 0xb08ac0. */
void FxAnim::SetAkEventIndex( FxSize eventIndex )
{
	_eventIndex = eventIndex;
}

/** 2012 0xb08ae0. */
void FxAnim::SetAkEventPointer( void* eventPointer )
{
	_eventPointer = eventPointer;
}

/** 2012 0xb08b00: the union of the curves' key time ranges, [0, 1] when there are no keys. */
void FxAnim::FindBounds()
{
	_startTime = FX_REAL_MAX;
	_endTime = FX_REAL_MIN;
	FxReal curveMinTime = 0.0f;
	FxReal curveMaxTime = 0.0f;
	FxSize numAnimCurves = _animCurves.Length();
	for( FxSize i = 0; i < numAnimCurves; ++i )
	{
		_animCurves[i].FindTimeExtents(curveMinTime, curveMaxTime);
		_startTime = _startTime <= curveMinTime ? _startTime : curveMinTime;
		_endTime = _endTime >= curveMaxTime ? _endTime : curveMaxTime;
	}
	if( _startTime == FX_REAL_MAX && _endTime == FX_REAL_MIN )
	{
		_startTime = 0.0f;
		_endTime = 1.0f;
	}
}

/** 2012 0xb08c10. */
FxSize FxAnim::_findAnimBoneWeight( const FxName& boneName ) const
{
	FxSize numBoneWeights = _boneWeights.Length();
	for( FxSize i = 0; i < numBoneWeights; ++i )
	{
		if( _boneWeights[i].boneName == boneName )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

/** 2012 0xbab430 (dynamic initializer). */
FxName FxAnimGroup::Default;

/** 2012 0xb08c80 (AllocateStaticClassFxAnimGroup). */
FX_IMPLEMENT_CLASS(FxAnimGroup, 0, FxNamedObject)

/** 2012 0xb08d10. */
FxAnimGroup::FxAnimGroup()
{
}

/** 2012 0xb08d80. */
FxAnimGroup::FxAnimGroup( const FxAnimGroup& other )
	: FxNamedObject(other)
	, _anims(other._anims)
{
}

/** 2012 0xb08df0. */
FxAnimGroup& FxAnimGroup::operator=( const FxAnimGroup& other )
{
	if( this != &other )
	{
		FxNamedObject::operator=(other);
		_anims = other._anims;
	}
	return *this;
}

/** 2012 0xb08e30. */
FxAnimGroup::~FxAnimGroup()
{
}

/** 2012 0xb08e90. */
FxBool FxAnimGroup::AddAnim( const FxAnim& anim )
{
	if( FindAnim(anim.GetName()) != FxInvalidIndex )
	{
		return FxFalse;
	}
	_anims.PushBack(anim);
	return FxTrue;
}

/** 2012 0xb08ed0. */
FxBool FxAnimGroup::RemoveAnim( const FxName& name )
{
	FxSize index = FindAnim(name);
	if( index == FxInvalidIndex )
	{
		return FxFalse;
	}
	_anims.Remove(index);
	return FxTrue;
}

/** 2012 0xb08f10. */
void FxAnimGroup::Serialize( FxArchive& arc )
{
	FxNamedObject::Serialize(arc);
	arc.SerializeClassVersion("FxAnimGroup");
	arc << _anims;
}

/** 2012 0xb155f0 (AllocateStaticClassFxAnimSet). */
FX_IMPLEMENT_CLASS(FxAnimSet, 0, FxNamedObject)

/** 2012 0xb15680. */
FxAnimSet::FxAnimSet()
{
}

/** 2012 0xb15700. */
FxAnimSet::FxAnimSet( const FxAnimSet& other )
	: FxNamedObject(other)
	, _owningActorName(other._owningActorName)
	, _animGroup(other._animGroup)
{
}

/** 2012 0xb15790. */
FxAnimSet& FxAnimSet::operator=( const FxAnimSet& other )
{
	if( this != &other )
	{
		FxNamedObject::operator=(other);
		_owningActorName = other._owningActorName;
		_animGroup = other._animGroup;
	}
	return *this;
}

/** 2012 0xb157e0. */
FxAnimSet::~FxAnimSet()
{
}

/** 2012 0xb15850. */
void FxAnimSet::SetOwningActorName( const FxName& owningActorName )
{
	_owningActorName = owningActorName;
}

/** 2012 0xb15870. */
const FxName& FxAnimSet::GetOwningActorName() const
{
	return _owningActorName;
}

/** 2012 0xb15890. */
void FxAnimSet::SetAnimGroup( const FxAnimGroup& animGroup )
{
	_animGroup = animGroup;
}

/** 2012 0xb158b0. */
const FxAnimGroup& FxAnimSet::GetAnimGroup() const
{
	return _animGroup;
}

/** 2012 0xb158d0. */
FxAnim* FxAnimSet::GetAnimPtr( FxSize index )
{
	return _animGroup.GetAnimPtr(index);
}

/** 2012 0xb158f0. */
void FxAnimSet::NullAllAnimUserDataPtrs()
{
	FxSize numAnims = _animGroup.GetNumAnims();
	for( FxSize i = 0; i < numAnims; ++i )
	{
		FxAnim* pAnim = _animGroup.GetAnimPtr(i);
		if( pAnim )
		{
			pAnim->SetUserData(0);
		}
	}
}

/** 2012 0xb15950. */
void FxAnimSet::Serialize( FxArchive& arc )
{
	FxNamedObject::Serialize(arc);
	arc.SerializeClassVersion("FxAnimSet");
	arc << _owningActorName << _animGroup;
}

/** 2012 0xb15bd0. */
FxBool FxLoadAnimSetFromMemory( FxAnimSet& animSet, const FxByte* pMemory, FxSize numBytes, void (*callbackFunction)(FxReal), FxReal updateFrequency )
{
	if( !pMemory )
	{
		return FxFalse;
	}
	FxArchive animSetArchive(FxArchiveStoreMemoryNoCopy::Create(pMemory, numBytes), FxArchive::AM_LinearLoad);
	animSetArchive.Open();
	if( !animSetArchive.IsValid() )
	{
		return FxFalse;
	}
	animSetArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
	animSetArchive << animSet;
	return FxTrue;
}

/** 2012 0xb15ca0: a directory pass over a null store, then the save into memory with that directory. */
FxBool FxSaveAnimSetToMemory( FxAnimSet& animSet, FxByte*& pMemory, FxSize& numBytes, FxArchive::FxArchiveByteOrder byteOrder, void (*callbackFunction)(FxReal), FxReal updateFrequency )
{
	if( pMemory )
	{
		return FxFalse;
	}
	FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << animSet;
	FxArchiveStoreMemory* pStore = FxArchiveStoreMemory::Create(0, 0);
	FxArchive animSetArchive(pStore, FxArchive::AM_Save, byteOrder);
	animSetArchive.Open();
	if( !animSetArchive.IsValid() )
	{
		return FxFalse;
	}
	animSetArchive.SetInternalDataState(directoryCreater.GetInternalData());
	animSetArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
	animSetArchive << animSet;
	numBytes = pStore->GetSize();
	pMemory = static_cast<FxByte*>(FxAllocate(numBytes));
	FxMemcpy(pMemory, pStore->GetMemory(), numBytes);
	return FxTrue;
}

} // namespace Face
} // namespace OC3Ent
