# WW-AI-Project
WumpusWorld AI Project

This is my final project entry for an Intro to AI class.
My code is only the code within the TonyAI folders under WW-AI-Project/Wumpus World/Wumpus_World_Cpp_Shell/src/.
Two versions of my AI can be found here; TonyAI1 and TonyAI2. These behave very differently from one another; TonyAI2 scores much higher than TonyAI1 does, using lessons learned from TonyAI1.

MyAI.cpp and MyAI.hpp under the /src folder is just the TonyAI2

The WumpusWorld environment (World.cpp, World.hpp, Agent.hpp, etc...) was provided by the professor.

Test worlds are provided under the Debug build folder.
A makefile is provided under Wumpus_World_Cpp_Shell; run the makefile using the make command.

After building using the make file, run the executable using the following commands:
-f Worlds     //This will run the AI on all of the test worlds and print out performance statistics and score
-d            //This will run the AI and show it step-by-step so that you can watch the AI solve the scenario
