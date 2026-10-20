/*=============================================================================
	FxBone.h - FaceFX 1.7.3.1 bone transform (fxbone.h). 48 bytes: FxNamedObject + pos, rot, scale.

	DISHONORED(port): agent FX1 (wave 26). Header inlines retail emitted: GetPos 0x3301b0 (fxbone.h:44; /OPT:ICF folded it with an
	unrelated Dishonored getter, so the PDB names it UDisAISubStateCower::GetDesiresFaceToRequest), GetScale 0x3301c0.
=============================================================================*/
#ifndef FxBone_H__
#define FxBone_H__

#include "FxNamedObject.h"
#include "FxVec3.h"

namespace OC3Ent
{
namespace Face
{

class FxBone : public FxNamedObject
{
	FX_DECLARE_CLASS(FxBone, FxNamedObject)
public:
	FxBone();
	FxBone(const FxName& name, const FxVec3& iPos, const FxQuat& iRot, const FxVec3& iScale);
	virtual ~FxBone();

	const FxVec3& GetPos() const
	{
		return _pos;
	}

	const FxQuat& GetRot() const
	{
		return _rot;
	}

	const FxVec3& GetScale() const
	{
		return _scale;
	}

	void SetPos(const FxVec3& iPos);
	void SetRot(const FxQuat& iRot);
	void SetScale(const FxVec3& iScale);

	virtual void Serialize(FxArchive& arc);

protected:
	FxVec3 _pos;
	FxQuat _rot;
	FxVec3 _scale;
};

} // namespace Face
} // namespace OC3Ent

#endif
