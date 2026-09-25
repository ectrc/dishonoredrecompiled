// EdgeAnimSmoke: decodes the Edge animations extracted by resources/tools/pdb/extract_edgeanim.py with the port
// (Engine/Src/EdgeAnimEvaluate.cpp) and compares every sample with the retail evaluator of Dishonored.exe.
//
//   EdgeAnimSmoke <data dir> [--retail <Dishonored.exe>] [--filter <substring>] [--dump <substring>] [--skeletons <dir>] [--synthetic <count>] [--verbose 1]
//
// Exit code = number of failed checks.
#include "Core.h"
#include "EdgeAnim.h"

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{
const DWORD RetailEvaluateRva = 0x5dada0;	// __edgeAnimEvaluate, 2013 (2012 0x622a50)
const DWORD SkeletonTag = 0x45533033;
const DWORD AnimationTag = 0x45413035;

typedef void (__cdecl* FRetailEvaluate)(void* OutJoints, float* OutUserChannels, const void* Anim, const void* FrameSetData, DWORD IntraFrameCount, DWORD FrameInteger, float FrameFraction);

/** Maps Dishonored.exe at a free address and applies its base relocations: the evaluator functions only call each other. */
struct FRetailImage
{
	BYTE* Base = nullptr;

	bool Load(const char* Path)
	{
		FILE* File = fopen(Path, "rb");
		if (!File)
		{
			return false;
		}
		std::vector<BYTE> Data;
		fseek(File, 0, SEEK_END);
		Data.resize(ftell(File));
		fseek(File, 0, SEEK_SET);
		fread(Data.data(), 1, Data.size(), File);
		fclose(File);
		const IMAGE_DOS_HEADER* Dos = (const IMAGE_DOS_HEADER*)Data.data();
		const IMAGE_NT_HEADERS32* Nt = (const IMAGE_NT_HEADERS32*)(Data.data() + Dos->e_lfanew);
		Base = (BYTE*)VirtualAlloc(nullptr, Nt->OptionalHeader.SizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!Base)
		{
			return false;
		}
		memcpy(Base, Data.data(), Nt->OptionalHeader.SizeOfHeaders);
		const IMAGE_SECTION_HEADER* Section = IMAGE_FIRST_SECTION(Nt);
		for (WORD Index = 0; Index < Nt->FileHeader.NumberOfSections; Index++, Section++)
		{
			const DWORD Size = Section->SizeOfRawData < Section->Misc.VirtualSize || Section->Misc.VirtualSize == 0 ? Section->SizeOfRawData : Section->Misc.VirtualSize;
			memcpy(Base + Section->VirtualAddress, Data.data() + Section->PointerToRawData, Size);
		}
		const DWORD Delta = (DWORD)(PTRINT)Base - Nt->OptionalHeader.ImageBase;
		const IMAGE_DATA_DIRECTORY& Relocs = Nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
		const BYTE* Block = Base + Relocs.VirtualAddress;
		const BYTE* End = Block + Relocs.Size;
		while (Block < End)
		{
			const IMAGE_BASE_RELOCATION* Header = (const IMAGE_BASE_RELOCATION*)Block;
			if (Header->SizeOfBlock == 0)
			{
				break;
			}
			const WORD* Entries = (const WORD*)(Header + 1);
			const DWORD Count = (Header->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			for (DWORD Entry = 0; Entry < Count; Entry++)
			{
				if ((Entries[Entry] >> 12) == IMAGE_REL_BASED_HIGHLOW)
				{
					*(DWORD*)(Base + Header->VirtualAddress + (Entries[Entry] & 0xFFF)) += Delta;
				}
			}
			Block += Header->SizeOfBlock;
		}
		return true;
	}

	FRetailEvaluate Evaluate() const
	{
		return (FRetailEvaluate)(Base + RetailEvaluateRva);
	}
};

struct FBlob
{
	std::string Name;
	BYTE* Data = nullptr;
	DWORD Size = 0;
};

bool ReadBlob(const std::string& Path, const std::string& Name, FBlob& Out)
{
	FILE* File = fopen(Path.c_str(), "rb");
	if (!File)
	{
		return false;
	}
	fseek(File, 0, SEEK_END);
	Out.Size = ftell(File);
	fseek(File, 0, SEEK_SET);
	Out.Data = (BYTE*)_aligned_malloc(Out.Size + 64, 16);
	memset(Out.Data, 0, Out.Size + 64);
	fread(Out.Data, 1, Out.Size, File);
	fclose(File);
	Out.Name = Name;
	return true;
}

std::vector<FBlob> ReadBlobs(const std::string& Dir, const char* Extension)
{
	std::vector<FBlob> Blobs;
	WIN32_FIND_DATAA Find;
	HANDLE Handle = FindFirstFileA((Dir + "\\*" + Extension).c_str(), &Find);
	if (Handle == INVALID_HANDLE_VALUE)
	{
		return Blobs;
	}
	do
	{
		FBlob Blob;
		if (ReadBlob(Dir + "\\" + Find.cFileName, Find.cFileName, Blob))
		{
			Blobs.push_back(Blob);
		}
	}
	while (FindNextFileA(Handle, &Find));
	FindClose(Handle);
	return Blobs;
}

const BYTE* SelfRelative(const DWORD& Field)
{
	return Field ? (const BYTE*)&Field + Field : nullptr;
}

struct FStats
{
	int Failures = 0;
	int Checks = 0;
	long long Samples = 0;
	long long Floats = 0;
	long long BitExact = 0;
	double MaxDiff = 0.0;
	double MaxNormError = 0.0;
	double MaxPoseNormError = 0.0;
	int LocomotionSequences = 0;
	int LocomotionParentMatches = 0;
	int LocomotionLocalMatches = 0;
	double MaxLocomotionError = 0.0;
	bool bVerbose = false;

	void Check(bool bPassed, const char* What, const std::string& Name)
	{
		Checks++;
		if (!bPassed)
		{
			Failures++;
			printf("FAIL  %s  %s\n", What, Name.c_str());
		}
	}
};

DWORD FloatBits(float Value)
{
	DWORD Bits;
	memcpy(&Bits, &Value, sizeof(Bits));
	return Bits;
}

const EdgeAnimSkeleton* FindSkeleton(const EdgeAnimAnimation* Anim, const std::vector<FBlob>& Skeletons, std::string& OutName)
{
	const DWORD* Hashes = (const DWORD*)(SelfRelative(Anim->offsetCustomData) + (Anim->offsetLocomotionDelta ? 32 : 0));
	for (const FBlob& Blob : Skeletons)
	{
		const EdgeAnimSkeleton* Skel = (const EdgeAnimSkeleton*)Blob.Data;
		INT Hint = 0;
		bool bAll = true;
		for (WORD Joint = 0; Joint < Anim->numJoints && bAll; Joint++)
		{
			bAll = EdgeAnimSkeletonGetJointIndexByHash(Skel, Hashes[Joint], Hint) != INDEX_NONE;
		}
		if (bAll)
		{
			OutName = Blob.Name;
			return Skel;
		}
	}
	return nullptr;
}

void DumpJoints(const char* Label, const EdgeAnimJointTransform* Joints, int NumJoints)
{
	for (int Joint = 0; Joint < NumJoints; Joint++)
	{
		const EdgeAnimJointTransform& J = Joints[Joint];
		printf("  %s %3d R(%+.6f %+.6f %+.6f %+.6f) T(%+.5f %+.5f %+.5f %+.3f)\n", Label, Joint,
			J.rotation.X, J.rotation.Y, J.rotation.Z, J.rotation.W, J.translation.X, J.translation.Y, J.translation.Z, J.translation.W);
	}
}

/** Deterministic synthetic animations for the decoders the cooked data never selects (RConst, STConst, ST; flags = 0). */
struct FSyntheticBuilder
{
	std::vector<BYTE> Bytes;
	DWORD State;

	explicit FSyntheticBuilder(DWORD Seed)
		: State(Seed * 2654435761u + 1)
	{
	}

	DWORD Random()
	{
		State ^= State << 13;
		State ^= State >> 17;
		State ^= State << 5;
		return State;
	}

	float RandomFloat(float Range)
	{
		return ((Random() & 0xFFFFFF) / 16777215.0f * 2.0f - 1.0f) * Range;
	}

	DWORD Tell() const
	{
		return (DWORD)Bytes.size();
	}

	void Align(DWORD Alignment)
	{
		while (Bytes.size() % Alignment)
		{
			Bytes.push_back(0);
		}
	}

	template<typename T> void Put(const T& Value)
	{
		const BYTE* Raw = (const BYTE*)&Value;
		Bytes.insert(Bytes.end(), Raw, Raw + sizeof(T));
	}

	template<typename T> void PutAt(DWORD Offset, const T& Value)
	{
		memcpy(&Bytes[Offset], &Value, sizeof(T));
	}

	void PutRotation48()
	{
		float Quat[4];
		float Length = 0.0f;
		for (int Index = 0; Index < 4; Index++)
		{
			Quat[Index] = RandomFloat(1.0f);
			Length += Quat[Index] * Quat[Index];
		}
		Length = sqrtf(Length);
		int Largest = 0;
		for (int Index = 0; Index < 4; Index++)
		{
			Quat[Index] /= Length;
			Largest = fabsf(Quat[Index]) > fabsf(Quat[Largest]) ? Index : Largest;
		}
		const float Flip = Quat[Largest] < 0.0f ? -1.0f : 1.0f;
		DWORD Stored[3];
		int StoredIndex = 0;
		for (int Index = 0; Index < 4; Index++)
		{
			if (Index != Largest)
			{
				const int Value = (int)((Quat[Index] * Flip + 0.70710677f) / 4.3159689e-05f + 0.5f);
				Stored[StoredIndex++] = (DWORD)(Value < 0 ? 0 : Value > 32767 ? 32767 : Value);
			}
		}
		const DWORD Low = (Stored[1] << 17) | (Stored[2] << 2) | (DWORD)Largest;
		const BYTE Key[6] = { (BYTE)(Stored[0] >> 8), (BYTE)Stored[0], (BYTE)(Low >> 24), (BYTE)(Low >> 16), (BYTE)(Low >> 8), (BYTE)Low };
		Bytes.insert(Bytes.end(), Key, Key + 6);
	}

	void PutVector12()
	{
		for (int Index = 0; Index < 3; Index++)
		{
			Put(RandomFloat(50.0f));
		}
	}

	void PutTable(DWORD Count, DWORD Alignment, WORD NumJoints)
	{
		const DWORD Padded = (Count + Alignment - 1) / Alignment * Alignment;
		for (DWORD Index = 0; Index < Padded; Index++)
		{
			Put((WORD)(Index < Count ? Random() % NumJoints : NumJoints));
		}
	}

	std::vector<BYTE> Build()
	{
		const WORD NumJoints = (WORD)(8 + Random() % 16);
		const DWORD ConstR = Random() % 11;
		const DWORD ConstT = Random() % 7;
		const DWORD ConstS = Random() % 5;
		const DWORD AnimR = 1 + Random() % 9;
		const DWORD AnimT = Random() % 7;
		const DWORD AnimS = Random() % 5;
		const DWORD IntraFrames = 1 + Random() % 40;
		const float Frequency = 30.0f;

		Bytes.assign(96, 0);
		PutTable(ConstR, 8, NumJoints);
		PutTable(ConstT, 4, NumJoints);
		PutTable(ConstS, 4, NumJoints);
		PutTable(AnimR, 4, NumJoints);
		PutTable(AnimT, 4, NumJoints);
		PutTable(AnimS, 4, NumJoints);
		Align(16);
		const DWORD ConstRPos = Tell();
		for (DWORD Index = 0; Index < (ConstR + 7) / 8 * 8; Index++)
		{
			PutRotation48();
		}
		Align(16);
		const DWORD ConstTPos = Tell();
		for (DWORD Index = 0; Index < (ConstT + 3) / 4 * 4; Index++)
		{
			PutVector12();
		}
		Align(16);
		const DWORD ConstSPos = Tell();
		for (DWORD Index = 0; Index < (ConstS + 3) / 4 * 4; Index++)
		{
			PutVector12();
		}
		const DWORD InfoPos = Tell();
		Put((WORD)0);
		Put((WORD)IntraFrames);
		Put((WORD)(IntraFrames + 1));
		Put((WORD)0);
		const DWORD DmaPos = Tell();
		Bytes.resize(Bytes.size() + 16, 0);
		const DWORD HashPos = Tell();
		for (WORD Joint = 0; Joint < NumJoints; Joint++)
		{
			Put((DWORD)Joint);
		}
		Align(16);

		const DWORD SetPos = Tell();
		Bytes.resize(Bytes.size() + 16, 0);
		const DWORD Channels = AnimR + AnimT + AnimS;
		std::vector<BYTE> Bits(Channels * IntraFrames);
		DWORD Keys[3] = { 0, 0, 0 };
		for (DWORD Channel = 0; Channel < Channels; Channel++)
		{
			for (DWORD Frame = 0; Frame < IntraFrames; Frame++)
			{
				Bits[Channel * IntraFrames + Frame] = (Random() % 5) < 2;
				Keys[Channel < AnimR ? 0 : Channel < AnimR + AnimT ? 1 : 2] += Bits[Channel * IntraFrames + Frame];
			}
		}
		const DWORD InitialPos = Tell();
		for (DWORD Index = 0; Index < AnimR; Index++)
		{
			PutRotation48();
		}
		const DWORD InitialTPos = Tell();
		for (DWORD Index = 0; Index < AnimT; Index++)
		{
			PutVector12();
		}
		const DWORD InitialSPos = Tell();
		for (DWORD Index = 0; Index < AnimS; Index++)
		{
			PutVector12();
		}
		const DWORD BitsPos = Tell();
		Bytes.resize(Bytes.size() + (Bits.size() + 7) / 8, 0);
		for (DWORD Bit = 0; Bit < Bits.size(); Bit++)
		{
			Bytes[BitsPos + Bit / 8] |= (BYTE)(Bits[Bit] << (7 - Bit % 8));
		}
		const DWORD IntraRPos = Tell();
		for (DWORD Index = 0; Index < Keys[0]; Index++)
		{
			PutRotation48();
		}
		const DWORD IntraTPos = Tell();
		for (DWORD Index = 0; Index < Keys[1]; Index++)
		{
			PutVector12();
		}
		const DWORD IntraSPos = Tell();
		for (DWORD Index = 0; Index < Keys[2]; Index++)
		{
			PutVector12();
		}
		const DWORD IntraSEnd = Tell();
		Align(16);
		const DWORD FinalPos = Tell();
		Bytes.resize(Bytes.size() + 16, 0);
		for (DWORD Index = 0; Index < AnimR; Index++)
		{
			PutRotation48();
		}
		const DWORD FinalTPos = Tell();
		for (DWORD Index = 0; Index < AnimT; Index++)
		{
			PutVector12();
		}
		const DWORD FinalSPos = Tell();
		for (DWORD Index = 0; Index < AnimS; Index++)
		{
			PutVector12();
		}
		Bytes.resize(Bytes.size() + 64, 0);
		const DWORD SetEnd = Tell();

		const WORD Sizes[8] = { (WORD)(InitialTPos - InitialPos), (WORD)(InitialSPos - InitialTPos), (WORD)(BitsPos - InitialSPos), 0,
			(WORD)(IntraTPos - IntraRPos), (WORD)(IntraSPos - IntraTPos), (WORD)(IntraSEnd - IntraSPos), 0 };
		const WORD FinalSizes[8] = { (WORD)(FinalTPos - (FinalPos + 16)), (WORD)(FinalSPos - FinalTPos), (WORD)(AnimS * 12), 0, 0, 0, 0, 0 };
		for (int Index = 0; Index < 8; Index++)
		{
			PutAt(SetPos + 2 * Index, Sizes[Index]);
			PutAt(FinalPos + 2 * Index, FinalSizes[Index]);
		}
		PutAt(DmaPos + 2, (WORD)(SetEnd - SetPos));
		PutAt(DmaPos + 4, SetPos);
		PutAt(DmaPos + 12, SetEnd);

		PutAt(0, AnimationTag);
		PutAt(4, (IntraFrames + 1) / Frequency);
		PutAt(8, Frequency);
		PutAt(12, (WORD)SetPos);
		PutAt(14, NumJoints);
		PutAt(16, (WORD)(IntraFrames + 2));
		PutAt(18, (WORD)2);
		const WORD Counts[8] = { (WORD)ConstR, (WORD)ConstT, (WORD)ConstS, 0, (WORD)AnimR, (WORD)AnimT, (WORD)AnimS, 0 };
		for (int Index = 0; Index < 8; Index++)
		{
			PutAt(22 + 2 * Index, Counts[Index]);
		}
		PutAt(38, (WORD)0);
		PutAt(56, DmaPos - 56);
		PutAt(60, InfoPos - 60);
		PutAt(64, ConstR ? ConstRPos - 64 : 0);
		PutAt(68, ConstT ? ConstTPos - 68 : 0);
		PutAt(72, ConstS ? ConstSPos - 72 : 0);
		PutAt(84, HashPos - 84);
		PutAt(88, (DWORD)(4 * NumJoints));
		return Bytes;
	}
};

void RotateByInverse(const FEdgeAnimVec4& Q, const double V[3], double Out[3])
{
	// conj(Q) * V * Q for a unit quaternion
	const double X = -Q.X, Y = -Q.Y, Z = -Q.Z, W = Q.W;
	const double TX = 2.0 * (Y * V[2] - Z * V[1]);
	const double TY = 2.0 * (Z * V[0] - X * V[2]);
	const double TZ = 2.0 * (X * V[1] - Y * V[0]);
	Out[0] = V[0] + W * TX + (Y * TZ - Z * TY);
	Out[1] = V[1] + W * TY + (Z * TX - X * TZ);
	Out[2] = V[2] + W * TZ + (X * TY - Y * TX);
}

/** Skeleton-order pose (EdgeAnimEvaluateSkeletonPose): finite, normalized, and the locomotion delta against the evaluated root. */
void TestSkeletonPose(const FBlob& Blob, const EdgeAnimSkeleton* Skel, FStats& Stats)
{
	const EdgeAnimAnimation* Anim = (const EdgeAnimAnimation*)Blob.Data;
	TArray<EdgeAnimJointTransform> Pose;
	TArray<EdgeAnimJointTransform> First;
	bool bFinite = true;
	for (int Frame = 0; Frame <= Anim->numFrames; Frame++)
	{
		const float Time = Frame == Anim->numFrames ? Anim->duration : Frame / Anim->sampleFrequency;
		if (!EdgeAnimEvaluateSkeletonPose(Anim, Skel, Time, FALSE, Pose))
		{
			bFinite = false;
			break;
		}
		for (int Joint = 0; Joint < Pose.Num(); Joint++)
		{
			const float* F = &Pose(Joint).rotation.X;
			for (int Index = 0; Index < 7; Index++)
			{
				bFinite &= std::isfinite(F[Index]) != 0;
			}
			const FEdgeAnimVec4& Q = Pose(Joint).rotation;
			const double Norm = sqrt((double)Q.X * Q.X + (double)Q.Y * Q.Y + (double)Q.Z * Q.Z + (double)Q.W * Q.W);
			Stats.MaxPoseNormError = fabs(Norm - 1.0) > Stats.MaxPoseNormError ? fabs(Norm - 1.0) : Stats.MaxPoseNormError;
		}
		if (Frame == 0)
		{
			First = Pose;
		}
	}
	Stats.Check(bFinite && Pose.Num() == Skel->numJoints, "skeleton pose finite", Blob.Name);
	if (!bFinite || !Anim->offsetLocomotionDelta)
	{
		return;
	}
	const float* Stored = (const float*)SelfRelative(Anim->offsetLocomotionDelta);
	const int Joint = Skel->locomotionJointIndex;
	const double Delta[3] = {
		(double)Pose(Joint).translation.X - First(Joint).translation.X,
		(double)Pose(Joint).translation.Y - First(Joint).translation.Y,
		(double)Pose(Joint).translation.Z - First(Joint).translation.Z };
	double Local[3];
	RotateByInverse(First(Joint).rotation, Delta, Local);
	double ParentError = 0.0;
	double LocalError = 0.0;
	double Length = 0.0;
	for (int Index = 0; Index < 3; Index++)
	{
		ParentError += (Delta[Index] - Stored[4 + Index]) * (Delta[Index] - Stored[4 + Index]);
		LocalError += (Local[Index] - Stored[4 + Index]) * (Local[Index] - Stored[4 + Index]);
		Length += (double)Stored[4 + Index] * Stored[4 + Index];
	}
	Stats.LocomotionSequences++;
	// the stored delta comes from the uncompressed source, the evaluated one from the quantized keys
	Stats.LocomotionParentMatches += sqrt(ParentError) <= 0.05 + 0.005 * sqrt(Length);
	Stats.LocomotionLocalMatches += sqrt(LocalError) <= 0.05 + 0.005 * sqrt(Length);
	Stats.MaxLocomotionError = sqrt(ParentError) > Stats.MaxLocomotionError ? sqrt(ParentError) : Stats.MaxLocomotionError;
	if (Stats.bVerbose)
	{
		printf("loco %s stored(%.4f %.4f %.4f) parent(%.4f %.4f %.4f) local(%.4f %.4f %.4f)\n", Blob.Name.c_str(),
			Stored[4], Stored[5], Stored[6], Delta[0], Delta[1], Delta[2], Local[0], Local[1], Local[2]);
	}
}

void TestAnimation(const FBlob& Blob, const std::vector<FBlob>& Skeletons, FRetailEvaluate Retail, bool bDump, FStats& Stats)
{
	const EdgeAnimAnimation* Anim = (const EdgeAnimAnimation*)Blob.Data;
	Stats.Check(Anim->tag == AnimationTag && Anim->sizeHeader <= Blob.Size, "header", Blob.Name);
	if (Anim->tag != AnimationTag)
	{
		return;
	}
	std::string SkeletonName;
	if (!Skeletons.empty())
	{
		const EdgeAnimSkeleton* Skel = FindSkeleton(Anim, Skeletons, SkeletonName);
		Stats.Check(Skel != nullptr, "skeleton by joint hashes", Blob.Name);
		if (Skel)
		{
			TestSkeletonPose(Blob, Skel, Stats);
		}
	}

	const int NumJoints = Anim->numJoints;
	const int Capacity = NumJoints + 1;
	EdgeAnimJointTransform* Ours = (EdgeAnimJointTransform*)_aligned_malloc(sizeof(EdgeAnimJointTransform) * Capacity, 16);
	EdgeAnimJointTransform* Theirs = (EdgeAnimJointTransform*)_aligned_malloc(sizeof(EdgeAnimJointTransform) * Capacity, 16);
	double SequenceMaxDiff = 0.0;
	long long SequenceMismatch = 0;
	const int NumSamples = 2 * Anim->numFrames + 1;
	for (int Sample = 0; Sample < NumSamples; Sample++)
	{
		const float Time = Sample == NumSamples - 1 ? Anim->duration : (Sample / 2 + (Sample & 1) * 0.37f) / Anim->sampleFrequency;
		FEdgeAnimFrameSample Frame;
		if (!EdgeAnimLocateFrame(Anim, Time, Frame))
		{
			Stats.Check(false, "locate frame", Blob.Name);
			break;
		}
		memset(Ours, 0xFF, sizeof(EdgeAnimJointTransform) * Capacity);
		memset(Theirs, 0xFF, sizeof(EdgeAnimJointTransform) * Capacity);
		EdgeAnimEvaluate(Ours, Anim, Frame);
		if (Retail)
		{
			Retail(Theirs, nullptr, Anim, Frame.FrameSetData, Frame.IntraFrameCount, Frame.FrameInteger, Frame.FrameFraction);
		}
		Stats.Samples++;
		for (int Joint = 0; Joint < NumJoints; Joint++)
		{
			const float* A = &Ours[Joint].rotation.X;
			const float* B = &Theirs[Joint].rotation.X;
			for (int Index = 0; Index < 8 && Retail; Index++)
			{
				Stats.Floats++;
				if (FloatBits(A[Index]) == FloatBits(B[Index]))
				{
					Stats.BitExact++;
					continue;
				}
				SequenceMismatch++;
				const double Diff = std::isnan(A[Index]) || std::isnan(B[Index]) ? 1e30 : fabs((double)A[Index] - (double)B[Index]);
				SequenceMaxDiff = Diff > SequenceMaxDiff ? Diff : SequenceMaxDiff;
			}
			const FEdgeAnimVec4& Q = Ours[Joint].rotation;
			if (FloatBits(Q.X) != 0xFFFFFFFF)
			{
				const double Norm = sqrt((double)Q.X * Q.X + (double)Q.Y * Q.Y + (double)Q.Z * Q.Z + (double)Q.W * Q.W);
				Stats.MaxNormError = fabs(Norm - 1.0) > Stats.MaxNormError ? fabs(Norm - 1.0) : Stats.MaxNormError;
			}
		}
		if (bDump && (Sample == 0 || Sample == 1))
		{
			printf("%s t=%.4f set@%d intra=%u frame=%u frac=%.4f\n", Blob.Name.c_str(), Time, (int)(Frame.FrameSetData - Blob.Data), Frame.IntraFrameCount, Frame.FrameInteger, Frame.FrameFraction);
			DumpJoints("port  ", Ours, NumJoints);
			if (Retail)
			{
				DumpJoints("retail", Theirs, NumJoints);
			}
		}
	}
	Stats.MaxDiff = SequenceMaxDiff > Stats.MaxDiff ? SequenceMaxDiff : Stats.MaxDiff;
	if (Retail)
	{
		char Detail[160];
		snprintf(Detail, sizeof(Detail), "retail match (%lld floats differ, max |d| %.3g)", SequenceMismatch, SequenceMaxDiff);
		Stats.Check(SequenceMismatch == 0, Detail, Blob.Name);
	}
	_aligned_free(Ours);
	_aligned_free(Theirs);
}
} // namespace

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("usage: EdgeAnimSmoke <data dir> [--retail <Dishonored.exe>] [--filter <substring>] [--dump <substring>] [--skeletons <dir>] [--synthetic <count>] [--verbose 1]\n");
		return 1;
	}
	const std::string Dir = argv[1];
	FStats Stats;
	const char* RetailPath = nullptr;
	const char* Filter = "";
	const char* Dump = nullptr;
	int Synthetic = 0;
	std::vector<std::string> SkeletonDirs(1, Dir);
	for (int Arg = 2; Arg + 1 < argc; Arg += 2)
	{
		if (!strcmp(argv[Arg], "--retail"))
		{
			RetailPath = argv[Arg + 1];
		}
		else if (!strcmp(argv[Arg], "--filter"))
		{
			Filter = argv[Arg + 1];
		}
		else if (!strcmp(argv[Arg], "--dump"))
		{
			Dump = argv[Arg + 1];
		}
		else if (!strcmp(argv[Arg], "--skeletons"))
		{
			SkeletonDirs.push_back(argv[Arg + 1]);
		}
		else if (!strcmp(argv[Arg], "--verbose"))
		{
			Stats.bVerbose = atoi(argv[Arg + 1]) != 0;
		}
		else if (!strcmp(argv[Arg], "--synthetic"))
		{
			Synthetic = atoi(argv[Arg + 1]);
		}
	}
	FRetailImage Image;
	FRetailEvaluate Retail = nullptr;
	if (RetailPath)
	{
		if (!Image.Load(RetailPath))
		{
			printf("cannot map %s\n", RetailPath);
			return 1;
		}
		Retail = Image.Evaluate();
		printf("retail evaluator mapped at %p\n", (void*)Retail);
	}

	std::vector<FBlob> Skeletons;
	for (const std::string& SkeletonDir : SkeletonDirs)
	{
		const std::vector<FBlob> More = ReadBlobs(SkeletonDir, ".skel");
		Skeletons.insert(Skeletons.end(), More.begin(), More.end());
	}
	for (const FBlob& Blob : Skeletons)
	{
		const EdgeAnimSkeleton* Skel = (const EdgeAnimSkeleton*)Blob.Data;
		Stats.Check(Skel->tag == SkeletonTag && Skel->sizeTotal == Blob.Size, "skeleton header", Blob.Name);
	}
	std::vector<FBlob> Animations = ReadBlobs(Dir, ".anim");
	int Tested = 0;
	for (const FBlob& Blob : Animations)
	{
		if (Blob.Name.find(Filter) == std::string::npos || ((const EdgeAnimAnimation*)Blob.Data)->tag != AnimationTag)
		{
			continue;
		}
		TestAnimation(Blob, Skeletons, Retail, Dump && Blob.Name.find(Dump) != std::string::npos, Stats);
		Tested++;
	}
	for (int Seed = 1; Seed <= Synthetic; Seed++)
	{
		FSyntheticBuilder Builder(Seed);
		const std::vector<BYTE> Bytes = Builder.Build();
		FBlob Blob;
		Blob.Name = "synthetic_" + std::to_string(Seed);
		Blob.Size = (DWORD)Bytes.size();
		Blob.Data = (BYTE*)_aligned_malloc(Bytes.size() + 64, 16);
		memset(Blob.Data, 0, Bytes.size() + 64);
		memcpy(Blob.Data, Bytes.data(), Bytes.size());
		TestAnimation(Blob, std::vector<FBlob>(), Retail, Dump && Blob.Name == Dump, Stats);
		_aligned_free(Blob.Data);
		Tested++;
	}
	printf("%d skeletons, %d animations, %lld samples; %lld/%lld floats bit-exact vs retail, max |diff| %.3g; max |quat|-1 %.3g (skeleton pose %.3g); locomotion delta = root delta in %d/%d (parent space, max |d| %.3g), %d/%d (start-local); %d/%d checks failed\n",
		(int)Skeletons.size(), Tested, Stats.Samples, Stats.BitExact, Stats.Floats, Stats.MaxDiff, Stats.MaxNormError, Stats.MaxPoseNormError,
		Stats.LocomotionParentMatches, Stats.LocomotionSequences, Stats.MaxLocomotionError, Stats.LocomotionLocalMatches, Stats.LocomotionSequences, Stats.Failures, Stats.Checks);
	return Stats.Failures;
}
