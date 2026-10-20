// CoreSmoke: the FaceFX 1.7.3.1 face graph (agent FX2, wave 26).
//  1. A face graph built in code (combiner -> bone poses through linear, quadratic and corrective links, a delta node)
//     compiles in retail's order and Ticks to the values derived by hand from FxCompiledFaceGraph::Tick 0xb114a0.
//  2. A register interpolates with FxHermiteInterpolate(start, end, alpha) 0xb12500 at hand-derived times.
//  3. A curve evaluates with FxAnimCurve::EvaluateAt 0xb16700 (Hermite and linear) at hand-derived times, and
//     InsertKey's autocomputed slope matches the formula of 0xb16220.
//  4. An anim set built in code survives FxSaveAnimSetToMemory / FxLoadAnimSetFromMemory.
//  5. With FACEFX_SMOKE_DIR (default ../agentFX2): every cooked head archive (fxa/*.fxa, counts in fxa_counts.csv from
//     resources/tools/facefx_graph_dump.py --csv) loads its compiled face graph with the dumper's node, link and
//     node-type counts and to the last byte, decompiles and recompiles to the same graph; every anim set (fxs/*.fxa,
//     fxs_counts.csv) loads with the dumper's anim, curve and key counts and FxSaveAnimSetToMemory writes it back
//     byte for byte.
// The graph classes are registered in retail FxSDKStartup's order (2012 0xb14930), which FX1's FxSDKStartup will take over.
// The FxActor wrapper around the graph (master bone list, anim groups, phoneme map) is FX3's; the reader below walks
// it with FX2's types only to reach the graph.
#include "FxCompiledFaceGraph.h"
#include "FxCombinerNode.h"
#include "FxDeltaNode.h"
#include "FxCurrentTimeNode.h"
#include "FxGenericTargetNode.h"
#include "FxBonePoseNode.h"
#include "FxAnim.h"

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

using namespace OC3Ent::Face;

typedef void (*FaceFXReportFunc)(bool, const char*, const char*);
typedef void (*FaceFXSkipFunc)(const char*, const char*);

static FaceFXReportFunc GReport = 0;

static bool StoreOverran()
{
#if FX2_FACEFX_CORE_STUB
	bool bOverran = FxArchiveStoreMemoryNoCopy::_overrun != FxFalse;
	FxArchiveStoreMemoryNoCopy::_overrun = FxFalse;
	return bOverran;
#else
	return false;
#endif
}

static bool Near( FxReal a, FxReal b )
{
	return fabs(a - b) < 1e-5f;
}

static void Check( bool bOk, const char* What, const char* Detail = "" )
{
	GReport(bOk, What, Detail);
}

static void RegisterGraphClasses()
{
	FxSDKStartup();
	FxLinkFn::Startup();
	FxFaceGraph::StaticClass();
	FxCombinerNode::StaticClass();
	FxDeltaNode::StaticClass();
	FxCurrentTimeNode::StaticClass();
	FxGenericTargetNode::StaticClass();
	FxMorphTargetNode::StaticClass();
	FxBonePoseNode::StaticClass();
	FUnrealFaceFXNode::StaticClass();
	FUnrealFaceFXMorphNode::StaticClass();
	FUnrealFaceFXMaterialParameterNode::StaticClass();
	FxFaceGraphNodeUserProperty::StaticClass();
	FxFaceGraphNodeLink::StaticClass();
	FxBone::StaticClass();
	FxAnimCurve::StaticClass();
	FxAnim::StaticClass();
	FxAnimGroup::StaticClass();
	FxAnimSet::StaticClass();
}

static FxLinkFnParameters Params( FxReal a, FxReal b = 0.0f, int Count = 1 )
{
	FxLinkFnParameters p;
	p.parameters.PushBack(a);
	if( Count > 1 )
	{
		p.parameters.PushBack(b);
	}
	return p;
}

static FxFaceGraphNode* NewNode( FxFaceGraphNode* pNode, const char* Name, FxReal Min, FxReal Max )
{
	pNode->SetName(FxName(Name));
	pNode->SetMin(Min);
	pNode->SetMax(Max);
	return pNode;
}

