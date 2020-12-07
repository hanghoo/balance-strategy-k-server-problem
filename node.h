#include <stdbool.h>
#include <vector>
using namespace std; 

/* Tree structure */
class Node
{
public:
	int x; // x coordinate
	int y; // y coordinate
	Node* predecessor; // previous one node
};

class Sever
{
public:
	Node node;
	int distance;
};




