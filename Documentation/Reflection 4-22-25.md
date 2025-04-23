During my time at Champlain College, I had the opportunity to work on a variety of low-level systems for game engines. However, these projects were often isolated small pieces we’d build for class, then move on from without ever seeing them come together into a full engine. For my final college project, I wanted to change that. I set out to unify all those individual systems into one cohesive project: _The Forge Engine.

I actually began prototyping The Forge during my sophomore year. At the time, I had no idea I’d return to it or keep pushing it forward but it stuck with me. So, what _is_ The Forge Engine? It’s the culmination of everything I’ve learned as an undergraduate. I built it with a focused goal and a clear design philosophy: to combine the C++ workflow of Unreal with the system architecture of Unity.

I love the idea of defining object types in the backend, compiling the engine, and seeing those changes reflected in the front end. For a student-built engine, full C++ reflection and a polished development pipeline weren’t feasible but this hybrid Unreal-style workflow fit my needs perfectly.

I wanted to blend the best parts of my favorite engines into something custom-built for a genre I care deeply about: 2D pixel-art multiplayer games. The Forge was designed specifically for that niche. Some of my favorite childhood games were simple 2D titles that pushed creative boundaries and brought friends together. With my background in network programming, I saw an opportunity to build something fun and technically challenging in that space.

Is this engine production-ready? Not at all. It’s full of bugs, rough edges, and plenty of crashes. But I focused on designing the systems with maintainability in mind so if I decide to keep working on it, I’ll be building on a solid, extensible foundation.
### What Worked Well
This project was a ton of fun, and I learned a lot but it also gave me a real sense of just how massive game engine development is and how many moving parts are involved. I'm super happy with how my core systems turned out, especially input, physics, and the Netcode portion. Another win was my build pipeline. I put a lot of care into separating engine code from game code, and it’s paid off in how clean and flexible the project feels. I’m really looking forward to expanding on that with a launcher that supports Python templates and automation tools down the line. 
### Challenges I Faced
The biggest headache right now is the creation and destruction of game objects and their components. There’s a long-standing bug that causes memory leaks once a certain number of objects are created, and my memory checkpoints have been flagging it consistently. That system, along with the overall level system, needs a serious overhaul.

I’ve also been wrestling with serialization. Right now, each component has a lot of repetitive code for JSON serialization, even though most of it overlaps. Ideally, I’d like to implement a basic form of C++ reflection to streamline the process and eliminate all that redundancy. Serialization is one of those systems I didn’t touch as much as I wanted to, but it’s high on the list for cleanup and refactor work. The goal is to keep future system changes easy to manage and serialization is a big part of that. 

The final big challenge was optimization. I was full steam ahead on building features and systems, and while the code is structured well and easy to follow, it’s definitely not optimized. There are a lot of spots where performance could be improved, especially when scaling up object counts or networking interactions. I made a conscious decision to focus more on architecture than raw speed for this version, but performance tuning is something I’d love to revisit once the foundation is fully stable.
### How This Project Improved My Development Skills
Working on _The Forge_ completely changed how I think about planning and implementing systems. Before this, I’d mostly focus on just getting things to work for myself. But through this project, I started thinking more intentionally about how other developers would interact with my systems—how to design clear interfaces while keeping the internals protected to maintain system integrity. That mindset shift didn’t just make things easier for others; it also helped _me_ when debugging or building on top of my own code. Writing clean, approachable systems means anyone (my future self included) can jump in and make sense of things without digging through a mess.

It also seriously leveled up my general knowledge of game engine architecture. I feel way more confident diving into a complex engine like Unreal and being able to follow along with how their systems are structured and why they work the way they do. It’s helped me move from just using tools to actually understanding what’s going on under the hood.
## What’s Next?
This project might have been my final college capstone, but I definitely don’t see it as finished. There’s still a ton I want to explore and build on. Here are a few of the things on my roadmap:

- **Optimization** – Now that most of the core systems are in place, I want to go back and focus on performance. This includes memory usage, update cycles, and anything that doesn’t scale well right now.
    
- **New Level System** – The current system works, but it’s clunky and hard to extend. I’m planning a complete redesign that will support more flexible scene composition and cleaner data handling.
    
- **Enhanced Editor** – I want to flesh out the editor tools with proper gizmos, more configurable settings for components, and a more complete content drawer for assets. Basically, I want it to feel less like a debug UI and more like a usable dev tool.
    
- **Launcher with Python Templates** – I’m really excited about building a launcher that can spin up new projects using Python templates. It would help automate project setup and streamline workflows for different kinds of games.
    

I’m also open to collaboration or feedback! If you're a student dev, systems nerd, or just curious about engine architecture, feel free to reach out. I’d love to hear your thoughts, suggestions, or even see what kinds of games you’d want to build on something like this.
## Closing Thoughts
Building _The Forge_ has been one of the most challenging and rewarding projects I’ve worked on. It pushed me to think like a systems programmer, a designer, and a toolsmith all at once. I learned a ton, not just about how game engines work, but about how I work best when building something from the ground up.

This engine isn’t perfect, but it represents a huge leap in my understanding of game development and low-level programming. And more than anything, it’s a foundation I’m proud of one I’m excited to keep building on.

If you're curious to check it out, here is the Github:

- [Engine Repository](https://github.com/mkirkpatrick802/The_Forge)

Thanks for reading!