static void SetReplace( FxRegister& Reg, FxReal Value )
{
	Reg.firstRegOp = VO_Replace;
	Reg.interpEndValue = Value;
	Reg.interpInverseDuration = 0.0f;
}

static void TestCodeBuiltGraph()
{
	FxFaceGraph graph;
	graph.AddNode(NewNode(new FxCombinerNode(), "Joy", 0.0f, 1.0f));
	graph.AddNode(NewNode(new FxCombinerNode(), "Disgust", 0.0f, 1.0f));
	FxBonePoseNode* pOpen = static_cast<FxBonePoseNode*>(NewNode(new FxBonePoseNode(), "open", 0.0f, 1.0f));
	pOpen->AddBone(FxBone(FxName("jaw"), FxVec3(0.0f, -1.0f, 0.0f), FxQuat(2.0f, 0.0f, 0.0f, 0.0f), FxVec3(1.0f, 1.0f, 1.0f)));
	graph.AddNode(pOpen);
	graph.AddNode(NewNode(new FxBonePoseNode(), "frown", 0.0f, 1.0f));
	graph.AddNode(NewNode(new FxBonePoseNode(), "PBM", 0.0f, 1.0f));
	graph.AddNode(NewNode(new FxDeltaNode(), "JoyDelta", 0.0f, 1.0f));

	Check(graph.Link(FxName("open"), FxName("Joy"), FxName("linear"), Params(0.5f, 0.1f, 2)) == LEC_None, "facefx graph: Link open <- Joy (linear m=0.5 b=0.1)");
	Check(graph.Link(FxName("frown"), FxName("Joy"), FxName("quadratic"), Params(-0.3f)) == LEC_None, "facefx graph: Link frown <- Joy (quadratic a=-0.3)");
	Check(graph.Link(FxName("PBM"), FxName("Joy"), FxName("linear"), Params(1.0f, 0.0f, 2)) == LEC_None, "facefx graph: Link PBM <- Joy (linear)");
	Check(graph.Link(FxName("PBM"), FxName("Disgust"), FxName("corrective"), Params(0.5f)) == LEC_None, "facefx graph: Link PBM <- Disgust (corrective 0.5)");
	Check(graph.Link(FxName("JoyDelta"), FxName("Joy"), FxName("linear"), Params(1.0f, 0.0f, 2)) == LEC_None, "facefx graph: Link JoyDelta <- Joy");
	Check(graph.Link(FxName("open"), FxName("Joy"), FxName("linear"), Params(1.0f)) == LEC_Duplicate, "facefx graph: duplicate link refused (LEC_Duplicate)");
	Check(graph.Link(FxName("Joy"), FxName("open"), FxName("linear"), Params(1.0f)) == LEC_Cycle, "facefx graph: cycle refused (LEC_Cycle)");
	Check(graph.Link(FxName("JoyDelta"), FxName("Disgust"), FxName("linear"), Params(1.0f)) == LEC_Refused, "facefx graph: second delta input refused (LEC_Refused)");
	Check(graph.CountNodes(FxBonePoseNode::StaticClass()) == 3 && graph.CountNodes(FxCombinerNode::StaticClass()) == 2
		&& graph.CountNodes(FxFaceGraphNode::StaticClass()) == 6, "facefx graph: jump lists count 3 bone poses, 2 combiners, 6 nodes");
	Check(Near(pOpen->GetBone(0).GetRot().w, 1.0f), "facefx graph: AddBone normalises the rotation (w 2 -> 1)");

	FxCompiledFaceGraph cg;
	cg.Compile(graph);
	const char* ExpectedOrder[] = { "Joy", "open", "frown", "Disgust", "PBM", "JoyDelta" };
	bool bOrder = cg.nodes.Length() == 6;
	for( FxSize i = 0; bOrder && i < 6; ++i )
	{
		bOrder = strcmp(cg.nodes[i].name.GetAsCstr(), ExpectedOrder[i]) == 0;
	}
	char Detail[256];
	sprintf_s(Detail, "%u nodes", cg.nodes.Length());
	Check(bOrder, "facefx compile: inputs before outputs (Joy, open, frown, Disgust, PBM, JoyDelta)", Detail);
	Check(cg.nodes[1].nodeType == NT_BonePose && cg.nodes[0].nodeType == NT_Combiner && cg.nodes[5].nodeType == NT_Delta
		&& cg.CountNodesOfType(NT_BonePose) == 3, "facefx compile: node types from the class names");
	Check(cg.nodes[4].inputLinks.Length() == 2 && cg.nodes[4].inputLinks[1].nodeIndex == 3 && cg.nodes[4].inputLinks[1].linkFnType == LFT_Corrective,
		"facefx compile: PBM's corrective link points at Disgust");

	FxArray<FxRegister> registers;
	for( FxSize i = 0; i < cg.nodes.Length(); ++i )
	{
		registers.PushBack(FxRegister());
	}
	SetReplace(registers[0], 0.8f);
	SetReplace(registers[3], 0.6f);
	FxBool hasBeenTicked = FxFalse;
	cg.Tick(1.0f, registers, hasBeenTicked);
	// Joy = 0.8 (register replace); open = 0.5*0.8 + 0.1; frown = clamp(-0.3*0.8^2) = 0;
	// PBM = (1 - min(1, max(0.6/1, -0.6/1) * 0.5)) * (1*0.8) = 0.7 * 0.8; JoyDelta = 0 on the first tick.
	sprintf_s(Detail, "Joy %g open %g frown %g PBM %g delta %g", cg.nodes[0].finalValue, cg.nodes[1].finalValue, cg.nodes[2].finalValue,
		cg.nodes[4].finalValue, cg.nodes[5].finalValue);
	Check(Near(cg.nodes[0].finalValue, 0.8f) && Near(cg.nodes[1].finalValue, 0.5f) && Near(cg.nodes[2].finalValue, 0.0f)
		&& Near(cg.nodes[4].finalValue, 0.56f) && Near(cg.nodes[5].finalValue, 0.0f), "facefx tick: bone weights 0.5 / 0 / 0.56 from Joy 0.8, Disgust 0.6", Detail);
	Check(Near(registers[1].value, 0.5f) && hasBeenTicked, "facefx tick: the register holds the node value");

	SetReplace(registers[0], 0.3f);
	registers[0].interpStartTime = 2.0f;
	cg.Tick(2.0f, registers, hasBeenTicked);
	sprintf_s(Detail, "Joy %g open %g delta %g", cg.nodes[0].finalValue, cg.nodes[1].finalValue, cg.nodes[5].finalValue);
	Check(Near(cg.nodes[0].finalValue, 0.3f) && Near(cg.nodes[1].finalValue, 0.25f) && Near(cg.nodes[5].finalValue, -0.5f),
		"facefx tick: second tick open 0.25, delta node -0.5", Detail);

	FxCompiledFaceGraph timeGraph;
	FxFaceGraph clockGraph;
	clockGraph.AddNode(NewNode(new FxCurrentTimeNode(), "Clock", 0.0f, 100.0f));
	timeGraph.Compile(clockGraph);
	FxArray<FxRegister> clockRegisters;
	clockRegisters.PushBack(FxRegister());
	FxBool clockTicked = FxFalse;
	timeGraph.Tick(3.5f, clockRegisters, clockTicked);
	Check(timeGraph.nodes.Length() == 1 && Near(timeGraph.nodes[0].finalValue, 3.5f), "facefx tick: current time node outputs the time");

	// Register interpolation: start 0, end 1, inverse duration 2 (0.5 s), first tick at 10 starts the clock.
	// alpha 0.25 at 10.125: ((0*2 - 1*2)*0.25 + 0*-3 + 1*3) * 0.25^2 + 0 = 2.5 * 0.0625 = 0.15625; alpha 0.5 at 10.25: 0.5.
	FxArray<FxRegister> interpRegisters;
	for( FxSize i = 0; i < cg.nodes.Length(); ++i )
	{
		interpRegisters.PushBack(FxRegister());
	}
	FxRegister& r = interpRegisters[0];
	r.firstRegOp = VO_Replace;
	r.interpStartValue = 0.0f;
	r.interpEndValue = 1.0f;
	r.interpInverseDuration = 2.0f;
	FxBool interpTicked = FxFalse;
	cg.Tick(10.0f, interpRegisters, interpTicked);
	FxReal v0 = cg.nodes[0].finalValue;
	cg.Tick(10.125f, interpRegisters, interpTicked);
	FxReal v1 = cg.nodes[0].finalValue;
	cg.Tick(10.25f, interpRegisters, interpTicked);
	FxReal v2 = cg.nodes[0].finalValue;
	sprintf_s(Detail, "%g %g %g", v0, v1, v2);
	Check(Near(v0, 0.0f) && Near(v1, 0.15625f) && Near(v2, 0.5f), "facefx tick: register Hermite ease 0 / 0.15625 / 0.5 at alpha 0 / 0.25 / 0.5", Detail);

	FxFaceGraph decompiled;
	cg.Decompile(decompiled);
	FxCompiledFaceGraph recompiled;
	recompiled.Compile(decompiled);
	bool bSame = recompiled.nodes.Length() == cg.nodes.Length();
	for( FxSize i = 0; bSame && i < cg.nodes.Length(); ++i )
	{
		bSame = recompiled.nodes[i].name == cg.nodes[i].name && recompiled.nodes[i].nodeType == cg.nodes[i].nodeType
			&& recompiled.nodes[i].inputLinks.Length() == cg.nodes[i].inputLinks.Length();
	}
	Check(bSame, "facefx compile: Decompile + Compile gives the same graph");

	Check(graph.RemoveNode(FxName("Disgust")) && graph.GetNumNodes() == 5 && pOpen->GetNumInputLinks() == 1
		&& static_cast<FxBonePoseNode*>(graph.FindNode(FxName("PBM")))->GetNumInputLinks() == 1, "facefx graph: RemoveNode unlinks its outputs");
	FxFaceGraph copy(graph);
	Check(copy.GetNumNodes() == 5 && copy.FindNode(FxName("open")) != graph.FindNode(FxName("open"))
		&& copy.FindNode(FxName("open"))->GetNumInputLinks() == 1
		&& static_cast<FxBonePoseNode*>(copy.FindNode(FxName("open")))->GetNumBones() == 1, "facefx graph: copy clones nodes, links and bones");
}

