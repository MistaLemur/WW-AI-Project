// ======================================================================
// FILE:        MyAI.cpp
//
// AUTHOR:      Anthony SuVasquez
// ======================================================================

#include "MyAI.hpp"

MyAI::MyAI() : Agent()
{
	// ======================================================================
	// YOUR CODE BEGINS
	// ======================================================================
	




	// ======================================================================
	// YOUR CODE ENDS
	// ======================================================================
}
	
Agent::Action MyAI::getAction
(
	bool stench,
	bool breeze,
	bool glitter,
	bool bump,
	bool scream
)
{
	// ======================================================================
	// YOUR CODE BEGINS
	// ======================================================================
	
	/*
	This is the function called each turn to determine the action of my AI's agent.
	*/
	
	Agent::Action myAction = CLIMB; //default action is to exit.

	//there are two AI states: exploration and return that call different functions.
	if (state == 'e') myAction = explorationState(stench, breeze, glitter, bump, scream);

	else if (state == 'r') myAction = returnState(stench, breeze, glitter, bump, scream);

	return myAction;
	// ======================================================================
	// YOUR CODE ENDS
	// ======================================================================
}

// ======================================================================
// YOUR CODE BEGINS
// ======================================================================
Agent::Action MyAI::explorationState(
	bool stench,
	bool breeze,
	bool glitter,
	bool bump,
	bool scream
) {
	/*
	This is the AI function for its exploration state.
	This state will explore the cave, and attempt to find the gold bar.
	*/
	time++; //increment the time counter

	
	if (breeze && x == 0 && y == 0) {
		/*
		This is a solution for an edge case where any move has a 50% chance of death.
		*/
		return CLIMB;
	}
	/*
	First take care of any preliminary processing, like updating the node graph or perceived room boundaries
	*/
	exploreTile(stench, breeze, glitter, bump, scream);
	/*
	Now decide on the action to take
	*/


	//if I smell the monster, I shoot an arrow immediately to either kill it or narrow down it's position.
	if (stench && stillHasArrow) {
		TileNode* frontNode = currentNode->getNeighbor(dir);
		if (frontNode != nullptr && frontNode->stenchCount > 0) {
			stillHasArrow = false;
			justShotArrow = true;
			return SHOOT;
		}
	}

	//if I just shot an arrow, I need to check if I heard the monster's death cry and update its possible position.
	if (justShotArrow) {
		justShotArrow = false;
		if (!scream) {
			TileNode* frontNode = currentNode->getNeighbor(dir);
			if (frontNode != nullptr) {
				frontNode->hasMonster = 0;
				frontNode->stenchCount = 0;
			}
			for (TileNode* an : allNodes) {
				if (!an->inBounds(min_x, min_y, max_x, max_y)) {
					an->stenchCount = 0;
					an->breezeCount = 0;
					an->hasMonster = 0;
					an->hasPit = 0;
					an->unreachable = true;
				}
			}
			std::list<TileNode*> nodes;
			for (TileNode* n : currentNode->getNeighbors()) {
				if (n->isExplored) continue;
				if (n->hasMonster == 0) continue;
				nodes.push_front(n);
			}
			if (nodes.size() == 1) {
				TileNode* n = nodes.front();
				n->hasMonster = 2;
				monsterNode = n;
				for (TileNode* an : allNodes) {
					if (monsterNode != an) {
						an->stenchCount = 0;
						an->hasMonster = 0;
					}
				}
			}
		}
	}

	if (scream) {
		killedMonster = true;
	}

	//std::cout << "Frontier Nodes: " << newFrontierNodes.size() << std::endl;

	//Look for a way to travel to my current destination.
	if (pathDestination != nullptr) {
		//if I've reached my current destination
		if (currentNode == pathDestination) pathDestination = nullptr;
		else {
			//If I haven't reached my destination...
			//figure out if the next step is dangerous to travel to
			TileNode* next = *pathIterator;
			if (next == nullptr) pathDestination = nullptr;
			else {
				bool dangerous = false;

				if (bump) dangerous = true;
				if(next->hasMonster > 0) dangerous = true;

				if (!dangerous) {
					for (DangerNode* danger : dangers) {
						if (dangerous) break;
						for (TileNode* dNode : danger->tiles) {
							if (dNode == next) {
								dangerous = true;
								break;
							}
						}
					}
				}

				if (dangerous) pathDestination = nullptr; //If this path is determined to be dangerous, find a new destination.
			}
		}
	}

	//find a new path if there's no destination
	if (pathDestination == nullptr) {
		TileNode* dest = nullptr;
		int destCost = 100;

		for (TileNode* node : newFrontierNodes) {
			if (node->x < min_x || node->x >= max_x || node->y < min_y || node->y >= max_y) continue;
			bool isDangerous = false;

			if (node->hasMonster > 0) isDangerous = true;

			for (DangerNode* danger : dangers) {
				if (danger == nullptr) continue;
				if (isDangerous) break;
				if (danger->tiles.size() == 0) continue;
				for (TileNode* dNode: danger->tiles){
					if (dNode == node) {
						isDangerous = true;
						break;
					}
				}
			}

			if (isDangerous) continue;

			int nodeCost = safeDistance(currentNode, node);
			
			if (nodeCost < destCost) {
				dest = node;
				destCost = nodeCost;
			}
		}

		if (dest != nullptr) {
			pathDestination = dest;
			pathIterator = safePath(currentNode, dest)->begin();
		}
		else {
			//In the case that I'm not able to find a safe path to the destination, exit the cave.
			glitter = true;
		}
	}


	//if I spot a glitter, pick up the gold bar
	if (glitter) { //If there is glitter, then grab the gold and change to the "return" state.
		state = 'r';
		return GRAB;
	}

	/* Choose a direction to move in. Move towards the next step in the path.*/
	int moveDir = dir;
	TileNode* next = *pathIterator;

	if (next->y > currentNode->y) moveDir = 1;
	if (next->y < currentNode->y) moveDir = 2;
	if (next->x > currentNode->x) moveDir = 4;
	if (next->x < currentNode->x) moveDir = 8;

	if (next != pathDestination && moveDir == dir) pathIterator++; //step the path iterator when I move forward

	return Move(moveDir);
}

