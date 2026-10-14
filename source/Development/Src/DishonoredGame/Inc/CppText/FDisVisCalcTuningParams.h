// FDisVisCalcTuningParams cpptext: included inside the generated struct body (dishonoredgameclasses.h).
// DISHONORED(port): agent GI (PHASE20 GI). A distance-to-visibility curve: either (param^exponent + bias) * scalar or a
// piecewise-linear table of FDisVisCalcTuningParamsCurvePoint, applied (added to / multiplied with) a visibility value.
// 2012 rvas Apply(param, value) 0x7495d0 / 2013 0x70b350 (203 B), Apply(param) 0x74d6a0 / 0x70eca0, GetRelevantVisCalcPoints
// 0x749470, FixupDefaults_DisVisCalcTuningParams 0x74d6f0 (a sort of the points). Bodies in Src/distweaks_pawnattention.cpp.
// DISHONORED(layout): m_Points is the generated `BYTE m_Points[12]` placeholder of a TArray<FDisVisCalcTuningParamsCurvePoint>
// the SDK dump did not type; it is read through Points() below.
public:
	FLOAT Apply( FLOAT _fParam, FLOAT _fApplyToThisValue ) const;
	FLOAT Apply( FLOAT _fParam ) const;
	void FixupDefaults_DisVisCalcTuningParams();
	const TArray< struct FDisVisCalcTuningParamsCurvePoint >& Points() const { return *reinterpret_cast< const TArray< struct FDisVisCalcTuningParamsCurvePoint >* >( m_Points ); }
private:
	void GetRelevantVisCalcPoints( FLOAT _fParam, const struct FDisVisCalcTuningParamsCurvePoint*& _rpOut_Point0, const struct FDisVisCalcTuningParamsCurvePoint*& _rpOut_Point1 ) const;