static void TestCurve()
{
	// k0 (0, 0, out 1), k1 (2, 1, in 0, out 0), k2 (3, 0.5).
	// Hermite at 1: dt 2, t 0.5, p0 0, p1 1, m0 2, m1 0: (((0-2+2+0)*0.5 + 0+3-4-0)*0.5 + 2)*0.5 + 0 = 0.75.
	// Hermite at 2.25: dt 1, t 0.25, p0 1, p1 0.5, m 0: ((2-1)*0.25 - 3 + 1.5)*0.25*0.25 + 1 = 0.921875; linear 0.875.
	FxAnimCurve curve;
	curve.SetName(FxName("open"));
	curve.InsertKey(FxAnimKey(2.0f, 1.0f, 0.0f, 0.0f));
	curve.InsertKey(FxAnimKey(0.0f, 0.0f, 0.0f, 1.0f));
	curve.InsertKey(FxAnimKey(3.0f, 0.5f, 0.0f, 0.0f));
	char Detail[256];
	FxReal a = curve.EvaluateAt(1.0f);
	FxReal b = curve.EvaluateAt(2.25f);
	FxReal c = curve.EvaluateAt(-1.0f);
	FxReal d = curve.EvaluateAt(5.0f);
	FxReal e = curve.EvaluateAt(0.5f);
	sprintf_s(Detail, "%g %g %g %g", a, b, c, d);
	Check(curve.GetNumKeys() == 3 && curve.GetKey(0).GetTime() == 0.0f && Near(a, 0.75f) && Near(b, 0.921875f) && Near(c, 0.0f) && Near(d, 0.5f),
		"facefx curve: Hermite 0.75 at 1, 0.921875 at 2.25, clamped 0 / 0.5 outside the keys", Detail);
	// Hermite at 0.5 (back past the cached key): t 0.25: ((0 - 2 + 2 + 0)*0.25 + 3 - 4)*0.25 + 2)*0.25 = 0.4375.
	sprintf_s(Detail, "%g", e);
	Check(Near(e, 0.4375f), "facefx curve: the cached bounding key resets when the time goes back (0.4375 at 0.5)", Detail);
	FxInterpolationType linear = IT_Linear;
	curve.SetInterpolator(linear);
	b = curve.EvaluateAt(2.25f);
	sprintf_s(Detail, "%g", b);
	Check(Near(b, 0.875f), "facefx curve: linear 0.875 at 2.25", Detail);
	// InsertKey at 1 with autocomputed slope: t 0.5, s0 2, s1 0: ((0 - 6 + 6 + 0)*0.25 + (6 - 0 - 8 - 0)*0.5 + 2) / 2 = 0.5.
	FxSize index = curve.InsertKey(FxAnimKey(1.0f, 0.75f), FxTrue);
	sprintf_s(Detail, "index %u slope %g/%g", index, curve.GetKey(1).GetSlopeIn(), curve.GetKey(1).GetSlopeOut());
	Check(index == 1 && Near(curve.GetKey(1).GetSlopeIn(), 0.5f) && Near(curve.GetKey(1).GetSlopeOut(), 0.5f), "facefx curve: InsertKey autocomputed slope 0.5", Detail);
	Check(curve.FindKey(2.0f) == 2 && curve.FindKey(2.5f) == FxInvalidIndex, "facefx curve: FindKey");

	FxAnim anim;
	anim.SetName(FxName("Line01"));
	anim.AddAnimCurve(curve);
	FxAnimCurve jaw;
	jaw.SetName(FxName("jaw"));
	jaw.InsertKey(FxAnimKey(-0.5f, 0.2f));
	jaw.InsertKey(FxAnimKey(4.0f, 0.1f));
	anim.AddAnimCurve(jaw);
	Check(Near(anim.GetStartTime(), -0.5f) && Near(anim.GetEndTime(), 4.0f), "facefx anim: FindBounds -0.5 .. 4");
	FxAnimSet set;
	set.SetName(FxName("Set"));
	FxAnimGroup group;
	group.SetName(FxName("Group"));
	group.AddAnim(anim);
	set.SetAnimGroup(group);
	FxByte* pMemory = 0;
	FxSize numBytes = 0;
	bool bSaved = FxSaveAnimSetToMemory(set, pMemory, numBytes) != 0;
	FxAnimSet loaded;
	bool bLoaded = bSaved && FxLoadAnimSetFromMemory(loaded, pMemory, numBytes) != 0;
	bool bSame = bLoaded && loaded.GetAnimGroup().GetNumAnims() == 1;
	if( bSame )
	{
		const FxAnim& l = loaded.GetAnimGroup().GetAnim(0);
		bSame = l.GetName() == FxName("Line01") && l.GetNumAnimCurves() == 2 && l.GetAnimCurve(0).GetNumKeys() == 4
			&& Near(l.GetAnimCurve(0).EvaluateAt(2.25f), 0.875f) && Near(l.GetAnimCurve(0).GetKey(1).GetSlopeOut(), 0.5f)
			&& Near(l.GetEndTime(), 4.0f) && l.GetAnimCurve(1).GetNumKeys() == 2;
	}
	sprintf_s(Detail, "%u bytes", numBytes);
	Check(bSame, "facefx anim set: save to memory and load back (packed keys, version 8)", Detail);
	if( pMemory )
	{
		FxFree(pMemory, numBytes);
	}
}

