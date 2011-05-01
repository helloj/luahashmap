
#include "lua_hash_map.hpp"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static int Internal_safestrcmp(const char* str1, const char* str2)
{
	if(NULL == str1 && NULL == str2)
	{
		return 0;
	}
	else if(NULL == str1)
	{
		return 1;
	}
	else if(NULL == str2)
	{
		return -1;
	}
	else
	{
		return strcmp(str1, str2);
	}
}

int main(int argc, char* argv[])
{

	const char* ret_string = NULL;
	const size_t MAX_ARRAY_SIZE = 10;
	const char* key_array[MAX_ARRAY_SIZE];
	size_t ret_size;
	size_t i;

	fprintf(stderr, "create\n");
	lhm::lua_hash_map<const char*, const char*> hash_map;


	fprintf(stderr, "LuaHashMap_InsertValueStringForKeyString\n");
	hash_map.insert(std::pair<const char*, const char*>("key1", "value1"));
						
//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value1", "key1");
	fprintf(stderr, "LuaHashMap_InsertValueStringForKeyString\n");
	hash_map.insert(std::pair<const char*, const char*>("key2", "value2"));

//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value2", "key2");
	fprintf(stderr, "LuaHashMap_InsertValueStringForKeyString\n");

//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value3", "key3");
//	LuaHashMap_InsertValueStringForKeyString(hash_map, NULL, "key3");
	hash_map.insert(std::pair<const char*, const char*>("key3", "value3"));

	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = hash_map["key1"];
	assert(0 == Internal_safestrcmp("value1", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);
	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = hash_map["key2"];
	assert(0 == Internal_safestrcmp("value2", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);
	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = hash_map["key3"];
	assert(0 == Internal_safestrcmp("value3", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);
//	hash_map["key3"] = "fee";
//	ret_string = hash_map["key3"];
//	fprintf(stderr, "ret_string=%s\n", ret_string);


	lhm::luahashmap_iterator<lhm::lua_hash_map<const char*, const char*> > iter = hash_map.find("key3");

	std::pair<const char*, const char*> ret_pair = *iter;
//	ret_string = hash_map["key1"];
//	assert(0 == Internal_safestrcmp("value1", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_pair.second);

	
#if 0

	ret_size = LuaHashMap_GetKeysString(hash_map, key_array, MAX_ARRAY_SIZE);
	assert(3 == ret_size);
	for(i=0; i<ret_size; i++)
	{
		fprintf(stderr, "Key[%d] is %s\n", i, key_array[i]);
	}

	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));


	LuaHashMap_Clear(hash_map);
	ret_size = LuaHashMap_GetKeysString(hash_map, key_array, MAX_ARRAY_SIZE);
	assert(0 == ret_size);


	assert(1 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be yes: %d\n", LuaHashMap_IsEmpty(hash_map));
	LuaHashMap_InsertValueStringForKeyString(hash_map, "value3", NULL);
		

#endif

	return 0;
}
