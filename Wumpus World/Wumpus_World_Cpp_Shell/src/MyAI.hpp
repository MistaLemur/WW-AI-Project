// ======================================================================
// FILE:        MyAI.hpp
//
// AUTHOR:      Miguel SuVasquez
// ======================================================================

#ifndef MYAI_LOCK
#define MYAI_LOCK

#include "Agent.hpp"
#include <iostream>
#include <list>
#include <map>

class MyAI : public Agent
{
public:
	MyAI ( void );
	
	Action getAction
	(
		bool stench,
		bool breeze,
		bool glitter,
		bool bump,
		bool scream
	);
	
	// ======================================================================
	// YOUR CODE BEGINS
	// ======================================================================

	
	class TileNode {

		public:

			int x;
			int y;



			int hasMonster = -1;
			/* This int represents whether or not this has a monster
			-1: No information
			0 : Definitely no monster
			1 : Potential Monster
			2 : Confirmed Monster
			*/

			int hasPit = -1;
			/* This int represents whether or not this has a pit
			-1: No information
			0 : Definitely no pit
			1 : Potential pit
			2 : Confirmed pit
			*/

			int breezeCount = 0;
			int stenchCount = 0; //This is incremented whenever a stench is adjacent to this tile
								 /*
								 1 stenchCount = 33% chance of this having the monster
								 2 stenchCount makes this almost completely guaranteed, because of how few monsters there are in the world
								 */

			bool isExplored = false; //This boolean represents whether or not the character has entered this tile specifically.
			bool unreachable = false; //This boolean represents whether or not this node is outside of bounds

			int exploredAdjacent = 0; //This counts how many adjacent nodes have been explored

			int visited = 0;//This counts how many times this node has been visited.

			//These four pointers point to adjacent nodes
			TileNode* up = nullptr;
			TileNode* down = nullptr;
			TileNode* left = nullptr;
			TileNode* right = nullptr;
			TileNode* parent = nullptr;

			TileNode(int newx, int newy) {
				x = newx;
				y = newy;
			}

			~TileNode() { /* The TileNode graph is meant to be deleted from the bottom left corner */
				if (y == 0 && right != nullptr) {
					delete right;
				}

				if (up != nullptr) {
					delete up;
				}
			}

			bool inBounds(int min_x, int min_y, int max_x, int max_y) {
				return x >= min_x && x < max_x && y >= min_y && y < max_y;
			}

			void onExplore() {
				if (isExplored) return;

				isExplored = true;

				hasPit = 0;
				hasMonster = 0;

				if (up != nullptr && !up->isExplored) {
					up->exploredAdjacent++;
				}
				if (down != nullptr && !down->isExplored) {
					down->exploredAdjacent++;
				}
				if (left != nullptr && !left->isExplored) {
					left->exploredAdjacent++;
				}
				if (right != nullptr && !right->isExplored) {
					right->exploredAdjacent++;
				}

			}

			void incrementStench() {
				//this function is called when adjacent to this node to increment the stenchCount.
				if (isExplored) return;

				stenchCount++;
				if (hasMonster == 1) {
					if (stenchCount > 1) hasMonster = 2;
				}
				else if (hasMonster < 0) {
					hasMonster = 1;
				}
			}

			void incrementBreeze() {
				//this function is called when adjacent to this node to increment the breezeCount.
				if (isExplored) return;

				breezeCount++;

				if (hasPit < 0) {
					hasPit = 1;
				}
				/*
				else if (hasPit == 1) {
					if (breezeCount > 2) {
						hasPit = 2; //This isn't necessarily true, but it's a pretty decent assumption
					}
				}
				*/
				
			}

