#pragma once
// DishonoredGame/inc/dispowercensus.h
// DISHONORED(bringup): agent DO. The measurement of the post-process settings path's ACTOR feed - what
// ADishonoredPlayerController::ModifyPostProcessSettings (2013 rva 0x6adeb0) found by name, which node controllers
// ticked, and what state the dark-vision controller is in - plus the tallboy appearance counters.
//
// Every switch is read on FIRST USE, never in a file-scope static: a file-scope
// `static UBOOL G... = ParseParam(appCmdLine(), ...)` in a static library runs before WinMain sets GCmdLine and is
// therefore always FALSE (agent CA, PHASE10 rule).

/** -dispower: one census line per second on the pp settings path. */
UBOOL DisPowerCensusEnabled();
/** -disdarkvisionpp: hold the dark-vision power's post-process active, which is the one bit the power itself sets. */
UBOOL DisDarkVisionForced();
/** -nodismodifypp: ModifyPostProcessSettings returns at once - the A leg of the pair. */
UBOOL DisModifyPpSuppressed();

void DisPowerReport( class UWorld* World, FLOAT DeltaSeconds );

/** Counters the ported passes raise; read by the census. */
extern INT GDisPpBridgeInits;
extern INT GDisPpBridgeNodesFound;
extern INT GDisPpBridgeNodesMissing;
extern INT GDisPpControllerTicks;
extern INT GDisPpModifyCalls;
extern INT GDisTallboyStiltsSet;
extern INT GDisTallboyLightsSpawned;

/** -dislookat: where the head bone actually is, on both of a character's two skeletal meshes. */
UBOOL DisLookAtCensusEnabled();
void DisLookAtReport( class UWorld* World, FLOAT DeltaSeconds );

/** -distallboy: what tallboy content is loaded, and what the two tallboy passes did to it.
    -distallboyspawn=<seconds>: spawn one from the first archetype found, in front of the player, once. */
UBOOL DisTallboyCensusEnabled();
void DisTallboyReport( class UWorld* World, FLOAT DeltaSeconds );

/** -nodislookataim turns the head-aim stand-in off (the A leg of the head pair); -dislookatplayer points every
    NPC's head at the player. Both read on first use. */
UBOOL DisLookAtAimStandInEnabled();
UBOOL DisLookAtPlayerEnabled();
