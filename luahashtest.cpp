
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

	
	
	ret_size = hash_map.size();
	assert(3 == ret_size);
	fprintf(stderr, "size=%d\n", ret_size);
	
	
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


//	lhm::luahashmap_iterator<lhm::lua_hash_map<const char*, const char*> > iter = hash_map.find("key3");
	lhm::lua_hash_map<const char*, const char*>::iterator iter = hash_map.find("key3");

	std::pair<const char*, const char*> ret_pair = *iter;
//	ret_string = hash_map["key1"];
//	assert(0 == Internal_safestrcmp("value1", ret_string));
	fprintf(stderr, "*iter (pair)=%s, %s\n", ret_pair.first, ret_pair.second);



	
	fprintf(stderr, "erasing key3\n");
	ret_size = hash_map.erase(iter);
	assert(1 == ret_size);
	
	ret_size = hash_map.size();
	assert(2 == ret_size);
	
	fprintf(stderr, "size=%d\n", ret_size);

	fprintf(stderr, "erasing key3 again\n");
	ret_size = hash_map.erase("key3");
	assert(0 == ret_size);


	fprintf(stderr, "erasing key2\n");
	ret_size = hash_map.erase("key2");
	assert(1 == ret_size);

	
	hash_map.insert(std::pair<const char*, const char*>("key2", "value2"));
	ret_size = hash_map.size();
	assert(2 == ret_size);

	hash_map.insert(std::pair<const char*, const char*>("key4", "value4"));
	ret_size = hash_map.size();
	assert(3 == ret_size);

	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		fprintf(stderr, "*iter (pair)=%s, %s\n", (*iter).first, (*iter).second);
	}
	
	
	assert(false == hash_map.empty());
	fprintf(stderr, "IsEmpty should be no: %d\n", hash_map.empty());


	hash_map.clear();
	ret_size = hash_map.size();
	assert(0 == ret_size);

	assert(true == hash_map.empty());
	fprintf(stderr, "IsEmpty should be yes: %d\n", hash_map.empty());

		

	return 0;
}
