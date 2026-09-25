/*=============================================================================
	bspatch.h: in-memory bsdiff patching (Arkane addition to Core).

	Declared here so ULinkerLoad::CreateLoader can patch a precached package
	image with the .bs file AsyncPreloadPackage read from
	..\..\DishonoredGame\Patches\<PackageFile>.bs (see Src/bspatch/bspatch.cpp).
=============================================================================*/

#pragma once

/**
 * Applies a BSDIFF40 patch to a memory buffer.
 *
 * @param BuffToPatch     the original ("old") data
 * @param BuffToPatchLen  size of BuffToPatch in bytes
 * @param PatchData       the .bs patch (BSDIFF40 header + three bzip2 streams)
 * @param PatchDataLen    size of PatchData in bytes
 * @param OutBuffer       receives the patched ("new") data, allocated with appMalloc; the caller frees it
 * @param OutBufferLen    receives the size of the allocation: new size + 1, as bsdiff's bspatch allocates it
 */
void ArkBsPatch( const BYTE* BuffToPatch, UINT BuffToPatchLen, const BYTE* PatchData, UINT PatchDataLen, BYTE*& OutBuffer, UINT& OutBufferLen );
