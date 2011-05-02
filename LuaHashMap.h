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

//#include "luaconf.h"
#include "lua.h"
	
#include <stdbool.h>
#include <stddef.h>
				
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


#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)
	/* Not ISO/IEC 9899:1999-compliant. */
	#if !defined(restrict)
		#define restrict
		#define __RESTRICT_KEYWORD_DEFINED__
	#endif
#endif
	

LuaHashMap* LuaHashMap_Create(void);
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



/* string, number */

/* number, number */
//void LuaHashMap_InsertKeyNumberValueNumber(LuaHashMap* hash_map, LuaHashMapNumber key_number, LuaHashMapNumber value_number); 

/* number, string */
//void LuaHashMap_InsertKeyNumberValueString(LuaHashMap* hash_map, LuaHashMapNumber key_number, const char* restrict value_string); 
void LuaHashMap_InsertValueStringForKeyNumber(LuaHashMap* hash_map, const char* restrict value_string, lua_Number key_number);


/* pointer, pointer */
void LuaHashMap_InsertValuePointerForKeyPointer(LuaHashMap* hash_map, void* value_pointer, void* key_pointer);

/* pointer, string */
void LuaHashMap_InsertValueStringForKeyPointer(LuaHashMap* hash_map, const char* restrict value_string, void* key_pointer);

/* string, pointer */
void LuaHashMap_InsertValuePointerForKeyString(LuaHashMap* hash_map, void* value_pointer, const char* restrict key_string);


/* pointer, number */
/* number, pointer */

/* string, integer */

/* integer, string */
void LuaHashMap_InsertValueStringForKeyInteger(LuaHashMap* hash_map, const char* restrict value_string, lua_Integer key_integer);

/* number, integer*/
/* integer, integer*/
/* integer, number */
//void LuaHashMap_InsertKeyNumberValueString(LuaHashMap* hash_map, LuaHashMapInteger key_integer, const char* restrict value_string); 


/* Get Functions */
const char* LuaHashMap_GetValueStringForKeyString(LuaHashMap* hash_map, const char* restrict key_string);
void* LuaHashMap_GetValuePointerForKeyString(LuaHashMap* hash_map, const char* restrict key_string);
void* LuaHashMap_GetValuePointerForKeyPointer(LuaHashMap* hash_map, void* key_pointer);

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

	
	
	
bool LuaHashMap_IteratorNext(LuaHashMapIterator* hash_iterator);
LuaHashMapIterator LuaHashMap_GetIteratorBeginForKeyString(LuaHashMap* hash_map);
LuaHashMapIterator LuaHashMap_GetIteratorEndForKeyString(LuaHashMap* hash_map);
LuaHashMapIterator LuaHashMap_GetIteratorForKeyString(LuaHashMap* hash_map, const char* key_string);
bool LuaHashMap_IteratorIsEqual(LuaHashMapIterator* hash_iterator1, LuaHashMapIterator* hash_iterator2);

	
void LuaHashMap_InsertValueStringAtIterator(LuaHashMapIterator* hash_iterator, const char* restrict value_string);
const char* LuaHashMap_GetValueStringAtIterator(LuaHashMapIterator* hash_iterator);
bool LuaHashMap_ExistsAtIterator(LuaHashMapIterator* hash_iterator);
void LuaHashMap_RemoveAtIterator(LuaHashMapIterator* hash_iterator);

	
#if defined(__RESTRICT_KEYWORD_DEFINED__)
	#undef restrict
	#undef __RESTRICT_KEYWORD_DEFINED__
#endif

#ifdef __cplusplus
}
#endif



#endif /* C_LUA_HASH_MAP_H */

