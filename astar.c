#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define N 7
#define abs(x) ((x)>0?(x):-(x))

typedef struct
{
	int x, y, score;
} dir_t;

static dir_t dir[] = {
	{0, -1, 10}, {0, 1, 10}, { -1, 0, 10}, {1, 0, 10},
	{ -1, -1, 14}, { -1, 1, 14}, { 1, 1, 14}, {1, -1, 14},//if only using 4 direction, you can comment this line
};

#define DIR (sizeof(dir)/sizeof(dir_t))

typedef struct node_s
{
	struct node_s *pre;
	struct node_s *parent;
	struct node_s *child;
	int x, y;
	int f, g, h;
} node_t;

typedef struct
{
	node_t *start, *end;
} node_mng_t;

typedef struct
{
	int x0, y0; //start position
	int x1, y1; //end position
	node_mng_t open_mng;
	node_mng_t close_mng;
} astar_t;

node_t* node_create(int x, int y, int g, int h, node_t *pre)
{
	node_t *node = (node_t*)malloc(sizeof(node_t));
	if (node == NULL) {
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->pre = pre;
	node->x = x;
	node->y = y;
	node->g = g;
	node->h = h;
	node->f = g + h;
	return node;
}

void node_free(node_mng_t *mng)
{
	if (mng) {
		for (node_t *child = mng->start; child; child = child->child) {
			free(child);
		}
	}
}

node_t* node_exist(node_mng_t *mng, int x, int y)
{
	if (mng) {
		for (node_t *child = mng->start; child; child = child->child) {
			if (child->x == x && child->y == y) {
				return child;
			}
		}
	}
	return NULL;
}

void node_add(node_mng_t *mng, node_t *node)
{
	if (mng && node) {
		if ((mng->start == NULL) || (mng->end == NULL)) {
			mng->start = mng->end = node;
		}
		else {
			if (node->f <= mng->start->f) {
				node->child = mng->start;
				mng->start->parent = node;
				mng->start = node;
				mng->start->parent = NULL;
			}
			else if (node->f >= mng->end->f) {
				mng->end->child = node;
				node->parent = mng->end;
				mng->end = node;
				mng->end->child = NULL;
			}
			else {
				for (node_t *child = mng->start->child; child; child = child->child) {
					if (node->f <= child->f) {
						node->child = child;
						node->parent = child->parent;
						child->parent->child = node;
						child->parent = node;
						break;
					}
				}
			}
		}
	}
}

void node_remove(node_mng_t *mng, node_t *node)
{
	if (mng && node) {
		if (node == mng->start) {
			mng->start = mng->start->child;
		}
		if (node == mng->end) {
			mng->end = mng->end->parent;
		}
		if (node->parent) {
			node->parent->child = node->child;
		}
		if (node->child) {
			node->child->parent = node->parent;
		}
	}
}

node_t* node_pop(node_mng_t *mng)
{
	if (mng) {
		node_t* node = mng->start;
		node_remove(mng, mng->start);
		return node;
	}
	return NULL;
}

int node_is_empty(node_mng_t *mng)
{
	if (mng) {
		return mng->start == NULL;
	}
	return 0;
}

#define OPEN_NODE(x, y, g, h, pre) \
	do{ \
		node_t *new_node = node_create(x, y, g, h, pre);\
		if (new_node == NULL) { /*memory not enough*/\
			return NULL; \
		} \
		node_add(&a->open_mng, new_node); \
		if (h == 0) {/*path is finded*/\
			return new_node; \
		} \
	}while(0)


/*
1. add start node to open list
2. repeat the following:
   a) look for the node with lowest f score on the open list.
   b) switch it to close list, and remove it from the open list.
   c) retrieve all its adjacent node:
   	  * if it is not walkable or if it is on close list, ignore it. otherwise do the following
   	  * if it isn`t on the open list, calculate its f, g, h and add it to the open list.
   	  *	if it is on the open list already, check to see if this path to that node is better, using g as the measure.
   	   	a lower g means that this is a better path. if so, change the parent of the node to the current node, 
   	   	and recalculate the g and f scores of the node. if you are keeping your open list sorted by f score, 
   	   	you may need to resort the list to account for the change.
   d) stop when you:
	  * add the target node to the open list, in which case the path has been found, or
	  * fail to find the target node, and the open list is empty. in this case, there is no path.
*/
node_t* astar_search(astar_t *a, int map[][N])
{
	if (a == NULL || map == NULL) {
		return NULL;
	}
	//open start node
	int h = abs(a->x0 - a->x1) + abs(a->y0 - a->y1);
	OPEN_NODE(a->x0, a->y0, 0, h, NULL);
	while (!node_is_empty(&a->open_mng)) {
		node_t *node = node_pop(&a->open_mng); // node with lowest f
		node_add(&a->close_mng, node);
		for (int i = 0; i < DIR; i++) {
			int x = node->x + dir[i].x;
			int y = node->y + dir[i].y;
			if (x >= 0 && x < N && y >= 0 && y < N) {
				if (map[y][x] || node_exist(&a->close_mng, x, y)) {
					continue;
				}
				node_t *opened_node = node_exist(&a->open_mng, x, y);
				if (opened_node) {
					int g = node->g + dir[i].score;
					if (opened_node->g > g) {
						opened_node->g = g;
						opened_node->f = opened_node->h + g;
						opened_node->pre = node;
						//resort nodes list
						node_remove(&a->open_mng, opened_node);
						node_add(&a->open_mng, opened_node);
					}
				}
				else {
					int g = node->g + dir[i].score;
					int h = abs(x - a->x1) + abs(y - a->y1);
					OPEN_NODE(x, y, g, h, node);
				}
			}
		}
	}
	return NULL;
}

void draw_map(int map[][N], node_t *end)
{
	if (map && end) {
		for (node_t *pre = end; pre; pre = pre->pre) {
			map[pre->y][pre->x] = 2;
		}
		for (int y = 0; y < N; y++) {
			for (int x = 0; x < N; x++) {
				switch (map[y][x]) {
				case 0:
					printf(".");
					break;
				case 1:
					printf("W");
					break;
				case 2:
					printf("O");
					break;
				default:;
				}
			}
			printf("\n");
		}
	}
}

static int map[N][N] = {
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0},
	{1, 1, 1, 0, 1, 1, 1},
	{0, 0, 0, 1, 1, 0, 0},
	{0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 1, 0, 1, 1},
	{0, 0, 0, 0, 0, 0, 0},
};

int main(int argc, char const * argv[])
{
	astar_t a;
	memset(&a, 0, sizeof(astar_t));
	a.x0 = 3;
	a.y0 = 2;
	a.x1 = 6;
	a.y1 = 6;
	node_t *end = astar_search(&a, map);
	draw_map(map, end);
	node_free(&a.open_mng);
	node_free(&a.close_mng);
	return 0;
}