Agent::Action MyAI::returnState(
	bool stench,
	bool breeze,
	bool glitter,
	bool bump,
	bool scream
) {
	//The return state follows the exploration parent of each node until it reaches the ladder.
	//This does not always yield optimal paths home, but is very simple to implement.
	//With Djiktras' algorithm implementation, it's not guaranteed to find the shortest path home because of time cost of turning.
	//Even implementing A* with varying the heuristic to try to compensate for turning time cost does not yield the shortest path.
	if (x == 0 && y == 0) {
		delete startNode;
		return CLIMB;
	}

	TileNode* nextNode = currentNode->parent;
	if (nextNode == nullptr) {
		return CLIMB;
	}

	int moveDir = dir;

	if (nextNode->y > y) moveDir = 1;
	if (nextNode->y < y) moveDir = 2;
	if (nextNode->x > x) moveDir = 4;
	if (nextNode->x < x) moveDir = 8;

	return Move(moveDir);
}


void MyAI::exploreTile(bool stench, bool breeze, bool glitter, bool bump, bool scream) {
/*
the exploreTile() function updates the AI's graph model of the cave with the current position.
Some of its sensors, the stench and breeze booleans, provide limited information about adjacent nodes from the current position.
This function uses that sensory data in the graph model to mark likely locations for dangers to the AI agent.
*/

	if (startNode == nullptr) {
		//If I have no start node, then I should initialize a new graph
		startNode = new TileNode(0, 0);
		currentNode = startNode;
		allNodes.push_back(currentNode);
	}

	if (bump) { //If I ran into the boundary update graph's boundary limits.
		if (dir == 1) {
			max_y = y;
		}

		if (dir == 2) {
			min_y = y;
		}

		if (dir == 4) {
			max_x = x;
		}

		if (dir == 8) {
			min_x = x;
		}

		if (currentNode != nullptr) {
			currentNode->unreachable = true;
		}

		currentNode = lastNode;
	}

	if (currentNode == nullptr) {
		currentNode = lastNode;
	}

	currentNode->visited++;
	lastNode = currentNode;

	if (!currentNode->isExplored) { //This is my first time on this tile.
		currentNode->initAdjacentNodes(min_x, min_y, max_x, max_y, allNodes); //create unexplored nodes for my neighbors

		currentNode->onExplore(); //Run the exploration event on my current node
		newFrontierNodes.remove(currentNode); //Remove my node from the list of ones that are unexplored.

		//if breeze, increment breeze counts
		if (breeze) {
			DangerNode* danger = new DangerNode(currentNode->x, currentNode->y, 'b');
			//Create a new danger node here. Danger nodes keep track of where dangerous sensory data occured.
			
			//I also keep track of how many times dangerous sensory data occured for the neighbors of this node.
			//This is somewhat vestigial from the previous version of this AI
			if (currentNode->up != nullptr) {
				currentNode->up->incrementBreeze();
				if (currentNode->up->breezeCount > 0) newFrontierNodes.remove(currentNode->up);

				if(currentNode->up->hasPit > 0) danger->addNode(currentNode->up, nullptr);
			}

			if (currentNode->down != nullptr) {
				currentNode->down->incrementBreeze();
				if (currentNode->down->breezeCount > 0) newFrontierNodes.remove(currentNode->down);

				if (currentNode->down->hasPit > 0) danger->addNode(currentNode->down, nullptr);
			}

			if (currentNode->right != nullptr) {
				currentNode->right->incrementBreeze();
				if (currentNode->right->breezeCount > 0) newFrontierNodes.remove(currentNode->right);

				if (currentNode->right->hasPit > 0) danger->addNode(currentNode->right, nullptr);
			}
			if (currentNode->left != nullptr) {
				currentNode->left->incrementBreeze();
				if (currentNode->left->breezeCount > 0) newFrontierNodes.remove(currentNode->left);

				if (currentNode->left->hasPit > 0) danger->addNode(currentNode->left, nullptr);
			}

			dangers.push_back(danger);
		}

		//if stench, increment stench counts and maybe deduce monster location
		if (stench) {
			/*
			If I smell the monster's stench, I do a similar behavior. However, since there's only 1 monster in the cave...
			It is very easy to deduce its location, so I do not create a danger node for it.
			*/
			if (currentNode->up != nullptr) {
				currentNode->up->incrementStench();
				if (currentNode->up->stenchCount > 1) {
					monsterNode = currentNode->up;
					newFrontierNodes.remove(currentNode->up);
				}
			}
			if (currentNode->down != nullptr) {
				currentNode->down->incrementStench();
				if (currentNode->down->stenchCount > 1) {
					monsterNode = currentNode->down;
					newFrontierNodes.remove(currentNode->down);
				}
			}
			if (currentNode->right != nullptr) {
				currentNode->right->incrementStench();
				if (currentNode->right->stenchCount > 1) {
					monsterNode = currentNode->right;
					newFrontierNodes.remove(currentNode->right);
				}
			}
			if (currentNode->left != nullptr) {
				currentNode->left->incrementStench();
				if (currentNode->left->stenchCount > 1) {
					monsterNode = currentNode->left;
					newFrontierNodes.remove(currentNode->left);
				}
			}

		}

		//mark adjacent nodes as safe if no stench or no breeze
		for (TileNode* node : currentNode->getNeighbors()) {
			if (node == nullptr) continue;
			if (!breeze) {
				node->hasPit = 0;
				node->breezeCount = 0;

				for (DangerNode* danger : dangers) {
					danger->tiles.remove(node);
				}
			}
			if (!stench) {
				node->hasMonster = 0;
				node->stenchCount = 0;
				if (monsterNode == node) monsterNode = nullptr;
			}
		}

		//add neighbors to frontier nodes if they have not been explored yet
		for (TileNode* node : currentNode->getNeighbors()) {
			if (node == nullptr) continue;
			if (node->isExplored) continue;
			if (node->breezeCount > 1) continue;
			if (node->stenchCount > 1) continue;

			bool contains = false;

			for (TileNode* node2 : newFrontierNodes) {
				if (node2 == node) {
					contains = true;
					break;
				}
			}

			if (contains) continue;
			newFrontierNodes.push_back(node);
		}
	}

}

