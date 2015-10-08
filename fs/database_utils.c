#include "database_utils.h"
#include "fatfs.h"
#include "json.h"
#include "json-builder.h"
#include "json_utils.h"
#include "database_structures.h"

#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

char* file2string(char* filename)
{
	FIL my_file;
	if (f_open(&my_file, filename, FA_READ) != FR_OK)
	{
		return NULL;
	} else {
		// kwerky way to get clean file size
		uint32_t file_size = f_size(&my_file);
		char* str = malloc(file_size);
		
		// read the file in a buffer
		f_lseek(&my_file, 0);
		uint32_t bytesread; // TODO check bytesead agains lenght
		FRESULT res = f_read(&my_file, str, file_size, (UINT*)&bytesread);
		str[file_size-1] = '\0';
		f_close(&my_file);
		if (res != FR_OK)
		{
			return NULL;
		} else {
			return str;
		}
	}
}


// TODO return some different error codes for different problems
// instead of just a freaking NULL
database_t* string2database(char* json)
{	
	database_t* db;

	json_settings settings = {};
	settings.value_extra = json_builder_extra;

	char error[128];
	json_value * arr = json_parse_ex(&settings, json, strlen(json), error);

	// check file version number
	//char ver[tok[1].end-tok[1].start+1];
	//memcpy(ver, json+tok[1].start, tok[1].end-tok[1].start+1);
	//ver[tok[1].end-tok[1].start] = '\0';
	//bool ver_ok = !strcmp(ver, "1.00");

	json_value * ver = json_obj_get(arr, "version");
	if (ver == json_none)
		return NULL;
	bool ver_ok = !strcmp(ver->u.string.ptr, "1");

	if (!ver_ok) {
		return NULL;
	} else {
		// get database name and number of groups
		json_value * name = json_obj_get(arr, "name");
		char * db_name = malloc(name->u.string.length);
		strcpy(db_name, name->u.string.ptr);

		//group_t *groups = NULL;
		//uint32_t err = parse_groups(json, tok, 3, &groups);
		//if (err < 0)
		//	return NULL;
		
		json_value * children = json_obj_get(arr, "children");
		uint32_t nb_children = children->u.array.length;

		motif_t *motifs = NULL;
		parse_motifs(children, &motifs);

		db = new_database(db_name, nb_children, motifs);
	}

	return db;
}


uint32_t parse_points(json_value * points_array, point_t** points)
{
	uint32_t nb_points = points_array->u.array.length;
	*points = NULL;

	for (int i=0; i<nb_points; i++)
	{
		uint8_t x = (uint8_t)(points_array->u.array.values[i]->u.array.values[0]->u.integer);
		uint8_t y = (uint8_t)(points_array->u.array.values[i]->u.array.values[1]->u.integer);
		uint8_t z = (uint8_t)(points_array->u.array.values[i]->u.array.values[2]->u.integer);
		
		new_point_queue(x,y,z,points);	
	}

	return 0;
}

uint32_t parse_options(json_value * options_array, option_t** options)
{
	uint32_t nb_options = options_array->u.array.length;
	*options = NULL;

	for (int i=0; i<nb_options; i++)
	{
		json_value * option = options_array->u.array.values[i];

		json_value * this_type = json_obj_get(option, "option");
		if (!strcmp(this_type->u.string.ptr, "blink"))
		{
			json_value * this_period = json_obj_get(option, "period");
			json_value * this_point = json_obj_get(option, "point");
			new_blink_option_queue(	this_period->u.integer,
									this_point->u.array.values[0]->u.integer,
									this_point->u.array.values[1]->u.integer,
									this_point->u.array.values[2]->u.integer,
									options);
		} else
		if (!strcmp(this_type->u.string.ptr, "duplicate"))
		{
			json_value * this_i = json_obj_get(option, "i");
			json_value * this_j = json_obj_get(option, "j");
			json_value * this_k = json_obj_get(option, "k");
			new_duplicate_option_queue(
									this_i->u.array.values[0]->u.integer,this_i->u.array.values[1]->u.integer,this_i->u.array.values[2]->u.integer,
									this_j->u.array.values[0]->u.integer,this_j->u.array.values[1]->u.integer,this_j->u.array.values[2]->u.integer,
									this_k->u.array.values[0]->u.integer,this_k->u.array.values[1]->u.integer,this_k->u.array.values[2]->u.integer,
									options);
		}
	}
	
	return 0;
}


uint32_t parse_motifs(json_value * motifs_array, motif_t** motifs)
{
	uint32_t nb_motifs = motifs_array->u.array.length;
	*motifs = NULL;

	for (int i=0; i<nb_motifs; i++)
	{

		json_value * motif = motifs_array->u.array.values[i];

		json_value * this_name = json_obj_get(motif, "name");
		char * motif_name = malloc(this_name->u.string.length);
		strcpy(motif_name, this_name->u.string.ptr);

		json_value * this_desc = json_obj_get(motif, "description");
		char * motif_desc = malloc(this_desc->u.string.length);
		strcpy(motif_desc, this_desc->u.string.ptr);

		json_value * this_image = json_obj_get(motif, "image");
		char * motif_image = malloc(this_image->u.string.length);
		strcpy(motif_image, this_image->u.string.ptr);

		point_t* points = NULL;
		json_value * this_points = json_obj_get(motif, "points");
		parse_points(this_points, &points);

		option_t* options = NULL;
		json_value * this_options = json_obj_get(motif, "options");
		parse_options(this_options, &options);

		new_motif_queue(motif_name, motif_desc, motif_image, points, options, motifs);
	}

	return 0;
}
/*
uint32_t parse_groups(char* json, jsmntok_t* tok, uint32_t index, group_t** groups)
{
	*groups = NULL;

	uint32_t nb_groups = tok[index].size;

	index++;
	
	for (int i=0; i<nb_groups; i++)
	{
		index++;
		
		char* group_name = malloc(sizeof(char)*(tok[index].end-tok[index].start+1));
		memcpy(group_name, json+tok[index].start,tok[index].end-tok[index].start+1);
		group_name[tok[index].end-tok[index].start] = '\0';

		index++;

		uint32_t nb_motifs = tok[index].size;

		motif_t* motifs = NULL;
		index = parse_motifs(json, tok, index, &motifs);

		new_group_queue(group_name, nb_motifs, motifs, groups);
	}

	return index;
}
*/