			void initAdjacentNodes(int min_x, int min_y, int max_x, int max_y, std::list<TileNode*> &nodesList) {
				if (up == nullptr && max_y >= y+1) {

					for (TileNode* otherNode : nodesList) {
						if (otherNode->x == x && otherNode->y == y + 1) {
							up = otherNode;
							break;
						}
					}

					if (up == nullptr) {
						up = new TileNode(x, y + 1);
						up->parent = this;
						nodesList.push_back(up);
					}

					up->down = this;
				}

				if (down == nullptr && min_x <= y - 1) {

					for (TileNode* otherNode : nodesList) {
						if (otherNode->x == x && otherNode->y == y - 1) {
							down = otherNode;
							break;
						}
					}

					if (down == nullptr) {
						down = new TileNode(x, y - 1);
						down->parent = this;
						nodesList.push_back(down);
					}

					down->up = this;
				}

				if (right == nullptr && max_x >= x + 1) {

					for (TileNode* otherNode : nodesList) {
						if (otherNode->x == x + 1 && otherNode->y == y) {
							right = otherNode;
							break;
						}
					}

					if (right == nullptr) {
						right = new TileNode(x + 1, y);
						right->parent = this;
						nodesList.push_back(right);
					}
					right->left = this;
				}

				if (left == nullptr && min_x <= x - 1) {

					for (TileNode* otherNode : nodesList) {
						if (otherNode->x == x - 1 && otherNode->y == y) {
							left = otherNode;
							break;
						}
					}

					if (left == nullptr) {
						left = new TileNode(x - 1, y);
						left->parent = this;
						nodesList.push_back(left);
					}
					left->right = this;
				}

			}

			/*
			int exploreCost(float breezeMult = 1, float stenchMult = 1, float monsterMult = 1) { //heuristic cost of exploring this node
				int cost = 0;

				//std::cout << "\n EXPLORE COST? (" << x << " " << y << ") :" << visited << ", " << breezeCount << ", " << stenchCount << "::" << hasMonster << ", " << hasPit << ", " << unreachable << "\n";

				if (visited) cost += visited * 3; // * 2

				//if (hasPit != 0 && breezeCount > 0) cost += (int) (20 * breezeCount * breezeMult); // 14
				if (hasMonster != 0 && stenchCount > 0) cost += (int) (20 * stenchCount * stenchMult); // 15

				if (hasMonster == 2) cost += (int)(1000 * monsterMult);
				if (hasPit == 2) cost += 1000;

				if (unreachable) cost += 1000;

				return cost;
			}
			*/

			TileNode* getNeighbor(int dir) {
				switch (dir) {
				case 1: 
					return up;
				case 2:
					return down;
				case 4:
					return right;
				case 8:
					return left;
				}
				return nullptr;
			}

			std::list<TileNode*> getNeighbors() {
				std::list<TileNode*> neighbors;

				if (up != nullptr) neighbors.push_back(up);
				if (down != nullptr) neighbors.push_back(down);
				if (right != nullptr) neighbors.push_back(right);
				if (left != nullptr) neighbors.push_back(left);

				return neighbors;
			}
	};
 
	class DangerNode {
		//A danger node handles detected dangers and their effects upon the internal representation of the map.
		//For example: everytime a breeze is found, a dangerNode is created.
		public:
			std::list<TileNode*> tiles = std::list<TileNode*>(0); //These are tiles affected
			char dangerType = 0; //null char initial.
			//dangerType = 'b' - Breeze
			//dangerType = 's' - Stench

			int x=0, y=0; //the x and y coordinates where this danger originated.

			void addNode(TileNode* node, std::list<TileNode*>* ignoreList) {
				if (ignoreList != nullptr) {

					for (TileNode* listNode : *ignoreList) {
						if (listNode == node) return;
					}
				}
				tiles.push_back(node);
			}

			void removeNode(TileNode* node) {
				tiles.remove(node);
			}

			DangerNode(int newx, int newy, char dType) {
				x = newx;
				y = newy;
				dangerType = dType;
			}

	};

	TileNode* startNode; //node located at 0,0
	TileNode* currentNode; //node that I'm presently located at
	TileNode* lastNode;
	TileNode* monsterNode;

	std::list<TileNode*> newFrontierNodes;
	std::list<TileNode*> dangerousFrontierNodes;
	std::list<TileNode*> exploredNodes;
	std::list<TileNode*> allNodes;
	std::list<DangerNode*> dangers;

	// These are the discovered dimensions of the cave
	int max_x = 10;
	int max_y = 10;
	int min_x = 0;
	int min_y = 0;

	//These are the current coordinates of the myAI
	int x = 0;
	int y = 0;

	int dir = 4; //This stores the direction I'm pointed in.
	//1 for up
	//2 for down
	//4 for right
	//8 for left

