
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





#if LUAHASHMAP_SUPPORTS_GENERICS

// Toggle this in case the variable argument macros need to be disabled
#define LUAHASHMAP_DISABLE_VA_ARGS_MACROS 0

//#include <inttypes.h>


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
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, (const char*)("value1"), somekey_nonconst);
#else
	LuaHashMap_SetValue(hash_map, (const char*)("value1"), somekey_nonconst);
#endif

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
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
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
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
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

	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	fprintf(stderr, "LuaHashMap_Exists (iterator) for key3 should be found\n");

	exists = LuaHashMap_Exists(hash_map, (const char*)("key3"));
	assert(true == exists);
	fprintf(stderr, "LuaHashMap_Exists (key) for key3 should be found\n");
	
	LuaHashMap_SetValue(&the_iterator, (const char*)("value5"));

	ret_val = LuaHashMap_GetValueString(hash_map, (const char*)("key3"));
	assert(0 == Internal_safestrcmp("value5", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	
	
	
	LuaHashMap_SetValue(hash_map, (const char*)("value5"), (const char*)("key3"));
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key3"));
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	LuaHashMap_SetValue(hash_map, (const char*)("value5"), (const char*)("key3"), (sizeof("value5")/sizeof(char))-1, (sizeof("key3")/sizeof(char))-1);
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	LuaHashMap_SetValue(&the_iterator, (const char*)("value5"), (sizeof("value5")/sizeof(char))-1);
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	
	
	size_t out_string_length = 0;
	ret_val = LuaHashMap_GetValueString(hash_map, (const char*)("key3"), &out_string_length, (sizeof("key3")/sizeof(char))-1);
	assert( (sizeof("value5")/sizeof(char))-1 == out_string_length);

	ret_val = LuaHashMap_GetValueString(&the_iterator, &out_string_length);
	assert( (sizeof("value5")/sizeof(char))-1 == out_string_length);

	
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, (const char*)("key3"));
	assert(false == exists);
	fprintf(stderr, "LuaHashMap_Exists (key) for key3 should not be found\n");
#endif /* !LUAHASHMAP_DISABLE_VA_ARGS_MACROS */
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

static void* s_valuePointer1 = (void*)0x1;
static void* s_valuePointer2 = (void*)0x2;
static void* s_valuePointer3 = (void*)0x3;
static void* s_valuePointer4 = (void*)0x4;
static void* s_valuePointer5 = (void*)0x5;


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

#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer1, somekey_nonconst);
#else
	LuaHashMap_SetValue(hash_map, s_valuePointer1, somekey_nonconst);
#endif
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer2, (const char*)("key2"));
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer3, (char*)("key3"));


	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, (const char*)("key1"));
	assert(s_valuePointer1 == ret_val);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, (const char*)("key2"));
	assert(s_valuePointer2 == ret_val);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	assert(s_valuePointer3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %zd\n", (size_t)LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);

	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, (const char*)("key2"));
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %zd\n", (size_t)LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
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
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	assert(s_valuePointer4 == ret_val);
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, (const char*)("key3"));
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, s_valuePointer5);
	
	ret_val = LuaHashMap_GetValuePointer(hash_map, (const char*)("key3"));
	assert(s_valuePointer5 == ret_val);
	
	ret_val = LuaHashMap_GetValuePointer(&the_iterator);
	assert(s_valuePointer5 == ret_val);
	

	LuaHashMap_SetValue(hash_map, s_valuePointer5, (const char*)("key3"));
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key3"));
	ret_val = LuaHashMap_GetValuePointer(&the_iterator);
	assert(s_valuePointer5 == ret_val);
	LuaHashMap_SetValue(hash_map, s_valuePointer5, (const char*)("key3"), (sizeof("key3")/sizeof(char))-1);
	ret_val = LuaHashMap_GetValuePointer(&the_iterator);
	assert(s_valuePointer5 == ret_val);

	
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, (const char*)("key3"));
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
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
	
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, 1.0, somekey_nonconst);
#else
	LuaHashMap_SetValue(hash_map, 1.0, somekey_nonconst);