struct FBoneListEntry
{
	FxBone refBone;
	FxQuat refBoneInverseRot;
	FxInt32 index;
	FxReal refWeight;
	FxSize numLinks;
};

static void ReadBoneList( FxArchive& arc, FxSize& NumBones, FxSize& NumLinks )
{
	arc.SerializeClassVersion("FxMasterBoneList", FxTrue, 2);
	FxSize length = 0;
	arc << length;
	NumBones = length;
	NumLinks = 0;
	for( FxSize i = 0; i < length; ++i )
	{
		FBoneListEntry entry;
		arc << entry.refBone << entry.refBoneInverseRot << entry.index << entry.refWeight << entry.numLinks;
		NumLinks += entry.numLinks;
		for( FxSize j = 0; j < entry.numLinks; ++j )
		{
			FxSize nodeIndex = 0;
			FxBone optimizedBone;
			arc << nodeIndex << optimizedBone;
		}
	}
}

static std::vector<std::map<std::string, std::string> > ReadCsv( const std::string& Path )
{
	std::vector<std::map<std::string, std::string> > Rows;
	std::ifstream In(Path.c_str());
	std::string Line;
	std::vector<std::string> Header;
	while( std::getline(In, Line) )
	{
		if( !Line.empty() && Line[Line.size() - 1] == '\r' )
		{
			Line.erase(Line.size() - 1);
		}
		std::vector<std::string> Cells;
		std::stringstream Stream(Line);
		std::string Cell;
		while( std::getline(Stream, Cell, ',') )
		{
			Cells.push_back(Cell);
		}
		if( Header.empty() )
		{
			Header = Cells;
			continue;
		}
		std::map<std::string, std::string> Row;
		for( size_t i = 0; i < Header.size() && i < Cells.size(); ++i )
		{
			Row[Header[i]] = Cells[i];
		}
		Rows.push_back(Row);
	}
	return Rows;
}

