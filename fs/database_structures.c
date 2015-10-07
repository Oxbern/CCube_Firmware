#include "database_structures.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

point_t* new_point_queue(uint8_t x, uint8_t y, uint8_t z, point_t** head)
{
	point_t* p = malloc(sizeof(point_t));
	p->x = x;
	p->y = y;
	p->z = z;
	p->next = NULL;
	if (*head == NULL)
	{
		*head = p;
		return p;
	}
	point_t* current = *head;
	// look for the last element of the list
	while(current->next != NULL)
		current = current->next;
	current->next = p;
	return p;
}

point_t* free_point(point_t* point)
{
	point_t* next = point->next;
	free(point);
	return next;
}

option_t* new_option_queue(char* name, uint32_t val, option_t** head)
{
	option_t* o = malloc(sizeof(option_t));
	o->name = malloc(strlen(name));
	o->name = strcpy(o->name, name);
	o->value = val;
	o->next = NULL;
	if (*head == NULL)
	{
		*head = o;
		return o;
	}
	option_t* current = *head;
	while(current->next != NULL)
		current = current->next;
	current->next = o;
	return o;
}

option_t* free_option(option_t* option)
{
	option_t* next = option->next;
	free(option->name);
	free(option);
	return next;
}

motif_t* new_motif_queue(char* name, char* desc, char* image, point_t* points, option_t* options, motif_t** head)
{
	motif_t* m = malloc(sizeof(motif_t));
	m->name = malloc(strlen(name));
	m->name = strcpy(m->name, name);
	m->desc = malloc(strlen(desc));
	m->desc = strcpy(m->desc, desc);
	m->image = malloc(strlen(image));
	m->image = strcpy(m->image, image);
	m->points = points;
	m->options = options;
	m->next = NULL;
	if (*head == NULL)
	{
		*head = m;
		return m;
	}
	motif_t* current = *head;
	while(current->next != NULL)
		current = current->next;
	current->next = m;
	return m;
}

motif_t* free_motif(motif_t* motif)
{
	motif_t* next = motif->next;
	free(motif->name);
	free(motif->desc);
	free(motif->image);
	point_t* p = motif->points;
	while (p != NULL)
		p = free_point(p);
	option_t* o = motif->options;
	while (o != NULL)
		o = free_option(o);
	free(motif);
	return next;
}

group_t* new_group_queue(char* name, uint32_t nb_motifs, motif_t* motifs, group_t** head)
{
	group_t* g = malloc(sizeof(group_t));
	g->name = malloc(strlen(name));
	g->name = strcpy(g->name, name);
	g->nb_motifs = nb_motifs;
	g->motifs = motifs;
	g->next = NULL;
	if (*head == NULL)
	{
		*head = g;
		return g;
	}
	group_t* current = *head;
	while(current->next != NULL)
		current = current->next;
	current->next = g;
	return g;
}

group_t* free_group(group_t* group)
{
	group_t* next = group->next;
	free(group->name);
	motif_t* m = group->motifs;
	while (m != NULL)
		m = free_motif(m);
	return next;
}

database_t* new_database(char* name, uint32_t nb_motifs, motif_t* motifs)
{
	database_t* database = malloc(sizeof(database_t));
	database->name = malloc(strlen(name));
	database->name = strcpy(database->name, name);
	database->nb_motifs = nb_motifs;
	database->motifs = motifs;
	return database;
}

void free_database(database_t* database)
{
	free(database->name);
	motif_t* g = database->motifs;
	while (g != NULL)
		g = free_motif(g);
	free(database);
}
