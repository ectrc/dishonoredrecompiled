/*=============================================================================
	FxActorInstance.h - FaceFX 1.7.3.1 actor instance (fxactorinstance.h): one playing face on an FxActor - the
	animation being played, its clock, and the register (one per compiled node) the face graph evaluates into.

	DISHONORED(port): agent FX3 (wave 26). 0x44 bytes, FxAnimPlaybackInfo 36 bytes with its double at offset 12
	(/Zp4, 2012 PDB). Header inlines retail emitted: IsPlayingAnim (engine units), GetCurrentAnimGroupName 0xb0d610.
	SetRegister is SetRegisterEx(name, op, value, duration, RO_Invalid, 0, 0) - the call USkeletalMeshComponent::
	SetFaceFXRegister (2012 0x335630) inlined. GetNumBones / GetBone read the master bone list blend
	(FxMasterBoneList::GetBlendedBone), as the engine's inlined bone pass (2012 0x347b90) does.
	GetAnimPlaybackState / GetCurrentAnim / GetCurrentAnimName / GetAnimCurrentTime are read-only accessors this port adds
	for the engine's -fx3diag log (no retail counterpart).
=============================================================================*/
#ifndef FxActorInstance_H__
#define FxActorInstance_H__

#include "FxActor.h"

namespace OC3Ent
{
namespace Face
{

enum FxAnimPlaybackState
{
	APS_StartAudio = 0,
	APS_Playing,
	APS_Stopped
};

class FxActorInstance : public FxNamedObject
{
	FX_DECLARE_CLASS(FxActorInstance, FxNamedObject)
public:
	FxActorInstance(FxActor* actor = NULL);
	virtual ~FxActorInstance();

	FxActor* GetActor()
	{
		return _pActor;
	}

	void SetActor(FxActor* pActor);

	FxBool IsOpenInStudio() const
	{
		return _isOpenInStudio;
	}

	void SetIsOpenInStudio(FxBool isOpenInStudio);

	FxBool GetAllowNonAnimTick() const
	{
		return _allowNonAnimTick;
	}

	void SetAllowNonAnimTick(FxBool allowNonAnimTick);

	FxBool PlayAnim(const FxName& animName, const FxName& groupName = FxAnimGroup::Default, FxBool ignoreNegativeTime = FxFalse);
	void StopAnim();

	FxBool IsPlayingAnim() const
	{
		return _animPlaybackInfo.state != APS_Stopped;
	}

	FxAnimPlaybackState GetAnimPlaybackState() const
	{
		return _animPlaybackInfo.state;
	}

	const FxAnim* GetCurrentAnim() const
	{
		return _animPlaybackInfo.pAnim;
	}

	/** 2012 0xb0d610. */
	const FxName& GetCurrentAnimGroupName() const
	{
		return _animPlaybackInfo.currentAnimGroupName;
	}

	const FxName& GetCurrentAnimName() const
	{
		return _animPlaybackInfo.currentAnimName;
	}

	FxReal GetAnimCurrentTime() const
	{
		return _animPlaybackInfo.currentTime;
	}

	virtual void BeginFrame();
	virtual void EndFrame();

	FxAnimPlaybackState Tick(const FxDReal appTime, const FxReal audioTime);
	void ForceTick(const FxName& animName, const FxName& groupName, FxReal forcedAnimationTime);

	FxBool SetRegisterEx(const FxName& regName, FxRegisterOp firstRegOp, FxReal firstValue, FxReal firstInterpDuration,
		FxRegisterOp nextRegOp, FxReal nextValue, FxReal nextInterpDuration);

	FxBool SetRegister(const FxName& regName, FxReal newValue, FxRegisterOp regOp, FxReal interpDuration)
	{
		return SetRegisterEx(regName, regOp, newValue, interpDuration, RO_Invalid, 0.0f, 0.0f);
	}

	void SetAllRegisters(FxRegisterOp firstRegOp, FxReal firstValue, FxReal firstInterpDuration,
		FxRegisterOp nextRegOp, FxReal nextValue, FxReal nextInterpDuration);
	FxReal GetRegister(const FxName& regName);
	FxBool GetRegisterState(const FxName& regName, FxRegister& regState);
	FxReal GetOriginalRegisterValue(const FxName& regName);

	FxSize GetNumBones()
	{
		return _pActor->GetMasterBoneList().GetNumRefBones();
	}

	void GetBone(FxSize index, FxVec3& bonePos, FxQuat& boneRot, FxVec3& boneScale, FxReal& boneWeight)
	{
		_pActor->GetMasterBoneList().GetBlendedBone(index, _pActor->GetCompiledFaceGraph(), bonePos, boneRot, boneScale, boneWeight);
	}

protected:
	struct FxAnimPlaybackInfo
	{
		FxAnimPlaybackInfo();
		~FxAnimPlaybackInfo();

		FxName currentAnimGroupName;
		FxName currentAnimName;
		FxAnim* pAnim;
		FxDReal startTime;
		FxReal currentTime;
		FxReal leadinDuration;
		FxBool hasStartedAudio;
		FxBool ignoreNegativeTime;
		FxAnimPlaybackState state;
	};

	FxActor* _pActor;
	FxBool _isOpenInStudio;
	FxBool _allowNonAnimTick;
	FxAnimPlaybackInfo _animPlaybackInfo;
	FxArray<FxRegister> _registers;
	FxBool _hasBeenTicked;
};

} // namespace Face
} // namespace OC3Ent

#endif
