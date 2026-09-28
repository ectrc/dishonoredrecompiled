// ADisTallboyNPCPawn cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): the tallboy is the one ADishonoredNPCPawn subclass with an appearance pass of its own, and it is
// two passes, not one:
//   ApplyTweakChanges_Derived (2013 rva 0x77daa0, 2012 0x7f2440) puts UDisTweaks_TallboyNPCPawn::m_pStiltsSkeletalMesh
//     on m_pStiltsMesh and binds it to the body - THE STILTS, which agent DI's hand-over attributed to
//     PostBeginPlay_Body;
//   PostBeginPlay_Body (0x781270, 2012 0x7f24d0) spawns the searchlight actor, sizes its light component from the
//     tweaks, seeds the spotlight manager from it and starts the light's particle system and ambient sound.
public:
	virtual void ApplyTweakChanges_Derived();
	virtual void PostBeginPlay_Body();
	/** DISHONORED(port): 2013 rva 0x76eff0 (2012 0x7c99b0). */
	void CreateLightParticleSystem( class UParticleSystem* ParticleSystem );
