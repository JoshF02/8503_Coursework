Note - to run the code, use CMake to generate a new project


Youtube Video Link
https://youtu.be/Q8Xz2w_ecww?si=GsHHwANC_iu9zI82


Screenshots

 ![image](https://github.com/JoshF02/8503_Coursework/assets/95030736/53d00318-691d-4f1e-9363-559ea2d44efd)

This screenshot shows the starting area, showing the player character which can move and jump using forces, and be rotated with either torque (free rotation enabled) or a function to make it face the camera direction. There are 2 spheres demonstrating the use of elasticity within collision resolution, as the steel ball (right) is much less bouncy than the rubber ball (left). These balls land on a plane, showcasing plane-sphere collision.


![image](https://github.com/JoshF02/8503_Coursework/assets/95030736/c006c6f8-a56a-40af-8531-425f4d4a9788)

In the second screenshot a bridge made using constraints can be seen, with a staircase up to it and items at either end. On the left is a key, which is used to open a door later on. On the right is a coin power up, which are placed throughout the level and give a speed boost when collected by either the player or an enemy. The coin object is controlled with a state machine, which rotates it back and forth over time. An enemy is also shown, which is also controlled by a state machine. The enemies follow their pre-set patrol paths, unless the player comes within range of their field of vision (determined via raycast) in which case they chase the player until line of sight is broken. If they catch the player, they ‘die’ and are shown a game over screen (shown later).

 
 ![image](https://github.com/JoshF02/8503_Coursework/assets/95030736/51dfe612-bf01-4be6-92a5-463e49ed0b76)

This screenshot shows the first door of the level, which is controlled via a simple pressure plate which permanently opens it. Later on there is another pressure plate controlled door, but that one operates on a timer so the player must place an item on it in order for the door to stay open.


![image](https://github.com/JoshF02/8503_Coursework/assets/95030736/8aa985d4-90a8-4502-995b-b4a32b8c3920)

Here you can see the key from earlier being held by the player, as objects can be picked up and either dropped or thrown. Throwing an item at an enemy will destroy them if it hits them. The key must collide with the door on the left in order to open it. The key uses an OBB collision volume, as do the cubes on the right. The walls and floors use AABBs, the player and goose use spheres, and the basic enemies use capsules. Detection has been implemented for all of these collisions, as well as for raycasts to each of them.


![image](https://github.com/JoshF02/8503_Coursework/assets/95030736/fa669638-0273-4825-9c57-76f87213bf33)

Here the alarm is shown, which the goose will navigate to if the player collects the heist item before chasing them. Collecting the heist item constrains it to the player (shown below). The alarm causes the exit to lock, meaning the player must go the long way out via the way they came in. The goose will detect and then chase the player if they are within the maze and either are holding the heist item, or have entered the field of vision of the goose and there is direct line of sight to them. If detected, the goose will pathfind to the player and if close enough, move directly to them, killing them on collision. The player remains detected for 10 seconds after being spotted, so they must evade the goose for 10 seconds to stop being chased. If detection is lost, the goose will go to the last place it saw the player, and then resume its patrolling of the maze. When patrolling, it pathfinds to various pre-set points throughout the maze until it detects the player. All of this is controlled by a behaviour tree, made up of multiple sequences, selectors and actions representing its behaviour.


![image](https://github.com/JoshF02/8503_Coursework/assets/95030736/d7e2df8e-f723-4fdc-9de3-7710945d93f8)

This screenshot shows the goose, the constrained heist item, and the exit. The pathfind debug lines have been left on to show the decisions being made by the goose. If the heist item is brought back to the starting area, the player wins the game.


![image](https://github.com/JoshF02/8503_Coursework/assets/95030736/d2cd3ebc-647f-4802-bb0f-b9791a705f5a)

Here the pause menu is shown, which is brought up with F3. The player can either unpause the game, restart the level, or exit the program.


![image](https://github.com/JoshF02/8503_Coursework/assets/95030736/2200a434-49e1-49a8-958e-dcf2396f2596)

Here the game over screen is shown, which will be coloured depending on if the player wins or loses. Their total score is shown, as well as the number of coins collected, and they have options to return to the menu to play again, or to exit. They win by bringing the heist item back, and lose by dying or running out of time.


![image](https://github.com/JoshF02/8503_Coursework/assets/95030736/152327f5-97f4-423a-9c00-6ff24fe7a9cd)

Here the networked high score table is shown, which is stored on the server and sent out to clients when a new client connects or when a new score is entered by a client. The client ID of the client that achieved the score is shown next to it. A new client can be created by pressing J, which connects to the server and is sent the high score table.


Key Binds
•	WASD - move player with forces
•	Space - jump
•	Mouse - rotate player and camera

•	E - pick up item
•	R - drop held item
•	F - throw held item
•	Mouse scroll wheel - rotate held item

•	F3 - pause (menu controls shown there)
•	1 (in menu) - start / restart
•	2 (in menu) - unpause
•	F3 (on game over screen) - return to menu

•	Up arrow - enable free rotation (camera rotation separate from character rotation)
•	Down arrow - disable free rotation
•	Left arrow - rotate character left with torque (if free rotation enabled)
•	Right arrow - rotate character right with torque (if free rotation enabled)

•	J - create new client (using current window)
•	H - show/hide high score table
