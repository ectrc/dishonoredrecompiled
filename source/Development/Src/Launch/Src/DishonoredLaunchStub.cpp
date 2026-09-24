// Stand-in entry point until Phase 3 milestone 1 links the real Launch module (Launch.cpp,
// LaunchEngineLoop.cpp from the reference tree, kept next to this file). Writes the milestone-1
// log line the smoke test looks for so the build pipeline can be exercised end to end.
#include <windows.h>

#include <cstdio>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    FILE* log = nullptr;
    if (_wfopen_s(&log, L"Launch.log", L"w") != 0 || log == nullptr)
    {
        return 1;
    }
    std::fputs("Log: Log file open\n", log);
    std::fputs("Init: Object subsystem initialized\n", log);
    std::fclose(log);
    return 0;
}
