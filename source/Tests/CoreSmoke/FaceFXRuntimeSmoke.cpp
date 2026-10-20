// CoreSmoke: the FaceFX 1.7.3.1 runtime (agent FX3, wave 26) on retail data - the elite guard's head
// (Skm_EliteGuard_Head_FaceFX, L_Tower_Script) and the anim set holding his 'SheWaiting' line
// (anim F3EAC6D40633C67014DEC53E298F9F7E = the blurb's Wwise event minus "Play_").
//  1. FxActor loads the head through an FxArchive to the last byte, with the dumper's bone, node and phoneme-map counts;
//     FxSaveActorToMemory writes it back byte for byte; FxLoadActorFromMemory loads it too.
//  2. The anim set loads and mounts (FindAnimGroup finds it after the owned groups), the anim's bounds are the keys'.
//  3. An FxActorInstance plays the anim: the first Tick with no audio time reports APS_Playing (the anim starts
//     0.359 s before its audio), the one that reaches anim time 0 reports APS_StartAudio exactly once, the end stops it.
//  5. Every cooked head of FX2's set (FACEFX_SMOKE_DIR, default ../agentFX2: fxa/*.fxa listed in fxa_counts.csv) loads as an
//     FxActor to the last byte with the dumper's bone and phoneme-map counts and FxSaveActorToMemory writes it back byte for byte.
//  4. At anim times 0.5 / 1.0 / 1.5 s the 'open' node and the jaw, lower lip and head bones equal the hand derivation of
//     build/agentFX3/py/fx3_handcalc.py (expected.txt), and the jaw moves away from its reference pose.
// FACEFX_RUNTIME_SMOKE_DIR names the folder with head.fxa, animset.fxa, expected.txt (default build/agentFX3/fx3, then
// ../agentFX3/fx3).
#include "FxActorInstance.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace OC3Ent::Face;

static void (*GRuntimeReport)(bool, const char*, const char*) = NULL;
static void (*GRuntimeSkip)(const char*, const char*) = NULL;

static void RuntimeCheck(bool bOk, const std::string& What, const std::string& Detail = "")
{
	GRuntimeReport(bOk, What.c_str(), Detail.c_str());
}

static bool ReadBytes(const std::string& Path, std::vector<FxByte>& Out)
{
	std::ifstream In(Path.c_str(), std::ios::binary);
	if( !In )
	{
		return false;
	}
	Out.assign(std::istreambuf_iterator<char>(In), std::istreambuf_iterator<char>());
	return !Out.empty();
}

static std::string FindRuntimeDir()
{
	const char* Env = getenv("FACEFX_RUNTIME_SMOKE_DIR");
	if( Env && *Env )
	{
		return Env;
	}
	const char* Candidates[] = { "build/agentFX3/fx3", "../agentFX3/fx3" };
	for( const char* Candidate : Candidates )
	{
		if( std::ifstream((std::string(Candidate) + "/expected.txt").c_str()) )
		{
			return Candidate;
		}
	}
	return "";
}

struct FExpectedBone
{
	double Values[7];
};

static std::string Describe(const FxVec3& Pos, const FxQuat& Rot)
{
	char Buffer[256];
	sprintf_s(Buffer, "pos %.6f %.6f %.6f rot %.6f %.6f %.6f %.6f", Pos.x, Pos.y, Pos.z, Rot.w, Rot.x, Rot.y, Rot.z);
	return Buffer;
}