	int time = 0;
	int timeThreshold = 133;
	int backtrack = 0;
	float backtrackThreshold = 0.756f;

	char state = 'e'; //This stores the state machine's state ID.
	//e for "Exploration State"
	//r for "Return State"
	//char comparisons are better than string comparisons when it comes to storing the state machine ID.

	bool killedMonster = false;
	bool stillHasArrow = true;
	bool justShotArrow = false;


	Agent::Action explorationState(
		bool stench,
		bool breeze,
		bool glitter,
		bool bump,
		bool scream
	); //This function takes care of anything that happens before acquiring the gold or finishing exploration
	
	Agent::Action riskState(
		bool stench,
		bool breeze,
		bool glitter,
		bool bump,
		bool scream
	);//This function includes decision making on when to take risks on breezes.

	Agent::Action returnState(
		bool stench,
		bool breeze,
		bool glitter,
		bool bump,
		bool scream
	); //This function takes care of anything that happens after finishing exploration or acquiring the gold

	void exploreTile(
		bool stench,
		bool breeze,
		bool glitter,
		bool bump,
		bool scream
	); //This function updates the internal model of the world after movement.

	TileNode* pathDestination = nullptr;
	std::list<TileNode*>::iterator pathIterator;


	

	Agent::Action Move(int moveDir); //This function moves the agent towards the given direction

	//This is a crude implementation of djikstra's algorithm
	std::list<TileNode*>* safePath(TileNode* source, TileNode* destination) 
	{
		if (source == destination) return 0;

		std::list<TileNode*> open;
		std::list<TileNode*> closed;
		std::map<TileNode*, TileNode*> parentMap;
		std::map<TileNode*, int> costMap;


		TileNode* current = source;
		costMap[current] = 0;
		open.push_front(current);

		do {
			current = open.front();
			open.pop_front();
			closed.push_back(current);

			if (current == destination) break;

			for (TileNode* neighbor : current->getNeighbors()) {

				if (!neighbor->isExplored && neighbor != destination) continue;
				if (neighbor->x < min_x || neighbor->x >= max_x || neighbor->y < min_y || neighbor->y >= max_y) continue;

				int cost = costMap[current] + 1;

				bool contains = false;;
				for (TileNode* node : open) {
					if (node == neighbor) {
						contains = true;
						break;
					}
				}

				for (TileNode* node : closed) {
					if (node == neighbor) {
						contains = true;
						break;
					}
				}
				if (cost < costMap[neighbor] && costMap[neighbor] != 0) {
					parentMap[neighbor] = current;
					costMap[neighbor] = cost;

				} else if (!contains) {
					open.push_back(neighbor);
					parentMap[neighbor] = current;
					costMap[neighbor] = cost;
				}
			}


		} while (open.size() > 0);

		if (current != destination) return nullptr;

		std::list<TileNode*>* path = new std::list<TileNode*>();

		while (current != source && current != nullptr) {
			path->push_front(current);
			current = parentMap[current];
		}

		return path;
	}

	int safeDistance(TileNode* source, TileNode* destination) {
		int dist = 0;
		std::list<TileNode*>* path = safePath(source, destination);

		if (path == nullptr) return 99999;

		std::list<TileNode*>::iterator pi = path->begin(); //pathIterator
		TileNode* current = source;
		int step = 0;
		int stepDir = dir;

		while (step < path->size() && current != destination) {
			TileNode* next = *pi;
			if (next == nullptr) return 99999;

			int nextDir = stepDir;
			if (next->y > y) nextDir = 1;
			else if (next->y < y) nextDir = 2;
			else if (next->x < x) nextDir = 8;
			else if (next->x > x) nextDir = 4;

			if (nextDir != stepDir) dist++;
			if ((stepDir == 4 && nextDir == 8) || (stepDir == 8 && nextDir == 4)) dist++;
			if ((stepDir == 1 && nextDir == 2) || (stepDir == 2 && nextDir == 1)) dist++;

			current = next;
			dist++;
			pi++;
			step++;
		}

		delete path;
		return dist;
	}

	// ======================================================================
	// YOUR CODE ENDS
	// ======================================================================

};

#endif
