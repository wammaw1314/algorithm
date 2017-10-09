#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAP_SIZE 7
#define abs(x) ((x)>0?(x):(-x))

#define h(x,y,xe,ye) (abs(x-xe)+abs(y-ye))

typedef struct dir_s
{
	int x, y, score;
}dir_t;

static dir_t dir[] = {
	{1, 0, 10}, {0, 1, 10}, {-1, 0, 10}, {0, -1, 10},
	{1, 1, 14}, {-1, 1, 14}, {-1, -1, 14}, {1, -1, 14},
};

#define DIR_N (sizeof(dir)/sizeof(dir_t))

typedef struct node_s
{
	struct node_s *p;
	struct node_s *child;
	int x, y;
	int f,g,h;
}node_t;

typedef struct node_list_s
{
	node_t *top;
}node_list_t;

typedef struct astar_s
{
	int sx, sy;
	int ex, ey;
	node_list_t open;
	node_list_t close;
}astar_t;

node_t *node_cearte(int x, int y, int g, int h, node_t *p)
{
	node_t *node = (node_t*)malloc(sizeof(node_t));
	if (node == NULL)
		return NULL;

	memset(node, 0, sizeof(node_t));
	node->p = p;
	node->child = NULL;
	node->x = x;
	node->y = y;
	node->g = g;
	node->h = h;
	node->f = g + h;
	return node;
}

void list_free(node_list_t *list)
{
	if (list == NULL)
		return;

	for (node_t *node = list->top; node; node = list->top)
	{
		list->top = list->top->child;
		free(node);
		node = NULL;
	}
}

node_t *node_exist(node_list_t *list, int x, int y)
{
	if (list == NULL)
		return NULL;

	for (node_t *child=list->top; child; child=child->child)
	{
		if (child->x == x && child->y == y)
		{
			return child;
		}
	}
	return NULL;
}

int node_add(node_list_t *list, node_t *node)
{
	if (list && node)
	{
		if (list->top == NULL)
		{
			list->top = node;
		}
		node_t *last=NULL;
		for (node_t *n=list->top; n; n=n->child)
		{
			if (node->f <= n->f)
			{
				break;
			}
			last = n;
		}
		if (last)
		{
			node->child = last->child;
			last->child = node;
		}
		else
		{
			node->child = list->top->child;
			list->top = node;
		}
	}
	return 0;
}

void node_remove(node_list_t *list, node_t *node)
{
	if (list && node)
	{
		node_t *last=NULL;
		for (node_t *child=list->top; child; child=child->child)
		{
			if (child->x == node->x && child->y == node->y)
			{
				break;
			}
			last = child;
		}
		if (last)
		{
			last->child = node->child;
			node->child = NULL;
		}
		else
		{
			list->top = list->top->child;
			node->child = NULL;
		}
	}
}

node_t* node_pop(node_list_t *list)
{
	if (list==NULL || list->top==NULL)
		return NULL;

	node_t *node = list->top;
	list->top = list->top->child;
	return node;
}

int list_not_empty(node_list_t *list)
{
	return list && list->top!=NULL;
}

int astart_seach(astar_t *a, int m[][MAP_SIZE], node_t* end)
{
	if (a == NULL || m == NULL)
	{
		return -1;
	}
	// open start
	node_t *start = node_cearte(a->sx, a->sy, 0, h(a->sx, a->sy, a->ex, a->ey), NULL);
	if (start == NULL)
		return -1;
	node_add(&a->open, start);

	node_t *nd = NULL;
	while(list_not_empty(&a->open))
	{
		nd = node_pop(&a->open);
		if (nd == NULL)
			break;
		node_add(&a->close, nd);
		for (int i = 0; i < DIR_N; i++)
		{
			int x = nd->x + dir[i].x;
			int y = nd->y + dir[i].y;
			if (x>=0 && x<MAP_SIZE && y>=0 && y<MAP_SIZE)
			{
				if (m[x][y] || node_exist(&a->close, x, y))
					continue;

				node_t *op_node = node_exist(&a->open, x, y);
				if (op_node)
				{
					int g = nd->g + dir[i].score;
					if (g < op_node->g)
					{
						op_node->g = g;
						op_node->f = g + op_node->f;
						op_node->p = nd;
						node_remove(&a->open,op_node);
						node_add(&a->open,op_node);
					}
				}
				else
				{
					int g = nd->g + dir[i].score;
					op_node = node_cearte(x, y, g, h(x, y, a->ex, a->ey), nd);
					if (op_node == NULL)
						break;
					node_add(&a->open, op_node);
					if (x==a->ex && y==a->ey)
					{
						*end = *op_node;
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

int draw_map(int m[][MAP_SIZE], node_t* end)
{
	if (m && end)
	{
		for (node_t *p=end; p; p=p->p)
		{
			m[p->x][p->y] = 2;
		}
		for (int x = 0; x < MAP_SIZE; x++)
		{
			for (int y = 0; y < MAP_SIZE; y++)
			{
				switch(m[x][y])
				{
					case 0:
						printf(".");
						break;
					case 1:
						printf("W");
						break;
					case 2:
						printf("0");
						break;
				}
			}
			printf("\n");
		}
	}
	return 0;
}

static int map[MAP_SIZE][MAP_SIZE] = {
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0},
	{0, 1, 1, 0, 1, 1, 1},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
};

int main(int argc, char const *argv[])
{
	astar_t a;
	node_t end;
	memset(&a, 0, sizeof(astar_t));
	a.sx = 1;
	a.sy = 1;
	a.ex = 6;
	a.ey = 6;
	int r = astart_seach(&a, map, &end);
	if (r > 0)
	{
		draw_map(map, &end);
	}
	list_free(&a.open);
	list_free(&a.close);
	return 0;
}