/*
 LuaHashMap
 Copyright (C) 2011-2012 PlayControl Software, LLC. 
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

#if defined(LUAHASHMAP_DONT_EXPOSE_LUA_HEADER)
	#if !defined(lua_Number)
		#define lua_Number double
	#endif
	#if !defined(lua_Integer)
		#define lua_Integer ptrdiff_t
	#endif
	#if !defined(lua_State)
		#define lua_State void
	#endif
#else
	#include "lua.h"
#endif
	
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
	/** @cond DOXYGEN_SHOULD_IGNORE_THIS */
	
	/* Note: For Doxygen to produce clean output, you should set the 
	 * PREDEFINED option to remove DECLSPEC, CALLCONVENTION, and
	 * the DOXYGEN_SHOULD_IGNORE_THIS blocks.
	 * PREDEFINED = DOXYGEN_SHOULD_IGNORE_THIS=1 DECLSPEC= CALLCONVENTION=
	 */
	
	/** Windows needs to know explicitly which functions to export in a DLL. */

#ifdef BUILD_LUAHASHMAP_AS_DLL
	#ifdef WIN32
		#define LUAHASHMAP_EXPORT __declspec(dllexport)
	#elif defined(__GNUC__) && __GNUC__ >= 4
		#define LUAHASHMAP_EXPORT __attribute__ ((visibility("default")))
	#else
		#define LUAHASHMAP_EXPORT
	#endif
#else
	#define LUAHASHMAP_EXPORT
#endif /* BUILD_LUAHASHMAP_AS_DLL */
	
	
/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */
	

typedef struct LuaHashMap LuaHashMap;

typedef int LuaHashMap_InternalGlobalKeyType;
	
/* Mental Model: LuaHashMapIterators (unlike LuaHashMap) are stack objects. No dynamic memory is required.
 * This allows you to use iterators without worrying about leaking.
 */
LUAHASHMAP_EXPORT struct LuaHashMapIterator
{
	/* These are all implementation details.
	 * You should probably not directly touch.
	 */
	LuaHashMap* hashMap;
	LuaHashMap_InternalGlobalKeyType whichTable;
	int keyType;
	bool atEnd;
	union LuaHashMapKeyType
	{
		const char* keyString;
		lua_Number keyNumber;
/*		lua_Integer keyInteger; */
		void* keyPointer;
	} currentKey;
};

typedef struct LuaHashMapIterator LuaHashMapIterator;

/* No longer used. */
/*
#define LUAHASHMAP_KEYSTRING_TYPE		0x01
#define LUAHASHMAP_KEYPOINTER_TYPE		0x02
#define LUAHASHMAP_KEYNUMBER_TYPE		0x04
#define LUAHASHMAP_KEYINTEGER_TYPE		0x08

#define LUAHASHMAP_VALUESTRING_TYPE		0x10
#define LUAHASHMAP_VALUEPOINTER_TYPE	0x20
#define LUAHASHMAP_VALUENUMBER_TYPE		0x40
#define LUAHASHMAP_VALUEINTEGER_TYPE	0x80
*/
	
LUAHASHMAP_EXPORT LuaHashMap* LuaHashMap_Create(void);
LUAHASHMAP_EXPORT LuaHashMap* LuaHashMap_CreateWithAllocator(lua_Alloc the_allocator, void* user_data);

LUAHASHMAP_EXPORT LuaHashMap* LuaHashMap_CreateWithSizeHints(int number_of_array_elements, int number_of_hash_elements);
LUAHASHMAP_EXPORT LuaHashMap* LuaHashMap_CreateWithAllocatorAndSizeHints(lua_Alloc the_allocator, void* user_data, int number_of_array_elements, int number_of_hash_elements);

/* Special Memory Optimization: Allows you to create new LuaHashMaps from an existing one which will share the same lua_State under the hood.
 * My measurements of a new lua_State instance seem to take about 4-5KB on 64-bit Mac. This will avoid incuring that cost.
 * Technically speaking, the original and shared maps are peers of each other. The implementation does not make a distinction 
 * about which one the original is so any hash map with the lua_State you want to share may be passed in as the parameter.
 * Make sure to free any shared maps with FreeShare() before you close the final hash map with Free() as Free() calls lua_close().
 */
LUAHASHMAP_EXPORT LuaHashMap* LuaHashMap_CreateShare(LuaHashMap* original_hash_map);
LUAHASHMAP_EXPORT LuaHashMap* LuaHashMap_CreateShareWithSizeHints(LuaHashMap* original_hash_map, int number_of_array_elements, int number_of_hash_elements);


