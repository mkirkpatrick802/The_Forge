#include "LoadProgress.h"

namespace
{
    // Process-wide, for the same reason System's log mirror is: the thing that reports
    // progress and the thing that displays it are several call layers apart and neither
    // should have to know the other exists.
    Engine::LoadProgress* g_sink = nullptr;
}

void Engine::SetLoadProgressSink(LoadProgress* sink)
{
    g_sink = sink;
}

Engine::LoadProgress* Engine::GetLoadProgressSink()
{
    return g_sink;
}

void Engine::ReportLoadProgress(const float fraction, const std::string& stage)
{
    if (g_sink != nullptr)
        g_sink->Report(fraction, stage);
}
