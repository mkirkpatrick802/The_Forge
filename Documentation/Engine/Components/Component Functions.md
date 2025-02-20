---
tags:
  - Components
---
#### Order of Operations
---
**Constructor**
* This is called when the program is initialized. It should be used to purely initialize member variables, and bind events, but nothing involving vendors should be used here.

**On Activation**
* This is called when the component becomes active on a game object. It can be used to access vendors like SDL or OpenGL. Other components on the game object cannot be accessed within this function.

**Start**
* This is the earliest other components on the game object can be accessed. It is called right before the first update frame of this component.

**Update**
* This is called every frame of the game. 