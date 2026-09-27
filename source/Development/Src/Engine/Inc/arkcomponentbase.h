#pragma once
// Engine/inc/arkcomponentbase.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (23):
//   0x527010  public: virtual __thiscall FArkComponentBase::~FArkComponentBase(void)
//   0x527020  public: virtual class FName __thiscall IDisRatTargetInterface::GetRandomJointNameForEating(void)const
//   0x631990  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FArkComponentLocomotion>(void)
//   0x631a00  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FArkComponentLookat>(void)
//   0x631a70  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FArkComponentAvoidance>(void)
//   0x631ae0  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FArkComponentFaceTo>(void)
//   0x631b50  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FArkComponentMeshOffset>(void)
//   0x631bc0  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisAIKnowledgeComponent>(void)
//   0x631c30  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisAIMonitorPawnReachability>(void)
//   0x631ca0  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisAIMonitorReaction>(void)
//   0x631d10  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentAnimPlayer>(void)
//   0x631d80  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentHitReactionThrow>(void)
//   0x631df0  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentObservable>(void)
//   0x631e60  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentPlague>(void)
//   0x631ed0  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentRetargeting>(void)
//   0x631f40  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentTuneCombat>(void)
//   0x631fb0  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentTuneProtection>(void)
//   0x632020  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentVisionNPC>(void)
//   0x632090  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentVisionWatchTower>(void)
//   0x632100  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentVisionRiverKrust>(void)
//   0x632170  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisMonitorNPCAttention>(void)
//   0x6321e0  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentRBDamages>(void)
//   0x632250  private: static class FArkComponentBase * __cdecl FArkComponentCreatorRegister::CreatorFn<class FDisComponentLODManager>(void)

/*-----------------------------------------------------------------------------
	DISHONORED(port): Arkane's non-UObject component system, the base every Ark/Dis "component" derives from.
	A component is heap allocated, owned by a UArkComponentContainer (Engine/Src/arkcomponentcontainer.cpp), found by
	an integer type id and started/stopped in bulk with the container. The ids and the FName each type reports are
	transcribed from the IsOfType / GetType / GetInternalName bodies of all 21 retail component types (2012 rvas
	0x5336e0..0x8c21b0, 2013 0x4f96f0..0x874db0); the creator table is what UArkComponentContainer::AddNewComponentByID
	looks a saved component up in.
-----------------------------------------------------------------------------*/

class AActor;

/**
 * Component type ids. The value is the constant each retail component's IsOfType compares against, so the numbering is
 * retail's and the gaps are retail's: 0 is the anim player (which retail declares with the id the enumeration starts
 * at), 101..105 are the Engine-side Ark components, 200..214 the DishonoredGame ones.
 */
enum EArkComponentType
{
	ArkCpntType_AnimPlayer					= 0,
	ArkCpntType_Locomotion					= 101,
	ArkCpntType_FaceTo						= 102,
	ArkCpntType_Lookat						= 103,
	ArkCpntType_MeshOffset					= 104,
	ArkCpntType_Avoidance					= 105,
	DisCpntType_HitReactionThrow			= 200,
	DisCpntType_LODManager					= 201,
	DisCpntType_RBDamages					= 202,
	DisCpntType_Observable					= 203,
	DisCpntType_Retargeting					= 204,
	DisCpntType_VisionNPC					= 205,
	DisCpntType_VisionWatchtower			= 206,
	DisCpntType_VisionRiverKrust			= 207,
	DisCpntType_AIMonitorPawnReachability	= 208,
	DisCpntType_Plague						= 209,
	DisCpntType_AIKnowledge					= 210,
	DisCpntType_AIMonitorReaction			= 211,
	DisCpntType_TuneProtection				= 212,
	DisCpntType_TuneCombat					= 213,
	DisCpntType_MonitorNPCReaction			= 214,
};

// DISHONORED(port): 2013 rva 0x4e7620 (2012 0x527010) is the base destructor; the rest of the vtable is the ten slots of
// FArkComponentBase_vtbl (2012 PDB, 40 bytes) in this order: ~, IsOfType, GetType, GetName, ManageReferences, Serialize,
// GetMemoryFootprint, Starting, Stopping, GetAllocatedSize. The slot order is load-bearing: the container dispatches
// Starting through +28, Stopping through +32, ManageReferences through +16, Serialize through +20, GetMemoryFootprint
// through +24 and GetType through +8 by offset.
class FArkComponentBase
{
public:
	/**
	 * What a component is handed when the container is collecting references: exactly one of the two is set, the object
	 * array when UArkComponentContainer::AddReferencedObjects is walking for the collector, the archive when
	 * UArkComponentContainer::Serialize is.
	 */
	class FGCHelper
	{
	public:
		FGCHelper( TArray<UObject*>* InObjectArray, FArchive* InArchive )
			: m_pObjectArray( InObjectArray )
			, m_pArchive( InArchive )
		{}