Agent::Action MyAI::Move(int moveDir) {
	/*
	This function turns a desired direction of movement into a verb for the Agent.
	The Agent can only turn left, turn right, or walk forward.
	*/

	if (dir != moveDir) {
		Agent::Action returnAction = TURN_LEFT;

		if (dir == 1 && moveDir == 4) returnAction = TURN_RIGHT;
		if (dir == 4 && moveDir == 2) returnAction = TURN_RIGHT;
		if (dir == 2 && moveDir == 8) returnAction = TURN_RIGHT;
		if (dir == 8 && moveDir == 1) returnAction = TURN_RIGHT;

		if (returnAction == TURN_LEFT) {
			if (dir == 1) dir = 8;
			else if (dir == 2) dir = 4;
			else if (dir == 4) dir = 1;
			else if (dir == 8) dir = 2;
		}
		else if (returnAction == TURN_RIGHT) {
			if (dir == 1) dir = 4;
			else if (dir == 2) dir = 8;
			else if (dir == 4) dir = 2;
			else if (dir == 8) dir = 1;
		}

		return returnAction;
	}
	else {
		currentNode = currentNode->getNeighbor(dir);

		if (currentNode != nullptr) {
			x = currentNode->x;
			y = currentNode->y;
			if (currentNode->isExplored) backtrack++;
		}

		return FORWARD;
	}
}

// ======================================================================
// YOUR CODE ENDS
// ======================================================================
