#include "json.h"
#include "string.h"

json_value * json_obj_get(json_value *json, const char *index)
{
	if (json->type != json_object)
		return json_none;

    for (unsigned int i = 0; i < json->u.object.length; ++ i)
       if (!strcmp (json->u.object.values [i].name, index))
          return json->u.object.values [i].value;

    return json_none;
}
