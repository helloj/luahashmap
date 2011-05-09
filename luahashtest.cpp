
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

int DoKeyStringValueString()
{

	const char* ret_string = NULL;
	const size_t MAX_ARRAY_SIZE = 10;
	const char* key_array[MAX_ARRAY_SIZE];
	size_t ret_size;
	size_t i;

	fprintf(stderr, "create\n");
	lhm::lua_hash_map<const char*, const char*> hash_map;


	fprintf(stderr, "insert 1\n");
	hash_map.insert(std::pair<const char*, const char*>("key1", "value1"));
						
//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value1", "key1");
	fprintf(stderr, "insert 2\n");
	hash_map.insert(std::pair<const char*, const char*>("key2", "value2"));

//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value2", "key2");
	fprintf(stderr, "insert 3\n");

//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value3", "key3");
//	LuaHashMap_InsertValueStringForKeyString(hash_map, NULL, "key3");
	hash_map.insert(std::pair<const char*, const char*>("key3", "value3"));

	
	
	ret_size = hash_map.size();
	assert(3 == ret_size);
	fprintf(stderr, "size=%d\n", ret_size);
	
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR

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
	

#endif
	
//	lhm::lua_hash_map<const char*, void*>::iterator iter = hash_map.find("key3");

	lhm::lua_hash_map<const char*, const char*>::iterator iter;

	iter = hash_map.find("key1");
	fprintf(stderr, "*iter (pair)=%s, %s\n", (*iter).first, (*iter).second);
	assert(0 == Internal_safestrcmp("value1", (*iter).second));

	iter = hash_map.find("key2");
	fprintf(stderr, "*iter (pair)=%s, %s\n", (*iter).first, (*iter).second);
	assert(0 == Internal_safestrcmp("value2", (*iter).second));

	iter = hash_map.find("key3");
	fprintf(stderr, "*iter (pair)=%s, %s\n", (*iter).first, (*iter).second);
	assert(0 == Internal_safestrcmp("value3", (*iter).second));


	iter = hash_map.find("key3");
	std::pair<const char*, const char*> ret_pair = *iter;

	
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


static void* s_valuePointer1 = (void*)0x1;
static void* s_valuePointer2 = (void*)0x2;
static void* s_valuePointer3 = (void*)0x3;
static void* s_valuePointer4 = (void*)0x4;

int DoKeyStringValuePointer()
{
	
	const char* ret_string = NULL;
	const size_t MAX_ARRAY_SIZE = 10;
	const char* key_array[MAX_ARRAY_SIZE];
	size_t ret_size;
	size_t i;
	
	fprintf(stderr, "create\n");
	lhm::lua_hash_map<const char*, void*> hash_map;
	
	
	fprintf(stderr, "insert 1\n");
	hash_map.insert(std::pair<const char*, void*>("key1", s_valuePointer1));
	
	//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value1", "key1");
	fprintf(stderr, "insert 2\n");
	hash_map.insert(std::pair<const char*, void*>("key2", s_valuePointer2));
	
	//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value2", "key2");
	fprintf(stderr, "insert 3\n");
	
	//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value3", "key3");
	//	LuaHashMap_InsertValueStringForKeyString(hash_map, NULL, "key3");
	hash_map.insert(std::pair<const char*, void*>("key3", s_valuePointer3));
	
	
	
	ret_size = hash_map.size();
	assert(3 == ret_size);
	fprintf(stderr, "size=%d\n", ret_size);

	
	//	lhm::luahashmap_iterator<lhm::lua_hash_map<const char*, const char*> > iter = hash_map.find("key3");
	lhm::lua_hash_map<const char*, void*>::iterator iter = hash_map.find("key3");
	
	std::pair<const char*, void*> ret_pair = *iter;
	fprintf(stderr, "*iter (pair)=%s, %x\n", ret_pair.first, ret_pair.second);
	
	
	
	
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
	
	
	hash_map.insert(std::pair<const char*, void*>("key2", s_valuePointer2));
	ret_size = hash_map.size();
	assert(2 == ret_size);
	
	hash_map.insert(std::pair<const char*, void*>("key4", s_valuePointer4));
	ret_size = hash_map.size();
	assert(3 == ret_size);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		fprintf(stderr, "*iter (pair)=%s, %x\n", (*iter).first, (*iter).second);
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

int main(int argc, char* argv[])
{
	DoKeyStringValueString();
	DoKeyStringValuePointer();
	return 0;
}
