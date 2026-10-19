// WwiseVorbisSmoke: agent IH2's check of the ported Wwise 2012.1 Vorbis decoder (Src/AkVorbisDecoder) on one retail media
// item, the voice line 0x5c7ca16c (sound 0x3e2a98d4, bank 0x87628435 in CookedPCConsole\Bk_E43C0E7F49D6DA8943110CA3F7328499.INT,
// "Corvo, she's waiting, you should go see the empress."; mono 44.1 kHz, 256/2048 blocks, 542 packets).
//
// The header is set up the way CAkSrcBankVorbis::StartStream / DecodeVorbisHeader (2012 0x976640 / 0x9764f0) do it, then
// the packets are decoded 1024 frames at a time through DecodeVorbis exactly as CAkSrcBankVorbis::GetBuffer (2012 0x9762f0)
// calls it. The reference is independent of the port:
//   python resources/tools/wwise_vorbis_ref.py --container Bk_E43C0E7F49D6DA8943110CA3F7328499.INT --media 0x5c7ca16c
//          --pcm build/agentIH2/V/ref_5c7ca16c.s16
// The second half runs the same media through the ported source node, CAkSrcBankVorbis::StartStream + GetBuffer, and
// checks the same frame count and RMS envelope (agent IH2 fork S).
// rewraps the item into a standard Ogg Vorbis stream (codebooks expanded from the retail exe's g_packedCodebookData) and
// decodes it with ffmpeg 8.1: 117526 frames (= the fmt chunk's dwTotalPCMFrames), sum 32791, crc32 0xaafdd8be, and the
// RMS of every 1024 frames below. Two float decoders round differently, so the bit-exact checksum is reported, not
// required; the checks are the frame count, the RMS envelope, and (when the reference PCM file is present,
// WWISE_VORBIS_REF, default build/agentIH2/V/ref_5c7ca16c.s16 from the repo root) the SNR against the reference samples.
#if DISHONORED_WITH_WWISE

#define WIN32_LEAN_AND_MEAN
#pragma pack(push, 8)
#include <windows.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AkVorbisCodebookMgr.h>
#include <AkSrcBankVorbis.h>
#include <AkPBI.h>
#pragma pack(pop)

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

extern AkMemPoolId g_LEngineDefaultPoolId;

typedef void ( *WwiseReportFunc )( bool Passed, const char * Name, const char * Detail );
typedef void ( *WwiseSkipFunc )( const char * Name, const char * Reason );

