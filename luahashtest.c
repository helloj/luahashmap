
#include "LuaHashMap.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>


int main(int argc, char* argv[])
{

	const char* ret_string = NULL;
	const size_t MAX_ARRAY_SIZE = 10;
	const char* key_array[MAX_ARRAY_SIZE];
	size_t ret_size;
	size_t i;

	fprintf(stderr, "create\n");
	LuaHashMap* hash_map = LuaHashMap_Create();

	fprintf(stderr, "LuaHashMap_InsertValueStringForKeyString\n");
	LuaHashMap_InsertValueStringForKeyString(hash_map, "value1", "key1");
	fprintf(stderr, "LuaHashMap_InsertValueStringForKeyString\n");

	LuaHashMap_InsertValueStringForKeyString(hash_map, "value2", "key2");
	fprintf(stderr, "LuaHashMap_InsertValueStringForKeyString\n");

	LuaHashMap_InsertValueStringForKeyString(hash_map, "value3", "key3");


	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = LuaHashMap_GetValueStringForKeyString(hash_map, "key1");
	assert(0 == strcmp("value1", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);
	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = LuaHashMap_GetValueStringForKeyString(hash_map, "key2");
	assert(0 == strcmp("value2", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);
	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = LuaHashMap_GetValueStringForKeyString(hash_map, "key3");
	assert(0 == strcmp("value3", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);

	ret_size = LuaHashMap_GetKeysString(hash_map, key_array, MAX_ARRAY_SIZE);
	assert(3 == ret_size);
	for(i=0; i<ret_size; i++)
	{
		fprintf(stderr, "Key[%d] is %s\n", i, key_array[i]);
	}

	


	LuaHashMap_Free(hash_map);

	return 0;
}