/* Important Note: This closes the lua_State. If there are any shared hash maps (used CreateShare), 
 * the shared hash maps should be freed before this is called.
 */
LUAHASHMAP_EXPORT void LuaHashMap_Free(LuaHashMap* hash_map);
LUAHASHMAP_EXPORT void LuaHashMap_FreeShare(LuaHashMap* hash_map);

/* This is only for backdoor access. Most people should never use this */
LUAHASHMAP_EXPORT lua_State* LuaHashMap_GetLuaState(LuaHashMap* hash_map);
	
/* string, string */
/* Note: Inserting NULL values is like deleting a field. */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringForKeyString(LuaHashMap* hash_map, const char* restrict value_string, const char* restrict key_string);
/* string, pointer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValuePointerForKeyString(LuaHashMap* hash_map, void* value_pointer, const char* restrict key_string);	
/* string, number */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueNumberForKeyString(LuaHashMap* hash_map, lua_Number value_number, const char* restrict key_string);
/* string, integer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueIntegerForKeyString(LuaHashMap* hash_map, lua_Integer value_integer, const char* restrict key_string);


/* pointer, string */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringForKeyPointer(LuaHashMap* hash_map, const char* restrict value_string, void* key_pointer);
/* pointer, pointer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValuePointerForKeyPointer(LuaHashMap* hash_map, void* value_pointer, void* key_pointer);
/* pointer, number */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueNumberForKeyPointer(LuaHashMap* hash_map, lua_Number value_number, void* key_pointer);
/* pointer, integer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueIntegerForKeyPointer(LuaHashMap* hash_map, lua_Integer value_integer, void* key_pointer);
	
	
/* number, string */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringForKeyNumber(LuaHashMap* hash_map, const char* restrict value_string, lua_Number key_number);
/* number, pointer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValuePointerForKeyNumber(LuaHashMap* hash_map, void* value_pointer, lua_Number key_number);
/* number, number */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueNumberForKeyNumber(LuaHashMap* hash_map, lua_Number value_number, lua_Number key_number);
/* number, integer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueIntegerForKeyNumber(LuaHashMap* hash_map, lua_Integer value_integer, lua_Number key_number);

	
/* integer, string */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringForKeyInteger(LuaHashMap* hash_map, const char* restrict value_string, lua_Integer key_integer);
/* integer, pointer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValuePointerForKeyInteger(LuaHashMap* hash_map, void* value_pointer, lua_Integer key_integer);
/* integer, number */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueNumberForKeyInteger(LuaHashMap* hash_map, lua_Number value_number, lua_Integer key_integer);
/* integer, integer*/
LUAHASHMAP_EXPORT void LuaHashMap_SetValueIntegerForKeyInteger(LuaHashMap* hash_map, lua_Integer value_integer, lua_Integer key_integer);
	


/* Get Functions */
LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyString(LuaHashMap* hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT void* LuaHashMap_GetValuePointerForKeyString(LuaHashMap* hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetValueNumberForKeyString(LuaHashMap* hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetValueIntegerForKeyString(LuaHashMap* hash_map, const char* restrict key_string);


LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyPointer(LuaHashMap* hash_map, void* key_pointer);
LUAHASHMAP_EXPORT void* LuaHashMap_GetValuePointerForKeyPointer(LuaHashMap* hash_map, void* key_pointer);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetValueNumberForKeyPointer(LuaHashMap* hash_map, void* key_pointer);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetValueIntegerForKeyPointer(LuaHashMap* hash_map, void* key_pointer);

	
LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
LUAHASHMAP_EXPORT void* LuaHashMap_GetValuePointerForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetValueNumberForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetValueIntegerForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
	
LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);
LUAHASHMAP_EXPORT void* LuaHashMap_GetValuePointerForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetValueNumberForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetValueIntegerForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);


/* Exists Functions*/
LUAHASHMAP_EXPORT bool LuaHashMap_ExistsKeyString(LuaHashMap* hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT bool LuaHashMap_ExistsKeyPointer(LuaHashMap* hash_map, void* key_pointer);
LUAHASHMAP_EXPORT bool LuaHashMap_ExistsKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
LUAHASHMAP_EXPORT bool LuaHashMap_ExistsKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);


/* Remove functions */
LUAHASHMAP_EXPORT void LuaHashMap_RemoveKeyString(LuaHashMap* hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT void LuaHashMap_RemoveKeyPointer(LuaHashMap* hash_map, void* key_pointer);
LUAHASHMAP_EXPORT void LuaHashMap_RemoveKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
LUAHASHMAP_EXPORT void LuaHashMap_RemoveKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);


/* Clear List */
/* This removes all entries, but doesn't shrink the hash (doesn't reclaim memory). */
LUAHASHMAP_EXPORT void LuaHashMap_Clear(LuaHashMap* hash_map);
/* This removes all entries and resets the hash size to 0 (reclaims memory). */
LUAHASHMAP_EXPORT void LuaHashMap_Purge(LuaHashMap* hash_map);

LUAHASHMAP_EXPORT bool LuaHashMap_IsEmpty(LuaHashMap* hash_map);

/* Iterator functions */
LUAHASHMAP_EXPORT bool LuaHashMap_IteratorNext(LuaHashMapIterator* hash_iterator);


LUAHASHMAP_EXPORT LuaHashMapIterator LuaHashMap_GetIteratorAtBegin(LuaHashMap* hash_map);
LUAHASHMAP_EXPORT LuaHashMapIterator LuaHashMap_GetIteratorAtEnd(LuaHashMap* hash_map);
LUAHASHMAP_EXPORT LuaHashMapIterator LuaHashMap_GetIteratorForKeyString(LuaHashMap* hash_map, const char* key_string);
LUAHASHMAP_EXPORT LuaHashMapIterator LuaHashMap_GetIteratorForKeyPointer(LuaHashMap* hash_map, void* key_pointer);
LUAHASHMAP_EXPORT LuaHashMapIterator LuaHashMap_GetIteratorForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
LUAHASHMAP_EXPORT LuaHashMapIterator LuaHashMap_GetIteratorForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);

