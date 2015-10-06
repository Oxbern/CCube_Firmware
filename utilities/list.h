#ifndef __LISTS_H_
#define __LISTS_H_

typedef void (*freeFunction)(void *);

typedef bool (*listIterator)(void *);

typedef struct list_node {
	void * value;
	struct list * next;
} list_node_t;

typedef struct {
	uint32_t logical_lenght;
	uint32_t element_size;
	list_node_t *head;
	list_node_t *tail;
	freeFunction freeFn;
} list;

void list_new(list * list, int element_size, freeFunction freeFn);
void list_destroy(list *list);

void list_prepend(list * list, void * element);
void list_append(list * list, void * element);
uint32_t list_size(list * list);

void list_for_each(list * list, listIterator iterator);
void list_head(list *list, void *element, bool removeFromList);
void list_tail(list *list, void *element);

#endif