static bool ReadFile( const std::string& Path, std::vector<FxByte>& Bytes )
{
	std::ifstream In(Path.c_str(), std::ios::binary);
	if( !In )
	{
		return false;
	}
	Bytes.assign(std::istreambuf_iterator<char>(In), std::istreambuf_iterator<char>());
	return !Bytes.empty();
}

static size_t Num( std::map<std::string, std::string>& Row, const char* Key )
{
	return static_cast<size_t>(atol(Row[Key].c_str()));
}

static void TestHeadArchives( const std::string& Dir, FaceFXSkipFunc Skip )
{
	std::vector<std::map<std::string, std::string> > Rows = ReadCsv(Dir + "/fxa_counts.csv");
	if( Rows.empty() )
	{
		Skip("facefx archives: head graphs", (Dir + "/fxa_counts.csv missing (facefx_graph_dump.py --csv)").c_str());
		return;
	}
	const char* TypeColumns[NT_NumNodeTypes] = { "nt_Combiner", "nt_Delta", "nt_CurrentTime", "nt_GenericTarget", "nt_BonePose",
		"nt_MorphTarget", "nt_MaterialParameterUE3", "nt_MorphTargetUE3" };
	int NumOk = 0;
	size_t TotalNodes = 0;
	std::string FirstFailure;
	for( size_t r = 0; r < Rows.size(); ++r )
	{
		std::map<std::string, std::string>& Row = Rows[r];
		std::vector<FxByte> Bytes;
		if( !ReadFile(Dir + "/fxa/" + Row["file"], Bytes) )
		{
			FirstFailure = FirstFailure.empty() ? Row["file"] + ": unreadable" : FirstFailure;
			continue;
		}
		FxArchiveStoreMemoryNoCopy* pStore = FxArchiveStoreMemoryNoCopy::Create(&Bytes[0], static_cast<FxSize>(Bytes.size()));
		FxArchive arc(pStore, FxArchive::AM_LinearLoad);
		arc.Open();
		FxNamedObject actorName;
		actorName.Serialize(arc);
		arc.SerializeClassVersion("FxActor");
		FxSize NumBones = 0;
		FxSize NumBoneLinks = 0;
		ReadBoneList(arc, NumBones, NumBoneLinks);
		FxCompiledFaceGraph cg;
		arc << cg;
		FxAnimGroup defaultGroup;
		FxArray<FxAnimGroup> groups;
		arc << defaultGroup << groups;
		arc.SerializeClassVersion("FxPhonemeMap");
		FxSize numMappings = 0;
		arc << numMappings;
		for( FxSize i = 0; i < numMappings; ++i )
		{
			FxInt32 phoneme = 0;
			FxName target;
			FxReal amount = 0.0f;
			arc << phoneme << target << amount;
		}
		FxArray<FxName> targetNames;
		arc << targetNames;

		FxSize NumLinks = 0;
		for( FxSize i = 0; i < cg.nodes.Length(); ++i )
		{
			NumLinks += cg.nodes[i].inputLinks.Length();
		}
		bool bOk = !StoreOverran() && static_cast<size_t>(pStore->Tell()) == Bytes.size()
			&& cg.nodes.Length() == Num(Row, "nodes") && NumLinks == Num(Row, "links")
			&& NumBones == Num(Row, "bones") && NumBoneLinks == Num(Row, "bone_links")
			&& 1 + groups.Length() == Num(Row, "groups") && numMappings == Num(Row, "phoneme_mappings")
			&& actorName.GetName() == FxName(Row["actor"].c_str());
		for( int t = 0; bOk && t < NT_NumNodeTypes; ++t )
		{
			bOk = cg.CountNodesOfType(static_cast<FxCompiledFaceGraphNodeType>(t)) == Num(Row, TypeColumns[t]);
		}
		for( FxSize i = 0; bOk && i < cg.nodes.Length(); ++i )
		{
			bOk = cg.FindNodeIndex(cg.nodes[i].name) == i;
			for( FxSize j = 0; bOk && j < cg.nodes[i].inputLinks.Length(); ++j )
			{
				bOk = cg.nodes[i].inputLinks[j].nodeIndex < i;
			}
		}
		if( bOk )
		{
			FxFaceGraph graph;
			cg.Decompile(graph);
			FxCompiledFaceGraph recompiled;
			recompiled.Compile(graph);
			bOk = graph.GetNumNodes() == cg.nodes.Length() && recompiled.nodes.Length() == cg.nodes.Length();
			FxSize RecompiledLinks = 0;
			for( FxSize i = 0; bOk && i < recompiled.nodes.Length(); ++i )
			{
				RecompiledLinks += recompiled.nodes[i].inputLinks.Length();
				bOk = cg.FindNodeIndex(recompiled.nodes[i].name) != FxInvalidIndex && recompiled.nodes[i].nodeType == cg.nodes[cg.FindNodeIndex(recompiled.nodes[i].name)].nodeType;
			}
			bOk = bOk && RecompiledLinks == NumLinks;
			if( bOk )
			{
				FxArray<FxRegister> registers;
				for( FxSize i = 0; i < cg.nodes.Length(); ++i )
				{
					registers.PushBack(FxRegister());
				}
				FxBool hasBeenTicked = FxFalse;
				cg.Tick(0.0f, registers, hasBeenTicked);
				for( FxSize i = 0; bOk && i < cg.nodes.Length(); ++i )
				{
					bOk = cg.nodes[i].finalValue >= cg.nodes[i].nodeMin && cg.nodes[i].finalValue <= cg.nodes[i].nodeMax;
				}
			}
		}
		if( bOk )
		{
			++NumOk;
			TotalNodes += cg.nodes.Length();
		}
		else if( FirstFailure.empty() )
		{
			char Detail[512];
			sprintf_s(Detail, "%s: nodes %u/%u links %u/%u bones %u/%u read %u/%u", Row["file"].c_str(), cg.nodes.Length(), (unsigned)Num(Row, "nodes"),
				NumLinks, (unsigned)Num(Row, "links"), NumBones, (unsigned)Num(Row, "bones"), pStore->Tell(), (unsigned)Bytes.size());
			FirstFailure = Detail;
		}
	}
	char Detail[512];
	sprintf_s(Detail, "%d/%u archives, %u nodes%s%s", NumOk, (unsigned)Rows.size(), (unsigned)TotalNodes, FirstFailure.empty() ? "" : "; first failure ", FirstFailure.c_str());
	Check(NumOk == (int)Rows.size(), "facefx archives: every head's compiled graph loads with the dumper's counts, recompiles and ticks", Detail);
}