static void TestEveryHead()
{
	const char* Env = getenv("FACEFX_SMOKE_DIR");
	std::string Dir = Env && *Env ? Env : "../agentFX2";
	std::ifstream Csv((Dir + "/fxa_counts.csv").c_str());
	if( !Csv )
	{
		Dir = "build/agentFX2";
		Csv.open((Dir + "/fxa_counts.csv").c_str());
	}
	if( !Csv )
	{
		GRuntimeSkip("FaceFX runtime: every cooked head as an FxActor", "no fxa_counts.csv in FACEFX_SMOKE_DIR / ../agentFX2 / build/agentFX2");
		return;
	}
	std::string Line;
	std::getline(Csv, Line);
	int Heads = 0, Loaded = 0, Resaved = 0;
	std::string Failures;
	while( std::getline(Csv, Line) )
	{
		std::vector<std::string> Fields;
		std::stringstream Row(Line);
		std::string Field;
		while( std::getline(Row, Field, ',') )
		{
			Fields.push_back(Field);
		}
		if( Fields.size() < 20 )
		{
			continue;
		}
		std::vector<FxByte> Bytes;
		if( !ReadBytes(Dir + "/fxa/" + Fields[19], Bytes) )
		{
			continue;
		}
		++Heads;
		FxActor* Actor = new FxActor();
		FxArchive Load(FxArchiveStoreMemoryNoCopy::Create(&Bytes[0], static_cast<FxSize>(Bytes.size())), FxArchive::AM_LinearLoad);
		Load.Open();
		Load << *Actor;
		if( Load.Tell() == static_cast<FxInt32>(Bytes.size()) && std::to_string(Actor->GetMasterBoneList().GetNumRefBones()) == Fields[11]
			&& std::to_string(Actor->GetPhonemeMap().GetNumMappingEntries()) == Fields[16] && std::string(Actor->GetNameAsCstr()) == Fields[0] )
		{
			++Loaded;
		}
		else
		{
			Failures += " load:" + Fields[0];
		}
		FxByte* Saved = NULL;
		FxSize SavedSize = 0;
		if( FxSaveActorToMemory(*Actor, Saved, SavedSize) && SavedSize == Bytes.size() && memcmp(Saved, &Bytes[0], SavedSize) == 0 )
		{
			++Resaved;
		}
		else
		{
			Failures += " save:" + Fields[0];
		}
		if( Saved )
		{
			FxFree(Saved, SavedSize);
		}
		delete Actor;
	}
	RuntimeCheck(Heads > 0 && Loaded == Heads, "FaceFX runtime: every cooked head loads as an FxActor to the last byte",
		std::to_string(Loaded) + " of " + std::to_string(Heads) + Failures);
	RuntimeCheck(Heads > 0 && Resaved == Heads, "FaceFX runtime: every cooked head re-saves byte for byte", std::to_string(Resaved) + " of " + std::to_string(Heads));
	printf("FaceFX runtime: %d heads loaded, %d re-saved byte for byte\n", Loaded, Resaved);
}

