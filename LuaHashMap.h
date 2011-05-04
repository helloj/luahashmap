/*
 LuaHashMap
 Copyright (C) 2011 PlayControl Software, LLC. 
 Eric Wing <ewing . public @ playcontrol.net>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

 */

#ifndef C_LUA_HASH_MAP_H
#define C_LUA_HASH_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)
	/* Not ISO/IEC 9899:1999-compliant. */
	#if !defined(restrict)
		#define restrict
		#define __LUAHASHMAP_RESTRICT_KEYWORD_DEFINED__
	#endif
	
	#if !defined(bool)
		#define bool char
        #define __LUAHASHMAP_BOOL_KEYWORD_DEFINED__
	#endif
	#if !defined(false)
		#define false (bool)0
        #define __LUAHASHMAP_FALSE_KEYWORD_DEFINED__
	#endif
	#if !defined(true)
		#define true (bool)1
        #define __LUAHASHMAP_TRUE_KEYWORD_DEFINED__
	#endif
#else
	#include <stdbool.h>
#endif

#include <stddef.h>
#include "lua.h"
	
				
typedef struct LuaHashMap LuaHashMap;

struct LuaHashMapIterator
{
	/* These are all implementation details.
	 * You should probably not directly touch.
	 */
	LuaHashMap* hashMap;
	const char* whichTable;
	bool atEnd;
	union LuaHashMapKeyType
	{
		const char* keyString;
		lua_Number keyNumber;
		lua_Integer keyInteger;
		void* keyPointer;
	} currentKey;
};

typedef struct LuaHashMapIterator LuaHashMapIterator;
/*
typedef double LuaHashMapNumber;
typedef int LuaHashMapInteger;
*/

#define LUAHASHMAP_KEYSTRING_TYPE		0x01
#define LUAHASHMAP_KEYPOINTER_TYPE		0x02
#define LUAHASHMAP_KEYNUMBER_TYPE		0x04
#define LUAHASHMAP_KEYINTEGER_TYPE		0x08

#define LUAHASHMAP_VALUESTRING_TYPE		0x10
#define LUAHASHMAP_VALUEPOINTER_TYPE	0x20
#define LUAHASHMAP_VALUENUMBER_TYPE		0x40
#define LUAHASHMAP_VALUEINTEGER_TYPE	0x80

LuaHashMap* LuaHashMap_Create(void);
LuaHashMap* LuaHashMap_CreateWithAllocator(lua_Alloc the_allocator, void* user_data);

LuaHashMap* LuaHashMap_CreateWithSizeHints(int number_of_array_elements, int number_of_hash_elements, int key_type, int value_type);
	LuaHashMap* LuaHashMap_CreateWithAllocatorAndSizeHints(lua_Alloc the_allocator, void* user_data, int number_of_array_elements, int number_of_hash_elements, int key_type, int value_type);

/* Note: If created with an external Lua state, it will not delete the underlying Lua state. */
void LuaHashMap_Free(LuaHashMap* hash_map);

//LuaHashMap* LuaHashMap_CreateWithLuaState(struct lua_State* lua_state);
//LuaHashMap* LuaHashMap_CreateWithAllocator(lua_Allocator the_allocator);

//LuaHashMap* LuaHashMap_CreateInNamespace(const char* name_space);
//LuaHashMap* LuaHashMap_CreateWithLuaStateInNamespace(struct lua_State* lua_state, const char* restrict name_space);
//LuaHashMap* LuaHashMap_Free(LuaHashMap* hash_map);

//struct lua_State* LuaHashMap_GetLuaState(void);

/* string, string */
/* Note: Inserting NULL values is like deleting a field. */
void LuaHashMap_InsertValueStringForKeyString(LuaHashMap* hash_map, const char* restrict value_string, const char* restrict key_string);
/* string, pointer */
void LuaHashMap_InsertValuePointerForKeyString(LuaHashMap* hash_map, void* value_pointer, const char* restrict key_string);	
/* string, number */
void LuaHashMap_InsertValueNumberForKeyString(LuaHashMap* hash_map, lua_Number value_number, const char* restrict key_string);
/* string, integer */
void LuaHashMap_InsertValueIntegerForKeyString(LuaHashMap* hash_map, lua_Integer value_integer, const char* restrict key_string);