namespace
{
const wchar_t * const VoiceContainer = L"D:\\RecompileDishonored\\Dishonored_Latest2026\\DishonoredGame\\CookedPCConsole\\Bk_E43C0E7F49D6DA8943110CA3F7328499.INT";
const AkUInt32 VoiceBankID = 0x87628435;
const AkUInt32 VoiceMediaID = 0x5c7ca16c;
const AkUInt32 ReferenceFrames = 117526;
const int ReferenceRms[ 115 ] =
{
	2, 115, 1507, 1356, 4733, 7588, 9170, 10210, 8544, 2450, 1753, 1481, 3469, 4688, 3488, 2497, 1670, 1208, 691, 1155,
	1636, 1202, 1200, 2800, 3187, 2498, 1482, 1632, 718, 507, 811, 1283, 2577, 4432, 5639, 5057, 4636, 4851, 3979, 2175,
	1103, 2443, 1883, 1807, 936, 796, 438, 546, 327, 380, 141, 59, 37, 59, 61, 829, 1867, 2763, 1910, 1900,
	1887, 2329, 3968, 2499, 1279, 1007, 816, 986, 2991, 6431, 7797, 6139, 7443, 6199, 2594, 2602, 2516, 2240, 2467, 1780,
	3180, 3185, 1959, 1721, 5246, 5019, 3812, 3355, 3852, 3898, 4235, 4228, 3855, 2890, 1440, 367, 63, 669, 708, 1523,
	2043, 1503, 1014, 649, 1182, 1499, 1541, 1677, 1420, 775, 479, 290, 113, 42, 5
};

std::vector<unsigned char> ReadFileBytes( const wchar_t * Path )
{
	std::vector<unsigned char> Bytes;
	FILE * File = _wfopen( Path, L"rb" );
	if( !File )
	{
		return Bytes;
	}
	fseek( File, 0, SEEK_END );
	const long Size = ftell( File );
	fseek( File, 0, SEEK_SET );
	Bytes.resize( Size > 0 ? (size_t)Size : 0 );
	if( Size > 0 )
	{
		fread( Bytes.data(), 1, (size_t)Size, File );
	}
	fclose( File );
	return Bytes;
}

/** The media item: AKPK sound-bank table -> bank -> DIDX entry -> DATA bytes (wwise_bank_dump.py's reading). */
bool FindVoiceMedia( const std::vector<unsigned char> & Pck, const unsigned char *& OutMedia, AkUInt32 & OutSize )
{
	if( Pck.size() < 28 || memcmp( Pck.data(), "AKPK", 4 ) != 0 )
	{
		return false;
	}
	const AkUInt32 * Header = (const AkUInt32 *)Pck.data();
	const size_t TableAt = 28 + Header[ 3 ];
	const AkUInt32 NumBanks = *(const AkUInt32 *)( Pck.data() + TableAt );
	for( AkUInt32 i = 0; i < NumBanks; ++i )
	{
		const AkUInt32 * Entry = (const AkUInt32 *)( Pck.data() + TableAt + 4 + 20 * i );
		if( Entry[ 0 ] != VoiceBankID )
		{
			continue;
		}
		const unsigned char * Bank = Pck.data() + (size_t)Entry[ 3 ] * ( Entry[ 1 ] ? Entry[ 1 ] : 1 );
		const AkUInt32 BankSize = Entry[ 2 ];
		const AkUInt32 * Didx = NULL;
		AkUInt32 DidxSize = 0;
		const unsigned char * Data = NULL;
		for( AkUInt32 Pos = 0; Pos + 8 <= BankSize; )
		{
			const AkUInt32 Size = *(const AkUInt32 *)( Bank + Pos + 4 );
			if( !memcmp( Bank + Pos, "DIDX", 4 ) )
			{
				Didx = (const AkUInt32 *)( Bank + Pos + 8 );
				DidxSize = Size;
			}
			else if( !memcmp( Bank + Pos, "DATA", 4 ) )
			{
				Data = Bank + Pos + 8;
			}
			Pos += 8 + Size;
		}
		for( AkUInt32 d = 0; Didx && Data && d < DidxSize / 12; ++d )
		{
			if( Didx[ d * 3 ] == VoiceMediaID )
			{
				OutMedia = Data + Didx[ d * 3 + 1 ];
				OutSize = Didx[ d * 3 + 2 ];
				return true;
			}
		}
	}
	return false;
}

const unsigned char * FindRiffChunk( const unsigned char * Media, AkUInt32 MediaSize, const char * Tag, AkUInt32 & OutSize )
{
	for( AkUInt32 Pos = 12; Pos + 8 <= MediaSize; )
	{
		const AkUInt32 Size = *(const AkUInt32 *)( Media + Pos + 4 );
		if( !memcmp( Media + Pos, Tag, 4 ) )
		{
			OutSize = Size;
			return Media + Pos + 8;
		}
		Pos += 8 + Size + ( Size & 1 );
	}
	return NULL;
}

/** The RMS of every 1024 frames against ffmpeg's decode of the rewrapped stream. */
void ReportRmsEnvelope( const std::vector<short> & Pcm, int Channels, const char * Name, WwiseReportFunc InReport )
{
	const AkUInt32 Frames = (AkUInt32)( Pcm.size() / Channels );
	int WorstBlock = -1;
	double WorstDelta = 0.0;
	const int NumBlocks = (int)( ( Frames + 1023 ) / 1024 );
	for( int b = 0; b < NumBlocks && b < 115; ++b )
	{
		double Energy = 0.0;
		const size_t First = (size_t)b * 1024 * Channels;
		const size_t Last = First + 1024 * Channels < Pcm.size() ? First + 1024 * Channels : Pcm.size();
		for( size_t i = First; i < Last; ++i )
		{
			Energy += (double)Pcm[ i ] * Pcm[ i ];
		}
		const double Rms = sqrt( Energy / (double)( Last - First ) );
		// The table is rounded to whole LSBs: a near-silent block (block 0 is 2.16) is compared against a floor of 64.
		const double Delta = fabs( Rms - ReferenceRms[ b ] ) / ( ReferenceRms[ b ] > 64 ? ReferenceRms[ b ] : 64.0 );
		if( Delta > WorstDelta )
		{
			WorstDelta = Delta;
			WorstBlock = b;
		}
	}
	char Detail[ 128 ];
	snprintf( Detail, sizeof( Detail ), "%d blocks, worst relative RMS delta %.4f at block %d", NumBlocks, WorstDelta, WorstBlock );
	InReport( NumBlocks == 115 && WorstDelta < 0.02, Name, Detail );
}

/**
 * The same media through the ported source node, CAkSrcBankVorbis (2012 0x976640 StartStream, 0x9762f0 GetBuffer), as
 * the voice pipeline drives it: StartStream on the PBI's in-memory media, then GetBuffer / ReleaseBuffer 1024 frames at a
 * time until AK_NoMoreData. The PBI is a zeroed CAkPBI holding only what a bank source reads (m_pDataPtr, m_uDataSize,
 * m_LoopCount 1, no notifications, no pending seek); no engine object is created.
 */
void TestBankVorbisSource( unsigned char * Media, AkUInt32 MediaSize, int Channels, WwiseReportFunc InReport )
{
	void * PbiStorage = _aligned_malloc( sizeof( CAkPBI ), 16 );
	memset( PbiStorage, 0, sizeof( CAkPBI ) );
	CAkPBI * Pbi = (CAkPBI *)PbiStorage;
	Pbi->m_pDataPtr = Media;
	Pbi->m_uDataSize = MediaSize;
	Pbi->m_LoopCount = 1;

	CAkSrcBankVorbis * Source = new CAkSrcBankVorbis( Pbi );
	const AKRESULT StartResult = Source->StartStream();
	char Detail[ 256 ];
	snprintf( Detail, sizeof( Detail ), "StartStream %d, %u total frames, media format %u Hz mask 0x%x block align %u",
		(int)StartResult, Source->m_uTotalSamples, Pbi->m_sMediaFormat.uSampleRate, (unsigned)Pbi->m_sMediaFormat.uChannelMask, (unsigned)Pbi->m_sMediaFormat.uBlockAlign );
	InReport( StartResult == AK_Success && Source->m_uTotalSamples == ReferenceFrames && Pbi->m_sMediaFormat.uBlockAlign == 2 * (AkUInt32)Channels,
		"Wwise CAkSrcBankVorbis::StartStream on the PBI's in-memory media", Detail );

	std::vector<short> Pcm;
	AkVPLState State;
	State.result = AK_DataReady;
	int Calls = 0;
	while( StartResult == AK_Success && State.result != AK_NoMoreData && State.result != AK_Fail && Calls < 100000 )
	{
		State.buffer.pData = NULL;
		State.buffer.uMaxFrames = LE_MAX_FRAMES_PER_BUFFER;
		State.buffer.uValidFrames = 0;
		Source->GetBuffer( State );
		if( State.result == AK_DataReady || State.result == AK_NoMoreData )
		{
			const short * Samples = (const short *)State.buffer.pData;
			Pcm.insert( Pcm.end(), Samples, Samples + State.buffer.uValidFrames * Channels );
		}
		Source->ReleaseBuffer();
		++Calls;
	}
	const AkUInt32 Frames = (AkUInt32)( Pcm.size() / Channels );
	snprintf( Detail, sizeof( Detail ), "%u frames in %d GetBuffer calls (last result %d), position %u", Frames, Calls, (int)State.result, Source->m_uCurSample );
	printf( "      %s\n", Detail );
	InReport( State.result == AK_NoMoreData && Frames == ReferenceFrames, "Wwise CAkSrcBankVorbis::GetBuffer: every frame of the voice line, ends AK_NoMoreData", Detail );
	ReportRmsEnvelope( Pcm, Channels, "Wwise CAkSrcBankVorbis::GetBuffer: RMS per 1024 frames within 2 % of ffmpeg's decode", InReport );

	Source->StopStream();
	delete Source;
	_aligned_free( PbiStorage );
}
}

