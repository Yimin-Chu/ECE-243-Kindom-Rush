ECE243 – Kingdom Rush (Tower Defense Game) on DE1-SoC

⭐ Implemented a real-time tower defense game on the DE1-SoC using the Nios V soft-core processor, rendering dynamic graphics on VGA and managing gameplay logic entirely in C running on the embedded system.

⭐ Designed a tile-based map and enemy pathfinding system, enabling enemies to autonomously navigate predefined routes across the battlefield while updating position and state in real time.

⭐ Developed a modular game engine architecture, separating rendering, enemy logic, tower behavior, collision detection, and resource management to support scalable gameplay features.

⭐ Implemented real-time VGA graphics rendering, drawing map grids, enemies, towers, and UI elements directly to the framebuffer while maintaining consistent refresh performance.

⭐ Built a tower attack and targeting system, where towers periodically detect enemies within range and apply damage using distance-based collision logic.

⭐ Implemented game state management and event loops, including enemy spawning, wave progression, player health tracking, and resource accumulation.

⭐ Optimized memory usage and data structures to support multiple enemies, towers, and projectiles within the limited memory and compute constraints of the embedded Nios V system.

⭐ Integrated hardware input controls from the DE1-SoC board, enabling interactive gameplay through switches/buttons for tower placement and game actions.

⭐ Developed the project using CPUlator simulation and deployed on real hardware, validating functionality across both simulation and the physical FPGA platform.

⭐ Delivered a fully playable embedded game demonstrating hardware–software co-design, combining low-level I/O control, graphics rendering, and game logic on an FPGA-based SoC platform.
