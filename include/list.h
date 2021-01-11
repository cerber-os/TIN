#ifndef _LIST_H
#define _LIST_H

#include <time.h>

typedef struct list_node {
	int fd;
	struct list_node *next;
} list_node;

list_node* list_create(int fd);
void list_destroy(list_node **list);
list_node* list_add(list_node *list, int fd);
list_node* list_remove_by_fd(list_node **list, int fd);
list_node* list_find_by_fd(list_node *list, int fd);

#endif