void TestWwiseVorbis( WwiseReportFunc InReport, WwiseSkipFunc InSkip )
{
	const std::vector<unsigned char> Pck = ReadFileBytes( VoiceContainer );
	const unsigned char * Media = NULL;
	AkUInt32 MediaSize = 0;
	if( !FindVoiceMedia( Pck, Media, MediaSize ) )
	{
		InSkip( "Wwise Vorbis decode of voice media 0x5c7ca16c", "retail Bk_E43C0E7F49D6DA8943110CA3F7328499.INT not found" );
		return;
	}
	AkUInt32 FmtSize = 0, DataSize = 0;
	const WaveFormatVorbis * Fmt = (const WaveFormatVorbis *)FindRiffChunk( Media, MediaSize, "fmt ", FmtSize );
	unsigned char * DataStart = (unsigned char *)FindRiffChunk( Media, MediaSize, "data", DataSize );
	InReport( Fmt && DataStart && FmtSize == sizeof( WaveFormatVorbis ) && Fmt->wFormatTag == 0xFFFF, "Wwise Vorbis media: 66-byte fmt chunk (WaveFormatVorbis, tag 0xFFFF) + data", NULL );
	if( !Fmt || !DataStart )
	{
		return;
	}

	const bool bOwnMemoryMgr = !AK::MemoryMgr::IsInitialized();
	if( bOwnMemoryMgr )
	{
		AkMemSettings MemSettings;
		MemSettings.uMaxNumPools = 16;
		AK::MemoryMgr::Init( &MemSettings );
	}
	const AkMemPoolId SavedPool = g_LEngineDefaultPoolId;
	const AkMemPoolId SmokePool = AK::MemoryMgr::CreatePool( NULL, 0x100000, 64, AkMalloc, 0 );
	g_LEngineDefaultPoolId = SmokePool;

	AkVorbisSourceState State;
	memset( &State, 0, sizeof( State ) );
	memcpy( &State.VorbisInfo, static_cast<const AkVorbisInfo *>( &Fmt->vorbisHeader ), sizeof( AkVorbisInfo ) );
	State.uSampleRate = Fmt->nSamplesPerSec;
	State.TremorInfo.uChannelMask = Fmt->dwChannelMask;
	const AkUInt32 TotalFrames = Fmt->vorbisHeader.dwTotalPCMFrames;
	const int Channels = Fmt->nChannels;

	unsigned char * Data = DataStart;
	if( State.VorbisInfo.dwSeekTableSize )
	{
		State.pSeekTable = (AkVorbisSeekTableItem *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, State.VorbisInfo.dwSeekTableSize );
		memcpy( State.pSeekTable, Data, State.VorbisInfo.dwSeekTableSize );
		Data += State.VorbisInfo.dwSeekTableSize;
	}
	State.TremorInfo.ReturnInfo.eDecoderState = INITIALIZED;
	ogg_packet Setup;
	Setup.buffer.size = *(const AkUInt16 *)Data;
	Setup.buffer.data = Data + 2;
	Setup.e_o_s = 0;
	Data += 2 + Setup.buffer.size;
	CAkVorbisAllocator * Allocator = g_VorbisCodebookMgr.Decodebook( State, NULL, &Setup );
	InReport( Allocator != NULL, "Wwise Vorbis setup packet: AkVorbisCodebookMgr::Decodebook (packed codebooks from the retail exe)", NULL );
	if( Allocator )
	{
		State.TremorInfo.VorbisDSPState.csi = (codec_setup_info *)Allocator->pStartAddress;
		char Detail[ 256 ];
		snprintf( Detail, sizeof( Detail ), "%d books, %d floors, %d residues, %d maps, %d modes, blocks %d/%d, %u of %u setup bytes used",
			State.TremorInfo.VorbisDSPState.csi->books, State.TremorInfo.VorbisDSPState.csi->floors, State.TremorInfo.VorbisDSPState.csi->residues,
			State.TremorInfo.VorbisDSPState.csi->maps, State.TremorInfo.VorbisDSPState.csi->modes, State.TremorInfo.VorbisDSPState.csi->blocksizes[ 0 ],
			State.TremorInfo.VorbisDSPState.csi->blocksizes[ 1 ], Allocator->CurrentSize, Allocator->MaxSize );
		InReport( (unsigned char *)Data - DataStart == (int)State.VorbisInfo.dwVorbisDataOffset && vorbis_dsp_init( &State.TremorInfo.VorbisDSPState, Channels ) == 0,
			"Wwise Vorbis header: audio packets start at dwVorbisDataOffset, vorbis_dsp_init", Detail );
		State.TremorInfo.ReturnInfo.eDecoderState = PACKET_STREAM;
		vorbis_dsp_restart( &State.TremorInfo.VorbisDSPState, 0, State.VorbisInfo.uLastGranuleExtra );

		std::vector<short> Pcm;
		std::vector<short> Block( 1024 * Channels );
		int Calls = 0;
		AKRESULT Status = AK_DataReady;
		while( Status != AK_NoMoreData && Status != AK_Fail && Calls < 100000 )
		{
			State.TremorInfo.uRequestedFrames = 1024;
			State.TremorInfo.uInputDataSize = (AkUInt32)( ( DataStart + DataSize ) - Data );
			State.TremorInfo.bNoMoreInputPackets = true;
			DecodeVorbis( &State.TremorInfo, State.VorbisInfo.uMaxPacketSize, Data, Block.data() );
			Status = State.TremorInfo.ReturnInfo.eDecoderStatus;
			if( Status != AK_Fail )
			{
				Data += State.TremorInfo.ReturnInfo.uInputBytesConsumed;
				Pcm.insert( Pcm.end(), Block.begin(), Block.begin() + State.TremorInfo.ReturnInfo.uFramesProduced * Channels );
			}
			++Calls;
		}
		const AkUInt32 Frames = (AkUInt32)( Pcm.size() / Channels );
		long long Sum = 0;
		int Peak = 0;
		for( size_t i = 0; i < Pcm.size(); ++i )
		{
			Sum += Pcm[ i ];
			Peak = abs( Pcm[ i ] ) > Peak ? abs( Pcm[ i ] ) : Peak;
		}
		snprintf( Detail, sizeof( Detail ), "%u frames in %d DecodeVorbis calls (status %d), fmt dwTotalPCMFrames %u, ffmpeg reference %u; sum %lld, peak %d",
			Frames, Calls, (int)Status, TotalFrames, ReferenceFrames, Sum, Peak );
		printf( "      %s\n", Detail );
		InReport( Status == AK_NoMoreData && Frames == TotalFrames && Frames == ReferenceFrames, "Wwise Vorbis decode: every frame of the voice line, ends AK_NoMoreData", Detail );

		ReportRmsEnvelope( Pcm, Channels, "Wwise Vorbis decode: RMS per 1024 frames within 2 % of ffmpeg's decode of the rewrapped stream", InReport );

		const char * RefPath = getenv( "WWISE_VORBIS_REF" ) ? getenv( "WWISE_VORBIS_REF" ) : "build/agentIH2/V/ref_5c7ca16c.s16";
		FILE * RefFile = fopen( RefPath, "rb" );
		if( !RefFile )
		{
			InSkip( "Wwise Vorbis decode: SNR against the ffmpeg reference samples", "no reference PCM (run wwise_vorbis_ref.py --pcm)" );
		}
		else
		{
			std::vector<short> Ref( Pcm.size() + 1 );
			const size_t RefCount = fread( Ref.data(), sizeof( short ), Ref.size(), RefFile );
			fclose( RefFile );
			double Signal = 0.0, Noise = 0.0;
			int MaxDiff = 0;
			for( size_t i = 0; i < Pcm.size() && i < RefCount; ++i )
			{
				Signal += (double)Ref[ i ] * Ref[ i ];
				Noise += (double)( Pcm[ i ] - Ref[ i ] ) * ( Pcm[ i ] - Ref[ i ] );
				MaxDiff = abs( Pcm[ i ] - Ref[ i ] ) > MaxDiff ? abs( Pcm[ i ] - Ref[ i ] ) : MaxDiff;
			}
			const double Snr = Noise > 0.0 ? 10.0 * log10( Signal / Noise ) : 999.0;
			snprintf( Detail, sizeof( Detail ), "%zu reference samples, SNR %.1f dB, max |diff| %d", RefCount, Snr, MaxDiff );
			printf( "      %s\n", Detail );
			InReport( RefCount == Pcm.size() && Snr > 40.0, "Wwise Vorbis decode: SNR > 40 dB against ffmpeg's samples", Detail );
		}

		vorbis_dsp_clear( &State.TremorInfo.VorbisDSPState );
		g_VorbisCodebookMgr.ReleaseCodebook( State );
	}
	TestBankVorbisSource( (unsigned char *)Media, MediaSize, Channels, InReport );
	InReport( g_VorbisCodebookMgr.m_codebooks.Length() == 0, "Wwise Vorbis codebook released (refcount back to 0)", NULL );
	if( State.pSeekTable )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, State.pSeekTable );
	}
	AK::MemoryMgr::DestroyPool( SmokePool );
	g_LEngineDefaultPoolId = SavedPool;
	if( bOwnMemoryMgr )
	{
		AK::MemoryMgr::Term();
	}
}

#endif