#endif
	LuaHashMap_SetValueForKey(hash_map, 2.2, (const char*)("key2"));
	LuaHashMap_SetValueForKey(hash_map, 3.3, (char*)("key3"));
	
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, (const char*)("key1"));
	assert(1.0 == ret_val);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, (const char*)("key2"));
	assert(2.2 == ret_val);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	assert(3.3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: " LUA_NUMBER_SCAN "\n", LuaHashMap_GetValueNumberAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, (const char*)("key2"));
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: " LUA_NUMBER_SCAN "\n", LuaHashMap_GetValueNumberAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
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
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	assert(4.4 == ret_val);
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, (const char*)("key3"));
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, 5.5);
	
	ret_val = LuaHashMap_GetValueNumber(hash_map, (const char*)("key3"));
	assert(5.5 == ret_val);
	
	ret_val = LuaHashMap_GetValueNumber(&the_iterator);
	assert(5.5 == ret_val);
	
	
	LuaHashMap_SetValue(hash_map, 5.5, (const char*)("key3"));
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key3"));
	ret_val = LuaHashMap_GetValueNumber(&the_iterator);
	assert(5.5 == ret_val);
	LuaHashMap_SetValue(hash_map, 5.5, (const char*)("key3"), (sizeof("key3")/sizeof(char))-1);
	ret_val = LuaHashMap_GetValueNumber(&the_iterator);
	assert(5.5 == ret_val);
	//	LuaHashMap_SetValue(&the_iterator, (const char*)("value5"), (sizeof("value5")/sizeof(char))-1);
	//	ret_val = LuaHashMap_GetValuePointer(&the_iterator);
	//	assert(s_valuePointer5 == ret_val);
	
	
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, (const char*)("key3"));
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
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
	
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, 1, somekey_nonconst);
#else
	LuaHashMap_SetValue(hash_map, 1, somekey_nonconst);
#endif
	LuaHashMap_SetValueForKey(hash_map, 2, (const char*)("key2"));
	LuaHashMap_SetValueForKey(hash_map, 3, (char*)("key3"));
	
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, (const char*)("key1"));
	assert(1 == ret_val);
	// %zd for size_t, %zd for ptrdiff_t
	// Lua doesn't seem to define a print token type for lua_Integer, but it usually is ptrdiff_t.
	fprintf(stderr, "ret_val=%td\n", ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, (const char*)("key2"));
	assert(2 == ret_val);
	fprintf(stderr, "ret_val=%td\n", ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, (const char*)("key3"));
	fprintf(stderr, "ret_val=%td\n", ret_val);
	assert(3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %td\n", LuaHashMap_GetValueIntegerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, (const char*)("key2"));
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, (const char*)("key2"));
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %td\n", LuaHashMap_GetValueIntegerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
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
	fprintf(stderr, "ret_val=%td\n", ret_val);
	assert(4 == ret_val);
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, (const char*)("key3"));
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, 5);
	
	ret_val = LuaHashMap_GetValueInteger(hash_map, (const char*)("key3"));
	assert(5 == ret_val);
	
	ret_val = LuaHashMap_GetValueInteger(&the_iterator);
	assert(5 == ret_val);
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, (const char*)("key3"));
	assert(false == exists);
	
	
	LuaHashMap_SetValue(hash_map, 5, (const char*)("key3"));
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, (const char*)("key3"));
	ret_val = LuaHashMap_GetValueInteger(&the_iterator);
	assert(5 == ret_val);
	LuaHashMap_SetValue(hash_map, 5, (const char*)("key3"), (sizeof("key3")/sizeof(char))-1);
	ret_val = LuaHashMap_GetValueInteger(&the_iterator);
	assert(5 == ret_val);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}


static void* s_keyPointer1 = (void*)0x1;
static void* s_keyPointer2 = (void*)0x2;
static void* s_keyPointer3 = (void*)0x3;



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
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, (const char*)("value1"), s_keyPointer1);
#else
	LuaHashMap_SetValue(hash_map, (const char*)("value1"), s_keyPointer1);