static void TestAnimSetArchives( const std::string& Dir, FaceFXSkipFunc Skip )
{
	std::vector<std::map<std::string, std::string> > Rows = ReadCsv(Dir + "/fxs_counts.csv");
	if( Rows.empty() )
	{
		Skip("facefx archives: anim sets", (Dir + "/fxs_counts.csv missing (facefx_graph_dump.py --csv)").c_str());
		return;
	}
	int NumOk = 0;
	size_t TotalKeys = 0;
	std::string FirstFailure;
	for( size_t r = 0; r < Rows.size(); ++r )
	{
		std::map<std::string, std::string>& Row = Rows[r];
		std::vector<FxByte> Bytes;
		bool bOk = ReadFile(Dir + "/fxs/" + Row["file"], Bytes);
		FxAnimSet set;
		bOk = bOk && FxLoadAnimSetFromMemory(set, &Bytes[0], static_cast<FxSize>(Bytes.size()));
		size_t NumCurves = 0;
		size_t NumKeys = 0;
		const FxAnimGroup& group = set.GetAnimGroup();
		for( FxSize a = 0; a < group.GetNumAnims(); ++a )
		{
			const FxAnim& anim = group.GetAnim(a);
			NumCurves += anim.GetNumAnimCurves();
			for( FxSize c = 0; c < anim.GetNumAnimCurves(); ++c )
			{
				NumKeys += anim.GetAnimCurve(c).GetNumKeys();
				const FxAnimCurve& curve = anim.GetAnimCurve(c);
				if( curve.GetNumKeys() )
				{
					bOk = bOk && curve.EvaluateAt(curve.GetKey(0).GetTime() - 1.0f) == curve.GetKey(0).GetValue();
				}
			}
		}
		bOk = bOk && !StoreOverran() && group.GetNumAnims() == Num(Row, "anims") && NumCurves == Num(Row, "curves")
			&& NumKeys == Num(Row, "keys") && set.GetName() == FxName(Row["animset"].c_str()) && group.GetName() == FxName(Row["group"].c_str());
		FxByte* pResaved = 0;
		FxSize numResaved = 0;
		bOk = bOk && FxSaveAnimSetToMemory(set, pResaved, numResaved) && numResaved == Bytes.size() && memcmp(pResaved, &Bytes[0], numResaved) == 0;
		if( pResaved )
		{
			FxFree(pResaved, numResaved);
		}
		if( bOk )
		{
			++NumOk;
			TotalKeys += NumKeys;
		}
		else if( FirstFailure.empty() )
		{
			FirstFailure = Row["file"];
		}
	}
	char Detail[512];
	sprintf_s(Detail, "%d/%u anim sets, %u keys%s%s", NumOk, (unsigned)Rows.size(), (unsigned)TotalKeys, FirstFailure.empty() ? "" : "; first failure ", FirstFailure.c_str());
	Check(NumOk == (int)Rows.size(), "facefx archives: every anim set loads with the dumper's anim/curve/key counts and re-saves byte-identical", Detail);
}

