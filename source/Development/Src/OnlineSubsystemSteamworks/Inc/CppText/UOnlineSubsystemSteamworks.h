// cpptext of UOnlineSubsystemSteamworks (included inside the generated class body by gen_classes_header.py --sdk).
// DISHONORED(port): UOnlineSubsystem::Tick is "must be overridden"; the retail override (2013 UOnlineSubsystemSteamworks::Tick)
// signs in locally when not logged in, ticks the game interface / voice / connection-status change and the Steam async tasks.
// The offline build (no Steam DLL) keeps the local sign-in and the connection-status check (OnlineSubsystemSteamworksOffline.cpp).
virtual void Tick(FLOAT DeltaTime);

// DISHONORED(port): the three async reads script calls on the startup and map path
// (OnlineSubsystemSteamworksReads.cpp): retail vtable slots 91 / 95 / 121 = +364 / +380 / +484, bodies at
// 2013 rvas 0x5ad8f0 / 0x5aaf30 / 0x5aaf90. Not declared virtual here: the retail class is reflected and
// our class layout is pinned by OnlineSubsystemSteamworksLayouts.h, so no vtable slot may move.
UBOOL ReadFriendsList(BYTE LocalUserNum, INT Count, INT StartingAt);
UBOOL ReadProfileSettings(BYTE LocalUserNum, class UOnlineProfileSettings* ProfileSettings, UBOOL bForceRead);
UBOOL ReadAchievements(BYTE LocalUserNum, INT TitleId, UBOOL bShouldReadText, UBOOL bShouldReadImages);

#if WITH_UE3_NETWORKING && WITH_STEAMWORKS
// DISHONORED(port): the Steam client side (OnlineSubsystemSteamworksClient.cpp): InitSteamworks
// 2013 rva 0x5ac1d0, TickSteamworksTasks 2013 rva 0x5aa980, and the two user-stats callback handlers
// 2012 rvas 0x5f07d0 / 0x5f0840 that SteamCallbackBridge forwards to.
UBOOL InitSteamworks();
void TickSteamworksTasks(FLOAT DeltaTime);
void OnUserStatsReceived(struct UserStatsReceived_t* CallbackData);
void OnUserStatsStored(struct UserStatsStored_t* CallbackData);
#endif
