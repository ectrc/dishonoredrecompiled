/*=============================================================================
	arkbloompartsrendering.h: Arkane's bloom-part pass.
	DISHONORED(written): Dishonored's build has arkbloompartsrendering.cpp but no header of its own; the set is declared
	where FDistortionPrimSet is, so its declaration lives here.
=============================================================================*/

#ifndef __ARKBLOOMPARTSRENDERING_H__
#define __ARKBLOOMPARTSRENDERING_H__

/**
 * DISHONORED(layout): 2012 PDB FArkBloomPartPrimSet, 12 bytes, one member `TArray<FPrimitiveSceneInfo*> Prims`.
 * FViewInfo carries four of them (@1856, one per DPG) right after DistortionPrimSet (@1808).
 */
class FArkBloomPartPrimSet
{
public:

	/**
	 * DISHONORED(port): 2012 rva 0x54e6c0. There is no 2013 address: the body is 16 bytes of TArray::AddItem and
	 * COMDAT-folds with unrelated functions, so the match table cannot place it. ViewInfo is taken and unused - unlike
	 * FTranslucentPrimSet, this set is not sorted.
	 */
	void AddScenePrimitive(FPrimitiveSceneInfo* PrimitiveSceneInfo,const FViewInfo& ViewInfo);

	/**
	 * DISHONORED(port): 2013 rva 0x524f00 - draw the bloom-part elements of every primitive of the set, and the view's
	 * own bloom-part mesh elements, into whatever target is bound.
	 * @return TRUE if anything was drawn
	 */
	UBOOL DrawBloomPrims(const class FViewInfo* ViewInfo,UINT DPGIndex,UBOOL bInitializeOffsets);

	INT NumPrims() const
	{
		return Prims.Num();
	}

	const FPrimitiveSceneInfo* GetPrim(INT i) const
	{
		check(i>=0 && i<NumPrims());
		return Prims(i);
	}

private:
	/** list of bloom-part prims added from the scene */
	TArray<FPrimitiveSceneInfo*> Prims;
};

namespace bloom
{
	/** DISHONORED(layout): bloom::URECT<UINT>, the scissor rect GaussianBlur is passed by value (16 bytes). */
	template<typename T>
	struct URECT
	{
		T mLeft;
		T mTop;
		T mRight;
		T mBottom;
	};

	/**
	 * DISHONORED(port): 2013 rva 0x51fe50 - two five-tap passes between two targets of the same size. Pass 0 reads
	 * iTexture and writes iSurface2, pass 1 reads iTexture2 and writes iSurface, so the result is left in the first pair.
	 */
	void GaussianBlur(
		const FSurfaceRHIRef& iSurface,
		const FTexture2DRHIRef& iTexture,
		const FSurfaceRHIRef& iSurface2,
		const FTexture2DRHIRef& iTexture2,
		UINT iTextureWidth,
		UINT iTextureHeight,
		URECT<UINT> iRect,
		UINT iKernelSpread,
		FLOAT iScale
		);
}

#endif
