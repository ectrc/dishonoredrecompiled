// FaceFX 1.7.3.1: FxFaceGraphNodeUserProperty (fxfacegraphshared.h), FxFaceGraphNodeLink (fxfacegraphnodelink.h),
// FxFaceGraphNode (fxfacegraphnode.h). Agent FX2, wave 26.
// Deviation: three SDK headers in one (they include each other in the SDK).
#ifndef FxFaceGraphNode_H__
#define FxFaceGraphNode_H__

#include "FxLinkFn.h"

namespace OC3Ent
{
namespace Face
{

enum FxInputOp
{
	IO_Invalid = -1,
	IO_Sum = 0,
	IO_Multiply,
	IO_Max,
	IO_Min
};

enum FxFaceGraphNodeUserPropertyType
{
	UPT_Integer = 0,
	UPT_Bool,
	UPT_Real,
	UPT_String,
	UPT_Choice
};

enum FxLinkErrorCode
{
	LEC_None = 0,
	LEC_Invalid,
	LEC_Duplicate,
	LEC_Cycle,
	LEC_Refused
};

class FxFaceGraphNodeUserProperty : public FxNamedObject
{
	FX_DECLARE_CLASS(FxFaceGraphNodeUserProperty, FxNamedObject)
public:
	FxFaceGraphNodeUserProperty( FxFaceGraphNodeUserPropertyType propertyType = UPT_Integer );
	FxFaceGraphNodeUserProperty( const FxFaceGraphNodeUserProperty& other );
	FxFaceGraphNodeUserProperty& operator=( const FxFaceGraphNodeUserProperty& other );
	virtual ~FxFaceGraphNodeUserProperty();

	FxFaceGraphNodeUserPropertyType GetPropertyType() const { return _propertyType; }
	FxInt32 GetIntegerProperty() const { return _integerProperty; }
	FxBool GetBoolProperty() const { return _integerProperty != 0; }
	FxReal GetRealProperty() const { return _realProperty; }
	const FxString& GetStringProperty() const { return _choices[0]; }
	const FxString& GetChoiceProperty() const { return _choices[_integerProperty]; }
	FxSize GetNumChoices() const { return _choices.Length(); }
	const FxString& GetChoice( FxSize index ) const { return _choices[index]; }
	void SetIntegerProperty( FxInt32 integerProperty );
	void SetBoolProperty( FxBool boolProperty );
	void SetRealProperty( FxReal realProperty );
	void SetStringProperty( const FxString& stringProperty );
	void SetChoiceProperty( const FxString& choiceProperty );
	void AddChoice( const FxString& choice );

	virtual void Serialize( FxArchive& arc );

private:
	FxFaceGraphNodeUserPropertyType _propertyType;
	FxInt32 _integerProperty;
	FxReal _realProperty;
	FxArray<FxString> _choices;
};

class FxFaceGraphNode;

class FxFaceGraphNodeLink : public FxObject, public FxDataContainer
{
	FX_DECLARE_CLASS(FxFaceGraphNodeLink, FxObject)
public:
	FxFaceGraphNodeLink();
	FxFaceGraphNodeLink( const FxName& inputName, FxFaceGraphNode* inputNode, const FxName& linkFunctionName,
		FxLinkFnType linkFunctionType, const FxLinkFnParameters& linkFunctionParams );
	FxFaceGraphNodeLink( const FxFaceGraphNodeLink& other );
	FxFaceGraphNodeLink& operator=( const FxFaceGraphNodeLink& other );
	virtual ~FxFaceGraphNodeLink();

	const FxName& GetNodeName() const { return _inputName; }
	/** 2012 0xb12a80. */
	const FxFaceGraphNode* GetNode() const { return _input; }
	FxFaceGraphNode* GetNode() { return _input; }
	void SetNode( FxFaceGraphNode* inputNode );
	/** 2012 0xb20fa0. */
	FxName GetLinkFnName() const { return _linkFnName; }
	FxLinkFnType GetLinkFnType() const { return _linkFnType; }
	const FxLinkFn* GetLinkFn() const { return FxLinkFn::FindLinkFunctionByType(_linkFnType); }
	void SetLinkFnName( const FxName& linkFn );
	void SetLinkFn( const FxLinkFn* linkFn );
	/** 2012 0xb123c0. */
	const FxLinkFnParameters& GetLinkFnParams() const { return _linkFnParams; }
	void SetLinkFnParams( const FxLinkFnParameters& linkFnParams );
	FxReal GetCorrectionFactor() const;
	void SetCorrectionFactor( FxReal newValue );

	virtual void Serialize( FxArchive& arc );

private:
	FxName _inputName;
	FxFaceGraphNode* _input;
	FxName _linkFnName;
	FxLinkFnType _linkFnType;
	FxLinkFnParameters _linkFnParams;
};

class FxFaceGraphNode : public FxNamedObject, public FxDataContainer
{
	FX_DECLARE_ABSTRACT_CLASS(FxFaceGraphNode, FxNamedObject)
public:
	FxFaceGraphNode();
	virtual ~FxFaceGraphNode();

	virtual FxFaceGraphNode* Clone() = 0;
	virtual void CopyData( FxFaceGraphNode* pOther );
	virtual FxBool AddInputLink( const FxFaceGraphNodeLink& newInputLink );

	/** 2012 0xb20da0. */
	FxBool IsA( const FxClass* classDesc ) const { return IsExactKindOf(classDesc); }
	/** 2012 0xb20dc0. */
	FxBool IsExactKindOf( const FxClass* classDesc ) const { const FxClass* testClass = GetClassDesc(); return testClass && testClass->IsExactKindOf(classDesc); }

	/** 2012 0xb123e0. */
	FxReal GetMin() const { return _min; }
	/** 2012 0xb12400. */
	FxReal GetMax() const { return _max; }
	/** 2012 0xb12420. */
	FxInputOp GetNodeOperation() const { return _inputOperation; }
	void SetMin( FxReal iMin );
	void SetMax( FxReal iMax );
	void SetNodeOperation( FxInputOp iNodeOperation );

	/** 2012 0xb12440. */
	FxSize GetNumInputLinks() const { return _inputs.Length(); }
	/** 2012 0xb12460. */
	const FxFaceGraphNodeLink& GetInputLink( FxSize index ) const { return _inputs[index]; }
	void ModifyInputLink( FxSize index, const FxFaceGraphNodeLink& inputLink );
	void RemoveInputLink( FxSize index );
	/** 2012 0xb129f0. */
	FxSize GetNumOutputs() const { return _outputs.Length(); }
	/** 2012 0xb20f30. */
	FxFaceGraphNode* GetOutput( FxSize index ) { return _outputs[index]; }
	void AddOutput( FxFaceGraphNode* pOutput );

	void AddUserProperty( const FxFaceGraphNodeUserProperty& userProperty );
	FxSize GetNumUserProperties() const;
	FxSize FindUserProperty( const FxName& userPropertyName ) const;
	const FxFaceGraphNodeUserProperty& GetUserProperty( FxSize index ) const;
	void ReplaceUserProperty( const FxFaceGraphNodeUserProperty& userProperty );

	virtual void Serialize( FxArchive& arc );

protected:
	friend class FxFaceGraph;

	FxBool _hasCycles() const;

	mutable FxBool _hasBeenVisited;
	FxReal _min;
	FxReal _max;
	FxInputOp _inputOperation;
	FxArray<FxFaceGraphNodeLink> _inputs;
	FxArray<FxFaceGraphNode*> _outputs;
	FxArray<FxFaceGraphNodeUserProperty> _userProperties;
};

} // namespace Face
} // namespace OC3Ent

#endif
