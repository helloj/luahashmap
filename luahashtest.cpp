
#include "lua_hash_map.hpp"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <iostream>

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
	size_t ret_val;
	size_t i;

	std::cerr << "create\n";
	lhm::lua_hash_map<const char*, const char*> hash_map;


	std::cerr << "insert1\n";
	hash_map.insert(std::pair<const char*, const char*>("key1", "value1"));
						
//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value1", "key1");
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<const char*, const char*>("key2", "value2"));

//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value2", "key2");
	std::cerr << "insert3\n";

//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value3", "key3");
//	LuaHashMap_InsertValueStringForKeyString(hash_map, NULL, "key3");
	hash_map.insert(std::pair<const char*, const char*>("key3", "value3"));

	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	const char* ret_string = NULL;

	std::cerr << "bracket[]\n";

	ret_string = hash_map["key1"];
	assert(0 == Internal_safestrcmp("value1", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;

	std::cerr << "bracket[]\n";

	ret_string = hash_map["key2"];
	assert(0 == Internal_safestrcmp("value2", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	std::cerr << "bracket[]\n";

	ret_string = hash_map["key3"];
	assert(0 == Internal_safestrcmp("value3", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	
	//	hash_map["key3"] = "fee";
	//	ret_string = hash_map["key3"];
	// std::cerr << "ret_string=" << ret_string << std::endl;
	

#endif
	
//	lhm::lua_hash_map<const char*, void*>::iterator iter = hash_map.find("key3");

	lhm::lua_hash_map<const char*, const char*>::iterator iter;

	iter = hash_map.find("key1");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value1", (*iter).second));

	iter = hash_map.find("key2");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value2", (*iter).second));

	iter = hash_map.find("key3");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value3", (*iter).second));


	iter = hash_map.find("key3");
	std::pair<const char*, const char*> ret_pair = *iter;

	
	std::cerr << "erasing key3\n";
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;

	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase("key3");
	assert(0 == ret_val);


	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase("key2");
	assert(1 == ret_val);

	
	hash_map.insert(std::pair<const char*, const char*>("key2", "value2"));
	ret_val = hash_map.size();
	assert(2 == ret_val);

	hash_map.insert(std::pair<const char*, const char*>("key4", "value4"));
	ret_val = hash_map.size();
	assert(3 == ret_val);

	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;


	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);

	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;

		

	return 0;
}


static void* s_valuePointer1 = (void*)0x1;
static void* s_valuePointer2 = (void*)0x2;
static void* s_valuePointer3 = (void*)0x3;
static void* s_valuePointer4 = (void*)0x4;

int DoKeyStringValuePointer()
{
	size_t ret_val;
	
	std::cerr << "create\n";

	lhm::lua_hash_map<const char*, void*> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<const char*, void*>("key1", s_valuePointer1));
	
	//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value1", "key1");
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<const char*, void*>("key2", s_valuePointer2));
	
	//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value2", "key2");
	std::cerr << "insert3\n";
	
	//	LuaHashMap_InsertValueStringForKeyString(hash_map, "value3", "key3");
	//	LuaHashMap_InsertValueStringForKeyString(hash_map, NULL, "key3");
	hash_map.insert(std::pair<const char*, void*>("key3", s_valuePointer3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;

	
	lhm::lua_hash_map<const char*, void*>::iterator iter;

	iter = hash_map.find("key1");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;

	assert(0x1 == (int)(*iter).second);
	
	iter = hash_map.find("key2");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0x2 == (int)(*iter).second);
	
	iter = hash_map.find("key3");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0x3 == (int)(*iter).second);
	

	iter = hash_map.find("key3");
	
	std::pair<const char*, void*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;

	
	
	
	std::cerr << "erasing key3\n";

	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase("key3");
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase("key2");
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<const char*, void*>("key2", s_valuePointer2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<const char*, void*>("key4", s_valuePointer4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;

	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}

int DoKeyStringValueNumber()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<const char*, lua_Number> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<const char*, lua_Number>("key1", 1.1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<const char*, lua_Number>("key2", 2.2));
	
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<const char*, lua_Number>("key3", 3.3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	
	lhm::lua_hash_map<const char*, lua_Number>::iterator iter;
	
	iter = hash_map.find("key1");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1.1 == (*iter).second);
	
	iter = hash_map.find("key2");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2.2 == (*iter).second);
	
	iter = hash_map.find("key3");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3.3 == (*iter).second);
	

	iter = hash_map.find("key3");
	
	std::pair<const char*, lua_Number> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase("key3");
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase("key2");
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<const char*, lua_Number>("key2", 2.5));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<const char*, lua_Number>("key4", 4.4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int DoKeyStringValueInteger()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<const char*, lua_Integer> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<const char*, lua_Integer>("key1", 1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<const char*, lua_Integer>("key2", 2));
	
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<const char*, lua_Integer>("key3", 3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	
	lhm::lua_hash_map<const char*, lua_Integer>::iterator iter;
	
	iter = hash_map.find("key1");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1 == (*iter).second);
	
	iter = hash_map.find("key2");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2 == (*iter).second);
	
	iter = hash_map.find("key3");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3 == (*iter).second);
	
	
	iter = hash_map.find("key3");
	
	std::pair<const char*, lua_Integer> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase("key3");
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase("key2");
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<const char*, lua_Integer>("key2", 2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<const char*, lua_Integer>("key4", 4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int main(int argc, char* argv[])
{
	DoKeyStringValueString();
	DoKeyStringValuePointer();
	DoKeyStringValueNumber();
	DoKeyStringValueInteger();
	return 0;
}
