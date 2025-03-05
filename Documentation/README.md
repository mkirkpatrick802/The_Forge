#### Getting Started
1. Clone the Repository – First, clone the repo to your local machine.
2. Run Setup – Navigate to the scripts folder and run the .bat for your os. This will generate the .sln (solution) file needed to open the project in your IDE.
3. When building the project in your IDE ensure that Project-Steel is the start-up project (Launcher is still in active development).
#### About
I want this engine to focus on 2D pixel art games that provides lots of tools for sprite editing and network programming. My favorite games are multiplayer and pixel art based, and I would like a tool that makes it quick and easy for me to make games of these genres. This engine will be built in C++ and the build system will be using Premake and Lua. 
#### Current Features
* Naïve JSON Serialization  
* Basic Editor
	- Level Creation / Selection
	- Game Object Creation / Editing
	- Content Browser
	- Basic Prefab System
- Component-Based Game Objects
- Physics System
	- Spatial Partitioning
	- Rigidbodies
* Flexible Render Pipeline
	* Custom Shaders
	* Visual Debugging Tools
* Netcode
	* Packet Splitting
	* P2P Connections
	* Client-Server Model (For Game Authority)
	* Per Object Serialization
#### Planned Features
* Enhanced Prefab Editing & Utilities
* Debug Commands and Profiling
* Streamlined JSON Serialization
* Children Game Objects
	* Local vs World Space
* Enhanced Editor Readability

#### LIBRARIES
* SDL2
* Glad / OpenGL
* FreeType
* ImGui
* Steam SDK
* GLM
* JSON
