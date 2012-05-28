
#include "LuaHashMap.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

//#define ENABLE_BENCHMARK

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

void TestSimpleKeyStringNumberValue()
{
	LuaHashMap* hash_map = LuaHashMap_Create();
	
	fprintf(stderr, "TestSimpleKeyStringNumberValue start\n");
	
	LuaHashMap_SetValueNumberForKeyString(hash_map, 3.99, "milk");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 4.349, "gas");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 2.99, "bread");
	
	
	fprintf(stderr, "Price of gas: %lf\n", LuaHashMap_GetValueNumberForKeyString(hash_map, "gas"));
	assert(3.99 == LuaHashMap_GetValueNumberForKeyString(hash_map, "milk"));
	assert(4.349 == LuaHashMap_GetValueNumberForKeyString(hash_map, "gas"));
	assert(2.99 == LuaHashMap_GetValueNumberForKeyString(hash_map, "bread"));
	assert(3 == LuaHashMap_Count(hash_map));
	LuaHashMap_Free(hash_map);
	
	fprintf(stderr, "TestSimpleKeyStringNumberValue done\n");

}

void TestSimpleKeyStringNumberValueWithIterator()
{	
	LuaHashMap* hash_map = LuaHashMap_Create();
	/* Better to declare when you assign it, but you need C99 */
	LuaHashMapIterator hash_iterator;
	
	fprintf(stderr, "TestSimpleKeyStringNumberValueWithIterator start\n");

	LuaHashMap_SetValueNumberForKeyString(hash_map, 3.99, "milk");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 4.349, "gas");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 2.99, "bread");
	
	hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Price of %s: %lf\n", 
			   LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
			   LuaHashMap_GetValueNumberAtIterator(&hash_iterator));
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));

	assert(3.99 == LuaHashMap_GetValueNumberForKeyString(hash_map, "milk"));
	assert(4.349 == LuaHashMap_GetValueNumberForKeyString(hash_map, "gas"));
	assert(2.99 == LuaHashMap_GetValueNumberForKeyString(hash_map, "bread"));
	assert(3 == LuaHashMap_Count(hash_map));

	LuaHashMap_Free(hash_map);
	
	fprintf(stderr, "TestSimpleKeyStringNumberValueWithIterator done\n");

}


void TestValueStringNULL()
{	
	LuaHashMap* hash_map = LuaHashMap_Create();
	/* Better to declare when you assign it, but you need C99 */
	LuaHashMapIterator hash_iterator;
	LuaHashMapIterator hash_iterator_end;

	fprintf(stderr, "TestValueStringNULL start\n");
	
	LuaHashMap_SetValueStringForKeyString(hash_map, "$3.99", "milk");
	/* Lua treats NULL strings as pushing nil which removes an entry */
	LuaHashMap_SetValueStringForKeyString(hash_map, NULL, "gas");
	LuaHashMap_SetValueStringForKeyString(hash_map, "$2.99", "bread");
	
	hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Price of %s: %s\n", 
			   LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
			   LuaHashMap_GetValueStringAtIterator(&hash_iterator));
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));

	/* Lua treats NULL strings as pushing nil which removes an entry. Thus we only have 2. */
	assert(2 == LuaHashMap_Count(hash_map));
	assert(false == LuaHashMap_ExistsKeyString(hash_map, "gas"));
	assert(true == LuaHashMap_ExistsKeyString(hash_map, "milk"));
	assert(true == LuaHashMap_ExistsKeyString(hash_map, "bread"));

	/* Lua treats NULL strings as pushing nil which removes an entry */
	LuaHashMap_SetValueStringForKeyString(hash_map, NULL, "bread");

	hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);	
	do
	{
		fprintf(stderr, "Price of %s: %s\n", 
			   LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
			   LuaHashMap_GetValueStringAtIterator(&hash_iterator));
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));
	assert(1 == LuaHashMap_Count(hash_map));
	assert(false == LuaHashMap_ExistsKeyString(hash_map, "gas"));
	assert(true == LuaHashMap_ExistsKeyString(hash_map, "milk"));
	assert(false == LuaHashMap_ExistsKeyString(hash_map, "bread"));
	
	LuaHashMap_Free(hash_map);
	fprintf(stderr, "TestValueStringNULL done\n");

	
}


