# Until Phase 3 milestone 1 the game exe is a stub; the reference Launch sources (Launch.cpp,
# LaunchEngineLoop.cpp, LaunchMisc.cpp, PIB.cpp) sit next to it and are not compiled yet.
add_executable(DishonoredGame WIN32 Src/DishonoredLaunchStub.cpp)
set_target_properties(DishonoredGame PROPERTIES OUTPUT_NAME "DishonoredGame" FOLDER "Game")
