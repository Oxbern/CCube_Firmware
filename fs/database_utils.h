#ifndef _DATABASE_UTILS_H_
#define _DATABASE_UTILS_H_

#include "stdint.h"
#include "database_structures.h"
#include "json.h"


/**
 * Don't forget to free ressources you don't need! :p
 * returns NULL if shit got real
 */
char* file2string(char* filename);

/**
 * parse string return database. simple.
 * oh and don't forget to free ;p
 */
database_t* string2database(char* json);

uint32_t parse_points(json_value * points_array, point_t** points);
//uint32_t parse_options(char* json, jsmntok_t* tok, uint32_t index, option_t** options);
uint32_t parse_motifs(json_value * motifs_array, motif_t** motifs);
//uint32_t parse_groups(char* json, jsmntok_t* tok, uint32_t index, group_t** groups);

#endif
