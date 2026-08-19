# The Forge

A custom C++20 engine for 2D pixel-art games, focused on sprite tooling and network programming. Built with Premake and Lua. The demo game is **Project-Steel**.

## Getting Started

```
Scripts\Setup-Solution-Windows.bat    once - installs premake and generates the solution
Scripts\Build.bat                     reflect -> generate -> build
Scripts\Run-Editor.bat                open the editor
```

Build through `Scripts\Build.bat` rather than the `.sln` directly. It runs reflection codegen, then premake, then MSBuild — in that order — which is what stops newly added source files from silently going missing from the build.

### Running

| Command | What it does | Needs Steam |
|---|---|---|
| `Scripts\Run-Editor.bat` | Editor / level authoring | No |
| `Scripts\Run-Server.bat` | Headless dedicated server | No |
| `Scripts\Run-Client.bat [address]` | Windowed client | No |
| `Project-Steel.exe` | Steam P2P multiplayer | Yes |

Useful flags: `--server`, `--connect <address>`, `--port <n>`, `--headless`, `--windowed`, `--no-steam`, `--net-loss <percent>`, `--net-selftest`.

`Scripts\Clean.bat` removes generated artefacts.

## Features

**Core** — component-based game objects with pooling and a registry; physics with spatial partitioning and rigidbodies; quad-tree collision; JSON serialization for levels and prefabs.

**Editor** — level creation and selection, game object editing, content browser, details panel, scene viewport, command terminal.

**Rendering** — custom shaders, visual debugging tools, and lazy GPU resource creation so level loading touches no GPU.

**Netcode** — host-authoritative world state replication with per-object serialization, client input replication, a Steam P2P transport, and a UDP transport with hand-rolled reliability, ordering and fragmentation.

**Tooling** — reflection codegen via `REFLECT()` / `REPLICATE()`, import-time asset metadata sidecars, and a headless dedicated server mode that needs no GPU and no Steam client.

## Status

The dedicated server model is partly built. The UDP transport is complete and tested under simulated packet loss, but game data is not yet routed over it — a client connects successfully and shows no world. Steam P2P remains the working multiplayer path.

## Libraries

SDL2 · Glad/OpenGL · FreeType · ImGui · Steam SDK · GLM · nlohmann/json · stb_image

## License

See `LICENSE.txt`.
