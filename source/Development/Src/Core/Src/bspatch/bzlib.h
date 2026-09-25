/*-------------------------------------------------------------*/
/*--- Public header file for the library.                   ---*/
/*---                                               bzlib.h ---*/
/*-------------------------------------------------------------*/

/* ------------------------------------------------------------------
   This file is part of bzip2/libbzip2, a program and library for
   lossless, block-sorting data compression.

   bzip2/libbzip2 version 1.0.6 of 6 September 2010
   Copyright (C) 1996-2010 Julian Seward <jseward@bzip.org>

   Please read the WARNING, DISCLAIMER and PATENTS sections in the
   README file.

   This program is released under the terms of the license contained
   in the file LICENSE.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

   2. The origin of this software must not be misrepresented; you must
      not claim that you wrote the original software.  If you use this
      software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   3. Altered source versions must be plainly marked as such, and must
      not be misrepresented as being the original software.

   4. The name of the author may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   ------------------------------------------------------------------ */

/* ------------------------------------------------------------------
   ALTERED SOURCE VERSION (Dishonored / Arkane, reconstructed by the
   recompilation project from the 2012 Shipping PDB):
   * only the decompression side of libbzip2 is kept (BZ2_bzDecompress*,
     BZ2_bzRead*); the compressor, bzBuffTo* helpers and the FILE based
     high-level API are gone,
   * the high-level read API works on an in-memory buffer (InMemoryFile)
     instead of a FILE*,
   * the declarations of bzlib_private.h that the decompressor needs are
     appended at the end of this header, guarded by BZ_PRIVATE,
   * built as C++; the public entry points keep C linkage and __stdcall
     (the PDB exports _BZ2_bzDecompress@4, _BZ2_bzRead@16, ...).
   ------------------------------------------------------------------ */

#ifndef _BZLIB_H
#define _BZLIB_H

#include <string.h>

/* windows.h (rpcndr.h) defines small as char; libbzip2 uses it as a parameter name */
#ifdef small
#undef small
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define BZ_RUN               0
#define BZ_FLUSH             1
#define BZ_FINISH            2

#define BZ_OK                0
#define BZ_RUN_OK            1
#define BZ_FLUSH_OK          2
#define BZ_FINISH_OK         3
#define BZ_STREAM_END        4
#define BZ_SEQUENCE_ERROR    (-1)
#define BZ_PARAM_ERROR       (-2)
#define BZ_MEM_ERROR         (-3)
#define BZ_DATA_ERROR        (-4)
#define BZ_DATA_ERROR_MAGIC  (-5)
#define BZ_IO_ERROR          (-6)
#define BZ_UNEXPECTED_EOF    (-7)
#define BZ_OUTBUFF_FULL      (-8)
#define BZ_CONFIG_ERROR      (-9)

typedef
   struct {
      char *next_in;
      unsigned int avail_in;
      unsigned int total_in_lo32;
      unsigned int total_in_hi32;

      char *next_out;
      unsigned int avail_out;
      unsigned int total_out_lo32;
      unsigned int total_out_hi32;

      void *state;

      void *(*bzalloc)(void *,int,int);
      void (*bzfree)(void *,void *);
      void *opaque;
   }
   bz_stream;


#ifndef BZ_IMPORT
#define BZ_EXPORT
#endif

/* Dishonored links the library statically into Core.lib; the entry points are
   exported with the stdcall convention the original Windows build used. */
#define BZ_API(func) __stdcall func
#define BZ_EXTERN extern


/*-- Core (low-level) library functions --*/

BZ_EXTERN int BZ_API(BZ2_bzDecompressInit) (
      bz_stream *strm,
      int       verbosity,
      int       small
   );

BZ_EXTERN int BZ_API(BZ2_bzDecompress) (
      bz_stream* strm
   );

BZ_EXTERN int BZ_API(BZ2_bzDecompressEnd) (
      bz_stream *strm
   );



/*-- High(er) level library functions --*/

#define BZ_MAX_UNUSED 5000

typedef void BZFILE;

#ifdef __cplusplus
}
#endif

/**
 * Replacement for the FILE* the stock high-level API reads from: a read-only
 * window over a memory buffer, with the handful of stdio operations libbzip2
 * needs (fread, fgetc, ungetc, ferror). Layout from the PDB: 12 bytes.
 */
struct InMemoryFile
{
   const unsigned char* m_pBuffer;
   const unsigned int   m_nBufferSize;
   unsigned int         m_nCurrentPosition;

