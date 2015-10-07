#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "json.h"
#include "json-builder.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
/*
static bool correct_extention(char * filename)
{
	char * point;
	if((point = strrchr(filename, '.')) != NULL)
	{
		if ((strcmp(point, ".ccdb") == 0) || (strcmp(point, ".lua") == 0))
		{
			return true;
		} else {
			return false;
		}	
	} else {
		return false;
	}
}*/

typedef struct folder {
	char * name;
	char * path;
	struct folder* next;
} folder_t;

folder_t * folders_to_check = NULL;

folder_t * new_folder(char * name, char * path)
{
	folder_t * nfold = malloc(sizeof(folder_t));
	nfold->name = malloc(strlen(name));
	strcpy(nfold->name, name);
	nfold->path = malloc(strlen(path));
	strcpy(nfold->path, path);
	nfold->next = NULL;
	return nfold;
}

void del_folder(folder_t * fold)
{
	free(fold->name);
	free(fold->path);
	free(fold);
}


/**
 * Require list != NULL
 */
void append_folder(folder_t * list, folder_t * fold)
{
	folder_t * current = list;
	while (current->next != NULL)
		current = current->next;
	current->next = fold;
}


/**
 * Require list != NULL
 */
folder_t * pop_folder(folder_t ** list)
{
	folder_t * next = (*list)->next;
	folder_t * to_pop = *list;
	to_pop->next = NULL;
	*list = next;
	return to_pop;
}