/* pointer, string */
void LuaHashMap_InsertValueStringForKeyPointer(LuaHashMap* hash_map, const char* restrict value_string, void* key_pointer);
/* pointer, pointer */
void LuaHashMap_InsertValuePointerForKeyPointer(LuaHashMap* hash_map, void* value_pointer, void* key_pointer);
/* pointer, number */
void LuaHashMap_InsertValueNumberForKeyPointer(LuaHashMap* hash_map, lua_Number value_number, void* key_pointer);
/* pointer, integer */
void LuaHashMap_InsertValueIntegerForKeyPointer(LuaHashMap* hash_map, lua_Integer value_integer, void* key_pointer);
	
	
/* number, string */
void LuaHashMap_InsertValueStringForKeyNumber(LuaHashMap* hash_map, const char* restrict value_string, lua_Number key_number);
/* number, pointer */
void LuaHashMap_InsertValuePointerForKeyNumber(LuaHashMap* hash_map, void* value_pointer, lua_Number key_number);
/* number, number */
void LuaHashMap_InsertValueNumberForKeyNumber(LuaHashMap* hash_map, lua_Number value_number, lua_Number key_number);
/* number, integer */
void LuaHashMap_InsertValueIntegerForKeyNumber(LuaHashMap* hash_map, lua_Integer value_integer, lua_Number key_number);

	
/* integer, string */
void LuaHashMap_InsertValueStringForKeyInteger(LuaHashMap* hash_map, const char* restrict value_string, lua_Integer key_integer);
/* integer, pointer */
void LuaHashMap_InsertValuePointerForKeyInteger(LuaHashMap* hash_map, void* value_pointer, lua_Integer key_integer);
/* integer, number */
void LuaHashMap_InsertValueNumberForKeyInteger(LuaHashMap* hash_map, lua_Number value_number, lua_Integer key_integer);
/* integer, integer*/
void LuaHashMap_InsertValueIntegerForKeyInteger(LuaHashMap* hash_map, lua_Integer value_integer, lua_Integer key_integer);
	


/* Get Functions */
const char* LuaHashMap_GetValueStringForKeyString(LuaHashMap* hash_map, const char* restrict key_string);
void* LuaHashMap_GetValuePointerForKeyString(LuaHashMap* hash_map, const char* restrict key_string);
lua_Number LuaHashMap_GetValueNumberForKeyString(LuaHashMap* hash_map, const char* restrict key_string);
lua_Integer LuaHashMap_GetValueIntegerForKeyString(LuaHashMap* hash_map, const char* restrict key_string);


const char* LuaHashMap_GetValueStringForKeyPointer(LuaHashMap* hash_map, void* key_pointer);
void* LuaHashMap_GetValuePointerForKeyPointer(LuaHashMap* hash_map, void* key_pointer);
lua_Number LuaHashMap_GetValueNumberForKeyPointer(LuaHashMap* hash_map, void* key_pointer);
lua_Integer LuaHashMap_GetValueIntegerForKeyPointer(LuaHashMap* hash_map, void* key_pointer);

	
const char* LuaHashMap_GetValueStringForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
void* LuaHashMap_GetValuePointerForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
lua_Number LuaHashMap_GetValueNumberForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
lua_Integer LuaHashMap_GetValueIntegerForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
	
const char* LuaHashMap_GetValueStringForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);
void* LuaHashMap_GetValuePointerForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);
lua_Number LuaHashMap_GetValueNumberForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);
lua_Integer LuaHashMap_GetValueIntegerForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);


/* Exists Functions*/
bool LuaHashMap_ExistsKeyString(LuaHashMap* hash_map, const char* restrict key_string);
bool LuaHashMap_ExistsKeyPointer(LuaHashMap* hash_map, void* key_pointer);
bool LuaHashMap_ExistsKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
bool LuaHashMap_ExistsKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);


/* Remove functions */
void LuaHashMap_RemoveKeyString(LuaHashMap* hash_map, const char* restrict key_string);
void LuaHashMap_RemoveKeyPointer(LuaHashMap* hash_map, void* key_pointer);
void LuaHashMap_RemoveKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
void LuaHashMap_RemoveKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);


