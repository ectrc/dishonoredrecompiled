// FaceFX 1.7.3.1: FxFaceGraphNodeUserProperty (fxfacegraphshared.cpp), FxFaceGraphNodeLink (fxfacegraphnodelink.cpp),
// FxFaceGraphNode (fxfacegraphnode.cpp). Agent FX2, wave 26, ported from the 2012 decompile.
#include "FxFaceGraphNode.h"

namespace OC3Ent
{
namespace Face
{

/** 2012 0xb29580 (AllocateStaticClassFxFaceGraphNodeUserProperty). */
FX_IMPLEMENT_CLASS(FxFaceGraphNodeUserProperty, 0, FxNamedObject)

/** 2012 0xb29610: a string property owns one empty choice to hold its value. */
FxFaceGraphNodeUserProperty::FxFaceGraphNodeUserProperty( FxFaceGraphNodeUserPropertyType propertyType )
	: _propertyType(propertyType)
	, _integerProperty(0)
	, _realProperty(0.0f)
{
	if( _propertyType == UPT_String )
	{
		_choices.Reserve(1);
		_choices.PushBack(FxString());
	}
}

/** 2012 0xb296e0. */
FxFaceGraphNodeUserProperty::FxFaceGraphNodeUserProperty( const FxFaceGraphNodeUserProperty& other )
	: FxNamedObject(other)
	, _propertyType(other._propertyType)
	, _integerProperty(other._integerProperty)
	, _realProperty(other._realProperty)
	, _choices(other._choices)
{
}

/** 2012 0xb29780. */
FxFaceGraphNodeUserProperty& FxFaceGraphNodeUserProperty::operator=( const FxFaceGraphNodeUserProperty& other )
{
	if( this != &other )
	{
		FxNamedObject::operator=(other);
		_propertyType = other._propertyType;
		_integerProperty = other._integerProperty;
		_realProperty = other._realProperty;
		_choices = other._choices;
	}
	return *this;
}

/** 2012 0xb297e0. */
FxFaceGraphNodeUserProperty::~FxFaceGraphNodeUserProperty()
{
}

/** 2012 0xb29840. */
void FxFaceGraphNodeUserProperty::SetIntegerProperty( FxInt32 integerProperty )
{
	if( _propertyType == UPT_Integer )
	{
		_integerProperty = integerProperty;
	}
}

/** 2012 0xb29860: no type check, as in retail. */
void FxFaceGraphNodeUserProperty::SetBoolProperty( FxBool boolProperty )
{
	_integerProperty = boolProperty;
}

/** 2012 0xb29890. */
void FxFaceGraphNodeUserProperty::SetRealProperty( FxReal realProperty )
{
	if( _propertyType == UPT_Real )
	{
		_realProperty = realProperty;
	}
}

/** 2012 0xb298b0. */
void FxFaceGraphNodeUserProperty::SetStringProperty( const FxString& stringProperty )
{
	if( _propertyType == UPT_String )
	{
		_choices[0] = stringProperty;
	}
}

/** 2012 0xb298e0: the last matching choice wins, the first when none does. */
void FxFaceGraphNodeUserProperty::SetChoiceProperty( const FxString& choiceProperty )
{
	if( _propertyType != UPT_Choice )
	{
		return;
	}
	_integerProperty = 0;
	FxSize numChoices = _choices.Length();
	for( FxSize i = 0; i < numChoices; ++i )
	{
		if( _choices[i] == choiceProperty )
		{
			_integerProperty = static_cast<FxInt32>(i);
		}
	}
}

/** 2012 0xb29960. */
void FxFaceGraphNodeUserProperty::AddChoice( const FxString& choice )
{
	_choices.PushBack(choice);
}

/** 2012 0xb29980. */
void FxFaceGraphNodeUserProperty::Serialize( FxArchive& arc )
{
	FxNamedObject::Serialize(arc);
	arc.SerializeClassVersion("FxFaceGraphNodeUserProperty");
	FxInt32 tempPropertyType = _propertyType;
	arc << tempPropertyType << _integerProperty << _realProperty << _choices;
	_propertyType = static_cast<FxFaceGraphNodeUserPropertyType>(tempPropertyType);
}

/** 2012 0xb2a240 (AllocateStaticClassFxFaceGraphNodeLink). */
FX_IMPLEMENT_CLASS(FxFaceGraphNodeLink, 0, FxObject)

/** 2012 0xb2a2d0. */
FxFaceGraphNodeLink::FxFaceGraphNodeLink()
	: _inputName(FxName::NullName)
	, _input(0)
	, _linkFnName(FxName::NullName)
	, _linkFnType(LFT_Invalid)
{
}

/** 2012 0xb2a380. */
FxFaceGraphNodeLink::FxFaceGraphNodeLink( const FxName& inputName, FxFaceGraphNode* inputNode, const FxName& linkFunctionName,
	FxLinkFnType linkFunctionType, const FxLinkFnParameters& linkFunctionParams )
	: _inputName(inputName)
	, _input(inputNode)
	, _linkFnName(linkFunctionName)
	, _linkFnType(linkFunctionType)
	, _linkFnParams(linkFunctionParams)
{
}

/** 2012 0xb2a430. */
FxFaceGraphNodeLink::FxFaceGraphNodeLink( const FxFaceGraphNodeLink& other )
	: FxObject()
	, FxDataContainer(other)
	, _inputName(other._inputName)
	, _input(other._input)
	, _linkFnName(other._linkFnName)
	, _linkFnType(other._linkFnType)
	, _linkFnParams(other._linkFnParams)
{
}

/** 2012 0xb2a510. */
FxFaceGraphNodeLink& FxFaceGraphNodeLink::operator=( const FxFaceGraphNodeLink& other )
{
	if( this == &other )
	{
		return *this;
	}
	FxDataContainer::operator=(other);
	_inputName = other._inputName;
	_input = other._input;
	_linkFnName = other._linkFnName;
	_linkFnType = other._linkFnType;
	_linkFnParams = other._linkFnParams;
	return *this;
}

/** 2012 0xb2a5c0. */
FxFaceGraphNodeLink::~FxFaceGraphNodeLink()
{
}

/** 2012 0xb2a640: a null node clears the name but keeps the old pointer, as in retail. */
void FxFaceGraphNodeLink::SetNode( FxFaceGraphNode* inputNode )
{
	if( inputNode )
	{
		_input = inputNode;
		_inputName = _input->GetName();
	}
	else
	{
		_inputName = FxName::NullName;
	}
}

/** 2012 0xb2a690. */
void FxFaceGraphNodeLink::SetLinkFnName( const FxName& linkFn )
{
	_linkFnName = linkFn;
	_linkFnType = FxLinkFn::FindLinkFunctionType(_linkFnName);
}

/** 2012 0xb2a6d0. */
void FxFaceGraphNodeLink::SetLinkFn( const FxLinkFn* linkFn )
{
	if( linkFn )
	{
		_linkFnType = linkFn->GetType();
		_linkFnName = linkFn->GetName();
	}
	else
	{
		_linkFnType = LFT_Invalid;
		_linkFnName = FxName::NullName;
	}
}

/** 2012 0xb2a730. */
void FxFaceGraphNodeLink::SetLinkFnParams( const FxLinkFnParameters& linkFnParams )
{
	_linkFnParams = linkFnParams;
}

/** 2012 0xb2a750. */
FxReal FxFaceGraphNodeLink::GetCorrectionFactor() const
{
	return _linkFnParams.parameters.Length() ? _linkFnParams.parameters[0] : 1.0f;
}

/** 2012 0xb2a780. */
void FxFaceGraphNodeLink::SetCorrectionFactor( FxReal newValue )
{
	if( _linkFnParams.parameters.Length() )
	{
		_linkFnParams.parameters[0] = newValue;
	}
	else
	{
		_linkFnParams.parameters.PushBack(newValue);
	}
}

/** 2012 0xb2a7c0: the link function travels by name; null links drop their parameters. */
void FxFaceGraphNodeLink::Serialize( FxArchive& arc )
{
	FxObject::Serialize(arc);
	arc.SerializeClassVersion("FxFaceGraphNodeLink");
	arc << _inputName << _linkFnName << _linkFnParams;
	_linkFnType = FxLinkFn::FindLinkFunctionType(_linkFnName);
	if( _linkFnType == LFT_Null )
	{
		_linkFnParams.parameters.Clear();
	}
}

/** 2012 0xb243c0 (AllocateStaticClassFxFaceGraphNode, class version 2, no ConstructObject). */
FX_IMPLEMENT_CLASS(FxFaceGraphNode, 2, FxNamedObject)

/** 2012 0xb24450. */
FxFaceGraphNode::FxFaceGraphNode()
	: _hasBeenVisited(FxFalse)
	, _min(0.0f)
	, _max(1.0f)
	, _inputOperation(IO_Sum)
{
}

/** 2012 0xb24510. */
FxFaceGraphNode::~FxFaceGraphNode()
{
}

/** 2012 0xb24590. */
void FxFaceGraphNode::CopyData( FxFaceGraphNode* pOther )
{
	if( !pOther )
	{
		return;
	}
	pOther->SetName(GetName());
	pOther->SetMin(GetMin());
	pOther->SetMax(GetMax());
	pOther->SetNodeOperation(GetNodeOperation());
	FxSize numUserProperties = GetNumUserProperties();
	for( FxSize i = 0; i < numUserProperties; ++i )
	{
		pOther->ReplaceUserProperty(GetUserProperty(i));
	}
}

/** 2012 0xb24630. */
void FxFaceGraphNode::SetMin( FxReal iMin )
{
	_min = iMin;
}

/** 2012 0xb24650. */
void FxFaceGraphNode::SetMax( FxReal iMax )
{
	_max = iMax;
}

/** 2012 0xb24670. */
void FxFaceGraphNode::SetNodeOperation( FxInputOp iNodeOperation )
{
	_inputOperation = iNodeOperation;
}

/** 2012 0xb24690. */
FxBool FxFaceGraphNode::AddInputLink( const FxFaceGraphNodeLink& newInputLink )
{
	_inputs.PushBack(newInputLink);
	return FxTrue;
}

/** 2012 0xb246b0. */
void FxFaceGraphNode::ModifyInputLink( FxSize index, const FxFaceGraphNodeLink& inputLink )
{
	_inputs[index] = inputLink;
}

/** 2012 0xb246e0. */
void FxFaceGraphNode::RemoveInputLink( FxSize index )
{
	_inputs.Remove(index);
}

/** 2012 0xb24700. */
void FxFaceGraphNode::AddOutput( FxFaceGraphNode* pOutput )
{
	_outputs.PushBack(pOutput);
}

/** 2012 0xb24720. */
void FxFaceGraphNode::AddUserProperty( const FxFaceGraphNodeUserProperty& userProperty )
{
	if( FindUserProperty(userProperty.GetName()) == FxInvalidIndex )
	{
		_userProperties.PushBack(userProperty);
	}
}

/** 2012 0xb24760. */
FxSize FxFaceGraphNode::GetNumUserProperties() const
{
	return _userProperties.Length();
}

/** 2012 0xb24780. */
FxSize FxFaceGraphNode::FindUserProperty( const FxName& userPropertyName ) const
{
	FxSize numUserProperties = _userProperties.Length();
	for( FxSize i = 0; i < numUserProperties; ++i )
	{
		if( _userProperties[i].GetName() == userPropertyName )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

/** 2012 0xb247f0. */
const FxFaceGraphNodeUserProperty& FxFaceGraphNode::GetUserProperty( FxSize index ) const
{
	return _userProperties[index];
}

/** 2012 0xb24810. */
void FxFaceGraphNode::ReplaceUserProperty( const FxFaceGraphNodeUserProperty& userProperty )
{
	FxSize index = FindUserProperty(userProperty.GetName());
	if( index != FxInvalidIndex )
	{
		_userProperties[index] = userProperty;
	}
}

/** 2012 0xb24860: version 1 added the user properties, version 2 the output count (to presize the array). */
void FxFaceGraphNode::Serialize( FxArchive& arc )
{
	FxNamedObject::Serialize(arc);
	FxUInt16 version = arc.SerializeClassVersion("FxFaceGraphNode");
	FxInt32 tempNodeOp = _inputOperation;
	arc << _min << _max << tempNodeOp << _inputs;
	_inputOperation = static_cast<FxInputOp>(tempNodeOp);
	if( version )
	{
		arc << _userProperties;
	}
	if( version >= 2 )
	{
		FxSize numOutputs = _outputs.Length();
		arc << numOutputs;
		if( arc.IsLoading() )
		{
			_outputs.Reserve(numOutputs);
		}
	}
}

/** 2012 0xb24940. */
FxBool FxFaceGraphNode::_hasCycles() const
{
	if( _hasBeenVisited )
	{
		return FxTrue;
	}
	_hasBeenVisited = FxTrue;
	FxBool inputHasCycles = FxFalse;
	FxSize numInputs = _inputs.Length();
	for( FxSize i = 0; i < numInputs; ++i )
	{
		inputHasCycles |= _inputs[i].GetNode()->_hasCycles();
	}
	_hasBeenVisited = FxFalse;
	return inputHasCycles;
}

} // namespace Face
} // namespace OC3Ent
