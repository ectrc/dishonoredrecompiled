/*-
 * Copyright 2003-2005 Colin Percival
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * bsdiff 4.3 bspatch.c, adapted by Arkane for Dishonored: the old file, the
 * patch and the new file are memory buffers instead of files (InMemoryFile
 * feeds the three bzip2 streams), main() became ArkBsPatch(), the fatal
 * errx() checks became engine checks. Reconstructed from the 2012 Shipping
 * decompile (bspatch.cpp:55 offtin, bspatch.cpp:76 ArkBsPatch).
 */

#include "CorePrivate.h"
#include "bspatch.h"
#include "bzlib.h"

/* off_t is a 32-bit long in the MSVC CRT; the Shipping code only reads the
   low four bytes and the sign bit, which is what this collapses to. */
static INT offtin( const BYTE* buf )
{
	INT y;

	y = buf[7] & 0x7F;
	y = y * 256; y += buf[6];
	y = y * 256; y += buf[5];
	y = y * 256; y += buf[4];
	y = y * 256; y += buf[3];
	y = y * 256; y += buf[2];
	y = y * 256; y += buf[1];
	y = y * 256; y += buf[0];

	if( buf[7] & 0x80 )
	{
		y = -y;
	}

	return y;
}

void ArkBsPatch( const BYTE* BuffToPatch, UINT BuffToPatchLen, const BYTE* PatchData, UINT PatchDataLen, BYTE*& OutBuffer, UINT& OutBufferLen )
{
	BZFILE* cpfbz2;
	BZFILE* dpfbz2;
	BZFILE* epfbz2;
	INT cbz2err;
	INT dbz2err;
	INT ebz2err;
	INT oldsize = BuffToPatchLen;
	INT newsize;
	INT bzctrllen;
	INT bzdatalen;
	BYTE buf[8];
	BYTE* nnew;
	INT oldpos;
	INT newpos;
	INT ctrl[3];
	INT lenread;
	INT i;

	/*
	File format:
		0	8	"BSDIFF40"
		8	8	X
		16	8	Y
		24	8	sizeof(newfile)
		32	X	bzip2(control block)
		32+X	Y	bzip2(diff block)
		32+X+Y	???	bzip2(extra block)
	with control block a set of triples (x,y,z) meaning "add x bytes
	from oldfile to x bytes from the diff block; copy y bytes from the
	extra block; seek forwards in oldfile by z bytes".
	*/

	/* Read header */
	const BYTE* header = PatchData;
	checkf( PatchDataLen >= 32, TEXT("ArkBsPatch: corrupt patch (%u bytes, no header)"), PatchDataLen );

	/* Check for appropriate magic */
	verify( memcmp( header, "BSDIFF40", 8 ) == 0 );

	/* Read lengths from header */
	bzctrllen = offtin( header + 8 );
	bzdatalen = offtin( header + 16 );
	newsize = offtin( header + 24 );
	checkf( bzctrllen >= 0 && bzdatalen >= 0 && newsize >= 0, TEXT("ArkBsPatch: corrupt patch") );

	/* Open the three bzip2 streams: control, diff and extra blocks */
	InMemoryFile cp( PatchData, PatchDataLen, 32 );
	cpfbz2 = BZ2_bzReadOpen( &cbz2err, &cp, 0, 0, NULL, 0 );
	checkf( cpfbz2 != NULL, TEXT("ArkBsPatch: BZ2_bzReadOpen, bz2err = %d"), cbz2err );

	InMemoryFile dp( PatchData, PatchDataLen, 32 + bzctrllen );
	dpfbz2 = BZ2_bzReadOpen( &dbz2err, &dp, 0, 0, NULL, 0 );
	checkf( dpfbz2 != NULL, TEXT("ArkBsPatch: BZ2_bzReadOpen, bz2err = %d"), dbz2err );

	InMemoryFile ep( PatchData, PatchDataLen, 32 + bzctrllen + bzdatalen );
	epfbz2 = BZ2_bzReadOpen( &ebz2err, &ep, 0, 0, NULL, 0 );
	checkf( epfbz2 != NULL, TEXT("ArkBsPatch: BZ2_bzReadOpen, bz2err = %d"), ebz2err );

	/* bspatch allocates newsize+1 bytes; Dishonored reports that allocation size to the caller */
	nnew = (BYTE*)appMalloc( newsize + 1 );
	OutBuffer = nnew;
	OutBufferLen = newsize + 1;

	oldpos = 0;
	newpos = 0;
	while( newpos < newsize )
	{
		/* Read control data */
		for( i = 0; i <= 2; i++ )
		{
			lenread = BZ2_bzRead( &cbz2err, cpfbz2, buf, 8 );
			checkf( lenread == 8 && (cbz2err == BZ_OK || cbz2err == BZ_STREAM_END), TEXT("ArkBsPatch: corrupt patch (control block, bz2err = %d)"), cbz2err );
			ctrl[i] = offtin( buf );
		}

		/* Sanity-check */
		checkf( newpos + ctrl[0] <= newsize, TEXT("ArkBsPatch: corrupt patch") );

		/* Read diff string */
		lenread = BZ2_bzRead( &dbz2err, dpfbz2, nnew + newpos, ctrl[0] );
		checkf( lenread == ctrl[0] && (dbz2err == BZ_OK || dbz2err == BZ_STREAM_END), TEXT("ArkBsPatch: corrupt patch (diff block, bz2err = %d)"), dbz2err );

		/* Add old data to diff string */
		for( i = 0; i < ctrl[0]; i++ )
		{
			if( (oldpos + i >= 0) && (oldpos + i < oldsize) )
			{
				nnew[newpos + i] += BuffToPatch[oldpos + i];
			}
		}

		/* Adjust pointers */
		newpos += ctrl[0];
		oldpos += ctrl[0];

		/* Sanity-check */
		checkf( newpos + ctrl[1] <= newsize, TEXT("ArkBsPatch: corrupt patch") );

		/* Read extra string */
		lenread = BZ2_bzRead( &ebz2err, epfbz2, nnew + newpos, ctrl[1] );
		checkf( lenread == ctrl[1] && (ebz2err == BZ_OK || ebz2err == BZ_STREAM_END), TEXT("ArkBsPatch: corrupt patch (extra block, bz2err = %d)"), ebz2err );

		/* Adjust pointers */
		newpos += ctrl[1];
		oldpos += ctrl[2];
	}

	/* Clean up the bzip2 reads */
	BZ2_bzReadClose( &cbz2err, cpfbz2 );
	BZ2_bzReadClose( &dbz2err, dpfbz2 );
	BZ2_bzReadClose( &ebz2err, epfbz2 );
}