void TestFaceFXRuntime(void (*InReport)(bool, const char*, const char*), void (*InSkip)(const char*, const char*))
{
	GRuntimeReport = InReport;
	GRuntimeSkip = InSkip;
	const std::string Dir = FindRuntimeDir();
	std::vector<FxByte> HeadBytes;
	std::vector<FxByte> AnimSetBytes;
	if( Dir.empty() || !ReadBytes(Dir + "/head.fxa", HeadBytes) || !ReadBytes(Dir + "/animset.fxa", AnimSetBytes) )
	{
		GRuntimeSkip("FaceFX runtime: retail head + anim set", "no head.fxa / animset.fxa / expected.txt in FACEFX_RUNTIME_SMOKE_DIR or build/agentFX3/fx3");
		return;
	}
	std::map<std::string, std::string> Scalars;
	std::map<std::string, FExpectedBone> Expected;
	std::map<std::string, int> BoneIndex;
	{
		std::ifstream In((Dir + "/expected.txt").c_str());
		std::string Line;
		while( std::getline(In, Line) )
		{
			std::istringstream Words(Line);
			std::string Key;
			Words >> Key;
			if( Key == "t" )
			{
				std::string Time, What;
				Words >> Time >> What;
				FExpectedBone Bone = {};
				for( int i = 0; i < 7 && Words; ++i )
				{
					Words >> Bone.Values[i];
				}
				Expected[Time + " " + What] = Bone;
			}
			else if( Key == "bone" )
			{
				std::string Name;
				int Index;
				Words >> Name >> Index;
				BoneIndex[Name] = Index;
			}
			else if( Key == "ref" )
			{
				std::string Name;
				Words >> Name;
				FExpectedBone Bone = {};
				for( int i = 0; i < 7; ++i )
				{
					Words >> Bone.Values[i];
				}
				Expected["ref " + Name] = Bone;
			}
			else
			{
				std::string Value;
				std::getline(Words >> std::ws, Value);
				Scalars[Key] = Value;
			}
		}
	}

	FxActor* Actor = new FxActor();
	{
		FxArchive Load(FxArchiveStoreMemoryNoCopy::Create(&HeadBytes[0], static_cast<FxSize>(HeadBytes.size())), FxArchive::AM_LinearLoad);
		Load.Open();
		RuntimeCheck(Load.IsValid() != FxFalse, "FaceFX runtime: head archive opens");
		Load << *Actor;
		RuntimeCheck(Load.Tell() == static_cast<FxInt32>(HeadBytes.size()), "FaceFX runtime: FxActor::Serialize consumes every byte of the head",
			std::to_string(Load.Tell()) + " of " + std::to_string(HeadBytes.size()));
	}
	RuntimeCheck(std::string(Actor->GetNameAsCstr()) == Scalars["actor"], "FaceFX runtime: actor name", Actor->GetNameAsCstr());
	RuntimeCheck(std::to_string(Actor->GetMasterBoneList().GetNumRefBones()) == Scalars["bones"], "FaceFX runtime: master bone list count",
		std::to_string(Actor->GetMasterBoneList().GetNumRefBones()));
	RuntimeCheck(std::to_string(Actor->GetCompiledFaceGraph().nodes.Length()) == Scalars["nodes"], "FaceFX runtime: compiled node count",
		std::to_string(Actor->GetCompiledFaceGraph().nodes.Length()));
	RuntimeCheck(std::to_string(Actor->GetPhonemeMap().GetNumMappingEntries()) == Scalars["phoneme_mappings"], "FaceFX runtime: phoneme map entries",
		std::to_string(Actor->GetPhonemeMap().GetNumMappingEntries()));
	RuntimeCheck(Actor->GetNumAnimGroups() == 1, "FaceFX runtime: a cooked head owns only the default group");
	{
		FxByte* Saved = NULL;
		FxSize SavedSize = 0;
		FxBool bSaved = FxSaveActorToMemory(*Actor, Saved, SavedSize);
		bool bSame = bSaved && SavedSize == HeadBytes.size() && memcmp(Saved, &HeadBytes[0], SavedSize) == 0;
		RuntimeCheck(bSame, "FaceFX runtime: FxSaveActorToMemory writes the retail head back byte for byte",
			std::to_string(SavedSize) + " bytes");
		if( Saved )
		{
			FxFree(Saved, SavedSize);
		}
		FxActor* Second = new FxActor();
		RuntimeCheck(FxLoadActorFromMemory(*Second, &HeadBytes[0], static_cast<FxSize>(HeadBytes.size())) != FxFalse
			&& Second->GetMasterBoneList().GetNumRefBones() == Actor->GetMasterBoneList().GetNumRefBones(), "FaceFX runtime: FxLoadActorFromMemory");
		delete Second;
	}
	RuntimeCheck(FxActor::AddActor(Actor) != FxFalse && FxActor::FindActor(Actor->GetName()) == Actor, "FaceFX runtime: AddActor / FindActor");

	FxAnimSet AnimSet;
	RuntimeCheck(FxLoadAnimSetFromMemory(AnimSet, &AnimSetBytes[0], static_cast<FxSize>(AnimSetBytes.size())) != FxFalse, "FaceFX runtime: anim set loads");
	FxMountUnmountExtendedInfo MountInfo = MUE_Duplicate;
	RuntimeCheck(Actor->MountAnimSet(AnimSet, &MountInfo) != FxFalse && MountInfo == MUE_None, "FaceFX runtime: MountAnimSet");
	const FxName GroupName(Scalars["group"].c_str());
	const FxName AnimName(Scalars["anim"].c_str());
	RuntimeCheck(Actor->FindAnimGroup(GroupName) == 1, "FaceFX runtime: the mounted group's index is numAnimGroups + 1",
		std::to_string(Actor->FindAnimGroup(GroupName)));
	FxAnim* Anim = Actor->GetAnimPtr(GroupName, AnimName);
	RuntimeCheck(Anim != NULL, "FaceFX runtime: GetAnimPtr finds the 'SheWaiting' anim");
	if( !Anim )
	{
		return;
	}
	RuntimeCheck(fabs(Anim->GetStartTime() - atof(Scalars["start"].c_str())) < 1e-6 && fabs(Anim->GetEndTime() - atof(Scalars["end"].c_str())) < 1e-6,
		"FaceFX runtime: anim bounds = first / last key", std::to_string(Anim->GetStartTime()) + " .. " + std::to_string(Anim->GetEndTime()));

	FxActorInstance* Instance = new FxActorInstance();
	Instance->SetActor(Actor);
	RuntimeCheck(Instance->PlayAnim(AnimName, GroupName) != FxFalse && Instance->IsPlayingAnim() != FxFalse, "FaceFX runtime: PlayAnim");
	const FxAnimPlaybackState First = Instance->Tick(10.0, -1.0f);
	RuntimeCheck(First == APS_Playing && fabs(Instance->GetAnimCurrentTime() - Anim->GetStartTime()) < 1e-6,
		"FaceFX runtime: the first tick starts at the anim's lead-in", std::to_string(Instance->GetAnimCurrentTime()));
	int StartAudioTicks = 0;
	double StartAudioTime = -1.0;
	FxAnimPlaybackState State = First;
	for( int Frame = 1; Frame <= 400 && State != APS_Stopped; ++Frame )
	{
		const double AppTime = 10.0 + Frame / 60.0;
		State = Instance->Tick(AppTime, -1.0f);
		if( State == APS_StartAudio )
		{
			++StartAudioTicks;
			StartAudioTime = AppTime - 10.0;
		}
	}
	RuntimeCheck(StartAudioTicks == 1 && StartAudioTime >= -Anim->GetStartTime() && StartAudioTime < -Anim->GetStartTime() + 1.0 / 60.0 + 1e-6,
		"FaceFX runtime: APS_StartAudio once, on the first tick at anim time >= 0", std::to_string(StartAudioTicks) + " at " + std::to_string(StartAudioTime));
	RuntimeCheck(State == APS_Stopped && !Instance->IsPlayingAnim(), "FaceFX runtime: the anim stops at its end time");

	Instance->PlayAnim(AnimName, GroupName);
	FxReal Weight;
	const int Jaw = BoneIndex["jaw_jnt"];
	const FExpectedBone& ExpectedRef = Expected["ref jaw_jnt"];
	const char* Times[] = { "0.5", "1.0", "1.5" };
	FxQuat JawRots[3];
	for( int TimeIndex = 0; TimeIndex < 3; ++TimeIndex )
	{
		const FxReal AnimTime = static_cast<FxReal>(atof(Times[TimeIndex]));
		Instance->Tick(20.0 + TimeIndex, AnimTime);
		const FxReal Open = Actor->GetCompiledFaceGraph().nodes[Actor->GetCompiledFaceGraph().FindNodeIndex(FxName("open"))].finalValue;
		const double ExpectedOpen = Expected[std::string(Times[TimeIndex]) + " open"].Values[0];
		RuntimeCheck(fabs(Open - ExpectedOpen) < 1e-5, std::string("FaceFX runtime: 'open' at anim time ") + Times[TimeIndex] + " = hand derivation",
			std::to_string(Open) + " vs " + std::to_string(ExpectedOpen));
		for( std::map<std::string, int>::const_iterator It = BoneIndex.begin(); It != BoneIndex.end(); ++It )
		{
			FxVec3 Pos, Scale;
			FxQuat Rot;
			Instance->GetBone(It->second, Pos, Rot, Scale, Weight);
			const FExpectedBone& E = Expected[std::string(Times[TimeIndex]) + " " + It->first];
			const double Got[7] = { Pos.x, Pos.y, Pos.z, Rot.w, Rot.x, Rot.y, Rot.z };
			double MaxError = 0.0;
			for( int i = 0; i < 7; ++i )
			{
				const double Error = fabs(Got[i] - E.Values[i]);
				MaxError = Error > MaxError ? Error : MaxError;
			}
			RuntimeCheck(MaxError < 1e-4, std::string("FaceFX runtime: ") + It->first + " at anim time " + Times[TimeIndex] + " = hand derivation",
				Describe(Pos, Rot) + " max error " + std::to_string(MaxError));
			if( It->second == Jaw )
			{
				JawRots[TimeIndex] = Rot;
			}
		}
	}
	const double JawTurn = fabs(JawRots[1].y - ExpectedRef.Values[5]);
	printf("FaceFX runtime: jaw_jnt rot.y reference %.6f, anim time 0.5 / 1.0 / 1.5 s: %.6f / %.6f / %.6f (open %.4f at 1.0 s)\n", ExpectedRef.Values[5],
		JawRots[0].y, JawRots[1].y, JawRots[2].y, Expected["1.0 open"].Values[0]);
	RuntimeCheck(JawTurn > 0.05 && fabs(JawRots[1].y - JawRots[0].y) > 0.05, "FaceFX runtime: the jaw turns away from its reference pose over the line",
		"rot.y ref " + std::to_string(ExpectedRef.Values[5]) + " -> " + std::to_string(JawRots[0].y) + " / " + std::to_string(JawRots[1].y) + " / " + std::to_string(JawRots[2].y));
	Instance->StopAnim();
	RuntimeCheck(Actor->UnmountAnimSet(GroupName) != FxFalse && Actor->FindAnimGroup(GroupName) == FxInvalidIndex, "FaceFX runtime: UnmountAnimSet");
	delete Instance;
	RuntimeCheck(FxActor::RemoveActor(Actor) != FxFalse && FxActor::FindActor(FxName(Scalars["actor"].c_str())) == NULL, "FaceFX runtime: RemoveActor frees the actor");
	TestEveryHead();
}