void TestValuePointerNULL()
{	
	LuaHashMap* hash_map = LuaHashMap_Create();
	/* Better to declare when you assign it, but you need C99 */
	LuaHashMapIterator hash_iterator;
	LuaHashMapIterator hash_iterator_end;

	fprintf(stderr, "TestValuePointerNULL start\n");
	
	LuaHashMap_SetValuePointerForKeyString(hash_map, (void*)399, "milk");
	/* Unlike strings, Lua seems to push NULL pointers as 0 which does not remove an entry. */
	LuaHashMap_SetValuePointerForKeyString(hash_map, NULL, "gas");
	LuaHashMap_SetValuePointerForKeyString(hash_map, (void*)299, "bread");
	
	hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Price of %s: %zd\n", 
				LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
				LuaHashMap_GetValuePointerAtIterator(&hash_iterator));
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));
	/* Unlike strings, Lua seems to push NULL pointers as 0 which does not remove an entry. Thus we have 3. */
	assert(3 == LuaHashMap_Count(hash_map));
	assert(true == LuaHashMap_ExistsKeyString(hash_map, "gas"));
	assert(true == LuaHashMap_ExistsKeyString(hash_map, "milk"));
	assert(true == LuaHashMap_ExistsKeyString(hash_map, "bread"));
	assert((void*)399 == LuaHashMap_GetValuePointerForKeyString(hash_map, "milk"));
	assert(NULL == LuaHashMap_GetValuePointerForKeyString(hash_map, "gas"));
	assert((void*)299 == LuaHashMap_GetValuePointerForKeyString(hash_map, "bread"));

	
	LuaHashMap_SetValuePointerForKeyString(hash_map, NULL, "bread");

	/* Showing a different way to loop */
	for(hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map), hash_iterator_end = LuaHashMap_GetIteratorAtEnd(hash_map);
		! LuaHashMap_IteratorIsEqual(&hash_iterator, &hash_iterator_end);
		LuaHashMap_IteratorNext(&hash_iterator)
	)
	{
		fprintf(stderr, "Price of %s: %zd\n", 
				LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
				LuaHashMap_GetValuePointerAtIterator(&hash_iterator));
		
	}
	assert(true == LuaHashMap_ExistsKeyString(hash_map, "gas"));
	assert(true == LuaHashMap_ExistsKeyString(hash_map, "milk"));
	assert(true == LuaHashMap_ExistsKeyString(hash_map, "bread"));
	assert((void*)399 == LuaHashMap_GetValuePointerForKeyString(hash_map, "milk"));
	assert(NULL == LuaHashMap_GetValuePointerForKeyString(hash_map, "gas"));
	assert(NULL == LuaHashMap_GetValuePointerForKeyString(hash_map, "bread"));
	
	LuaHashMap_Free(hash_map);
	fprintf(stderr, "TestValuePointerNULL done\n");
	
	
}


#define MAX_ARRAY_SIZE 10

int main(int argc, char* argv[])
{

	const char* ret_string = NULL;
	const char* key_array[MAX_ARRAY_SIZE];
	size_t ret_size;
	size_t i;
	LuaHashMap* hash_map;
	LuaHashMapIterator the_iterator;

	fprintf(stderr, "create\n");
	hash_map = LuaHashMap_Create();
//	hash_map = LuaHashMap_CreateWithSizeHints(0, 600000, LUAHASHMAP_KEYSTRING_TYPE, LUAHASHMAP_VALUESTRING_TYPE);
//	hash_map = LuaHashMap_CreateWithAllocatorAndSizeHints(l_alloc, NULL, 0, 600000, LUAHASHMAP_KEYSTRING_TYPE, LUAHASHMAP_VALUESTRING_TYPE);

	fprintf(stderr, "LuaHashMap_SetValueStringForKeyString\n");
	LuaHashMap_SetValueStringForKeyString(hash_map, "value1", "key1");
	fprintf(stderr, "LuaHashMap_SetValueStringForKeyString\n");

	LuaHashMap_SetValueStringForKeyString(hash_map, "value2", "key2");
	fprintf(stderr, "LuaHashMap_SetValueStringForKeyString\n");

	LuaHashMap_SetValueStringForKeyString(hash_map, "value3", "key3");
//	LuaHashMap_SetValueStringForKeyString(hash_map, NULL, "key3");



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
		fprintf(stderr, "Key[%zd] is %s\n", i, key_array[i]);
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
	LuaHashMap_SetValueStringForKeyString(hash_map, "value3", NULL);
	
#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();

	void* ret_ptr = NULL;	
	for(i=0; i<400000; i++)
	{
		// Mixed types not really supported. Don't do this.
		LuaHashMap_SetValuePointerForKeyPointer(hash_map, (void*)i, (void*)rand());
		LuaHashMap_SetValueIntegerForKeyInteger(hash_map, rand(), rand());
		ret_ptr = LuaHashMap_GetValuePointerForKeyPointer(hash_map, (void*)i);
//		LuaHashMap_RemoveKeyPointer(hash_map, ret_ptr);
	}
	fprintf(stderr, "num keys= %d\n", LuaHashMap_GetKeysInteger(hash_map, NULL, 0));
	
	LuaHashMap_Clear(hash_map);
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	assert(1 == LuaHashMap_IsEmpty(hash_map));
#endif


	TestSimpleKeyStringNumberValue();
	TestSimpleKeyStringNumberValueWithIterator();
	TestValuePointerNULL();
	TestValueStringNULL();
	
	LuaHashMap_Free(hash_map);
	fprintf(stderr, "Program passed all tests!\n");
	return 0;
}
