#include "list.h"

#include <stdlib>

list_t * add_queue(list_t * list, void * value)
{
	if (list == NULL)
		return NULL;
	list * current = list;
	while (current->next != NULL)
		current = current->next;
	list_t new_element = malloc(sizeof(list_t));
	current->next = new_element;
	new_element = malloc(sizeof(value));
}