/* Clear List */
void LuaHashMap_Clear(LuaHashMap* hash_map);
bool LuaHashMap_IsEmpty(LuaHashMap* hash_map);

/* List Functions */
size_t LuaHashMap_GetKeysString(LuaHashMap* hash_map, const char* keys_array[], size_t max_array_size);
size_t LuaHashMap_GetKeysPointer(LuaHashMap* hash_map, void* keys_array[], size_t max_array_size);
size_t LuaHashMap_GetKeysNumber(LuaHashMap* hash_map, lua_Number keys_array[], size_t max_array_size);
size_t LuaHashMap_GetKeysInteger(LuaHashMap* hash_map, lua_Integer keys_array[], size_t max_array_size);
	
	
/* Iterator functions */
bool LuaHashMap_IteratorNext(LuaHashMapIterator* hash_iterator);
	
LuaHashMapIterator LuaHashMap_GetIteratorAtBeginForKeyString(LuaHashMap* hash_map);
LuaHashMapIterator LuaHashMap_GetIteratorAtEndForKeyString(LuaHashMap* hash_map);
LuaHashMapIterator LuaHashMap_GetIteratorForKeyString(LuaHashMap* hash_map, const char* key_string);

LuaHashMapIterator LuaHashMap_GetIteratorAtBeginForKeyPointer(LuaHashMap* hash_map);
LuaHashMapIterator LuaHashMap_GetIteratorAtEndForKeyPointer(LuaHashMap* hash_map);
LuaHashMapIterator LuaHashMap_GetIteratorForKeyPointer(LuaHashMap* hash_map, void* key_pointer);

LuaHashMapIterator LuaHashMap_GetIteratorAtBeginForKeyNumber(LuaHashMap* hash_map);
LuaHashMapIterator LuaHashMap_GetIteratorAtEndForKeyNumber(LuaHashMap* hash_map);
LuaHashMapIterator LuaHashMap_GetIteratorForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);

LuaHashMapIterator LuaHashMap_GetIteratorAtBeginForKeyInteger(LuaHashMap* hash_map);
LuaHashMapIterator LuaHashMap_GetIteratorAtEndForKeyInteger(LuaHashMap* hash_map);
LuaHashMapIterator LuaHashMap_GetIteratorForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);

	
bool LuaHashMap_IteratorIsEqual(LuaHashMapIterator* hash_iterator1, LuaHashMapIterator* hash_iterator2);

	
void LuaHashMap_InsertValueStringAtIterator(LuaHashMapIterator* hash_iterator, const char* restrict value_string);
void LuaHashMap_InsertValuePointerAtIterator(LuaHashMapIterator* hash_iterator, void* value_pointer);
void LuaHashMap_InsertValueNumberAtIterator(LuaHashMapIterator* hash_iterator, lua_Number value_number);
void LuaHashMap_InsertValueIntegerAtIterator(LuaHashMapIterator* hash_iterator, lua_Integer value_integer);

	
	
const char* LuaHashMap_GetValueStringAtIterator(LuaHashMapIterator* hash_iterator);
	
	
bool LuaHashMap_ExistsAtIterator(LuaHashMapIterator* hash_iterator);
void LuaHashMap_RemoveAtIterator(LuaHashMapIterator* hash_iterator);

	
#if defined(__LUAHASHMAP_RESTRICT_KEYWORD_DEFINED__)
	#undef restrict
	#undef __LUAHASHMAP_RESTRICT_KEYWORD_DEFINED__
#endif
    
#if defined(__LUAHASHMAP_BOOL_KEYWORD_DEFINED__)
	#undef bool
	#undef __LUAHASHMAP_BOOL_KEYWORD_DEFINED__
#endif
    
#if defined(__LUAHASHMAP_FALSE_KEYWORD_DEFINED__)
	#undef false
	#undef __LUAHASHMAP_FALSE_KEYWORD_DEFINED__
#endif
    
#if defined(__LUAHASHMAP_TRUE_KEYWORD_DEFINED__)
	#undef true
	#undef __LUAHASHMAP_TRUE_KEYWORD_DEFINED__
#endif
    


#ifdef __cplusplus
}
#endif



#endif /* C_LUA_HASH_MAP_H */

