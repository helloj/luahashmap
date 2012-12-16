
#include "LuaHashMap.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>


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




#include <stddef.h>

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)

int DoKeyStringValueString()
{
	fprintf(stderr, "DoKeyStringValueString()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	const char* ret_val;
	size_t ret_count;
	bool exists;
	
	// String literals are a problem because the type is char[] and not char*.
	// An explicit typecast to char* must be used or the default case will be hit which goes to Pointer
	// http://www.robertgamble.net/2012/01/c11-generic-selections.html
	const char* somekey = "key1";
	char* somekey_nonconst = (char*)somekey; // testing non-const
	fprintf(stderr, "LuaHashMap_SetValueStringForKeyString\n");
	LuaHashMap_SetValueForKey(hash_map, (const char*)("value1"), somekey_nonconst);

	fprintf(stderr, "LuaHashMap_SetValueStringForKeyString\n");
	
	LuaHashMap_SetValueForKey(hash_map, (const char*)("value2"), (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_SetValueStringForKeyString\n");
	
	LuaHashMap_SetValueForKey(hash_map, (char*)("value3"), (char*)("key3"));

	

	
	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, (const char*)("key1"));
	assert(0 == Internal_safestrcmp("value1", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, (const char*)("key2"));
	assert(0 == Internal_safestrcmp("value2", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, (const char*)("key3"));
	assert(0 == Internal_safestrcmp("value3", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));

	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %s\n", LuaHashMap_GetValueStringAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	

	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%d\n", ret_count);
	assert(3 == ret_count);

	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);

	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	

	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, (const char*)("key2"));
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %s\n", LuaHashMap_GetValueStringAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%d\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);

	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	

	LuaHashMap_SetValueAtIterator(&the_iterator, (const char*)("value4"));

	ret_val = LuaHashMap_GetValueStringForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	assert(0 == Internal_safestrcmp("value4", ret_val));

	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

static void* s_valuePointer1 = (void*)0x1;
static void* s_valuePointer2 = (void*)0x2;
static void* s_valuePointer3 = (void*)0x3;
static void* s_valuePointer4 = (void*)0x4;


int DoKeyStringValuePointer()
{
	fprintf(stderr, "DoKeyStringValuePointer()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	void* ret_val;
	size_t ret_count;
	bool exists;
	
	// String literals are a problem because the type is char[] and not char*.
	// An explicit typecast to char* must be used or the default case will be hit which goes to Pointer
	// http://www.robertgamble.net/2012/01/c11-generic-selections.html
	const char* somekey = "key1";
	char* somekey_nonconst = (char*)somekey; // testing non-const

	LuaHashMap_SetValueForKey(hash_map, s_valuePointer1, somekey_nonconst);
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer2, (const char*)("key2"));
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer3, (char*)("key3"));


	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, (const char*)("key1"));
	assert(s_valuePointer1 == ret_val);
	fprintf(stderr, "ret_val=%d\n", ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, (const char*)("key2"));
	assert(s_valuePointer2 == ret_val);
	fprintf(stderr, "ret_val=%d\n", ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "ret_val=%d\n", ret_val);
	assert(s_valuePointer3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %d\n", LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%d\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, (const char*)("key2"));
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %d\n", LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%d\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, s_valuePointer4);
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "ret_val=%d\n", ret_val);
	assert(s_valuePointer4 == ret_val);
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

int DoKeyStringValueNumber()
{
	fprintf(stderr, "DoKeyStringValueNumber()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	lua_Number ret_val;
	size_t ret_count;
	bool exists;
	
	// String literals are a problem because the type is char[] and not char*.
	// An explicit typecast to char* must be used or the default case will be hit which goes to Pointer
	// http://www.robertgamble.net/2012/01/c11-generic-selections.html
	const char* somekey = "key1";
	char* somekey_nonconst = (char*)somekey; // testing non-const
	
	LuaHashMap_SetValueForKey(hash_map, 1.0, somekey_nonconst);
	LuaHashMap_SetValueForKey(hash_map, 2.2, (const char*)("key2"));
	LuaHashMap_SetValueForKey(hash_map, 3.3, (char*)("key3"));
	
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, (const char*)("key1"));
	assert(1.0 == ret_val);
	fprintf(stderr, "ret_val=%f\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, (const char*)("key2"));
	assert(2.2 == ret_val);
	fprintf(stderr, "ret_val=%f\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "ret_val=%f\n", ret_val);
	assert(3.3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %f\n", LuaHashMap_GetValueNumberAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%d\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, (const char*)("key2"));
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %f\n", LuaHashMap_GetValueNumberAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%d\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	

	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, 4.4);
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "ret_val=%f\n", ret_val);
	assert(4.4 == ret_val);
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

int DoKeyStringValueInteger()
{
	fprintf(stderr, "DoKeyStringValueInteger()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	lua_Integer ret_val;
	size_t ret_count;
	bool exists;
	
	// String literals are a problem because the type is char[] and not char*.
	// An explicit typecast to char* must be used or the default case will be hit which goes to Pointer
	// http://www.robertgamble.net/2012/01/c11-generic-selections.html
	const char* somekey = "key1";
	char* somekey_nonconst = (char*)somekey; // testing non-const
	
	LuaHashMap_SetValueForKey(hash_map, 1, somekey_nonconst);
	LuaHashMap_SetValueForKey(hash_map, 2, (const char*)("key2"));
	LuaHashMap_SetValueForKey(hash_map, 3, (char*)("key3"));
	
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, (const char*)("key1"));
	assert(1 == ret_val);
	fprintf(stderr, "ret_val=%d\n", ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, (const char*)("key2"));
	assert(2 == ret_val);
	fprintf(stderr, "ret_val=%d\n", ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "ret_val=%d\n", ret_val);
	assert(3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %d\n", LuaHashMap_GetValueIntegerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%d\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, (const char*)("key2"));
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %d\n", LuaHashMap_GetValueIntegerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%d\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, 4);
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "ret_val=%d\n", ret_val);
	assert(4 == ret_val);
	
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

#if 0


static void* s_keyPointer1 = (void*)0x1;
static void* s_keyPointer2 = (void*)0x2;
static void* s_keyPointer3 = (void*)0x3;
static void* s_keyPointer4 = (void*)0x4;

int DoKeyPointerValuePointer()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<void*, void*> hash_map;
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<void*, void*>(s_keyPointer1, s_valuePointer1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<void*, void*>(s_keyPointer2, s_valuePointer2));
	
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<void*, void*>(s_keyPointer3, s_valuePointer3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<void*, void*>::iterator iter;
	
	iter = hash_map.find(s_keyPointer1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert((void*)0x1 == (void*)(*iter).second);
	
	iter = hash_map.find(s_keyPointer2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x2 == (void*)(*iter).second);
	
	iter = hash_map.find(s_keyPointer3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x3 == (void*)(*iter).second);
	
	
	iter = hash_map.find(s_keyPointer3);
	
	std::pair<void*, void*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(s_keyPointer3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(s_keyPointer2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<void*, void*>(s_keyPointer2, s_valuePointer2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<void*, void*>(s_keyPointer4, s_valuePointer4));
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

struct SomeClass
{
	int someInt;
};


static SomeClass* s_keyPointerS1 = (SomeClass*)0x1;
static SomeClass* s_keyPointerS2 = (SomeClass*)0x2;
static SomeClass* s_keyPointerS3 = (SomeClass*)0x3;
static SomeClass* s_keyPointerS4 = (SomeClass*)0x4;

static SomeClass* s_valuePointerS1 = (SomeClass*)0x1;
static SomeClass* s_valuePointerS2 = (SomeClass*)0x2;
static SomeClass* s_valuePointerS3 = (SomeClass*)0x3;
static SomeClass* s_valuePointerS4 = (SomeClass*)0x4;


int DoKeyPointer2ValuePointer2()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<SomeClass*, SomeClass*> hash_map;
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<SomeClass*, SomeClass*>(s_keyPointerS1, s_valuePointerS1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<SomeClass*, SomeClass*>(s_keyPointerS2, s_valuePointerS2));
	
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<SomeClass*, SomeClass*>(s_keyPointerS3, s_valuePointerS3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<SomeClass*, SomeClass*>::iterator iter;
	
	iter = hash_map.find(s_keyPointerS1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert((void*)0x1 == (void*)(*iter).second);
	
	iter = hash_map.find(s_keyPointerS2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x2 == (void*)(*iter).second);
	
	iter = hash_map.find(s_keyPointerS3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x3 == (void*)(*iter).second);
	
	
	iter = hash_map.find(s_keyPointerS3);
	
	std::pair<void*, void*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(s_keyPointerS3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(s_keyPointerS2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<SomeClass*, SomeClass*>(s_keyPointerS2, s_valuePointerS2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<SomeClass*, SomeClass*>(s_keyPointerS4, s_valuePointerS4));
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


int DoKeyPointerValueString()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<void*, const char*> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<void*, const char*>(s_keyPointer1, "key1"));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<void*, const char*>(s_keyPointer2, "key2"));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<void*, const char*>(s_keyPointer3, "key3"));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<void*, const char*>::iterator iter;
	
	iter = hash_map.find(s_keyPointer1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
    assert(0 == Internal_safestrcmp("key1", (*iter).second));
	
	iter = hash_map.find(s_keyPointer2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
    assert(0 == Internal_safestrcmp("key2", (*iter).second));
	
	iter = hash_map.find(s_keyPointer3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
    assert(0 == Internal_safestrcmp("key3", (*iter).second));
	
	
	iter = hash_map.find(s_keyPointer3);
	
	std::pair<void*, const char*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(s_keyPointer3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(s_keyPointer2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<void*, const char*>(s_keyPointer2, "key2"));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<void*, const char*>(s_keyPointer4, "key4"));
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


int DoKeyPointerValueNumber()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<void*, lua_Number> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<void*, lua_Number>(s_keyPointer1, 1.1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<void*, lua_Number>(s_keyPointer2, 2.2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<void*, lua_Number>(s_keyPointer3, 3.3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<void*, lua_Number>::iterator iter;
	
	iter = hash_map.find(s_keyPointer1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1.1 == (*iter).second);
	
	iter = hash_map.find(s_keyPointer2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2.2 == (*iter).second);
	
	iter = hash_map.find(s_keyPointer3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3.3 == (*iter).second);
	
	
	iter = hash_map.find(s_keyPointer3);
	
	std::pair<void*, lua_Number> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(s_keyPointer3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(s_keyPointer2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<void*, lua_Number>(s_keyPointer2, 2.5));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<void*, lua_Number>(s_keyPointer4, 4.4));
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


int DoKeyPointerValueInteger()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<void*, lua_Integer> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<void*, lua_Integer>(s_keyPointer1, 1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<void*, lua_Integer>(s_keyPointer2, 2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<void*, lua_Integer>(s_keyPointer3, 3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<void*, lua_Integer>::iterator iter;
	
	iter = hash_map.find(s_keyPointer1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1 == (*iter).second);
	
	iter = hash_map.find(s_keyPointer2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2 == (*iter).second);
	
	iter = hash_map.find(s_keyPointer3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3 == (*iter).second);
	
	
	iter = hash_map.find(s_keyPointer3);
	
	std::pair<void*, lua_Integer> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(s_keyPointer3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(s_keyPointer2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<void*, lua_Integer>(s_keyPointer2, 2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<void*, lua_Integer>(s_keyPointer4, 4));
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







int DoKeyIntegerValuePointer()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Integer, void*> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Integer, void*>(1, s_valuePointer1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Integer, void*>(2, s_valuePointer2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Integer, void*>(3, s_valuePointer3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Integer, void*>::iterator iter;
	
	iter = hash_map.find(1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert((void*)0x1 == (void*)(*iter).second);
	
	iter = hash_map.find(2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x2 == (void*)(*iter).second);
	
	iter = hash_map.find(3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x3 == (void*)(*iter).second);
	
	
	iter = hash_map.find(3);
	
	std::pair<lua_Integer, void*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Integer, void*>(2, s_valuePointer2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Integer, void*>(4, s_valuePointer4));
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


int DoKeyIntegerValueString()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Integer, const char*> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Integer, const char*>(1, "value1"));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Integer, const char*>(2, "value2"));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Integer, const char*>(3, "value3"));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Integer, const char*>::iterator iter;
	
	iter = hash_map.find(1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(0 == Internal_safestrcmp("value1", (*iter).second));
	
	iter = hash_map.find(2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value2", (*iter).second));
	
	iter = hash_map.find(3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value3", (*iter).second));
	
	
	iter = hash_map.find(3);
	
	std::pair<lua_Integer, const char*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Integer, const char*>(2, "value2"));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Integer, const char*>(4, "value4"));
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



int DoKeyIntegerValueInteger()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Integer, lua_Integer> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Integer, lua_Integer>(1, 1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Integer, lua_Integer>(2, 2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Integer, lua_Integer>(3, 3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Integer, lua_Integer>::iterator iter;
	
	iter = hash_map.find(1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1 == (*iter).second);
	
	iter = hash_map.find(2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2 == (*iter).second);
	
	iter = hash_map.find(3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3 == (*iter).second);
	
	
	iter = hash_map.find(3);
	
	std::pair<lua_Integer, lua_Integer> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Integer, lua_Integer>(2, 2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Integer, lua_Integer>(4, 4));
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

int DoKeyIntegerValueNumber()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Integer, lua_Number> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Integer, lua_Number>(1, 1.1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Integer, lua_Number>(2, 2.2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Integer, lua_Number>(3, 3.3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Integer, lua_Number>::iterator iter;
	
	iter = hash_map.find(1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1.1 == (*iter).second);
	
	iter = hash_map.find(2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2.2 == (*iter).second);
	
	iter = hash_map.find(3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3.3 == (*iter).second);
	
	
	iter = hash_map.find(3);
	
	std::pair<lua_Integer, lua_Number> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Integer, lua_Number>(2, 2.5));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Integer, lua_Number>(4, 4.4));
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

















int DoKeyNumberValuePointer()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Number, void*> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Number, void*>(1.1, s_valuePointer1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Number, void*>(2.2, s_valuePointer2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Number, void*>(3.3, s_valuePointer3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Number, void*>::iterator iter;
	
	iter = hash_map.find(1.1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert((void*)0x1 == (void*)(*iter).second);
	
	iter = hash_map.find(2.2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x2 == (void*)(*iter).second);
	
	iter = hash_map.find(3.3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x3 == (void*)(*iter).second);
	
	
	iter = hash_map.find(3.3);
	
	std::pair<lua_Number, void*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3.3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2.2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Number, void*>(2.5, s_valuePointer2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Number, void*>(4.4, s_valuePointer4));
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


int DoKeyNumberValueString()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Number, const char*> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Number, const char*>(1.1, "value1"));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Number, const char*>(2.2, "value2"));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Number, const char*>(3.3, "value3"));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Number, const char*>::iterator iter;
	
	iter = hash_map.find(1.1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(0 == Internal_safestrcmp("value1", (*iter).second));
	
	iter = hash_map.find(2.2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value2", (*iter).second));
	
	iter = hash_map.find(3.3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value3", (*iter).second));
	
	
	iter = hash_map.find(3.3);
	
	std::pair<lua_Number, const char*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3.3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2.2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Number, const char*>(2.5, "value2"));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Number, const char*>(4.4, "value4"));
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



int DoKeyNumberValueInteger()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Number, lua_Integer> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Number, lua_Integer>(1.1, 1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Number, lua_Integer>(2.2, 2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Number, lua_Integer>(3.3, 3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Number, lua_Integer>::iterator iter;
	
	iter = hash_map.find(1.1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1 == (*iter).second);
	
	iter = hash_map.find(2.2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2 == (*iter).second);
	
	iter = hash_map.find(3.3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3 == (*iter).second);
	
	
	iter = hash_map.find(3.3);
	
	std::pair<lua_Number, lua_Integer> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3.3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2.2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Number, lua_Integer>(2.5, 2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Number, lua_Integer>(4.4, 4));
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

int DoKeyNumberValueNumber()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Number, lua_Number> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Number, lua_Number>(1.1, 1.1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Number, lua_Number>(2.2, 2.2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Number, lua_Number>(3.3, 3.3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Number, lua_Number>::iterator iter;
	
	iter = hash_map.find(1.1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1.1 == (*iter).second);
	
	iter = hash_map.find(2.2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2.2 == (*iter).second);
	
	iter = hash_map.find(3.3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3.3 == (*iter).second);
	
	
	iter = hash_map.find(3.3);
	
	std::pair<lua_Number, lua_Number> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3.3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2.2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Number, lua_Number>(2.5, 2.5));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Number, lua_Number>(4.4, 4.4));
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

#endif
#endif




int main(int argc, char* argv[])
{
#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 201112L)
	#warning "C11 test did not find a C11 compiler. The test will be skipped."
	fprintf(stderr, "This test must be compiled with a C11 compiler.");
	return 0;
		
	
	
#else
	

	DoKeyStringValueString();
	DoKeyStringValuePointer();
	DoKeyStringValueNumber();
	DoKeyStringValueInteger();

	
#endif
	

	fprintf(stderr, "Program passed all tests!\n");
	return 0;
}