   InMemoryFile( const unsigned char* pBuffer, unsigned int nBufferSize, unsigned int nStartPosition = 0 )
   :  m_pBuffer( pBuffer )
   ,  m_nBufferSize( nBufferSize )
   ,  m_nCurrentPosition( nStartPosition )
   {}

   static unsigned int fread( void* ptr, unsigned int size, unsigned int count, InMemoryFile* f )
   {
      unsigned int nBytes = size * count;
      if( nBytes > f->m_nBufferSize - f->m_nCurrentPosition )
      {
         nBytes = f->m_nBufferSize - f->m_nCurrentPosition;
      }
      memcpy( ptr, f->m_pBuffer + f->m_nCurrentPosition, nBytes );
      f->m_nCurrentPosition += nBytes;
      return nBytes;
   }

   static int fgetc( InMemoryFile* f )
   {
      if( f->m_nCurrentPosition >= f->m_nBufferSize )
      {
         return -1;
      }
      return f->m_pBuffer[f->m_nCurrentPosition++];
   }

   static int ungetc( int c, InMemoryFile* f )
   {
      f->m_nCurrentPosition--;
      return c;
   }

   static int ferror( InMemoryFile* )
   {
      return 0;
   }
};

#ifdef __cplusplus
extern "C" {
#endif

BZ_EXTERN BZFILE* BZ_API(BZ2_bzReadOpen) (
      int*  bzerror,
      InMemoryFile* f,
      int   verbosity,
      int   small,
      void* unused,
      int   nUnused
   );

BZ_EXTERN void BZ_API(BZ2_bzReadClose) (
      int*    bzerror,
      BZFILE* b
   );

BZ_EXTERN int BZ_API(BZ2_bzRead) (
      int*    bzerror,
      BZFILE* b,
      void*   buf,
      int     len
   );

#ifdef __cplusplus
}
#endif

#endif


/*-------------------------------------------------------------*/
/*--- Private header file for the library.                  ---*/
/*---                                       bzlib_private.h ---*/
/*--- (decompression subset, merged into bzlib.h)           ---*/
/*-------------------------------------------------------------*/

#if defined(BZ_PRIVATE) && !defined(_BZLIB_PRIVATE_H)
#define _BZLIB_PRIVATE_H

#include <stdlib.h>
#include <stdio.h>



/*-- General stuff. --*/

#define BZ_VERSION  "1.0.6, 6-Sept-2010"

typedef char            Char;
typedef unsigned char   Bool;
typedef unsigned char   UChar;
typedef int             Int32;
typedef unsigned int    UInt32;
typedef short           Int16;
typedef unsigned short  UInt16;

#define True  ((Bool)1)
#define False ((Bool)0)

#ifndef __GNUC__
#define __inline__  /* */
#endif

extern void BZ2_bz__AssertH__fail ( int errcode );
#define AssertH(cond,errnum) \
   { if (!(cond)) BZ2_bz__AssertH__fail ( errnum ); }

#if BZ_DEBUG
#define AssertD(cond,msg) \
   { if (!(cond)) {       \
      fprintf ( stderr,   \
        "\n\nlibbzip2(debug build): internal error\n\t%s\n", msg );\
      exit(1); \
   }}
#else
#define AssertD(cond,msg) /* */
#endif

#define VPrintf0(zf) \
   fprintf(stderr,zf)
#define VPrintf1(zf,za1) \
   fprintf(stderr,zf,za1)
#define VPrintf2(zf,za1,za2) \
   fprintf(stderr,zf,za1,za2)
#define VPrintf3(zf,za1,za2,za3) \
   fprintf(stderr,zf,za1,za2,za3)
#define VPrintf4(zf,za1,za2,za3,za4) \
   fprintf(stderr,zf,za1,za2,za3,za4)
#define VPrintf5(zf,za1,za2,za3,za4,za5) \
   fprintf(stderr,zf,za1,za2,za3,za4,za5)


#define BZALLOC(nnn) (strm->bzalloc)(strm->opaque,(nnn),1)
#define BZFREE(ppp)  (strm->bzfree)(strm->opaque,(ppp))


/*-- Header bytes. --*/

#define BZ_HDR_B 0x42   /* 'B' */
#define BZ_HDR_Z 0x5a   /* 'Z' */
#define BZ_HDR_h 0x68   /* 'h' */
#define BZ_HDR_0 0x30   /* '0' */

/*-- Constants for the back end. --*/

