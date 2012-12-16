
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


static void* s_keyPointer1 = (void*)0x1;
static void* s_keyPointer2 = (void*)0x2;
static void* s_keyPointer3 = (void*)0x3;
static void* s_keyPointer4 = (void*)0x4;



int DoKeyPointerValueString()
{
	fprintf(stderr, "DoKeyPointerValueString()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	const char* ret_val;
	size_t ret_count;
	bool exists;
	
	// String literals are a problem because the type is char[] and not char*.
	// An explicit typecast to char* must be used or the default case will be hit which goes to Pointer
	// http://www.robertgamble.net/2012/01/c11-generic-selections.html
	LuaHashMap_SetValueForKey(hash_map, (const char*)("value1"), s_keyPointer1);
	LuaHashMap_SetValueForKey(hash_map, (const char*)("value2"), s_keyPointer2);
	LuaHashMap_SetValueForKey(hash_map, (char*)("value3"), s_keyPointer3);
	

	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyPointer1);
	assert(0 == Internal_safestrcmp("value1", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyPointer2);
	assert(0 == Internal_safestrcmp("value2", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyPointer3);
	assert(0 == Internal_safestrcmp("value3", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);

	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %s\n", LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%d\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	

	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyPointer2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %s\n", LuaHashMap_GetValueStringAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%d\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyPointer3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	
	LuaHashMap_SetValueAtIterator(&the_iterator, (const char*)("value4"));
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyPointer3);
	fprintf(stderr, "ret_val=%s\n", ret_val);
	assert(0 == Internal_safestrcmp("value4", ret_val));
	
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}



int DoKeyPointerValuePointer()
{
	fprintf(stderr, "DoKeyPointerValuePointer()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	void* ret_val;
	size_t ret_count;
	bool exists;
	
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer1, s_keyPointer1);
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer2, s_keyPointer2);
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer3, s_keyPointer3);
	
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyPointer1);
	assert(s_valuePointer1 == ret_val);
	fprintf(stderr, "ret_val=%d\n", ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyPointer2);
	assert(s_valuePointer2 == ret_val);
	fprintf(stderr, "ret_val=%d\n", ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyPointer3);
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
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);

	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyPointer2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %d\n", LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%d\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyPointer3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, s_valuePointer4);
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyPointer3);
	fprintf(stderr, "ret_val=%d\n", ret_val);
	assert(s_valuePointer4 == ret_val);
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

int DoKeyPointerValueNumber()
{
	fprintf(stderr, "DoKeyPointerValueNumber()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	lua_Number ret_val;
	size_t ret_count;
	bool exists;
	
	LuaHashMap_SetValueForKey(hash_map, 1.0, s_keyPointer1);
	LuaHashMap_SetValueForKey(hash_map, 2.2, s_keyPointer2);
	LuaHashMap_SetValueForKey(hash_map, 3.3, s_keyPointer3);
	
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyPointer1);
	assert(1.0 == ret_val);
	fprintf(stderr, "ret_val=%f\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyPointer2);
	assert(2.2 == ret_val);
	fprintf(stderr, "ret_val=%f\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyPointer3);
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
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);

	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyPointer2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %f\n", LuaHashMap_GetValueNumberAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%d\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyPointer3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, 4.4);
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyPointer3);
	fprintf(stderr, "ret_val=%f\n", ret_val);
	assert(4.4 == ret_val);
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

int DoKeyPointerValueInteger()
{
	fprintf(stderr, "DoKeyPointerValueInteger()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	lua_Integer ret_val;
	size_t ret_count;
	bool exists;
	
	LuaHashMap_SetValueForKey(hash_map, 1, s_keyPointer1);
	LuaHashMap_SetValueForKey(hash_map, 2, s_keyPointer2);
	LuaHashMap_SetValueForKey(hash_map, 3, s_keyPointer3);
	
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyPointer1);
	assert(1 == ret_val);
	fprintf(stderr, "ret_val=%d\n", ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyPointer2);
	assert(2 == ret_val);
	fprintf(stderr, "ret_val=%d\n", ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyPointer3);
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
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);

	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyPointer2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %d\n", LuaHashMap_GetValueIntegerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%d\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyPointer3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, 4);
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyPointer3);
	fprintf(stderr, "ret_val=%d\n", ret_val);
	assert(4 == ret_val);
	
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

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
	
	DoKeyPointerValueString();
	DoKeyPointerValuePointer();
	DoKeyPointerValueNumber();
	DoKeyPointerValueInteger();

	
#endif
	

	fprintf(stderr, "Program passed all tests!\n");
	return 0;
}
