## Intro
## Object Philosophy
* Component-based design
	* Game Objects as containers
	* Components for behavior and data
	* Workflow
## Key Systems and Tools
### Component System
- How components are defined and managed
- Compile-time registration and serialization
### Serialization
- Markup-based automatic JSON serialization
- Support for custom types (e.g., `glm::vec2`)
- Use cases: saving scenes, debugging, network sync

### Physics and Collision
- 2D physics system with rigid bodies and collision responses
- Octree optimization for collision detection
- UE-style collision channels and response settings

### UI and Editor Tools
- Built-in UI system and font handling
- Custom in-engine level editor or external tools integration

### Build and Deployment Pipeline
- Use of Premake for build configuration
- Automated setup scripts for cross-platform support

## Multiplayer and Networking
- Steam P2P mid-game join support
- Lightweight netcode design philosophy