
#include "LuaHashMap.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <QuartzCore/QuartzCore.h>

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
	LuaHashMap* hash_map = LuaHashMap_Create();

	fprintf(stderr, "LuaHashMap_InsertValueStringForKeyString\n");
	LuaHashMap_InsertValueStringForKeyString(hash_map, "value1", "key1");
	fprintf(stderr, "LuaHashMap_InsertValueStringForKeyString\n");

	LuaHashMap_InsertValueStringForKeyString(hash_map, "value2", "key2");
	fprintf(stderr, "LuaHashMap_InsertValueStringForKeyString\n");

	LuaHashMap_InsertValueStringForKeyString(hash_map, "value3", "key3");
//	LuaHashMap_InsertValueStringForKeyString(hash_map, NULL, "key3");



	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = LuaHashMap_GetValueStringForKeyString(hash_map, "key1");
	assert(0 == Internal_safestrcmp("value1", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);
	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = LuaHashMap_GetValueStringForKeyString(hash_map, "key2");
	assert(0 == Internal_safestrcmp("value2", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);
	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = LuaHashMap_GetValueStringForKeyString(hash_map, "key3");
	assert(0 == Internal_safestrcmp("value3", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);

	ret_size = LuaHashMap_GetKeysString(hash_map, key_array, MAX_ARRAY_SIZE);
	assert(3 == ret_size);
	for(i=0; i<ret_size; i++)
	{
		fprintf(stderr, "Key[%d] is %s\n", i, key_array[i]);
	}

	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));



	LuaHashMapIterator the_iterator = LuaHashMap_GetIteratorAtBeginForKeyString(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %s\n", LuaHashMap_GetValueStringAtIterator(&the_iterator));		
	} while(LuaHashMap_IteratorNext(&the_iterator));



	LuaHashMap_Clear(hash_map);
	ret_size = LuaHashMap_GetKeysString(hash_map, key_array, MAX_ARRAY_SIZE);
	assert(0 == ret_size);
	
	
	assert(1 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be yes: %d\n", LuaHashMap_IsEmpty(hash_map));
	LuaHashMap_InsertValueStringForKeyString(hash_map, "value3", NULL);
	
	CFTimeInterval start_time = CACurrentMediaTime();

	void* ret_ptr = NULL;	
	for(i=0; i<400000; i++)
	{
		LuaHashMap_InsertValuePointerForKeyPointer(hash_map, (void*)i, (void*)rand());
		LuaHashMap_InsertValueIntegerForKeyInteger(hash_map, rand(), rand());
		ret_ptr = LuaHashMap_GetValuePointerForKeyPointer(hash_map, (void*)i);
//		LuaHashMap_RemoveKeyPointer(hash_map, ret_ptr);
	}
	fprintf(stderr, "num keys= %d\n", LuaHashMap_GetKeysInteger(hash_map, NULL, 0));
	
	LuaHashMap_Clear(hash_map);
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	assert(1 == LuaHashMap_IsEmpty(hash_map));

	LuaHashMap_Free(hash_map);
	fprintf(stderr, "Program passed all tests!\n");
	return 0;
}