#define BZ_MAX_ALPHA_SIZE 258
#define BZ_MAX_CODE_LEN    23

#define BZ_RUNA 0
#define BZ_RUNB 1

#define BZ_N_GROUPS 6
#define BZ_G_SIZE   50
#define BZ_N_ITERS  4

#define BZ_MAX_SELECTORS (2 + (900000 / BZ_G_SIZE))



/*-- Stuff for randomising repetitive blocks. --*/

extern Int32 BZ2_rNums[512];

#define BZ_RAND_DECLS                          \
   Int32 rNToGo;                               \
   Int32 rTPos;                                \

#define BZ_RAND_INIT_MASK                      \
   s->rNToGo = 0;                              \
   s->rTPos  = 0                               \

#define BZ_RAND_MASK ((s->rNToGo == 1) ? 1 : 0)

#define BZ_RAND_UPD_MASK                       \
   if (s->rNToGo == 0) {                       \
      s->rNToGo = BZ2_rNums[s->rTPos];         \
      s->rTPos++;                              \
      if (s->rTPos == 512) s->rTPos = 0;       \
   }                                           \
   s->rNToGo--;



/*-- Stuff for doing CRCs. --*/

extern UInt32 BZ2_crc32Table[256];

#define BZ_INITIALISE_CRC(crc_var)   \
{                                    \
   crc_var = 0xffffffffL;            \
}

#define BZ_FINALISE_CRC(crc_var)     \
{                                    \
   crc_var = ~(crc_var);             \
}

#define BZ_UPDATE_CRC(crc_var,cha)              \
{                                               \
   crc_var = (crc_var << 8) ^                   \
             BZ2_crc32Table[(crc_var >> 24) ^   \
                            ((UChar)cha)];      \
}



/*-- States and modes for decompression. --*/

#define BZ_X_IDLE        1
#define BZ_X_OUTPUT      2

#define BZ_X_MAGIC_1     10
#define BZ_X_MAGIC_2     11
#define BZ_X_MAGIC_3     12
#define BZ_X_MAGIC_4     13
#define BZ_X_BLKHDR_1    14
#define BZ_X_BLKHDR_2    15
#define BZ_X_BLKHDR_3    16
#define BZ_X_BLKHDR_4    17
#define BZ_X_BLKHDR_5    18
#define BZ_X_BLKHDR_6    19
#define BZ_X_BCRC_1      20
#define BZ_X_BCRC_2      21
#define BZ_X_BCRC_3      22
#define BZ_X_BCRC_4      23
#define BZ_X_RANDBIT     24
#define BZ_X_ORIGPTR_1   25
#define BZ_X_ORIGPTR_2   26
#define BZ_X_ORIGPTR_3   27
#define BZ_X_MAPPING_1   28
#define BZ_X_MAPPING_2   29
#define BZ_X_SELECTOR_1  30
#define BZ_X_SELECTOR_2  31
#define BZ_X_SELECTOR_3  32
#define BZ_X_CODING_1    33
#define BZ_X_CODING_2    34
#define BZ_X_CODING_3    35
#define BZ_X_MTF_1       36
#define BZ_X_MTF_2       37
#define BZ_X_MTF_3       38
#define BZ_X_MTF_4       39
#define BZ_X_MTF_5       40
#define BZ_X_MTF_6       41
#define BZ_X_ENDHDR_2    42
#define BZ_X_ENDHDR_3    43
#define BZ_X_ENDHDR_4    44
#define BZ_X_ENDHDR_5    45
#define BZ_X_ENDHDR_6    46
#define BZ_X_CCRC_1      47
#define BZ_X_CCRC_2      48
#define BZ_X_CCRC_3      49
#define BZ_X_CCRC_4      50



#define MTFA_SIZE 4096
#define MTFL_SIZE 16



/*-- Structure holding all the decompression-side stuff. --*/