void TestFaceFXGraph( FaceFXReportFunc InReport, FaceFXSkipFunc InSkip )
{
	GReport = InReport;
	RegisterGraphClasses();
	Check(FxLinkFn::GetNumLinkFunctions() == 12 && FxLinkFn::FindLinkFunctionType(FxName("linear")) == LFT_Linear
		&& FxLinkFn::FindLinkFunctionType(FxName("clamped linear")) == LFT_ClampedLinear
		&& strcmp(FxLinkFn::FindLinkFunctionByType(LFT_Null)->GetNameAsCstr(), "custom") == 0
		&& FxLinkFn::FindLinkFunction(FxName("linear"))->GetNumParams() == 2, "facefx link functions: 12 registered as retail's Startup");
	FxLinkFnParameters clamp;
	clamp.parameters.PushBack(2.0f);
	clamp.parameters.PushBack(0.5f);
	clamp.parameters.PushBack(0.25f);
	clamp.parameters.PushBack(1.0f);
	Check(Near(FxLinkFnEval(LFT_ClampedLinear, 0.25f, clamp), 0.25f) && Near(FxLinkFnEval(LFT_ClampedLinear, 1.0f, clamp), 1.25f)
		&& Near(FxLinkFnEval(LFT_Sqrt, -4.0f, Params(2.0f)), -4.0f) && Near(FxLinkFnEval(LFT_OneClamp, 4.0f, FxLinkFnParameters()), 0.25f)
		&& Near(FxLinkFnEval(LFT_Inverse, 0.0f, FxLinkFnParameters()), 0.0f) && Near(FxLinkFnEval(LFT_Constant, 9.0f, FxLinkFnParameters()), 1.0f),
		"facefx link functions: FxLinkFnEval clamped linear / sqrt / oneclamp / inverse / constant");
	TestCodeBuiltGraph();
	TestCurve();
	const char* DirEnv = getenv("FACEFX_SMOKE_DIR");
	std::string Dir = DirEnv ? DirEnv : "../agentFX2";
	TestHeadArchives(Dir, InSkip);
	TestAnimSetArchives(Dir, InSkip);
	FxLinkFn::Shutdown();
}
