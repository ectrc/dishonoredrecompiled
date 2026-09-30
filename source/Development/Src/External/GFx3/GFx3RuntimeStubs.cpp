// Scaleform GFx 3.3.89 - GENERATED bringup bodies, do not edit by hand.
// build/agentBB_gen_gfx3.py.
//
// DISHONORED(bringup): every virtual below is NON-PURE in the 2012 PDB, which means libgfx carries a
// real body for it and the reconstruction does not. They are defined here so the renderer seam and
// the tests link; each one is a line item for the runtime port (package BC). A pure virtual needs no
// entry, which is why this file is much shorter than the interface list: the runtime's own
// implementation classes provide those.
//
// Deleting an entry from this file as its real body lands is the intended workflow - the link will
// tell you if you removed one that is still needed.
#include "GFx3.h"

bool GFxFontLib::FindFont(GFxFontLib::FontResult* a0, const char* a1, unsigned int a2, GFxMovieDef* a3, GFxStateBag* a4, GFxResourceWeakLib* a5) { return false; }   // vt[1]
GFxState::StateType GFxFontPackParams::GetStateType() const { return GFxState::StateType(); }   // vt[1]
const char* GFxResourceKey::KeyInterface::GetFileURL(void* a0) const { return 0; }   // vt[6]
GFxResourceKey GFxResource::GetKey() { return GFxResourceKey(); }   // vt[1]
unsigned int GFxResource::GetResourceTypeCode() const { return 0; }   // vt[2]
GFxResourceReport* GFxResource::GetResourceReport() { return 0; }   // vt[3]
GImageInfoBase* GFxImageCreator::CreateImage(const GFxImageCreateInfo& a0) { return 0; }   // vt[1]
GFxState::StateType GFxImagePackParamsBase::GetStateType() const { return GFxState::StateType(); }   // vt[1]
void GTexture::ChangeHandler::OnChange(GRenderer* a0, GTexture::ChangeHandler::EventType a1) { }   // vt[1]
bool GTexture::ChangeHandler::Recreate(GRenderer* a0) { return false; }   // vt[2]
GImageInfoBase* GImageInfoBase::CreateSubImage(const GRect<int>& a0, GMemoryHeap* a1) { return 0; }   // vt[4]
GRect<int> GImageInfoBase::GetRect() const { return GRect<int>(); }   // vt[5]
unsigned int GImageInfoBase::GetImageInfoType() const { return 0; }   // vt[6]
GFxResourceId GFxImageResource::GetBaseImageId() { return GFxResourceId(); }   // vt[4]
GFxResource::ResourceUse GFxImageResource::GetImageUse() const { return GFxResource::ResourceUse(); }   // vt[5]
GFxStateBag* GFxStateBag::GetStateBagImpl() const { return 0; }   // vt[0]
void GFxStateBag::SetState(GFxState::StateType a0, GFxState* a1) { }   // vt[2]
GFxState* GFxStateBag::GetStateAddRef(GFxState::StateType a0) const { return 0; }   // vt[3]
void GFxStateBag::GetStatesAddRef(GFxState** a0, const GFxState::StateType* a1, unsigned int a2) const { }   // vt[4]
bool GFxLoader::CheckTagLoader(int a0) const { return false; }   // vt[5]
void GFxLog::LogMessageVarg(GFxLogConstants::LogMessageType a0, const char* a1, char* a2) { }   // vt[1]
void GFxProgressHandler::LoadTagUpdate(const GFxProgressHandler::TagInfo& a0, bool a1) { }   // vt[2]
void GRendererEventHandler::OnEvent(GRenderer* a0, GRendererEventHandler::EventType a1) { }   // vt[1]
void GRenderer::ScopedEventCallback(const char* a0) { }   // vt[1]
void GRenderer::SaveCurrentRenderTargetContents() { }   // vt[2]
void GRenderer::RestoreCurrentRenderTargetContents() { }   // vt[3]
void GRenderer::BeginFrame() { }   // vt[7]
void GRenderer::EndFrame() { }   // vt[8]
void GRenderer::ReleaseTempRenderTargets(unsigned int a0) { }   // vt[14]
bool GRenderer::PushUserData(GRenderer::UserData* a0) { return false; }   // vt[22]
void GRenderer::PopUserData() { }   // vt[23]
// vt[27] GRenderer::MakeViewAndPersp3D: the real body is in GFx3Support.cpp (agent FA).
void GRenderer::SetStereoParams(GRenderer::StereoParams a0) { }   // vt[28]
void GRenderer::SetStereoDisplay(GRenderer::StereoDisplay a0, bool a1) { }   // vt[29]
void GRenderer::DrawDistanceFieldBitmaps(GRenderer::BitmapDesc* a0, int a1, int a2, int a3, const GTexture* a4, const GMatrix2D& a5, const GRenderer::DistanceFieldParams& a6, GRenderer::CacheProvider* a7) { }   // vt[42]
bool GRenderer::AddEventHandler(GRendererEventHandler* a0) { return false; }   // vt[52]
void GRenderer::RemoveEventHandler(GRendererEventHandler* a0) { }   // vt[53]
GString GFxResourceReport::GetResourceName() const { return GString(); }   // vt[1]
GMemoryHeap* GFxResourceReport::GetResourceHeap() const { return 0; }   // vt[2]
void GFxResourceReport::GetStats(GStatBag* a0, bool a1) { }   // vt[3]
void GFxTextClipboard::OnTextStore(const wchar_t* a0, unsigned int a1) { }   // vt[1]
unsigned int GFxTranslator::GetCaps() const { return 0; }   // vt[1]
void GFxTranslator::Translate(GFxTranslator::TranslateInfo* a0) { }   // vt[2]
bool GFxTranslator::OnWordWrapping(GFxTranslator::LineFormatDesc* a0) { return false; }   // vt[3]
void GFxURLBuilder::BuildURL(GString* a0, const GFxURLBuilder::LocationInfo& a1) { }   // vt[1]
GImageInfoBase* GSubImageInfo::GetBaseImage() { return 0; }   // vt[7]
bool GSysAllocBase::initHeapEngine(const void* a0) { return false; }   // vt[1]
void GSysAllocBase::shutdownHeapEngine() { }   // vt[2]
bool GSysAllocPaged::ReallocInPlace(void* a0, unsigned int a1, unsigned int a2, unsigned int a3) { return false; }   // vt[6]
void* GSysAllocPaged::AllocSysDirect(unsigned int a0, unsigned int a1, unsigned int* a2, unsigned int* a3) { return 0; }   // vt[7]
bool GSysAllocPaged::FreeSysDirect(void* a0, unsigned int a1, unsigned int a2) { return false; }   // vt[8]
unsigned int GSysAllocPaged::GetBase() const { return 0; }   // vt[9]
unsigned int GSysAllocPaged::GetSize() const { return 0; }   // vt[10]
unsigned int GSysAllocPaged::GetFootprint() const { return 0; }   // vt[11]
unsigned int GSysAllocPaged::GetUsedSpace() const { return 0; }   // vt[12]
void GSysAllocPaged::VisitMem(GHeapMemVisitor* a0) const { }   // vt[13]
void GSysAllocPaged::VisitSegments(GHeapSegVisitor* a0, unsigned int a1, unsigned int a2) const { }   // vt[14]

// 57 bringup bodies: the non-pure virtuals libgfx implements and we do not yet.
