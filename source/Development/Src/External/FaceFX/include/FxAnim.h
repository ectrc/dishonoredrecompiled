// FaceFX 1.7.3.1: FxAnimBoneWeight, FxAnim, FxAnimGroup (fxanim.h) and FxAnimSet (fxanimset.h). Agent FX2, wave 26.
// Arkane delta (FxAnim class version 8): the UE3 sound cue fields (versions 1-7) became a Wwise event path, wav file,
// event index and runtime event pointer (SetAkEventPath/WavFile/Index/Pointer 0xb08a80-0xb08ae0).
#ifndef FxAnim_H__
#define FxAnim_H__

#include "FxAnimCurve.h"

namespace OC3Ent
{
namespace Face
{

struct FxAnimBoneWeight
{
	/** 2012 0xb0aa50 (boneWeight left as is, like retail). */
	FxAnimBoneWeight() {}

	FxName boneName;
	FxReal boneWeight;
};

FxArchive& operator<<( FxArchive& arc, FxAnimBoneWeight& animBoneWeight );

class FxAnim : public FxNamedObject, public FxDataContainer
{
	FX_DECLARE_CLASS(FxAnim, FxNamedObject)
public:
	FxAnim();
	FxAnim( const FxAnim& other );
	FxAnim& operator=( const FxAnim& other );
	virtual ~FxAnim();

	FxSize GetNumAnimCurves() const { return _animCurves.Length(); }
	const FxAnimCurve& GetAnimCurve( FxSize index ) const { return _animCurves[index]; }
	FxAnimCurve& GetAnimCurveM( FxSize index ) { return _animCurves[index]; }
	FxSize FindAnimCurve( const FxName& name ) const;
	FxBool AddAnimCurve( const FxAnimCurve& curve );
	FxBool InsertAnimCurve( const FxAnimCurve& curve, FxSize index );
	FxBool RemoveAnimCurve( const FxName& name );

	FxReal GetStartTime() const { return _startTime; }
	/** 2012 0xb0bbf0. */
	FxReal GetEndTime() const { return _endTime; }
	FxReal GetDuration() const { return _endTime - _startTime; }
	FxReal GetBlendInTime() const { return _blendInTime; }
	FxReal GetBlendOutTime() const { return _blendOutTime; }
	void SetBlendInTime( FxReal blendInTime );
	void SetBlendOutTime( FxReal blendOutTime );

	/** 2012 0xb129f0 (folded with FxFaceGraphNode::GetNumOutputs). */
	FxSize GetNumBoneWeights() const { return _boneWeights.Length(); }
	/** 2012 0xb0bc10. */
	const FxAnimBoneWeight& GetBoneWeight( FxSize index ) const { return _boneWeights[index]; }
	FxBool AddBoneWeight( const FxAnimBoneWeight& animBoneWeight );
	FxBool RemoveBoneWeight( const FxName& boneName );
	void RemoveAllBoneWeights();

	const FxString& GetAkEventPath() const { return _eventPath; }
	const FxString& GetAkEventWavFile() const { return _eventWavFile; }
	FxSize GetAkEventIndex() const { return _eventIndex; }
	void* GetAkEventPointer() const { return _eventPointer; }
	void SetAkEventPath( const FxString& eventPath );
	void SetAkEventWavFile( const FxString& eventWavFile );
	void SetAkEventIndex( FxSize eventIndex );
	void SetAkEventPointer( void* eventPointer );

	void FindBounds();

	virtual void Serialize( FxArchive& arc );

private:
	FxSize _findAnimBoneWeight( const FxName& boneName ) const;

	FxArray<FxAnimCurve> _animCurves;
	FxReal _startTime;
	FxReal _endTime;
	FxReal _blendInTime;
	FxReal _blendOutTime;
	FxArray<FxAnimBoneWeight> _boneWeights;
	FxString _eventPath;
	FxString _eventWavFile;
	FxSize _eventIndex;
	void* _eventPointer;
};

class FxAnimGroup : public FxNamedObject
{
	FX_DECLARE_CLASS(FxAnimGroup, FxNamedObject)
public:
	FxAnimGroup();
	FxAnimGroup( const FxAnimGroup& other );
	FxAnimGroup& operator=( const FxAnimGroup& other );
	virtual ~FxAnimGroup();

	FxSize GetNumAnims() const { return _anims.Length(); }
	const FxAnim& GetAnim( FxSize index ) const { return _anims[index]; }
	/** 2012 0x568720. */
	FxAnim* GetAnimPtr( FxSize index ) { return &_anims[index]; }
	/** 2012 0x22bad0. */
	FxSize FindAnim( const FxName& name ) const
	{
		for( FxSize i = 0; i < _anims.Length(); ++i )
		{
			if( _anims[i].GetName() == name )
			{
				return i;
			}
		}
		return FxInvalidIndex;
	}
	FxBool AddAnim( const FxAnim& anim );
	FxBool RemoveAnim( const FxName& name );

	virtual void Serialize( FxArchive& arc );

	static FxName Default;

private:
	FxArray<FxAnim> _anims;
};

class FxAnimSet : public FxNamedObject
{
	FX_DECLARE_CLASS(FxAnimSet, FxNamedObject)
public:
	FxAnimSet();
	FxAnimSet( const FxAnimSet& other );
	FxAnimSet& operator=( const FxAnimSet& other );
	virtual ~FxAnimSet();

	void SetOwningActorName( const FxName& owningActorName );
	const FxName& GetOwningActorName() const;
	void SetAnimGroup( const FxAnimGroup& animGroup );
	const FxAnimGroup& GetAnimGroup() const;
	FxAnim* GetAnimPtr( FxSize index );
	void NullAllAnimUserDataPtrs();

	virtual void Serialize( FxArchive& arc );

private:
	FxName _owningActorName;
	FxAnimGroup _animGroup;
};

FxBool FxLoadAnimSetFromMemory( FxAnimSet& animSet, const FxByte* pMemory, FxSize numBytes, void (*callbackFunction)(FxReal) = 0, FxReal updateFrequency = 0.01f );
FxBool FxSaveAnimSetToMemory( FxAnimSet& animSet, FxByte*& pMemory, FxSize& numBytes, FxArchive::FxArchiveByteOrder byteOrder = FxArchive::ABO_LittleEndian, void (*callbackFunction)(FxReal) = 0, FxReal updateFrequency = 0.01f );

} // namespace Face
} // namespace OC3Ent

#endif
