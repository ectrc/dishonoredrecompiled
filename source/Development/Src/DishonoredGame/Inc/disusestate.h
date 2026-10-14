// DishonoredGame/Inc/disusestate.h
// DISHONORED(port): agent GD (PHASE19 GD). The state parameters of the player's use-interaction FSM (ADishonoredPlayerController::
// m_pUseInteractionFSM): one per state, each naming its state class, and FDisUseState_AltInteract_Param carrying the
// interactable the alt-interact was asked for. Retail: FDisUseState_WaitForInput_Param 2012 0x731c20, FDisUseState_Finished_Param
// 0x731bc0, FDisUseState_Holster_Param 0x731bf0, FDisUseState_AltInteract_Param 0x734e00 (2013 0x6d5790) and its OnPending
// 0x72daa0 - which writes the managed object (the controller) into UDisUseState_Base::m_pPlayerController and the
// interactable into UDisUseState_AltInteract::m_pInteractable. The three parameterless OnPendings are ICF-folded in retail
// onto the same body minus the interactable, which is what the common base below is.
#ifndef _INC_DISUSESTATE
#define _INC_DISUSESTATE

struct FDisUseState_Param : public FDisNativeStateParam
{
	FDisUseState_Param( UClass* _pStateClass ) : FDisNativeStateParam( _pStateClass ) {}

	virtual void OnPending( UDishonoredNativeState* _pPendingState, UObject* _pManagedObject )
	{
		if( UDisUseState_Base* State = Cast<UDisUseState_Base>( _pPendingState ) )
		{
			State->m_pPlayerController = Cast<ADishonoredPlayerController>( _pManagedObject );
		}
	}
	virtual INT SizeOf() const { return sizeof( FDisUseState_Param ); }
};

struct FDisUseState_WaitForInput_Param : public FDisUseState_Param
{
	FDisUseState_WaitForInput_Param() : FDisUseState_Param( UDisUseState_WaitForInput::StaticClass() ) {}
	virtual INT SizeOf() const { return sizeof( FDisUseState_WaitForInput_Param ); }
};

struct FDisUseState_Finished_Param : public FDisUseState_Param
{
	FDisUseState_Finished_Param() : FDisUseState_Param( UDisUseState_Finished::StaticClass() ) {}
	virtual INT SizeOf() const { return sizeof( FDisUseState_Finished_Param ); }
};

struct FDisUseState_Holster_Param : public FDisUseState_Param
{
	FDisUseState_Holster_Param() : FDisUseState_Param( UDisUseState_Holster::StaticClass() ) {}
	virtual INT SizeOf() const { return sizeof( FDisUseState_Holster_Param ); }
};

struct FDisUseState_AltInteract_Param : public FDisUseState_Param
{
	IDisInteractableInterface* m_pInteractable;

	FDisUseState_AltInteract_Param( IDisInteractableInterface* const _pInteractable )
		: FDisUseState_Param( UDisUseState_AltInteract::StaticClass() )
		, m_pInteractable( _pInteractable )
	{}

	virtual void OnPending( UDishonoredNativeState* _pPendingState, UObject* _pManagedObject )
	{
		FDisUseState_Param::OnPending( _pPendingState, _pManagedObject );
		if( UDisUseState_AltInteract* State = Cast<UDisUseState_AltInteract>( _pPendingState ) )
		{
			UObject* Object = m_pInteractable ? m_pInteractable->GetUObjectInterfaceDisInteractableInterface() : NULL;
			State->m_pInteractable.SetObject( Object );
			State->m_pInteractable.SetInterface( Object ? m_pInteractable : NULL );
		}
	}
	virtual INT SizeOf() const { return sizeof( FDisUseState_AltInteract_Param ); }
};

#endif
