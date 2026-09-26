// Engine/src/akevent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (13):
//   0xac630  public: static void __cdecl UAkBaseSoundObject::InitializePrivateStaticClassUAkBaseSoundObject(void)
//   0xadeb0  public: virtual unsigned int __thiscall UAkEvent::IsAudible(class FVector const &, class FVector const &, class AActor *, int &, unsigned int)
//   0xb18f0  public: static class UClass * __cdecl UAkBaseSoundObject::GetPrivateStaticClassUAkBaseSoundObject(wchar_t const *)
//   0xb1980  public: static class UClass * __cdecl UAkEvent::GetPrivateStaticClassUAkEvent(wchar_t const *)
//   0xc3980  public: static class UClass * __cdecl UAkBaseSoundObject::StaticClassNoInline(void)
//   0xc39b0  public: static void __cdecl UAkEvent::InitializePrivateStaticClassUAkEvent(void)
//   0xc39d0  public: void __thiscall UAkEvent::ComputeAkID(void)
//   0xc3a70  private: void __thiscall UAkEvent::ComputeMaxRadius(void)
//   0xc7210  public: static class UClass * __cdecl UAkEvent::StaticClassNoInline(void)
//   0xc7240  public: virtual void __thiscall UAkEvent::PostRename(void)
//   0xc7250  public: void __thiscall UAkEvent::FixRequiredBank(void)
//   0xc7410  public: float __thiscall UAkEvent::GetMaxRadius(void)
//   0xceb00  public: virtual void __thiscall UAkEvent::PostLoad(void)

#include "EnginePrivate.h"

IMPLEMENT_CLASS(UAkBaseSoundObject);
IMPLEMENT_CLASS(UAkEvent);

// DISHONORED(port): 2013 rva 0xcdb10 (2012 0xceb00, akevent.cpp:17)
void UAkEvent::PostLoad()
{
	Super::PostLoad();
	ComputeAkID();
	FixRequiredBank();
}

// DISHONORED(port): a thunk to ComputeAkID in both exes (2012 rva 0xc7240, 5 bytes; the 2013 copy folds into ComputeAkID 0xc7840)
void UAkEvent::PostRename()
{
	ComputeAkID();
}

// DISHONORED(bringup): 2013 rva 0xc7840 (2012 0xc39d0) is m_akID = AK::SoundEngine::GetIDFromString(*GetName()), the Wwise name hash.
// Wwise 2012.1 is a user blocker (middleware.md) and the AkAudio module is a stub, so the id stays 0 and no event ever resolves.
void UAkEvent::ComputeAkID()
{
	m_akID = 0;
}

// DISHONORED(bringup): 2013 rva 0xc7aa0 (2012 0xc3a70) queries the Wwise object hierarchy for the largest attenuation radius of the
// event and stores -1 when it has none. Without Wwise every event keeps -1.
void UAkEvent::ComputeMaxRadius()
{
	m_fMaxRadius = -1.0f;
}

// DISHONORED(port): 2013 rva 0xcdb40 (2012 0xc7410): computed on first use, cached in m_fMaxRadius
FLOAT UAkEvent::GetMaxRadius()
{
	if( m_fMaxRadius < -1.0f )
	{
		ComputeMaxRadius();
	}
	return m_fMaxRadius;
}

// DISHONORED(port): 2013 rva 0xc78e0 (2012 0xc7250, akevent.cpp:36): an event without a bank gets the package's default bank
// WwiseDefaultBank_<Package>, found in the same package or created there and marked RF_ForceTagExp. UAkBank::Load() is the Wwise part
// and returns FALSE without a device.
void UAkEvent::FixRequiredBank()
{
	if( RequiredBank )
	{
		return;
	}
	UPackage* Outermost = GetOutermost();
	const FString DefaultBankName = FString( TEXT("WwiseDefaultBank_") ) + Outermost->GetName();
	UAkBank* Bank = FindObject<UAkBank>( Outermost, *DefaultBankName );
	if( !Bank )
	{
		Bank = ConstructObject<UAkBank>( UAkBank::StaticClass(), Outermost, FName( *DefaultBankName ) );
		if( Bank && !GIsCooking )
		{
			Bank->Load();
		}
	}
	if( Bank )
	{
		Bank->SetFlags( RF_ForceTagExp );
		RequiredBank = Bank;
	}
}

// DISHONORED(port): 2013 rva 0xb0bd0 (2012 0xadeb0, identical bytes): a line check from the listener to the source sets bIsOccluded;
// the event itself is always considered audible
UBOOL UAkEvent::IsAudible( const FVector& SourceLocation, const FVector& ListenerLocation, AActor* SourceActor, UBOOL& bIsOccluded, UBOOL bCheckOcclusion )
{
	if( bCheckOcclusion )
	{
		FCheckResult Hit(1.0f);
		GWorld->SingleLineCheck( Hit, SourceActor, SourceLocation, ListenerLocation, TRACE_World | TRACE_StopAtAnyHit, FVector(0,0,0) );
		bIsOccluded = Hit.Time < 1.0f;
	}
	return TRUE;
}
