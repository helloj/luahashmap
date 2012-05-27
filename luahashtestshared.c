
#include "LuaHashMap.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
//#include <inttypes.h>

#define ENABLE_BENCHMARK

#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
#include <QuartzCore/QuartzCore.h>
#endif

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



static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
	(void)ud;
	(void)osize;
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}
#define MAX_ARRAY_SIZE 10

void DoBenchMark(LuaHashMap* hash_map)
{
	
#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();
	size_t i;

	void* ret_ptr = NULL;	
	for(i=0; i<400000; i++)
	{
		LuaHashMap_InsertValuePointerForKeyPointer(hash_map, (void*)i, (void*)rand());
		LuaHashMap_InsertValueIntegerForKeyInteger(hash_map, rand(), rand());
		ret_ptr = LuaHashMap_GetValuePointerForKeyPointer(hash_map, (void*)i);
		//		LuaHashMap_RemoveKeyPointer(hash_map, ret_ptr);
	}
	fprintf(stderr, "num keys= %zu\n", LuaHashMap_GetKeysInteger(hash_map, NULL, 0));
	
	LuaHashMap_Purge(hash_map);
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	assert(1 == LuaHashMap_IsEmpty(hash_map));
#endif

}

void DoKeyStringValueString(LuaHashMap* hash_map)
{
	const char* ret_string = NULL;
	const char* key_array[MAX_ARRAY_SIZE];
	size_t ret_size;
	size_t i;
	LuaHashMapIterator the_iterator;

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
		fprintf(stderr, "Key[%zu] is %s\n", i, key_array[i]);
	}
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	/*
	 the_iterator = LuaHashMap_GetIteratorAtBeginForKeyString(hash_map);
	 */
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
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
	
#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();
	
	void* ret_ptr = NULL;	
	for(i=0; i<400000; i++)
	{
		LuaHashMap_InsertValuePointerForKeyPointer(hash_map, (void*)i, (void*)rand());
		LuaHashMap_InsertValueIntegerForKeyInteger(hash_map, rand(), rand());
		ret_ptr = LuaHashMap_GetValuePointerForKeyPointer(hash_map, (void*)i);
		//		LuaHashMap_RemoveKeyPointer(hash_map, ret_ptr);
	}
	fprintf(stderr, "num keys= %zu\n", LuaHashMap_GetKeysInteger(hash_map, NULL, 0));
	
	LuaHashMap_Clear(hash_map);
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	assert(1 == LuaHashMap_IsEmpty(hash_map));
#endif
	
}

void TestSharedSeparationWithNumberAndInt(LuaHashMap* hash_map_shared_keynumber_valuenumber, LuaHashMap* hash_map_shared_keyint_valueint)
{
	lua_Number ret_num;
	lua_Integer ret_int;
	LuaHashMapIterator keynum_valuenum_iterator;
	LuaHashMapIterator keyint_valueint_iterator;
	size_t count_keynum_valuenum = 0;
	size_t count_keyint_valueint = 0;

	fprintf(stderr, "LuaHashMap_InsertValueNumberForKeyNumber\n");
	LuaHashMap_InsertValueNumberForKeyNumber(hash_map_shared_keynumber_valuenumber, 100.0, 1.0);
	LuaHashMap_InsertValueNumberForKeyNumber(hash_map_shared_keynumber_valuenumber, 200.2, 2.2);
	LuaHashMap_InsertValueNumberForKeyNumber(hash_map_shared_keynumber_valuenumber, 300.3, 3.3);

	fprintf(stderr, "LuaHashMap_InsertValueIntegetForKeyInteger\n");
	LuaHashMap_InsertValueIntegerForKeyInteger(hash_map_shared_keyint_valueint, 1000, 1);
	LuaHashMap_InsertValueIntegerForKeyInteger(hash_map_shared_keyint_valueint, 2000, 2);
	LuaHashMap_InsertValueIntegerForKeyInteger(hash_map_shared_keyint_valueint, 3000, 3);

	
	fprintf(stderr, "LuaHashMap_GetValueNumberForKeyNumber 1.0\n");
	ret_num = LuaHashMap_GetValueNumberForKeyNumber(hash_map_shared_keynumber_valuenumber, 1.0);
	assert(100.0 == ret_num);

	fprintf(stderr, "LuaHashMap_GetValueNumberForKeyNumber 2.2\n");
	ret_num = LuaHashMap_GetValueNumberForKeyNumber(hash_map_shared_keynumber_valuenumber, 2.2);
	assert(200.2 == ret_num);

	fprintf(stderr, "LuaHashMap_GetValueNumberForKeyNumber 3.3\n");
	ret_num = LuaHashMap_GetValueNumberForKeyNumber(hash_map_shared_keynumber_valuenumber, 3.3);
	assert(300.3 == ret_num);

	
	fprintf(stderr, "LuaHashMap_GetValueIntegerForKeyInteger 1\n");
	ret_int = LuaHashMap_GetValueIntegerForKeyInteger(hash_map_shared_keyint_valueint, 1);
	assert(1000 == ret_int);
	
	fprintf(stderr, "LuaHashMap_GetValueIntegerForKeyInteger 2\n");
	ret_int = LuaHashMap_GetValueIntegerForKeyInteger(hash_map_shared_keyint_valueint, 2);
	assert(2000 == ret_int);
	
	fprintf(stderr, "LuaHashMap_GetValueIntegerForKeyInteger 3\n");
	ret_int = LuaHashMap_GetValueIntegerForKeyInteger(hash_map_shared_keyint_valueint, 3);
	assert(3000 == ret_int);

	
	keynum_valuenum_iterator = LuaHashMap_GetIteratorAtBegin(hash_map_shared_keynumber_valuenumber);
	keyint_valueint_iterator = LuaHashMap_GetIteratorAtBegin(hash_map_shared_keyint_valueint);
	
	assert(!LuaHashMap_IteratorIsEqual(&keynum_valuenum_iterator, &keyint_valueint_iterator));	

	count_keynum_valuenum = 0;
	do
	{
		fprintf(stderr, "Using LuaHashMap_GetValueNumberAtIterator iterator: %lf\n", LuaHashMap_GetValueNumberAtIterator(&keynum_valuenum_iterator));
		count_keynum_valuenum++;
	} while(LuaHashMap_IteratorNext(&keynum_valuenum_iterator));

	fprintf(stderr, "count_keynum_valuenum=%zu\n", count_keynum_valuenum);
	assert(3 == count_keynum_valuenum);
	assert(3 == LuaHashMap_Count(hash_map_shared_keynumber_valuenumber));

	
	
	count_keyint_valueint = 0;
	do
	{
		fprintf(stderr, "Using LuaHashMap_GetValueIntegerAtIterator iterator: %zd\n", LuaHashMap_GetValueIntegerAtIterator(&keyint_valueint_iterator));
		count_keyint_valueint++;
	} while(LuaHashMap_IteratorNext(&keyint_valueint_iterator));
	
	fprintf(stderr, "count_keyint_valueint=%zu\n", count_keyint_valueint);
	assert(3 == count_keyint_valueint);
	assert(3 == LuaHashMap_Count(hash_map_shared_keyint_valueint));

}