#endif
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
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
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
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
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
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyPointer3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, (const char*)("value5"));
	
	ret_val = LuaHashMap_GetValueString(hash_map, s_keyPointer3);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));

	
	LuaHashMap_SetValue(hash_map, (const char*)("value5"), s_keyPointer3);
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyPointer3);
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	LuaHashMap_SetValue(hash_map, (const char*)("value5"), s_keyPointer3, (sizeof("value5")/sizeof(char))-1);
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	LuaHashMap_SetValue(&the_iterator, (const char*)("value5"), (sizeof("value5")/sizeof(char))-1);
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));

	
	size_t out_string_length = 0;
	ret_val = LuaHashMap_GetValueString(hash_map, s_keyPointer3, &out_string_length);
	assert( (sizeof("value5")/sizeof(char))-1 == out_string_length);
	
	ret_val = LuaHashMap_GetValueString(&the_iterator, &out_string_length);
	assert( (sizeof("value5")/sizeof(char))-1 == out_string_length);
	
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyPointer3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
	
	
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
	
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer1, s_keyPointer1);
#else
	LuaHashMap_SetValue(hash_map, s_valuePointer1, s_keyPointer1);
#endif
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer2, s_keyPointer2);
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer3, s_keyPointer3);
	
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyPointer1);
	assert(s_valuePointer1 == ret_val);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyPointer2);
	assert(s_valuePointer2 == ret_val);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyPointer3);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	assert(s_valuePointer3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %zd\n", (size_t)LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);

	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyPointer2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %zd\n", (size_t)LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
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
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	assert(s_valuePointer4 == ret_val);
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyPointer3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, s_valuePointer5);
	
	ret_val = LuaHashMap_GetValuePointer(hash_map, s_keyPointer3);
	assert(s_valuePointer5 == ret_val);
	
	ret_val = LuaHashMap_GetValuePointer(&the_iterator);
	assert(s_valuePointer5 == ret_val);
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyPointer3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
	
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
	
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, 1.0, s_keyPointer1);
#else
	LuaHashMap_SetValue(hash_map, 1.0, s_keyPointer1);
#endif
	LuaHashMap_SetValueForKey(hash_map, 2.2, s_keyPointer2);
	LuaHashMap_SetValueForKey(hash_map, 3.3, s_keyPointer3);
	
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyPointer1);
	assert(1.0 == ret_val);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyPointer2);
	assert(2.2 == ret_val);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyPointer3);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	assert(3.3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: " LUA_NUMBER_SCAN "\n", LuaHashMap_GetValueNumberAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);

	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyPointer2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: " LUA_NUMBER_SCAN "\n", LuaHashMap_GetValueNumberAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
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
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	assert(4.4 == ret_val);
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyPointer3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, 5.5);
	
	ret_val = LuaHashMap_GetValueNumber(hash_map, s_keyPointer3);
	assert(5.5 == ret_val);
	
	ret_val = LuaHashMap_GetValueNumber(&the_iterator);
	assert(5.5 == ret_val);
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyPointer3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
	
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
	
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, 1, s_keyPointer1);
#else
	LuaHashMap_SetValue(hash_map, 1, s_keyPointer1);
#endif
	LuaHashMap_SetValueForKey(hash_map, 2, s_keyPointer2);
	LuaHashMap_SetValueForKey(hash_map, 3, s_keyPointer3);
	
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyPointer1);
	assert(1 == ret_val);
	fprintf(stderr, "ret_val=%td\n", ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyPointer2);
	assert(2 == ret_val);
	fprintf(stderr, "ret_val=%td\n", ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyPointer3);
	fprintf(stderr, "ret_val=%td\n", ret_val);
	assert(3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %td\n", LuaHashMap_GetValueIntegerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyPointer2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);

	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyPointer2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %td\n", LuaHashMap_GetValueIntegerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
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
	fprintf(stderr, "ret_val=%td\n", ret_val);
	assert(4 == ret_val);
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyPointer3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, 5);
	
	ret_val = LuaHashMap_GetValueInteger(hash_map, s_keyPointer3);
	assert(5 == ret_val);
	
	ret_val = LuaHashMap_GetValueInteger(&the_iterator);
	assert(5 == ret_val);
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyPointer3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	

	LuaHashMap_Free(hash_map);
	
	return 0;
}



lua_Number s_keyNumber1 = 1.0;
lua_Number s_keyNumber2 = 2.2;
lua_Number s_keyNumber3 = 3.3;


