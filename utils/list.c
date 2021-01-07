#include <stdlib.h>
#include <string.h>
#include "list.h"

list_node* list_create(int fd)
{
	list_node *l = malloc(sizeof(list_node));
	if (l != NULL) {
		l->next = NULL;
        l->fd = fd;
	}
	return l;
}

void list_destroy(list_node **list)
{
    list_node *current = NULL;
    if (list == NULL)
        return;
    while((*list)->next)
    {
        current = *list;
        (*list) = (*list)->next;
        free(current);
    }
}

list_node* list_add(list_node *list, int fd)
{
	list_node *new_node = list_create(fd);
	if (new_node != NULL) 
    {
        while(list->next)
            list = list->next;
        list->next = new_node;
	}
	return new_node;
}


void list_remove_by_fd(list_node **list, int fd)
{
	list_node *before = NULL, *current = NULL;
    if (list == NULL || *list == NULL) 
        return;

    // we will never remove "head" node, so this if is not needed, but I will leave it to be prepared for every case ;)
    if((*list)->fd == fd)
    {
        current = *list;
        (*list) = (*list)->next;
        free(current);
    }
    else
    {
        before = current = *list;
        while (current->next) 
        {
            if (current->fd != fd)
            {
                before = current;
                current = current->next;
            }
            else
            {
                if (current->next) 
                {
                    before->next = current->next;
                    free(current);
                }
                else
                {
                    free(current);
                    before->next = NULL;
                }
                break;
            }
        }        
    }    
}


list_node* list_find_by_fd(list_node *list, int fd)
{
	while (list) {
		if (list->fd == fd) 
            break;
		list = list->next;
	}
	return list;
}