LUAHASHMAP_EXPORT bool LuaHashMap_IteratorIsEqual(const LuaHashMapIterator* hash_iterator1, const LuaHashMapIterator* hash_iterator2);

	
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringAtIterator(LuaHashMapIterator* hash_iterator, const char* restrict value_string);
LUAHASHMAP_EXPORT void LuaHashMap_SetValuePointerAtIterator(LuaHashMapIterator* hash_iterator, void* value_pointer);
LUAHASHMAP_EXPORT void LuaHashMap_SetValueNumberAtIterator(LuaHashMapIterator* hash_iterator, lua_Number value_number);
LUAHASHMAP_EXPORT void LuaHashMap_SetValueIntegerAtIterator(LuaHashMapIterator* hash_iterator, lua_Integer value_integer);

	
LUAHASHMAP_EXPORT const char* LuaHashMap_GetKeyStringAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT void* LuaHashMap_GetKeyPointerAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetKeyNumberAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetKeyIntegerAtIterator(LuaHashMapIterator* hash_iterator);

LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT void* LuaHashMap_GetValuePointerAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetValueNumberAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetValueIntegerAtIterator(LuaHashMapIterator* hash_iterator);

LUAHASHMAP_EXPORT bool LuaHashMap_ExistsAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT void LuaHashMap_RemoveAtIterator(LuaHashMapIterator* hash_iterator);




/* Experimental Functions: These might be removed */
/* This is O(n). Since it is slow, it should be used sparingly. */
LUAHASHMAP_EXPORT size_t LuaHashMap_Count(LuaHashMap* hash_map);	

/* I don't think I really want to support mixed types in a single hashmap. But if I do, then you need to be able to figure out the type. */
LUAHASHMAP_EXPORT int LuaHashMap_GetKeyTypeAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT int LuaHashMap_GetValueTypeAtIterator(LuaHashMapIterator* hash_iterator);

/* List Functions */
/* The iterator functions are much cleaner than these. These are also O(n). 
 * Also, now that mixing key types in the same hash is no longer explictly forbidden/caught, 
 * these functions are not resilient to mixed keys.
 * Maybe these functions should be removed. 
 */
LUAHASHMAP_EXPORT size_t LuaHashMap_GetKeysString(LuaHashMap* hash_map, const char* keys_array[], size_t max_array_size);
LUAHASHMAP_EXPORT size_t LuaHashMap_GetKeysPointer(LuaHashMap* hash_map, void* keys_array[], size_t max_array_size);
LUAHASHMAP_EXPORT size_t LuaHashMap_GetKeysNumber(LuaHashMap* hash_map, lua_Number keys_array[], size_t max_array_size);
LUAHASHMAP_EXPORT size_t LuaHashMap_GetKeysInteger(LuaHashMap* hash_map, lua_Integer keys_array[], size_t max_array_size);
/* End Experiemental Functions */	



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