int DoKeyNumberValueString()
{
	fprintf(stderr, "DoKeyNumberValueString()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	const char* ret_val;
	size_t ret_count;
	bool exists;
	
	// String literals are a problem because the type is char[] and not char*.
	// An explicit typecast to char* must be used or the default case will be hit which goes to Pointer
	// http://www.robertgamble.net/2012/01/c11-generic-selections.html
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, (const char*)("value1"), s_keyNumber1);
#else
	LuaHashMap_SetValue(hash_map, (const char*)("value1"), s_keyNumber1);
#endif
	LuaHashMap_SetValueForKey(hash_map, (const char*)("value2"), s_keyNumber2);
	LuaHashMap_SetValueForKey(hash_map, (char*)("value3"), s_keyNumber3);
	

	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyNumber1);
	assert(0 == Internal_safestrcmp("value1", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyNumber2);
	assert(0 == Internal_safestrcmp("value2", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyNumber3);
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
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	

	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyNumber2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %s\n", LuaHashMap_GetValueStringAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyNumber3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	
	LuaHashMap_SetValueAtIterator(&the_iterator, (const char*)("value4"));
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyNumber3);
	fprintf(stderr, "ret_val=%s\n", ret_val);
	assert(0 == Internal_safestrcmp("value4", ret_val));
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyNumber3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, (const char*)("value5"));
	
	ret_val = LuaHashMap_GetValueString(hash_map, s_keyNumber3);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	
	
	
	LuaHashMap_SetValue(hash_map, (const char*)("value5"), s_keyNumber3);
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyNumber3);
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	LuaHashMap_SetValue(hash_map, (const char*)("value5"), s_keyNumber3, (sizeof("value5")/sizeof(char))-1);
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	LuaHashMap_SetValue(&the_iterator, (const char*)("value5"), (sizeof("value5")/sizeof(char))-1);
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	
	size_t out_string_length = 0;
	ret_val = LuaHashMap_GetValueString(hash_map, s_keyNumber3, &out_string_length);
	assert( (sizeof("value5")/sizeof(char))-1 == out_string_length);

	ret_val = LuaHashMap_GetValueString(&the_iterator, &out_string_length);
	assert( (sizeof("value5")/sizeof(char))-1 == out_string_length);
	
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyNumber3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}



int DoKeyNumberValuePointer()
{
	fprintf(stderr, "DoKeyNumberValuePointer()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	void* ret_val;
	size_t ret_count;
	bool exists;
	
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer1, s_keyNumber1);
#else
	LuaHashMap_SetValue(hash_map, s_valuePointer1, s_keyNumber1);
#endif
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer2, s_keyNumber2);
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer3, s_keyNumber3);
	
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyNumber1);
	assert(s_valuePointer1 == ret_val);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyNumber2);
	assert(s_valuePointer2 == ret_val);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyNumber3);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	assert(s_valuePointer3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %zd\n", (size_t)LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);

	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyNumber2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %zd\n", (size_t)LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyNumber3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, s_valuePointer4);
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyNumber3);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	assert(s_valuePointer4 == ret_val);
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyNumber3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, s_valuePointer5);
	
	ret_val = LuaHashMap_GetValuePointer(hash_map, s_keyNumber3);
	assert(s_valuePointer5 == ret_val);
	
	ret_val = LuaHashMap_GetValuePointer(&the_iterator);
	assert(s_valuePointer5 == ret_val);
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyNumber3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

int DoKeyNumberValueNumber()
{
	fprintf(stderr, "DoKeyNumberValueNumber()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	lua_Number ret_val;
	size_t ret_count;
	bool exists;
	
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, 1.0, s_keyNumber1);
#else
	LuaHashMap_SetValue(hash_map, 1.0, s_keyNumber1);
#endif
	LuaHashMap_SetValueForKey(hash_map, 2.2, s_keyNumber2);
	LuaHashMap_SetValueForKey(hash_map, 3.3, s_keyNumber3);
	
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyNumber1);
	assert(1.0 == ret_val);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyNumber2);
	assert(2.2 == ret_val);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyNumber3);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	assert(3.3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: " LUA_NUMBER_SCAN "\n", LuaHashMap_GetValueNumberAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);

	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyNumber2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: " LUA_NUMBER_SCAN "\n", LuaHashMap_GetValueNumberAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyNumber3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, 4.4);
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyNumber3);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	assert(4.4 == ret_val);
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyNumber3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, 5.5);
	
	ret_val = LuaHashMap_GetValueNumber(hash_map, s_keyNumber3);
	assert(5.5 == ret_val);
	
	ret_val = LuaHashMap_GetValueNumber(&the_iterator);
	assert(5.5 == ret_val);
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyNumber3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

