// ======================================================================
// FILE:        MyAI.cpp
//
// AUTHOR:      Abdullah Younis
//
// DESCRIPTION: This file contains your agent class, which you will
//              implement. You are responsible for implementing the
//              'getAction' function and any helper methods you feel you
//              need.
//
// NOTES:       - If you are having trouble understanding how the shell
//                works, look at the other parts of the code, as well as
//                the documentation.
//
//              - You are only allowed to make changes to this portion of
//                the code. Any changes to other portions of the code will
//                be lost when the tournament runs your code.
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
	
	Agent::Action myAction = CLIMB; //default action

	if (startNode == nullptr) {
		startNode = new TileNode(0, 0);
		currentNode = startNode;
		allNodes.push_back(currentNode);
	}

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
	time++;

	
	if (breeze && x == 0 && y == 0) {
		return CLIMB;
	}
	/*
	First take care of any preliminary processing, like updating the node graph or perceived room boundaries
	*/
	exploreUpdate(stench, breeze, glitter, bump, scream);
	/*
	Now decide on the action to take
	*/

	//if glitter, pick shit up
	if (glitter) { //If there is glitter, then grab the gold and change to the "return" state.
		state = 'r';
		return GRAB;
	}


	//if stench, pewpew
	if (stench && stillHasArrow) {
		TileNode* frontNode = currentNode->getNeighbor(dir);
		if (frontNode != nullptr && frontNode->stenchCount > 0) {
			stillHasArrow = false;
			justShotArrow = true;
			return SHOOT;
		}
	}

	//if pewpew, deduce monster status and position
	if (justShotArrow){
		justShotArrow = false;
		if(!scream) {
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


	/* Choose a direction to move in. Always move in the direction of lowest cost from current */
	int upCost = 1;
	int downCost = 1;
	int rightCost = 1;
	int leftCost = 1;

	float breezeMult = 1, stenchMult = 1, monsterMult = 1;

	int moveDir = dir;

	//Add directional costs to moving in such and such direction. Also take cave boundaries into account
	if (dir == 1) {
		downCost += 2;
		leftCost += 1;
		rightCost += 1;

		if (y == max_y) upCost += 9999;
	}
	else if (dir == 2) {
		upCost += 2;
		leftCost += 1;
		rightCost += 1;

		if (y == min_y) downCost += 9999;
	}
	else if (dir == 4) {
		leftCost += 2;
		upCost += 1;
		downCost += 1;

		if (x == max_x) rightCost += 9999;
	}
	else if (dir == 8) {
		rightCost += 2;
		upCost += 1;
		downCost += 1;

		if (x == min_x) leftCost += 9999;
	}

	if (monsterNode != nullptr) {
		stenchMult = 0;
	}

	if (killedMonster) {
		monsterMult = 0;
		stenchMult = 0;
	}

	//Add a positive cost to explored nodes. Prioritize exploration!
	if (currentNode->up != nullptr) upCost += currentNode->up->exploreCost(breezeMult, stenchMult, monsterMult);
	else upCost += 9999;
	
	if (currentNode->down != nullptr) downCost += currentNode->down->exploreCost(breezeMult, stenchMult, monsterMult);
	else downCost += 9999;
	
	if (currentNode->right != nullptr) rightCost += currentNode->right->exploreCost(breezeMult, stenchMult, monsterMult);
	else rightCost += 9999;

	if (currentNode->left != nullptr) leftCost += currentNode->left->exploreCost(breezeMult, stenchMult, monsterMult);
	else leftCost += 9999;

	if (currentNode->up == monsterNode) upCost += (int)(10000 * monsterMult);
	if (currentNode->down == monsterNode) downCost += (int)(10000 * monsterMult);
	if (currentNode->right == monsterNode) rightCost += (int)(10000 * monsterMult);
	if (currentNode->left == monsterNode) leftCost += (int)(10000 * monsterMult);

	/*
	std::cout << "\n upCost: " << upCost;
	std::cout << "\n downCost: " << downCost;
	std::cout << "\n rightCost: " << rightCost;
	std::cout << "\n leftCost: " << leftCost;
	std::cout << "\n";
	*/
	

	if (upCost <= downCost    && upCost < rightCost   && upCost < leftCost)		 moveDir = 1;
	if (downCost <= upCost    && downCost < rightCost && downCost < leftCost)	 moveDir = 2;
	if (rightCost < downCost && rightCost < upCost   && rightCost <= leftCost)	 moveDir = 4;
	if (leftCost < downCost  && leftCost < upCost    && leftCost <= rightCost)	 moveDir = 8;

	if (unknownUnexploredNodes.size() == 0) {
		state = 'r';
		return CLIMB;
	}

	if (time > timeThreshold) {
		state = 'r';
		return CLIMB;
	}

	if (backtrack > (int)(allNodes.size() * backtrackThreshold)){ 
		state = 'r';
		return CLIMB;
	}

	/*
	std::cout << "\n time: " << time << " (" << x << " " << y << ") " << dir << ", " << moveDir << "\n";
	*/

	Move(moveDir);

}

Agent::Action MyAI::returnState(
	bool stench,
	bool breeze,
	bool glitter,
	bool bump,
	bool scream
) {
	//The return state just runs A* on already-explored nodes. This ensures the shortest path but only through nodes that we have already explored.
	//Performing additional exploration in this state has the potential to discover a shorter path, but ultimately it is more likely to take up more time.
	//This flavor of A* uses manhattan heuristic + some heuristic cost for turning.
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


void MyAI::exploreUpdate(
	bool stench,
	bool breeze,
	bool glitter,
	bool bump,
	bool scream) {
	

	if (bump) { //If I ran into the boundary...
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

		for (TileNode* an : allNodes) {
			if (!an->inBounds(min_x, min_y, max_x, max_y)) {
				an->stenchCount = 0;
				an->breezeCount = 0;
				an->hasMonster = 0;
				an->hasPit = 0;
				an->unreachable = true;
			}
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
		currentNode->initAdjacentNodes(min_x, min_y, max_x, max_y, allNodes);

		currentNode->onExplore();
		unknownUnexploredNodes.remove(currentNode);

		//if breeze, increment breeze counts
		if (breeze) {
			if (currentNode->up != nullptr) {
				currentNode->up->incrementBreeze();
				if (currentNode->up->breezeCount > 0) unknownUnexploredNodes.remove(currentNode->up);
			}

			if (currentNode->down != nullptr) {
				currentNode->down->incrementBreeze();
				if (currentNode->down->breezeCount > 0) unknownUnexploredNodes.remove(currentNode->down);
			}

			if (currentNode->right != nullptr) {
				currentNode->right->incrementBreeze();
				if (currentNode->right->breezeCount > 0) unknownUnexploredNodes.remove(currentNode->right);
			}
			if (currentNode->left != nullptr) {
				currentNode->left->incrementBreeze();
				if (currentNode->left->breezeCount > 0) unknownUnexploredNodes.remove(currentNode->left);
			}
		}

		//if stench, increment stench counts and maybe deduce monster location
		if (stench) {
			if (currentNode->up != nullptr) {
				currentNode->up->incrementStench();
				if (currentNode->up->stenchCount > 1) {
					monsterNode = currentNode->up;
					unknownUnexploredNodes.remove(currentNode->up);
				}
			}
			if (currentNode->down != nullptr) {
				currentNode->down->incrementStench();
				if (currentNode->down->stenchCount > 1) {
					monsterNode = currentNode->down;
					unknownUnexploredNodes.remove(currentNode->down);
				}
			}
			if (currentNode->right != nullptr) {
				currentNode->right->incrementStench();
				if (currentNode->right->stenchCount > 1) {
					monsterNode = currentNode->right;
					unknownUnexploredNodes.remove(currentNode->right);
				}
			}
			if (currentNode->left != nullptr) {
				currentNode->left->incrementStench();
				if (currentNode->left->stenchCount > 1) {
					monsterNode = currentNode->left;
					unknownUnexploredNodes.remove(currentNode->left);
				}
			}

		}

		//mark adjacent nodes as safe if no stench or no breeze
		for (TileNode* node : currentNode->getNeighbors()) {
			if (node == nullptr) continue;
			if (node->isExplored) continue;
			if (!breeze) {
				node->hasPit = 0;
				node->breezeCount = 0;
			}
			if (!stench) {
				node->hasMonster = 0;
				node->stenchCount = 0;
				if (monsterNode == node) monsterNode = nullptr;
			}
		}

		//add neighbors to frontier nodes
		for (TileNode* node : currentNode->getNeighbors()) {
			if (node == nullptr) continue;
			if (node->isExplored) continue;
			if (node->breezeCount > 1) continue;
			if (node->stenchCount > 1) continue;

			bool contains = false;

			for (TileNode* node2 : unknownUnexploredNodes) {
				if (node2 == node) {
					contains = true;
					break;
				}
			}

			if (contains) continue;
			unknownUnexploredNodes.push_back(node);
		}
	}
}

Agent::Action MyAI::Move(int moveDir) {

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