// ======================================================================
// FILE:        MyAI.hpp
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

#ifndef MYAI_LOCK
#define MYAI_LOCK

#include "Agent.hpp"
#include <iostream>
#include <list>

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

			int stenchCount = 0; //This is incremented whenever a stench is adjacent to this tile
								 /*
								 1 stenchCount = 33% chance of this having the monster
								 2 stenchCount makes this almost completely guaranteed, because of how few monsters there are in the world
								 */

			int breezeCount = 0; //This is incremented whenever a breeze is adjacent to this tile.

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
				else if (hasPit == 1) {
					if (breezeCount > 2) {
						hasPit = 2; //This isn't necessarily true, but it's a pretty decent assumption
					}
				}
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

			int exploreCost(float breezeMult = 1, float stenchMult = 1, float monsterMult = 1) { //heuristic cost of exploring this node
				int cost = 0;

				//std::cout << "\n EXPLORE COST? (" << x << " " << y << ") :" << visited << ", " << breezeCount << ", " << stenchCount << "::" << hasMonster << ", " << hasPit << ", " << unreachable << "\n";

				if (visited) cost += visited * 3; // * 2

				if (hasPit != 0 && breezeCount > 0) cost += (int) (20 * breezeCount * breezeMult); // 14
				if (hasMonster != 0 && stenchCount > 0) cost += (int) (21 * stenchCount * stenchMult); // 15

				if (hasMonster == 2) cost += (int)(1000 * monsterMult);
				if (hasPit == 2) cost += 1000;

				if (unreachable) cost += 1000;

				return cost;
			}

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
 
	TileNode* startNode; //node located at 0,0
	TileNode* currentNode; //node that I'm presently located at
	TileNode* lastNode;
	TileNode* monsterNode;

	std::list<TileNode*> unknownUnexploredNodes;
	std::list<TileNode*> allNodes;

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
	int timeThreshold = 125;
	int backtrack = 0;
	float backtrackThreshold = 0.756;

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
	
	Agent::Action returnState(
		bool stench,
		bool breeze,
		bool glitter,
		bool bump,
		bool scream
	); //This function takes care of anything that happens after finishing exploration or acquiring the gold

	void exploreUpdate(
		bool stench,
		bool breeze,
		bool glitter,
		bool bump,
		bool scream
	); //This function takes care of updating the world model when moving

	Agent::Action Move(int moveDir); //This function moves the agent towards the given direction

	// ======================================================================
	// YOUR CODE ENDS
	// ======================================================================

};

#endif