int DoKeyNumberValueInteger()
{
	fprintf(stderr, "DoKeyNumberValueInteger()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	lua_Integer ret_val;
	size_t ret_count;
	bool exists;
	
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, 1, s_keyNumber1);
#else
	LuaHashMap_SetValue(hash_map, 1, s_keyNumber1);
#endif
	LuaHashMap_SetValueForKey(hash_map, 2, s_keyNumber2);
	LuaHashMap_SetValueForKey(hash_map, 3, s_keyNumber3);
	
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyNumber1);
	assert(1 == ret_val);
	fprintf(stderr, "ret_val=%td\n", ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyNumber2);
	assert(2 == ret_val);
	fprintf(stderr, "ret_val=%td\n", ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyNumber3);
	fprintf(stderr, "ret_val=%td\n", ret_val);
	assert(3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %td\n", LuaHashMap_GetValueIntegerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);

	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyNumber2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %td\n", LuaHashMap_GetValueIntegerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyNumber2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyNumber3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, 4);
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyNumber3);
	fprintf(stderr, "ret_val=%td\n", ret_val);
	assert(4 == ret_val);
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyNumber3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, 5);
	
	ret_val = LuaHashMap_GetValueInteger(hash_map, s_keyNumber3);
	assert(5 == ret_val);
	
	ret_val = LuaHashMap_GetValueInteger(&the_iterator);
	assert(5 == ret_val);
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyNumber3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}




lua_Number s_keyInteger1 = 1;
lua_Number s_keyInteger2 = 2;
lua_Number s_keyInteger3 = 3;


int DoKeyIntegerValueString()
{
	fprintf(stderr, "DoKeyIntegerValueString()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	const char* ret_val;
	size_t ret_count;
	bool exists;
	
	// String literals are a problem because the type is char[] and not char*.
	// An explicit typecast to char* must be used or the default case will be hit which goes to Pointer
	// http://www.robertgamble.net/2012/01/c11-generic-selections.html
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, (const char*)("value1"), s_keyInteger1);
#else
	LuaHashMap_SetValue(hash_map, (const char*)("value1"), s_keyInteger1);
#endif
	LuaHashMap_SetValueForKey(hash_map, (const char*)("value2"), s_keyInteger2);
	LuaHashMap_SetValueForKey(hash_map, (char*)("value3"), s_keyInteger3);
	
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyInteger1);
	assert(0 == Internal_safestrcmp("value1", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyInteger2);
	assert(0 == Internal_safestrcmp("value2", ret_val));
	fprintf(stderr, "ret_val=%s\n", ret_val);
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyInteger3);
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
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyInteger2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %s\n", LuaHashMap_GetValueStringAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyInteger3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	
	LuaHashMap_SetValueAtIterator(&the_iterator, (const char*)("value4"));
	
	ret_val = LuaHashMap_GetValueStringForKey(hash_map, s_keyInteger3);
	fprintf(stderr, "ret_val=%s\n", ret_val);
	assert(0 == Internal_safestrcmp("value4", ret_val));
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyInteger3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, (const char*)("value5"));
	
	ret_val = LuaHashMap_GetValueString(hash_map, s_keyInteger3);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	
	
	LuaHashMap_SetValue(hash_map, (const char*)("value5"), s_keyInteger3);
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyInteger3);
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	LuaHashMap_SetValue(hash_map, (const char*)("value5"), s_keyInteger3, (sizeof("value5")/sizeof(char))-1);
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	LuaHashMap_SetValue(&the_iterator, (const char*)("value5"), (sizeof("value5")/sizeof(char))-1);
	ret_val = LuaHashMap_GetValueString(&the_iterator);
	assert(0 == Internal_safestrcmp("value5", ret_val));
	
	
	size_t out_string_length = 0;
	ret_val = LuaHashMap_GetValueString(hash_map, s_keyInteger3, &out_string_length);
	assert( (sizeof("value5")/sizeof(char))-1 == out_string_length);

	ret_val = LuaHashMap_GetValueString(&the_iterator, &out_string_length);
	assert( (sizeof("value5")/sizeof(char))-1 == out_string_length);
	
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyInteger3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}