		TArray<UObject*>*	m_pObjectArray;
		FArchive*			m_pArchive;
	};

	FArkComponentBase()
		: m_pOwner( NULL )
		, m_bStarted( FALSE )
		, m_bPendingStop( FALSE )
	{}

	virtual ~FArkComponentBase() {}
	virtual UBOOL IsOfType( INT _Type ) const { return FALSE; }
	virtual INT GetType() const { return ArkCpntType_AnimPlayer; }
	virtual FName GetName() const { return NAME_None; }
	virtual void ManageReferences( FGCHelper* _pHelper ) {}
	virtual void Serialize( FArchive& _rArchive ) {}
	virtual DWORD GetMemoryFootprint() const { return sizeof( FArkComponentBase ); }
	virtual void Starting() {}
	virtual void Stopping() {}
	virtual DWORD GetAllocatedSize() const { return 0; }

	AActor*	m_pOwner;
	UBOOL	m_bStarted;
	UBOOL	m_bPendingStop;
};

/**
 * DISHONORED(port): the per-type declaration every retail component carries. It emits the three type virtuals and the
 * function-local static FName that GetName returns (retail's GetInternalName, e.g. 2012 rva 0x57ee90 for the locomotion
 * component, which builds L"ArkCpntType_Locomotion" once with FNAME_Add and copies it out), plus the compile-time id the
 * container's GetFirstComponent<T> passes to IsOfType.
 */
#define ARKCOMPONENT_DECLARE_TYPE( TypeId, InternalName )								\
public:																					\
	enum { ARK_COMPONENT_TYPE = TypeId };												\
	virtual UBOOL IsOfType( INT _Type ) const { return _Type == TypeId; }				\
	virtual INT GetType() const { return TypeId; }										\
	virtual FName GetName() const { return GetInternalName(); }							\
	static FName GetInternalName()														\
	{																					\
		static FName s_Name( TEXT( InternalName ) );									\
		return s_Name;																	\
	}

/**
 * DISHONORED(port): one static instance per component type adds itself to the creator table at start-up; the table is
 * what AddNewComponentByID (2013 rva 0x534100, 2012 0x5759a0) searches by id to rebuild a component a save file names.
 * CreatorFn<T> is the 23 one-line instantiations the skeleton banner above lists (2012 0x631990..0x632250,
 * 2013 0x5eb640..0x5ebf00): an appMalloc of sizeof(T) followed by T's constructor.
 * DISHONORED(written): retail keeps the table in a plain global (g_ComponentCreators). Ours is a function-local static so
 * that a component type registering from another translation unit's static constructor cannot run before the array is
 * constructed - retail gets away with the global only because of link order.
 */
class FArkComponentCreatorRegister
{
public:
	typedef FArkComponentBase* (*CreatorFnType)();

	FArkComponentCreatorRegister( INT _ID, CreatorFnType _pCreatorFn )
		: m_ID( _ID )
		, m_pCreatorFn( _pCreatorFn )
	{
		GetRegistry().AddItem( this );
	}

	template< class ComponentType > static FArkComponentBase* CreatorFn()
	{
		return new ComponentType;
	}

	static TArray<FArkComponentCreatorRegister*>& GetRegistry()
	{
		static TArray<FArkComponentCreatorRegister*> s_ComponentCreators;
		return s_ComponentCreators;
	}

	const INT			m_ID;
	const CreatorFnType	m_pCreatorFn;
};

/** Pairs with ARKCOMPONENT_DECLARE_TYPE: put one in the component's .cpp so a saved component can be recreated by id. */
#define ARKCOMPONENT_IMPLEMENT_TYPE( ComponentClass )									\
	static FArkComponentCreatorRegister ComponentClass##_CreatorRegister(				\
		ComponentClass::ARK_COMPONENT_TYPE,												\
		&FArkComponentCreatorRegister::CreatorFn< ComponentClass > );
