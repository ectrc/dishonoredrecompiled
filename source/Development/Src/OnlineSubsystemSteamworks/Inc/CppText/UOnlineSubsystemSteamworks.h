// cpptext of UOnlineSubsystemSteamworks (included inside the generated class body by gen_classes_header.py --sdk).
// DISHONORED(port): UOnlineSubsystem::Tick is "must be overridden"; the retail override (2013 UOnlineSubsystemSteamworks::Tick)
// signs in locally when not logged in, ticks the game interface / voice / connection-status change and the Steam async tasks.
// The offline build (no Steam DLL) keeps the local sign-in and the connection-status check (OnlineSubsystemSteamworksOffline.cpp).
virtual void Tick(FLOAT DeltaTime);