int DoKeyIntegerValuePointer()
{
	fprintf(stderr, "DoKeyIntegerValuePointer()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	void* ret_val;
	size_t ret_count;
	bool exists;
	
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer1, s_keyInteger1);
#else
	LuaHashMap_SetValue(hash_map, s_valuePointer1, s_keyInteger1);
#endif
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer2, s_keyInteger2);
	LuaHashMap_SetValueForKey(hash_map, s_valuePointer3, s_keyInteger3);
	
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyInteger1);
	assert(s_valuePointer1 == ret_val);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyInteger2);
	assert(s_valuePointer2 == ret_val);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyInteger3);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	assert(s_valuePointer3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %zd\n", (size_t)LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyInteger2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %zd\n", (size_t)LuaHashMap_GetValuePointerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyInteger3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, s_valuePointer4);
	ret_val = LuaHashMap_GetValuePointerForKey(hash_map, s_keyInteger3);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	assert(s_valuePointer4 == ret_val);
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyInteger3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, s_valuePointer5);
	
	ret_val = LuaHashMap_GetValuePointer(hash_map, s_keyInteger3);
	assert(s_valuePointer5 == ret_val);
	
	ret_val = LuaHashMap_GetValuePointer(&the_iterator);
	assert(s_valuePointer5 == ret_val);
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyInteger3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

int DoKeyIntegerValueNumber()
{
	fprintf(stderr, "DoKeyIntegerValueNumber()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	lua_Number ret_val;
	size_t ret_count;
	bool exists;
	
#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	LuaHashMap_SetValueForKey(hash_map, 1.0, s_keyInteger1);
#else
	LuaHashMap_SetValue(hash_map, 1.0, s_keyInteger1);
#endif
	LuaHashMap_SetValueForKey(hash_map, 2.2, s_keyInteger2);
	LuaHashMap_SetValueForKey(hash_map, 3.3, s_keyInteger3);
	
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyInteger1);
	assert(1.0 == ret_val);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyInteger2);
	assert(2.2 == ret_val);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyInteger3);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	assert(3.3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: " LUA_NUMBER_SCAN "\n", LuaHashMap_GetValueNumberAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyInteger2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: " LUA_NUMBER_SCAN "\n", LuaHashMap_GetValueNumberAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyInteger3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, 4.4);
	ret_val = LuaHashMap_GetValueNumberForKey(hash_map, s_keyInteger3);
	fprintf(stderr, "ret_val=" LUA_NUMBER_SCAN "\n", ret_val);
	assert(4.4 == ret_val);
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyInteger3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, 5.5);
	
	ret_val = LuaHashMap_GetValueNumber(hash_map, s_keyInteger3);
	assert(5.5 == ret_val);
	
	ret_val = LuaHashMap_GetValueNumber(&the_iterator);
	assert(5.5 == ret_val);
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyInteger3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	

	LuaHashMap_Free(hash_map);
	
	return 0;
}