int main(int argc, char* argv[])
{


	LuaHashMap* hash_map_main;
	LuaHashMap* hash_map_shared_keystring_valuestring;
	LuaHashMap* hash_map_shared_keynum_valuenum;
	LuaHashMap* hash_map_shared_keyint_valueint;

	fprintf(stderr, "create\n");
	sleep(2);
	hash_map_main = LuaHashMap_Create();
	sleep(2);
//	hash_map = LuaHashMap_CreateWithSizeHints(0, 600000, LUAHASHMAP_KEYSTRING_TYPE, LUAHASHMAP_VALUESTRING_TYPE);
//	hash_map = LuaHashMap_CreateWithAllocatorAndSizeHints(l_alloc, NULL, 0, 600000, LUAHASHMAP_KEYSTRING_TYPE, LUAHASHMAP_VALUESTRING_TYPE);

	hash_map_shared_keystring_valuestring = LuaHashMap_CreateShare(hash_map_main);

	DoKeyStringValueString(hash_map_main);
	DoKeyStringValueString(hash_map_shared_keystring_valuestring);
	
	// Note: luaL_unref may recycle the ref number for the next lua_ref. So the test is to make sure we don't get any stale values.
	fprintf(stderr, "Freeing share hash_map_shared_keystring_valuestring. (Note: Next new share should not have any values from this one.)\n" );
	LuaHashMap_FreeShare(hash_map_shared_keystring_valuestring);
	sleep(2);


	fprintf(stderr, "LuaHashMap_Purge\n" );
	LuaHashMap_Purge(hash_map_main);
	sleep(2);

	fprintf(stderr, "DoBenchMark LuaHashMap_Purge\n" );
	DoBenchMark(hash_map_main);
	
	fprintf(stderr, "Creating num and int shares\n" );

	hash_map_shared_keynum_valuenum = LuaHashMap_CreateShare(hash_map_main);
	sleep(2);
	hash_map_shared_keyint_valueint = LuaHashMap_CreateShare(hash_map_main);
	sleep(2);

	
	TestSharedSeparationWithNumberAndInt(hash_map_shared_keynum_valuenum, hash_map_shared_keyint_valueint);
	sleep(2);

	LuaHashMap_FreeShare(hash_map_shared_keyint_valueint);
	sleep(2);

	LuaHashMap_FreeShare(hash_map_shared_keynum_valuenum);
	sleep(2);

	LuaHashMap_Free(hash_map_main);
	sleep(2);

	fprintf(stderr, "Program passed all tests!\n");
	return 0;
}
