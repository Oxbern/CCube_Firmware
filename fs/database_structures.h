#ifndef _DATABASE_STRUCTURES_H_
#define _DATABASE_STRUCTURES_H_

#include <inttypes.h>


typedef struct point {
	uint8_t x, y, z;
	struct point* next;
} point_t;

enum option_type {
	BLINK,
	DUPLICATE
};

typedef struct blink_params {
	uint32_t period;
	uint8_t x, y, z;
} blink_params_t;

typedef struct duplicate_params {
	uint8_t ix, iy, iz;
	uint8_t jx, jy, jz;
	uint8_t kx, ky, kz;
} duplicate_params_t;

typedef struct option {
	enum option_type type;
	void * params;
	struct option* next;
} option_t;

typedef struct motif {
	char* name;
	char* desc;
	char* image;
	point_t* points;
	option_t* options;
	struct motif* next;
} motif_t;

typedef struct group {
	char* name;
	uint32_t nb_motifs;
	motif_t* motifs;
	struct group* next;
} group_t;

typedef struct database {
	char* name;
	uint32_t nb_motifs;
	motif_t* motifs;
} database_t;


point_t* new_point_queue(uint8_t x, uint8_t y, uint8_t z, point_t** head);

point_t* free_point(point_t* point);

option_t* new_blink_option_queue(uint32_t period, uint8_t x, uint8_t y, uint8_t z, option_t** head);

option_t* new_duplicate_option_queue(uint8_t ix, uint8_t iy, uint8_t iz, uint8_t jx, uint8_t jy, uint8_t jz, uint8_t kx, uint8_t ky, uint8_t kz, option_t** head);

option_t* free_option(option_t* option);

motif_t* new_motif_queue(char* name, char* desc, char* image, point_t* points, option_t* options, motif_t** head);

motif_t* free_motif(motif_t* motif);

group_t* new_group_queue(char* name, uint32_t nb_motifs, motif_t* motifs, group_t** head);

group_t* free_group(group_t* group);

database_t* new_database(char* name, uint32_t nb_motifs, motif_t* motifs);

void free_database(database_t* database);

#endif