typedef
   struct {
      /* pointer back to the struct bz_stream */
      bz_stream* strm;

      /* state indicator for this stream */
      Int32    state;

      /* for doing the final run-length decoding */
      UChar    state_out_ch;
      Int32    state_out_len;
      Bool     blockRandomised;
      BZ_RAND_DECLS;

      /* the buffer for bit stream reading */
      UInt32   bsBuff;
      Int32    bsLive;

      /* misc administratium */
      Int32    blockSize100k;
      Bool     smallDecompress;
      Int32    currBlockNo;
      Int32    verbosity;

      /* for undoing the Burrows-Wheeler transform */
      Int32    origPtr;
      UInt32   tPos;
      Int32    k0;
      Int32    unzftab[256];
      Int32    nblock_used;
      Int32    cftab[257];
      Int32    cftabCopy[257];

      /* for undoing the Burrows-Wheeler transform (FAST) */
      UInt32   *tt;

      /* for undoing the Burrows-Wheeler transform (SMALL) */
      UInt16   *ll16;
      UChar    *ll4;

      /* stored and calculated CRCs */
      UInt32   storedBlockCRC;
      UInt32   storedCombinedCRC;
      UInt32   calculatedBlockCRC;
      UInt32   calculatedCombinedCRC;

      /* map of bytes used in block */
      Int32    nInUse;
      Bool     inUse[256];
      Bool     inUse16[16];
      UChar    seqToUnseq[256];

      /* for decoding the MTF values */
      UChar    mtfa   [MTFA_SIZE];
      Int32    mtfbase[256 / MTFL_SIZE];
      UChar    selector   [BZ_MAX_SELECTORS];
      UChar    selectorMtf[BZ_MAX_SELECTORS];
      UChar    len  [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];

      Int32    limit  [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
      Int32    base   [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
      Int32    perm   [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
      Int32    minLens[BZ_N_GROUPS];

      /* save area for scalars in the main decompress code */
      Int32    save_i;
      Int32    save_j;
      Int32    save_t;
      Int32    save_alphaSize;
      Int32    save_nGroups;
      Int32    save_nSelectors;
      Int32    save_EOB;
      Int32    save_groupNo;
      Int32    save_groupPos;
      Int32    save_nextSym;
      Int32    save_nblockMAX;
      Int32    save_nblock;
      Int32    save_es;
      Int32    save_N;
      Int32    save_curr;
      Int32    save_zt;
      Int32    save_zn;
      Int32    save_zvec;
      Int32    save_zj;
      Int32    save_gSel;
      Int32    save_gMinlen;
      Int32*   save_gLimit;
      Int32*   save_gBase;
      Int32*   save_gPerm;
   }
   DState;



/*-- Macros for decompression. --*/

#define BZ_GET_FAST(cccc)                     \
    /* c_tPos is unsigned, hence test < 0 is pointless. */ \
    if (s->tPos >= (UInt32)100000 * (UInt32)s->blockSize100k) return True; \
    s->tPos = s->tt[s->tPos];                 \
    cccc = (UChar)(s->tPos & 0xff);           \
    s->tPos >>= 8;

#define BZ_GET_FAST_C(cccc)                   \
    /* c_tPos is unsigned, hence test < 0 is pointless. */ \
    if (c_tPos >= (UInt32)100000 * (UInt32)ro_blockSize100k) return True; \
    c_tPos = c_tt[c_tPos];                    \
    cccc = (UChar)(c_tPos & 0xff);            \
    c_tPos >>= 8;

#define SET_LL4(i,n)                                          \
   { if (((i) & 0x1) == 0)                                    \
        s->ll4[(i) >> 1] = (s->ll4[(i) >> 1] & 0xf0) | (n); else    \
        s->ll4[(i) >> 1] = (s->ll4[(i) >> 1] & 0x0f) | ((n) << 4);  \
   }

#define GET_LL4(i)                             \
   ((((UInt32)(s->ll4[(i) >> 1])) >> (((i) << 2) & 0x4)) & 0xF)

#define SET_LL(i,n)                          \
   { s->ll16[i] = (UInt16)(n & 0x0000ffff);  \
     SET_LL4(i, n >> 16);                    \
   }

#define GET_LL(i) \
   (((UInt32)s->ll16[i]) | (GET_LL4(i) << 16))

#define BZ_GET_SMALL(cccc)                            \
    /* c_tPos is unsigned, hence test < 0 is pointless. */ \
    if (s->tPos >= (UInt32)100000 * (UInt32)s->blockSize100k) return True; \
    cccc = BZ2_indexIntoF ( s->tPos, s->cftab );    \
    s->tPos = GET_LL(s->tPos);


/*-- externs for decompression. --*/

extern Int32
BZ2_indexIntoF ( Int32, Int32* );

extern Int32
BZ2_decompress ( DState* );

extern void
BZ2_hbCreateDecodeTables ( Int32*, Int32*, Int32*, UChar*,
                           Int32,  Int32, Int32 );


#endif

/*-------------------------------------------------------------*/
/*--- end                                           bzlib.h ---*/
/*-------------------------------------------------------------*/
