/*=============================================================================
	FxActorInstance.cpp - FaceFX 1.7.3.1 actor instance (fxactorinstance.cpp): animation playback and registers.

	DISHONORED(port): agent FX3 (wave 26), from the 2012 decompile (FxActorInstance.obj). The instance's clock is
	the caller's (UE3 passes USkeletalMeshComponent::m_fFaceFxTickTime, an accumulated DeltaTime); an animation starts
	at the first Tick after PlayAnim. An audio time >= 0 re-bases the clock on the audio; with -1 the animation runs on
	the caller's clock from its (possibly negative) start time, and Tick returns APS_StartAudio once, on the first
	tick at which the animation time reaches 0 - that is when the engine posts the voice (DisFaceFxStartAudio).
=============================================================================*/
#include "FxActorInstance.h"

namespace OC3Ent
{
namespace Face
{

static_assert(sizeof(FxRegister) == 48, "FxRegister: 2012 PDB sizeof 48");
static_assert(sizeof(FxActorInstance) == 0x44, "FxActorInstance: AllocateStaticClassFxActorInstance 0x44");

static const FxReal FxInvalidTime = static_cast<FxReal>(FX_INT32_MIN);

/** 2012 0xb0ab60 (AllocateStaticClassFxActorInstance). */
FX_IMPLEMENT_CLASS(FxActorInstance, 0, FxNamedObject)

/** 2012 0xb0abf0. */
FxActorInstance::FxActorInstance(FxActor* actor)
	: _pActor(NULL)
	, _isOpenInStudio(FxFalse)
	, _allowNonAnimTick(FxFalse)
	, _hasBeenTicked(FxFalse)
{
	SetActor(actor);
}

/** 2012 0xb0aca0. */
FxActorInstance::~FxActorInstance()
{
	if( _pActor )
	{
		_pActor->RemoveInstance(this);
	}
}

/** 2012 0xb0ad30. One default register per compiled node; the instance list of the actor is kept in step. */
void FxActorInstance::SetActor(FxActor* pActor)
{
	FxBool isSameActor = FxFalse;
	if( _pActor && pActor )
	{
		isSameActor = _pActor == pActor;
	}
	if( _pActor )
	{
		if( !isSameActor )
		{
			_pActor->RemoveInstance(this);
		}
		_registers.Clear();
		_pActor = NULL;
	}
	_pActor = pActor;
	if( _pActor )
	{
		if( !isSameActor )
		{
			_pActor->AddInstance(this);
		}
		FxCompiledFaceGraph& cg = _pActor->GetCompiledFaceGraph();
		FxSize numNodes = cg.nodes.Length();
		_registers.Reserve(numNodes);
		for( FxSize i = 0; i < numNodes; ++i )
		{
			_registers.PushBack(FxRegister());
		}
	}
}

/** 2012 0xb0ae20. */
void FxActorInstance::SetIsOpenInStudio(FxBool isOpenInStudio)
{
	_isOpenInStudio = isOpenInStudio;
}

/** 2012 0xb0ae40. */
void FxActorInstance::SetAllowNonAnimTick(FxBool allowNonAnimTick)
{
	_allowNonAnimTick = allowNonAnimTick;
}

/** 2012 0xb0ae60. */
FxBool FxActorInstance::PlayAnim(const FxName& animName, const FxName& groupName, FxBool ignoreNegativeTime)
{
	StopAnim();
	_animPlaybackInfo.pAnim = _pActor->GetAnimPtr(groupName, animName);
	if( !_animPlaybackInfo.pAnim )
	{
		return FxFalse;
	}
	_animPlaybackInfo.currentAnimGroupName = groupName;
	_animPlaybackInfo.currentAnimName = animName;
	_animPlaybackInfo.ignoreNegativeTime = ignoreNegativeTime;
	_animPlaybackInfo.state = APS_Playing;
	return FxTrue;
}

/** 2012 0xb0aed0. */
void FxActorInstance::StopAnim()
{
	_animPlaybackInfo.currentAnimGroupName = FxName::NullName;
	_animPlaybackInfo.currentAnimName = FxName::NullName;
	_animPlaybackInfo.pAnim = NULL;
	_animPlaybackInfo.startTime = FxInvalidTime;
	_animPlaybackInfo.currentTime = FxInvalidTime;
	_animPlaybackInfo.leadinDuration = FxInvalidTime;
	_animPlaybackInfo.hasStartedAudio = FxFalse;
	_animPlaybackInfo.ignoreNegativeTime = FxFalse;
	_animPlaybackInfo.state = APS_Stopped;
}

/** 2012 0xb0af50. */
void FxActorInstance::BeginFrame()
{
}

/** 2012 0xb0af60. */
void FxActorInstance::EndFrame()
{
}

/** 2012 0xb0af70. */
FxAnimPlaybackState FxActorInstance::Tick(const FxDReal appTime, const FxReal audioTime)
{
	FxCompiledFaceGraph& cg = _pActor->GetCompiledFaceGraph();
	if( _animPlaybackInfo.state == APS_Stopped )
	{
		if( _allowNonAnimTick )
		{
			cg.Tick(static_cast<FxReal>(appTime), _registers, _hasBeenTicked);
			return APS_Playing;
		}
		if( _isOpenInStudio )
		{
			cg.Tick(static_cast<FxReal>(appTime), _registers, _hasBeenTicked);
		}
		return APS_Stopped;
	}
	if( _animPlaybackInfo.pAnim )
	{
		FxMasterBoneList& masterBoneList = _pActor->GetMasterBoneList();
		masterBoneList.ResetRefBoneWeights();
		FxSize numBoneWeights = _animPlaybackInfo.pAnim->GetNumBoneWeights();
		for( FxSize i = 0; i < numBoneWeights; ++i )
		{
			const FxAnimBoneWeight& boneWeight = _animPlaybackInfo.pAnim->GetBoneWeight(i);
			masterBoneList.SetRefBoneCurrentWeight(boneWeight.boneName, boneWeight.boneWeight);
		}
		if( _animPlaybackInfo.state == APS_Playing )
		{
			if( FxDRealEquality(_animPlaybackInfo.startTime, static_cast<FxDReal>(FxInvalidTime)) )
			{
				FxReal animStartTime = _animPlaybackInfo.pAnim->GetStartTime();
				if( animStartTime >= 0.0f || _animPlaybackInfo.ignoreNegativeTime )
				{
					_animPlaybackInfo.leadinDuration = 0.0f;
				}
				else
				{
					_animPlaybackInfo.leadinDuration = FxAbs(animStartTime);
				}
				_animPlaybackInfo.startTime = appTime;
			}
			if( audioTime >= 0.0f )
			{
				_animPlaybackInfo.currentTime = audioTime;
				_animPlaybackInfo.startTime = appTime - audioTime - _animPlaybackInfo.leadinDuration;
			}
			else
			{
				FxReal elapsed = static_cast<FxReal>(appTime - _animPlaybackInfo.startTime);
				_animPlaybackInfo.currentTime = elapsed - _animPlaybackInfo.leadinDuration;
			}
			FxSize numCurves = _animPlaybackInfo.pAnim->GetNumAnimCurves();
			for( FxSize index = 0; index < numCurves; ++index )
			{
				const FxAnimCurve& curve = _animPlaybackInfo.pAnim->GetAnimCurve(index);
				FxSize nodeIndex = cg.FindNodeIndex(curve.GetName());
				if( nodeIndex != FxInvalidIndex )
				{
					cg.nodes[nodeIndex].trackValue = curve.EvaluateAt(_animPlaybackInfo.currentTime);
				}
			}
		}
		cg.Tick(static_cast<FxReal>(appTime), _registers, _hasBeenTicked);
		if( _animPlaybackInfo.state == APS_Playing && _animPlaybackInfo.currentTime >= _animPlaybackInfo.pAnim->GetEndTime() )
		{
			StopAnim();
		}
		if( _animPlaybackInfo.state == APS_Playing && !_animPlaybackInfo.hasStartedAudio && _animPlaybackInfo.currentTime >= 0.0f )
		{
			_animPlaybackInfo.hasStartedAudio = FxTrue;
			return APS_StartAudio;
		}
	}
	return _animPlaybackInfo.state;
}

/** 2012 0xb0b260. Evaluates the animation at a given time and stops it again (UE3 matinee scrubbing). */
void FxActorInstance::ForceTick(const FxName& animName, const FxName& groupName, FxReal forcedAnimationTime)
{
	PlayAnim(animName, groupName, FxFalse);
	SetAllRegisters(RO_None, 0.0f, 0.0f, RO_Invalid, 0.0f, 0.0f);
	if( _animPlaybackInfo.pAnim )
	{
		FxMasterBoneList& masterBoneList = _pActor->GetMasterBoneList();
		masterBoneList.ResetRefBoneWeights();
		FxSize numBoneWeights = _animPlaybackInfo.pAnim->GetNumBoneWeights();
		for( FxSize i = 0; i < numBoneWeights; ++i )
		{
			const FxAnimBoneWeight& boneWeight = _animPlaybackInfo.pAnim->GetBoneWeight(i);
			masterBoneList.SetRefBoneCurrentWeight(boneWeight.boneName, boneWeight.boneWeight);
		}
		FxCompiledFaceGraph& cg = _pActor->GetCompiledFaceGraph();
		FxSize numCurves = _animPlaybackInfo.pAnim->GetNumAnimCurves();
		for( FxSize index = 0; index < numCurves; ++index )
		{
			const FxAnimCurve& curve = _animPlaybackInfo.pAnim->GetAnimCurve(index);
			FxSize nodeIndex = cg.FindNodeIndex(curve.GetName());
			if( nodeIndex != FxInvalidIndex )
			{
				cg.nodes[nodeIndex].trackValue = curve.EvaluateAt(forcedAnimationTime);
			}
		}
		cg.Tick(forcedAnimationTime, _registers, _hasBeenTicked);
	}
	StopAnim();
}

/** 2012 0xb0b3d0. The interpolation starts from the register's value (load ops) or its last interpolated value. */
FxBool FxActorInstance::SetRegisterEx(const FxName& regName, FxRegisterOp firstRegOp, FxReal firstValue, FxReal firstInterpDuration,
	FxRegisterOp nextRegOp, FxReal nextValue, FxReal nextInterpDuration)
{
	FxSize regIndex = _pActor->GetCompiledFaceGraph().FindNodeIndex(regName);
	if( regIndex == FxInvalidIndex )
	{
		return FxFalse;
	}
	FxRegister& reg = _registers[regIndex];
	FxBool isLoadOp = firstRegOp == RO_LoadAdd || firstRegOp == RO_LoadMultiply || firstRegOp == RO_LoadReplace;
	reg.interpStartValue = isLoadOp ? reg.value : reg.interpLastValue;
	reg.interpEndValue = firstValue;
	reg.interpNextEndValue = nextValue;
	reg.interpInverseDuration = firstInterpDuration == 0.0f ? 0.0f : 1.0f / firstInterpDuration;
	reg.interpNextInverseDuration = nextInterpDuration == 0.0f ? 0.0f : 1.0f / nextInterpDuration;
	reg.interpStartTime = 0.0f;
	reg.firstRegOp = FxRegister::GetUserOpForRegOp(firstRegOp);
	reg.nextRegOp = nextRegOp;
	reg.isInterpolating = FxFalse;
	return FxTrue;
}

/** 2012 0xb0b560. */
void FxActorInstance::SetAllRegisters(FxRegisterOp firstRegOp, FxReal firstValue, FxReal firstInterpDuration,
	FxRegisterOp nextRegOp, FxReal nextValue, FxReal nextInterpDuration)
{
	FxSize numRegisters = _registers.Length();
	for( FxSize i = 0; i < numRegisters; ++i )
	{
		FxRegister& reg = _registers[i];
		FxBool isLoadOp = firstRegOp == RO_LoadAdd || firstRegOp == RO_LoadMultiply || firstRegOp == RO_LoadReplace;
		reg.interpStartValue = isLoadOp ? reg.value : reg.interpLastValue;
		reg.interpEndValue = firstValue;
		reg.interpNextEndValue = nextValue;
		reg.interpInverseDuration = firstInterpDuration == 0.0f ? 0.0f : 1.0f / firstInterpDuration;
		reg.interpNextInverseDuration = nextInterpDuration == 0.0f ? 0.0f : 1.0f / nextInterpDuration;
		reg.interpStartTime = 0.0f;
		reg.firstRegOp = FxRegister::GetUserOpForRegOp(firstRegOp);
		reg.nextRegOp = nextRegOp;
		reg.isInterpolating = FxFalse;
	}
}

/** 2012 0xb0b6f0. */
FxReal FxActorInstance::GetRegister(const FxName& regName)
{
	FxSize regIndex = _pActor->GetCompiledFaceGraph().FindNodeIndex(regName);
	if( regIndex == FxInvalidIndex )
	{
		return 0.0f;
	}
	return _registers[regIndex].value;
}

/** 2012 0xb0b740. */
FxBool FxActorInstance::GetRegisterState(const FxName& regName, FxRegister& regState)
{
	FxSize regIndex = _pActor->GetCompiledFaceGraph().FindNodeIndex(regName);
	if( regIndex == FxInvalidIndex )
	{
		return FxFalse;
	}
	regState = _registers[regIndex];
	return FxTrue;
}

/** 2012 0xb0b790. The value the last SetRegister asked for. */
FxReal FxActorInstance::GetOriginalRegisterValue(const FxName& regName)
{
	FxSize regIndex = _pActor->GetCompiledFaceGraph().FindNodeIndex(regName);
	if( regIndex == FxInvalidIndex )
	{
		return 0.0f;
	}
	return _registers[regIndex].interpEndValue;
}

/** 2012 0xb0b840. */
FxActorInstance::FxAnimPlaybackInfo::FxAnimPlaybackInfo()
	: pAnim(NULL)
	, startTime(FxInvalidTime)
	, currentTime(FxInvalidTime)
	, leadinDuration(FxInvalidTime)
	, hasStartedAudio(FxFalse)
	, ignoreNegativeTime(FxFalse)
	, state(APS_Stopped)
{
}

/** 2012 0xb0b910. */
FxActorInstance::FxAnimPlaybackInfo::~FxAnimPlaybackInfo()
{
}

} // namespace Face
} // namespace OC3Ent