int DoKeyIntegerValueInteger()
{
	fprintf(stderr, "DoKeyIntegerValueInteger()\n");
	LuaHashMap* hash_map = LuaHashMap_Create();
	LuaHashMapIterator the_iterator;
	lua_Integer ret_val;
	size_t ret_count;
	bool exists;

#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS	
	LuaHashMap_SetValueForKey(hash_map, 1, s_keyInteger1);
#else
	LuaHashMap_SetValue(hash_map, 1, s_keyInteger1);
#endif
	LuaHashMap_SetValueForKey(hash_map, 2, s_keyInteger2);
	LuaHashMap_SetValueForKey(hash_map, 3, s_keyInteger3);
	
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyInteger1);
	assert(1 == ret_val);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyInteger2);
	assert(2 == ret_val);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyInteger3);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	assert(3 == ret_val);
	
	
	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));
	
	
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %zd\n", (size_t)LuaHashMap_GetValueIntegerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 3=%zd\n", ret_count);
	assert(3 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be true (1): %d\n", exists);
	assert(true == exists);
	
	
	fprintf(stderr, "removing key2\n");
	LuaHashMap_RemoveKey(hash_map, s_keyInteger2);
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %zd\n", (size_t)LuaHashMap_GetValueIntegerAtIterator(&the_iterator));
	} while(LuaHashMap_IteratorNext(&the_iterator));
	
	ret_count = LuaHashMap_Count(hash_map);
	fprintf(stderr, "LuaHashMap_Count() should be 2=%zd\n", ret_count);
	assert(2 == ret_count);
	
	exists = LuaHashMap_ExistsKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_ExistsKey for key2 should be false (0): %d\n", exists);
	assert(false == exists);
	
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyInteger2);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key2 should not be found\n");
	assert(true == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	the_iterator = LuaHashMap_GetIteratorForKey(hash_map, s_keyInteger3);
	fprintf(stderr, "LuaHashMap_GetIteratorForKey for key3 should be found\n");
	assert(false == LuaHashMap_IteratorIsNotFound(&the_iterator));
	
	LuaHashMap_SetValueAtIterator(&the_iterator, 4);
	ret_val = LuaHashMap_GetValueIntegerForKey(hash_map, s_keyInteger3);
	fprintf(stderr, "ret_val=%zd\n", (size_t)ret_val);
	assert(4 == ret_val);
	
	
	
	// Testing different number of arguments macros
#if !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	exists = LuaHashMap_Exists(&the_iterator);
	assert(true == exists);
	
	exists = LuaHashMap_Exists(hash_map, s_keyInteger3);
	assert(true == exists);
	
	LuaHashMap_SetValue(&the_iterator, 5);
	
	ret_val = LuaHashMap_GetValueInteger(hash_map, s_keyInteger3);
	assert(5 == ret_val);
	
	ret_val = LuaHashMap_GetValueInteger(&the_iterator);
	assert(5 == ret_val);
	
	LuaHashMap_Remove(&the_iterator);
	exists = LuaHashMap_Exists(hash_map, s_keyInteger3);
	assert(false == exists);
#endif // !LUAHASHMAP_DISABLE_VA_ARGS_MACROS
	
	
	LuaHashMap_Free(hash_map);
	
	return 0;
}

#endif




int main(int argc, char* argv[])
{
//#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 201112L)
#if !LUAHASHMAP_SUPPORTS_GENERICS
	// Ugh. Visual Studio doesn't support #warning
	// And clang doesn't like Visual Studio's pragma message format.
	// I don't feel like doing all compiler checks here. clang won't hit this case.
	#pragma message ("C11 test did not find C11 _Generic support. The test will be skipped.")
	fprintf(stderr, "This test must be compiled with a C11 compiler.");
	return 0;
	
	
#else
	
	// I originally added a flag to disable the variable argument macros when I discovered my original technique relied on a compiler extension (comma pasting extension).
	// I have rewritten the code to remove that dependency so everything should be standard now.
	// But because this was such a pain to insert the first time, I have left the macros in place and just renamed them in case I need to disable these again.
	#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
		fprintf(stderr, "Warning: __VA_ARGS__ macros for C11 _Generic support has been disabled\n.");
	#endif

	DoKeyStringValueString();
	DoKeyStringValuePointer();
	DoKeyStringValueNumber();
	DoKeyStringValueInteger();
	
	DoKeyPointerValueString();
	DoKeyPointerValuePointer();
	DoKeyPointerValueNumber();
	DoKeyPointerValueInteger();

	DoKeyNumberValueString();
	DoKeyNumberValuePointer();
	DoKeyNumberValueNumber();
	DoKeyNumberValueInteger();
	
	DoKeyIntegerValueString();
	DoKeyIntegerValuePointer();
	DoKeyIntegerValueNumber();
	DoKeyIntegerValueInteger();
	
	
#endif
	

	fprintf(stderr, "Program passed all tests!\n");
		
	#if LUAHASHMAP_DISABLE_VA_ARGS_MACROS
		fprintf(stderr, "(Remember that the  __VA_ARGS__ macros for C11 _Generic support were disabled)\n.");
	#endif

	return 0;
}
