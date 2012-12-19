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
/**
@mainpage LuaHashMap: An easy to use hash table implementation for C.
LuaHashMap is a hash table library/implementation for use in C. Instead of reinventing 
the wheel again to implement a hash table for C, LuaHashMap cleverly wraps Lua.

Lua is designed to be embedded in C programs and is fast and lightweight. 
And tables are the sole data structure in Lua so the underlying hash table is 
fast, reliable, well documented/understood, and proven. 

So rather than reinventing a new hash table implementation,
LuaHashMap simply wraps Lua's to mimimize bugs and leverage known performance and behaviors.
But to you (the end user/developer), LuaHashMap provides a simple/straight-forward C API 
for hash tables and Lua is simply an implementation detail, so if you don't know anything 
about Lua, it is not a problem. (But for those wondering about the implementation details, LuaHashMap itself is completely written in C and uses Lua's C API exclusively without any actual Lua script code.)

LuaHashMap is designed to be easy to use and convenient. 
LuaHashMap can work with keys and values of types string, number, and pointer.
The API provides explicit function names for each permutation of types to 
avoid macro hell and hard to debug situations. (Though C11 users, check out the _Generic macros.)
And there are no hashing functions you need to provide. (All of this was already tuned by Lua.)


There are two ways to access 



I said you didn't need to know anything about Lua. Well, that's mostly true, 
but there are some interesting details you should know about in Lua which may be useful.

lua_Number and lua_Integer
Canonical (unmodified) Lua only has one number type, which by default is double. 
Double has enough bits to store a 32-bit integer without any loss of precision which is why this can work.
But you may notice that LuaHashMap has APIs to deal with both lua_Number (floating point) and lua_Integer (integer).
Canonical Lua converts everything to lua_Number so if you provide a lua_Number key that happens to be the same value as a lua_Integer key,
these may in fact be the same key. (There are patches for Lua that may give you separate values.) 

Lua internalizes strings
As an implementation detail, all strings in Lua are internalized which means there is only once instance of a string within Lua for duplicate strings. 
This potentially can be exploited to yield further optimizations in your code. 
LuaHashMap does not rely on this fact, 
but do note that the APIs return the internalized string pointer when you add a key (which may be different than the one you fed the API).
This was intended to allow you to exploit this fact if you choose.

Lua does not have a moving garbage collector nor does it move its memory around invalidating pointers.
LuaHashMap does exploit an implementation detail of Lua for functions that return strings, 
such as const char* LuaHashMap_GetValueStringForKeyString. 
In the Lua API, you communicate between C and Lua using a stack API provided by Lua.
When you are finished with an operation, the stack should generally be returned to the state you started in.
That means when this function finishes, the Lua stack should be reset. 
Technically, the pointer to string that is returned is not guaranteed by the Lua specification to still be valid.
But as an implementation detail, the Lua authors have stated that this pointer is still valid as long as the string is still in Lua (not removed).

Lua tables have an array optimization
If you create a hash with sequential, continuous integer keys, you may trigger an optimization in Lua which treats that an array. 
This will yield very performance fast times as Lua is working with C arrays instead of hashing.




Advanced tricks:
CreateShare
Unfortunately, using the standard API, each LuaHashMap instance also creates an entirely new Lua virtual machine instance.
On a 64-bit machine, this amounts to about 4KB of RAM. For large data sets, you will not notice, 
but for small data sets and a lot of separate instances of LuaHashMap, this might add up.
So an addition to the standard API, a CreateShare API exists to allow you to create a new instance of LuaHashMap
without creating an entirely new virtual machine instance. Instead it will simply create a new Lua table instance
in the existing virtual machine and not incur any more RAM overhead.
Every CreateShare should be balanced by FreeShare. Free should balance the original Create and should be the very last thing to be called. (Free will destroy the entire virtual machine instance.)


LuaHashMap_GetKeyTypeAtIterator
LuaHashMap_GetValueTypeAtIterator


Cache


C11 _Generic

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

/* This define should generally not be used. 
 It is here mostly for hacking/experimentation and dirty tricks to get going for non-production code.
 Make sure these defines match what was actually used in your Lua library.
 */
#if defined(LUAHASHMAP_DONT_EXPOSE_LUA_HEADER)
	#if !defined(lua_Number)
		#define lua_Number double
		#define __LUAHASHMAP_LUA_NUMBER_DEFINED__	
	#endif
	#if !defined(lua_Integer)
		#define lua_Integer ptrdiff_t
		#define __LUAHASHMAP_LUA_INTEGER_DEFINED__		
	#endif
	#if !defined(lua_State)
		#define lua_State void
		#define __LUAHASHMAP_LUA_STATE_DEFINED__			
	#endif
	#if !defined(lua_h) /* You detect nor undo a typedef */
		typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);
		#define __LUAHASHMAP_LUA_ALLOC_DEFINED__					
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

#ifdef LUAHASHMAP_BUILD_AS_DLL
	#ifdef WIN32
		#define LUAHASHMAP_EXPORT __declspec(dllexport)
	#elif defined(__GNUC__) && __GNUC__ >= 4
		#define LUAHASHMAP_EXPORT __attribute__ ((visibility("default")))
	#else
		#define LUAHASHMAP_EXPORT
	#endif
#else
	#define LUAHASHMAP_EXPORT
#endif /* LUAHASHMAP_BUILD_AS_DLL */
	
	
/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */

/**
 * Struct that contains the version information of this library.
 * This represents the library's version as three levels: major revision
 * (increments with massive changes, additions, and enhancements),
 * minor revision (increments with backwards-compatible changes to the
 * major revision), and patchlevel (increments with fixes to the minor
 * revision).
 * @see LUAHASHMAP_GET_COMPILED_VERSION, LuaHashMap_GetLinkedVersion
 */
typedef struct LuaHashMapVersion
{
	unsigned char major;
	unsigned char minor;
	unsigned char patch;
} LuaHashMapVersion;

/* Printable format: "%d.%d.%d", MAJOR, MINOR, PATCHLEVEL
 */
#define LUAHASHMAP_MAJOR_VERSION		0
#define LUAHASHMAP_MINOR_VERSION		2
#define LUAHASHMAP_PATCHLEVEL			0

/**
 * This macro fills in a version structure with the version of the
 * library you compiled against. This is determined by what header the
 * compiler uses. Note that if you dynamically linked the library, you might
 * have a slightly newer or older version at runtime. That version can be
 * determined with LuaHashMap_GetLinkedVersion(), which, unlike 
 * LUAHASHMAP_GET_COMPILED_VERSION, is not a macro.
 *
 * @note When compiled with SDL, this macro can be used to fill a version structure 
 * compatible with SDL_version.
 *
 * @param X A pointer to a LuaHashMapVersion struct to initialize.
 *
 * @see LuaHashMapVersion, LuaHashMap_GetLinkedVersion
 */
#define LUAHASHMAP_GET_COMPILED_VERSION(X)		\
	{											\
		(X)->major = LUAHASHMAP_MAJOR_VERSION;	\
		(X)->minor = LUAHASHMAP_MINOR_VERSION;	\
		(X)->patch = LUAHASHMAP_PATCHLEVEL;		\
	}

/**
 * Gets the library version of the dynamically linked library you are using.
 * This gets the version of the library that is linked against your program.
 * If you are using a shared library (DLL) version, then it is
 * possible that it will be different than the version you compiled against.
 *
 * This is a real function; the macro LUAHASHMAP_GET_COMPILED_VERSION 
 * tells you what version of the library you compiled against:
 *
 * @code
 * LuaHashMapVersion compiled;
 * LuaHashMapVersion linked;
 *
 * LUAHASHMAP_GET_COMPILED_VERSION(&compiled);
 * LuaHashMap_GetLinkedVersion(&linked);
 * printf("We compiled against version %d.%d.%d ...\n",
 *           compiled.major, compiled.minor, compiled.patch);
 * printf("But we linked against version %d.%d.%d.\n",
 *           linked.major, linked.minor, linked.patch);
 * @endcode
 *
 * @see LuaHashMapVersion, LUAHASHMAP_GET_COMPILED_VERSION
 */
LUAHASHMAP_EXPORT const LuaHashMapVersion* LuaHashMap_GetLinkedVersion(void);

typedef struct LuaHashMap LuaHashMap;

typedef int LuaHashMap_InternalGlobalKeyType;

struct LUAHASHMAP_EXPORT LuaHashMapStringContainer
{
	size_t stringLength;
	const char* stringPointer;
};

typedef struct LuaHashMapStringContainer LuaHashMapStringContainer;


union LuaHashMapKeyValueType
{
/*	const char* theString; */
	/* If the size of lua_Number is the same or greater than the StringContainer, we succeed in saving some space using a union.
	 * This can happen in 32-bit where double is 8-bytes and size_t + pointer is 4-bytes + 4-bytes == 8 bytes. */
	LuaHashMapStringContainer theString;
	lua_Number theNumber;
/*		lua_Integer theInteger; */
	void* thePointer;
};
	
/* Mental Model: LuaHashMapIterators (unlike LuaHashMap) are stack objects. No dynamic memory is required.
 * This allows you to use iterators without worrying about leaking.
 */
struct LUAHASHMAP_EXPORT LuaHashMapIterator
{
	/* These are all implementation details.
	 * You should probably not directly touch.
	 */
	union LuaHashMapKeyValueType currentKey;
	union LuaHashMapKeyValueType currentValue;
	LuaHashMap* hashMap;
	LuaHashMap_InternalGlobalKeyType whichTable;
	int keyType;
	int valueType;
	bool atEnd;
	bool isNext;
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
	
	
/* string, string */
/* Note: Inserting NULL for string values is like deleting a field, but not for pointer values */
/* Note: String returned is the Lua internalized pointer for the key string */
LUAHASHMAP_EXPORT const char* LuaHashMap_SetValueStringForKeyString(LuaHashMap* restrict hash_map, const char* value_string, const char* key_string);
LUAHASHMAP_EXPORT const char* LuaHashMap_SetValueStringForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* value_string, const char* key_string, size_t value_string_length, size_t key_string_length);
/* string, pointer */
LUAHASHMAP_EXPORT const char* LuaHashMap_SetValuePointerForKeyString(LuaHashMap* hash_map, void* value_pointer, const char* key_string);	
LUAHASHMAP_EXPORT const char* LuaHashMap_SetValuePointerForKeyStringWithLength(LuaHashMap* hash_map, void* value_pointer, const char* key_string, size_t key_string_length);	
/* string, number */
LUAHASHMAP_EXPORT const char* LuaHashMap_SetValueNumberForKeyString(LuaHashMap* restrict hash_map, lua_Number value_number, const char* restrict key_string);
LUAHASHMAP_EXPORT const char* LuaHashMap_SetValueNumberForKeyStringWithLength(LuaHashMap* restrict hash_map, lua_Number value_number, const char* restrict key_string, size_t key_string_length);
/* string, integer */
LUAHASHMAP_EXPORT const char* LuaHashMap_SetValueIntegerForKeyString(LuaHashMap* restrict hash_map, lua_Integer value_integer, const char* restrict key_string);
LUAHASHMAP_EXPORT const char* LuaHashMap_SetValueIntegerForKeyStringWithLength(LuaHashMap* restrict hash_map, lua_Integer value_integer, const char* restrict key_string, size_t key_string_length);


/* pointer, string */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringForKeyPointer(LuaHashMap* hash_map, const char* value_string, void* key_pointer);
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringForKeyPointerWithLength(LuaHashMap* hash_map, const char* value_string, void* key_pointer, size_t value_string_length);
/* pointer, pointer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValuePointerForKeyPointer(LuaHashMap* hash_map, void* value_pointer, void* key_pointer);
/* pointer, number */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueNumberForKeyPointer(LuaHashMap* hash_map, lua_Number value_number, void* key_pointer);
/* pointer, integer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueIntegerForKeyPointer(LuaHashMap* hash_map, lua_Integer value_integer, void* key_pointer);
	
	
/* number, string */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringForKeyNumber(LuaHashMap* restrict hash_map, const char* restrict value_string, lua_Number key_number);
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringForKeyNumberWithLength(LuaHashMap* restrict hash_map, const char* restrict value_string, lua_Number key_number, size_t value_string_length);
/* number, pointer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValuePointerForKeyNumber(LuaHashMap* hash_map, void* value_pointer, lua_Number key_number);
/* number, number */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueNumberForKeyNumber(LuaHashMap* hash_map, lua_Number value_number, lua_Number key_number);
/* number, integer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueIntegerForKeyNumber(LuaHashMap* hash_map, lua_Integer value_integer, lua_Number key_number);

	
/* integer, string */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringForKeyInteger(LuaHashMap* restrict hash_map, const char* restrict value_string, lua_Integer key_integer);
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringForKeyIntegerWithLength(LuaHashMap* restrict hash_map, const char* restrict value_string, lua_Integer key_integer, size_t value_string_length);
/* integer, pointer */
LUAHASHMAP_EXPORT void LuaHashMap_SetValuePointerForKeyInteger(LuaHashMap* hash_map, void* value_pointer, lua_Integer key_integer);
/* integer, number */
LUAHASHMAP_EXPORT void LuaHashMap_SetValueNumberForKeyInteger(LuaHashMap* hash_map, lua_Number value_number, lua_Integer key_integer);
/* integer, integer*/
LUAHASHMAP_EXPORT void LuaHashMap_SetValueIntegerForKeyInteger(LuaHashMap* hash_map, lua_Integer value_integer, lua_Integer key_integer);
	


/* Get Functions */
LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t* value_string_length_return, size_t key_string_length);
LUAHASHMAP_EXPORT void* LuaHashMap_GetValuePointerForKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT void* LuaHashMap_GetValuePointerForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetValueNumberForKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetValueNumberForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetValueIntegerForKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetValueIntegerForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length);



LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyPointer(LuaHashMap* hash_map, void* key_pointer);
LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyPointerWithLength(LuaHashMap* hash_map, void* key_pointer, size_t* value_string_length_return);
LUAHASHMAP_EXPORT void* LuaHashMap_GetValuePointerForKeyPointer(LuaHashMap* hash_map, void* key_pointer);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetValueNumberForKeyPointer(LuaHashMap* hash_map, void* key_pointer);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetValueIntegerForKeyPointer(LuaHashMap* hash_map, void* key_pointer);

	
LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyNumberWithLength(LuaHashMap* hash_map, lua_Number key_number, size_t* value_string_length_return);
LUAHASHMAP_EXPORT void* LuaHashMap_GetValuePointerForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetValueNumberForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetValueIntegerForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);

LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);
LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringForKeyIntegerWithLength(LuaHashMap* hash_map, lua_Integer key_integer, size_t* value_string_length_return);
LUAHASHMAP_EXPORT void* LuaHashMap_GetValuePointerForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetValueNumberForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetValueIntegerForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);


/* Exists Functions*/
LUAHASHMAP_EXPORT bool LuaHashMap_ExistsKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT bool LuaHashMap_ExistsKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length);
LUAHASHMAP_EXPORT bool LuaHashMap_ExistsKeyPointer(LuaHashMap* hash_map, void* key_pointer);
LUAHASHMAP_EXPORT bool LuaHashMap_ExistsKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
LUAHASHMAP_EXPORT bool LuaHashMap_ExistsKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);


/* Remove functions */
LUAHASHMAP_EXPORT void LuaHashMap_RemoveKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT void LuaHashMap_RemoveKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length);
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

/* Find functions: Returns an iterator at the designated position if found. 
 @see LuaHashMap_IteratorIsNotFound, LuaHashMap_ExistsKeyString, LuaHashMap_ExistsKeyPointer, LuaHashMap_ExistsKeyNumber, LuaHashMap_ExistsKeyInteger 
 */
LUAHASHMAP_EXPORT LuaHashMapIterator LuaHashMap_GetIteratorForKeyString(LuaHashMap* restrict hash_map, const char* restrict key_string);
LUAHASHMAP_EXPORT LuaHashMapIterator LuaHashMap_GetIteratorForKeyStringWithLength(LuaHashMap* restrict hash_map, const char* restrict key_string, size_t key_string_length);
LUAHASHMAP_EXPORT LuaHashMapIterator LuaHashMap_GetIteratorForKeyPointer(LuaHashMap* hash_map, void* key_pointer);
LUAHASHMAP_EXPORT LuaHashMapIterator LuaHashMap_GetIteratorForKeyNumber(LuaHashMap* hash_map, lua_Number key_number);
LUAHASHMAP_EXPORT LuaHashMapIterator LuaHashMap_GetIteratorForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer);

/* Returns true if the iterator is bad (i.e. you tried to get an iterator for a key that doesn't exist. End iterators are distinct from NotFound. */
LUAHASHMAP_EXPORT bool LuaHashMap_IteratorIsNotFound(const LuaHashMapIterator* hash_iterator);
/* Returns true if two iterators are pointing to the same location */
LUAHASHMAP_EXPORT bool LuaHashMap_IteratorIsEqual(const LuaHashMapIterator* hash_iterator1, const LuaHashMapIterator* hash_iterator2);

	
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringAtIterator(LuaHashMapIterator* restrict hash_iterator, const char* restrict value_string);
LUAHASHMAP_EXPORT void LuaHashMap_SetValueStringAtIteratorWithLength(LuaHashMapIterator* restrict hash_iterator, const char* restrict value_string, size_t value_string_length);
LUAHASHMAP_EXPORT void LuaHashMap_SetValuePointerAtIterator(LuaHashMapIterator* hash_iterator, void* value_pointer);
LUAHASHMAP_EXPORT void LuaHashMap_SetValueNumberAtIterator(LuaHashMapIterator* hash_iterator, lua_Number value_number);
LUAHASHMAP_EXPORT void LuaHashMap_SetValueIntegerAtIterator(LuaHashMapIterator* hash_iterator, lua_Integer value_integer);

	
LUAHASHMAP_EXPORT const char* LuaHashMap_GetKeyStringAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT const char* LuaHashMap_GetKeyStringAtIteratorWithLength(LuaHashMapIterator* hash_iterator, size_t* key_string_length_return);
LUAHASHMAP_EXPORT size_t LuaHashMap_GetKeyStringLengthAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT void* LuaHashMap_GetKeyPointerAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetKeyNumberAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetKeyIntegerAtIterator(LuaHashMapIterator* hash_iterator);

LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT const char* LuaHashMap_GetValueStringAtIteratorWithLength(LuaHashMapIterator* hash_iterator, size_t* value_string_length_return);
LUAHASHMAP_EXPORT void* LuaHashMap_GetValuePointerAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetValueNumberAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetValueIntegerAtIterator(LuaHashMapIterator* hash_iterator);

LUAHASHMAP_EXPORT bool LuaHashMap_ExistsAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT void LuaHashMap_RemoveAtIterator(LuaHashMapIterator* hash_iterator);




/* Experimental Functions: These might be removed, modified, or made permanent. */
/* This is O(n). Since it is slow, it should be used sparingly. */
LUAHASHMAP_EXPORT size_t LuaHashMap_Count(LuaHashMap* hash_map);	

/* I don't know if I really want to support mixed types in a single hashmap. But if I do, then you need to be able to figure out the type. */
LUAHASHMAP_EXPORT int LuaHashMap_GetKeyTypeAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT int LuaHashMap_GetValueTypeAtIterator(LuaHashMapIterator* hash_iterator);

/* Gets the value contained (copied) in the iterator as opposed to doing an actual hash map lookup.
 * If you have just retrieved the iterator and have not altered anything in the hash map behind the back of the iterator,
 * then this value should be the same as what is in the actual hash map.
 */
LUAHASHMAP_EXPORT int LuaHashMap_GetCachedValueTypeAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT const char* LuaHashMap_GetCachedValueStringAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT const char* LuaHashMap_GetCachedValueStringAtIteratorWithLength(LuaHashMapIterator* hash_iterator, size_t* value_string_length_return);
LUAHASHMAP_EXPORT size_t LuaHashMap_GetCachedValueStringLengthAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT void* LuaHashMap_GetCachedValuePointerAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT lua_Number LuaHashMap_GetCachedValueNumberAtIterator(LuaHashMapIterator* hash_iterator);
LUAHASHMAP_EXPORT lua_Integer LuaHashMap_GetCachedValueIntegerAtIterator(LuaHashMapIterator* hash_iterator);


/* This is only for backdoor access. This is for very advanced use cases that want to directly interact with the Lua State. */
LUAHASHMAP_EXPORT lua_State* LuaHashMap_GetLuaState(LuaHashMap* hash_map);

/* This is for very advanced use cases that want to directly interact with the Lua State. 
 An understanding of the implementation details of LuaHashMap is strongly recommended in order to avoid trampling over each other.
 */	
LUAHASHMAP_EXPORT LuaHashMap* LuaHashMap_CreateShareFromLuaState(lua_State* lua_state);
LUAHASHMAP_EXPORT LuaHashMap* LuaHashMap_CreateShareFromLuaStateWithAllocatorAndSizeHints(lua_State* lua_state, lua_Alloc the_allocator, void* user_data, int number_of_array_elements, int number_of_hash_elements);
LUAHASHMAP_EXPORT LuaHashMap* LuaHashMap_CreateShareFromLuaStateWithSizeHints(lua_State* lua_state, int number_of_array_elements, int number_of_hash_elements);

/* C11 introduces _Generic which presents some interesting possibilities. */
#if ( defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) ) \
	|| ( defined(__clang__) && ( __has_feature(c_generic_selections) || __has_extension(c_generic_selections) ) )



/**
 * LuaHashMap_SetValueForKey
 *
 * This C11 _Generic macro essentially overloads all the 3 parameter SetValue<T>ForKey<T> functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_SetValueForKey(hash_map, value, key) \
	_Generic((hash_map), LuaHashMap*: _Generic((value), \
		const char*: _Generic((key), \
			const char*: LuaHashMap_SetValueStringForKeyString, \
			char*: LuaHashMap_SetValueStringForKeyString, \
			void*: LuaHashMap_SetValueStringForKeyPointer, \
			float: LuaHashMap_SetValueStringForKeyNumber, \
			double: LuaHashMap_SetValueStringForKeyNumber, \
			long double: LuaHashMap_SetValueStringForKeyNumber, \
			char: LuaHashMap_SetValueStringForKeyInteger, \
			unsigned char: LuaHashMap_SetValueStringForKeyInteger, \
			short: LuaHashMap_SetValueStringForKeyInteger, \
			unsigned short: LuaHashMap_SetValueStringForKeyInteger, \
			int: LuaHashMap_SetValueStringForKeyInteger, \
			unsigned int: LuaHashMap_SetValueStringForKeyInteger, \
			long: LuaHashMap_SetValueStringForKeyInteger, \
			unsigned long: LuaHashMap_SetValueStringForKeyInteger, \
			default: LuaHashMap_SetValueStringForKeyPointer),\
		char*: _Generic((key), \
			const char*: LuaHashMap_SetValueStringForKeyString, \
			char*: LuaHashMap_SetValueStringForKeyString, \
			void*: LuaHashMap_SetValueStringForKeyPointer, \
			float: LuaHashMap_SetValueStringForKeyNumber, \
			double: LuaHashMap_SetValueStringForKeyNumber, \
			long double: LuaHashMap_SetValueStringForKeyNumber, \
			char: LuaHashMap_SetValueStringForKeyInteger, \
			unsigned char: LuaHashMap_SetValueStringForKeyInteger, \
			short: LuaHashMap_SetValueStringForKeyInteger, \
			unsigned short: LuaHashMap_SetValueStringForKeyInteger, \
			int: LuaHashMap_SetValueStringForKeyInteger, \
			unsigned int: LuaHashMap_SetValueStringForKeyInteger, \
			long: LuaHashMap_SetValueStringForKeyInteger, \
			unsigned long: LuaHashMap_SetValueStringForKeyInteger, \
			default: LuaHashMap_SetValueStringForKeyPointer),\
    	void*: _Generic((key), \
			const char*: LuaHashMap_SetValuePointerForKeyString, \
			char*: LuaHashMap_SetValuePointerForKeyString, \
			void*: LuaHashMap_SetValuePointerForKeyPointer, \
			float: LuaHashMap_SetValuePointerForKeyNumber, \
			double: LuaHashMap_SetValuePointerForKeyNumber, \
			long double: LuaHashMap_SetValuePointerForKeyNumber, \
			char: LuaHashMap_SetValuePointerForKeyInteger, \
			unsigned char: LuaHashMap_SetValuePointerForKeyInteger, \
			short: LuaHashMap_SetValuePointerForKeyInteger, \
			unsigned short: LuaHashMap_SetValuePointerForKeyInteger, \
			int: LuaHashMap_SetValuePointerForKeyInteger, \
			unsigned int: LuaHashMap_SetValuePointerForKeyInteger, \
			long: LuaHashMap_SetValuePointerForKeyInteger, \
			unsigned long: LuaHashMap_SetValuePointerForKeyInteger, \
			default: LuaHashMap_SetValuePointerForKeyPointer),\
    	float: _Generic((key), \
			const char*: LuaHashMap_SetValueNumberForKeyString, \
			char*: LuaHashMap_SetValueNumberForKeyString, \
			void*: LuaHashMap_SetValueNumberForKeyPointer, \
			float: LuaHashMap_SetValueNumberForKeyNumber, \
			double: LuaHashMap_SetValueNumberForKeyNumber, \
			long double: LuaHashMap_SetValueNumberForKeyNumber, \
			char: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned char: LuaHashMap_SetValueNumberForKeyInteger, \
			short: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned short: LuaHashMap_SetValueNumberForKeyInteger, \
			int: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned int: LuaHashMap_SetValueNumberForKeyInteger, \
			long: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned long: LuaHashMap_SetValueNumberForKeyInteger, \
			default: LuaHashMap_SetValueNumberForKeyPointer),\
    	double: _Generic((key), \
			const char*: LuaHashMap_SetValueNumberForKeyString, \
			char*: LuaHashMap_SetValueNumberForKeyString, \
			void*: LuaHashMap_SetValueNumberForKeyPointer, \
			float: LuaHashMap_SetValueNumberForKeyNumber, \
			double: LuaHashMap_SetValueNumberForKeyNumber, \
			long double: LuaHashMap_SetValueNumberForKeyNumber, \
			char: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned char: LuaHashMap_SetValueNumberForKeyInteger, \
			short: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned short: LuaHashMap_SetValueNumberForKeyInteger, \
			int: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned int: LuaHashMap_SetValueNumberForKeyInteger, \
			long: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned long: LuaHashMap_SetValueNumberForKeyInteger, \
			default: LuaHashMap_SetValueNumberForKeyPointer),\
    	long double: _Generic((key), \
			const char*: LuaHashMap_SetValueNumberForKeyString, \
			char*: LuaHashMap_SetValueNumberForKeyString, \
			void*: LuaHashMap_SetValueNumberForKeyPointer, \
			float: LuaHashMap_SetValueNumberForKeyNumber, \
			double: LuaHashMap_SetValueNumberForKeyNumber, \
			long double: LuaHashMap_SetValueNumberForKeyNumber, \
			char: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned char: LuaHashMap_SetValueNumberForKeyInteger, \
			short: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned short: LuaHashMap_SetValueNumberForKeyInteger, \
			int: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned int: LuaHashMap_SetValueNumberForKeyInteger, \
			long: LuaHashMap_SetValueNumberForKeyInteger, \
			unsigned long: LuaHashMap_SetValueNumberForKeyInteger, \
			default: LuaHashMap_SetValueNumberForKeyPointer),\
    	char: _Generic((key), \
			const char*: LuaHashMap_SetValueIntegerForKeyString, \
			char*: LuaHashMap_SetValueIntegerForKeyString, \
			void*: LuaHashMap_SetValueIntegerForKeyPointer, \
			float: LuaHashMap_SetValueIntegerForKeyNumber, \
			double: LuaHashMap_SetValueIntegerForKeyNumber, \
			long double: LuaHashMap_SetValueIntegerForKeyNumber, \
			char: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
			short: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
			int: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
			long: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
			default: LuaHashMap_SetValueIntegerForKeyPointer), \
    	unsigned char: _Generic((key), \
			const char*: LuaHashMap_SetValueIntegerForKeyString, \
			char*: LuaHashMap_SetValueIntegerForKeyString, \
			void*: LuaHashMap_SetValueIntegerForKeyPointer, \
			float: LuaHashMap_SetValueIntegerForKeyNumber, \
			double: LuaHashMap_SetValueIntegerForKeyNumber, \
			long double: LuaHashMap_SetValueIntegerForKeyNumber, \
			char: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
			short: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
			int: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
			long: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
			default: LuaHashMap_SetValueIntegerForKeyPointer), \
    	short: _Generic((key), \
			const char*: LuaHashMap_SetValueIntegerForKeyString, \
			char*: LuaHashMap_SetValueIntegerForKeyString, \
			void*: LuaHashMap_SetValueIntegerForKeyPointer, \
			float: LuaHashMap_SetValueIntegerForKeyNumber, \
			double: LuaHashMap_SetValueIntegerForKeyNumber, \
			long double: LuaHashMap_SetValueIntegerForKeyNumber, \
			char: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
			short: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
			int: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
			long: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
			default: LuaHashMap_SetValueIntegerForKeyPointer), \
    	unsigned short: _Generic((key), \
			const char*: LuaHashMap_SetValueIntegerForKeyString, \
			char*: LuaHashMap_SetValueIntegerForKeyString, \
			void*: LuaHashMap_SetValueIntegerForKeyPointer, \
			float: LuaHashMap_SetValueIntegerForKeyNumber, \
			double: LuaHashMap_SetValueIntegerForKeyNumber, \
			long double: LuaHashMap_SetValueIntegerForKeyNumber, \
			char: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
			short: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
			int: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
			long: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
			default: LuaHashMap_SetValueIntegerForKeyPointer), \
    	int: _Generic((key), \
			const char*: LuaHashMap_SetValueIntegerForKeyString, \
			char*: LuaHashMap_SetValueIntegerForKeyString, \
			void*: LuaHashMap_SetValueIntegerForKeyPointer, \
			float: LuaHashMap_SetValueIntegerForKeyNumber, \
			double: LuaHashMap_SetValueIntegerForKeyNumber, \
			long double: LuaHashMap_SetValueIntegerForKeyNumber, \
			char: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
			short: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
			int: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
			long: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
			default: LuaHashMap_SetValueIntegerForKeyPointer), \
    	unsigned int: _Generic((key), \
			const char*: LuaHashMap_SetValueIntegerForKeyString, \
			char*: LuaHashMap_SetValueIntegerForKeyString, \
			void*: LuaHashMap_SetValueIntegerForKeyPointer, \
			float: LuaHashMap_SetValueIntegerForKeyNumber, \
			double: LuaHashMap_SetValueIntegerForKeyNumber, \
			long double: LuaHashMap_SetValueIntegerForKeyNumber, \
			char: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
			short: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
			int: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
			long: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
			default: LuaHashMap_SetValueIntegerForKeyPointer), \
    	long: _Generic((key), \
			const char*: LuaHashMap_SetValueIntegerForKeyString, \
			char*: LuaHashMap_SetValueIntegerForKeyString, \
			void*: LuaHashMap_SetValueIntegerForKeyPointer, \
			float: LuaHashMap_SetValueIntegerForKeyNumber, \
			double: LuaHashMap_SetValueIntegerForKeyNumber, \
			long double: LuaHashMap_SetValueIntegerForKeyNumber, \
			char: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
			short: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
			int: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
			long: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
			default: LuaHashMap_SetValueIntegerForKeyPointer), \
    	unsigned long: _Generic((key), \
			const char*: LuaHashMap_SetValueIntegerForKeyString, \
			char*: LuaHashMap_SetValueIntegerForKeyString, \
			void*: LuaHashMap_SetValueIntegerForKeyPointer, \
			float: LuaHashMap_SetValueIntegerForKeyNumber, \
			double: LuaHashMap_SetValueIntegerForKeyNumber, \
			long double: LuaHashMap_SetValueIntegerForKeyNumber, \
			char: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
			short: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
			int: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
			long: LuaHashMap_SetValueIntegerForKeyInteger, \
			unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
			default: LuaHashMap_SetValueIntegerForKeyPointer), \
		default: LuaHashMap_SetValuePointerForKeyPointer) \
	) \
	(hash_map, value, key)


/**
 * LuaHashMap_SetValueForKeyWithLength
 *
 * This C11 _Generic macro essentially overloads all the 4 parameter SetValue<T>ForKey<T>WithLength functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_SetValueForKeyWithLength(hash_map, value, key, strlength) \
	_Generic((hash_map), LuaHashMap*: _Generic((value), \
		const char*: _Generic((key), \
			void*: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				short: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				int: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				long: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				default: LuaHashMap_SetValueStringForKeyPointerWithLength), \
			float: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				default: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			double: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				default: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			long double: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				default: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			char: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned char: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			short: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned short: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			int: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned int: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			long: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned long: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			default: LuaHashMap_SetValueStringForKeyPointerWithLength), \
		char*: _Generic((key), \
			void*: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				short: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				int: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				long: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyPointerWithLength, \
				default: LuaHashMap_SetValueStringForKeyPointerWithLength), \
			float: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				default: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			double: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				default: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			long double: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				default: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			char: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned char: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			short: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned short: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			int: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned int: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			long: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned long: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				default: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			default: LuaHashMap_SetValueStringForKeyPointerWithLength), \
		void*: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				short: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				int: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				long: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				default: LuaHashMap_SetValuePointerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				short: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				int: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				long: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				default: LuaHashMap_SetValuePointerForKeyStringWithLength), \
			default: LuaHashMap_SetValuePointerForKeyStringWithLength), \
		float: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				default: LuaHashMap_SetValueNumberForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				default: LuaHashMap_SetValueNumberForKeyStringWithLength), \
			default: LuaHashMap_SetValueNumberForKeyStringWithLength), \
		double: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				default: LuaHashMap_SetValueNumberForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				default: LuaHashMap_SetValueNumberForKeyStringWithLength), \
			default: LuaHashMap_SetValueNumberForKeyStringWithLength), \
		long double: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				default: LuaHashMap_SetValueNumberForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				default: LuaHashMap_SetValueNumberForKeyStringWithLength), \
			default: LuaHashMap_SetValueNumberForKeyStringWithLength), \
		char: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
		unsigned char: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
		short: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
		unsigned short: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
		int: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
		unsigned int: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
		long: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
		unsigned long: _Generic((key), \
			const char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			default: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
		default: LuaHashMap_SetValuePointerForKeyStringWithLength) \
	) \
	(hash_map, value, key, strlength)


/**
 * LuaHashMap_SetValueAtIterator
 *
 * This C11 _Generic macro essentially overloads all the 2 parameter LuaHashMap_SetValue<T>AtIterator functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_SetValueAtIterator(hash_iterator, value) \
	_Generic((hash_iterator), LuaHashMapIterator*: _Generic((value), \
		const char*: LuaHashMap_SetValueStringAtIterator, \
		char*: LuaHashMap_SetValueStringAtIterator, \
		void*: LuaHashMap_SetValuePointerAtIterator, \
		float: LuaHashMap_SetValueNumberAtIterator, \
		double: LuaHashMap_SetValueNumberAtIterator, \
		long double: LuaHashMap_SetValueNumberAtIterator, \
		char: LuaHashMap_SetValueIntegerAtIterator, \
		unsigned char: LuaHashMap_SetValueIntegerAtIterator, \
		short: LuaHashMap_SetValueIntegerAtIterator, \
		unsigned short: LuaHashMap_SetValueIntegerAtIterator, \
		int: LuaHashMap_SetValueIntegerAtIterator, \
		unsigned int: LuaHashMap_SetValueIntegerAtIterator, \
		long: LuaHashMap_SetValueIntegerAtIterator, \
		unsigned long: LuaHashMap_SetValueIntegerAtIterator, \
		default: LuaHashMap_SetValuePointerAtIterator) \
	) \
	(hash_iterator, value)


/* Adapted from http://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros */
	/* Private multiple macros for each different number of arguments */
	#define LUAHASHMAP_SETVALUE_5(A,B,C,D,E) LuaHashMap_SetValueStringForKeyStringWithLength(A,B,C,D,E)
	#define LUAHASHMAP_SETVALUE_4(A,B,C,D) LuaHashMap_SetValueForKeyWithLength(A,B,C,D)
	/* I wanted to recursively reuse LuaHashMap_SetValueForKey, but clang gives me an undefined symbol error. 
	 * Maybe this is a clang bug? So I end up with a big copy-and-paste duplication.
	 * Also, omitting the defaults in this case gives me compile errors.
	 */
	#define LUAHASHMAP_SETVALUE_3(A,B,C) \
		_Generic((A), \
			LuaHashMap*: _Generic((B), \
				const char*: _Generic((C), \
					const char*: LuaHashMap_SetValueStringForKeyString, \
					char*: LuaHashMap_SetValueStringForKeyString, \
					void*: LuaHashMap_SetValueStringForKeyPointer, \
					float: LuaHashMap_SetValueStringForKeyNumber, \
					double: LuaHashMap_SetValueStringForKeyNumber, \
					long double: LuaHashMap_SetValueStringForKeyNumber, \
					char: LuaHashMap_SetValueStringForKeyInteger, \
					unsigned char: LuaHashMap_SetValueStringForKeyInteger, \
					short: LuaHashMap_SetValueStringForKeyInteger, \
					unsigned short: LuaHashMap_SetValueStringForKeyInteger, \
					int: LuaHashMap_SetValueStringForKeyInteger, \
					unsigned int: LuaHashMap_SetValueStringForKeyInteger, \
					long: LuaHashMap_SetValueStringForKeyInteger, \
					unsigned long: LuaHashMap_SetValueStringForKeyInteger, \
					default: LuaHashMap_SetValueStringForKeyPointer),\
				char*: _Generic((C), \
					const char*: LuaHashMap_SetValueStringForKeyString, \
					char*: LuaHashMap_SetValueStringForKeyString, \
					void*: LuaHashMap_SetValueStringForKeyPointer, \
					float: LuaHashMap_SetValueStringForKeyNumber, \
					double: LuaHashMap_SetValueStringForKeyNumber, \
					long double: LuaHashMap_SetValueStringForKeyNumber, \
					char: LuaHashMap_SetValueStringForKeyInteger, \
					unsigned char: LuaHashMap_SetValueStringForKeyInteger, \
					short: LuaHashMap_SetValueStringForKeyInteger, \
					unsigned short: LuaHashMap_SetValueStringForKeyInteger, \
					int: LuaHashMap_SetValueStringForKeyInteger, \
					unsigned int: LuaHashMap_SetValueStringForKeyInteger, \
					long: LuaHashMap_SetValueStringForKeyInteger, \
					unsigned long: LuaHashMap_SetValueStringForKeyInteger, \
					default: LuaHashMap_SetValueStringForKeyPointer),\
				void*: _Generic((C), \
					const char*: LuaHashMap_SetValuePointerForKeyString, \
					char*: LuaHashMap_SetValuePointerForKeyString, \
					void*: LuaHashMap_SetValuePointerForKeyPointer, \
					float: LuaHashMap_SetValuePointerForKeyNumber, \
					double: LuaHashMap_SetValuePointerForKeyNumber, \
					long double: LuaHashMap_SetValuePointerForKeyNumber, \
					char: LuaHashMap_SetValuePointerForKeyInteger, \
					unsigned char: LuaHashMap_SetValuePointerForKeyInteger, \
					short: LuaHashMap_SetValuePointerForKeyInteger, \
					unsigned short: LuaHashMap_SetValuePointerForKeyInteger, \
					int: LuaHashMap_SetValuePointerForKeyInteger, \
					unsigned int: LuaHashMap_SetValuePointerForKeyInteger, \
					long: LuaHashMap_SetValuePointerForKeyInteger, \
					unsigned long: LuaHashMap_SetValuePointerForKeyInteger, \
					default: LuaHashMap_SetValuePointerForKeyPointer),\
				float: _Generic((C), \
					const char*: LuaHashMap_SetValueNumberForKeyString, \
					char*: LuaHashMap_SetValueNumberForKeyString, \
					void*: LuaHashMap_SetValueNumberForKeyPointer, \
					float: LuaHashMap_SetValueNumberForKeyNumber, \
					double: LuaHashMap_SetValueNumberForKeyNumber, \
					long double: LuaHashMap_SetValueNumberForKeyNumber, \
					char: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned char: LuaHashMap_SetValueNumberForKeyInteger, \
					short: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned short: LuaHashMap_SetValueNumberForKeyInteger, \
					int: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned int: LuaHashMap_SetValueNumberForKeyInteger, \
					long: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned long: LuaHashMap_SetValueNumberForKeyInteger, \
					default: LuaHashMap_SetValueNumberForKeyPointer),\
				double: _Generic((C), \
					const char*: LuaHashMap_SetValueNumberForKeyString, \
					char*: LuaHashMap_SetValueNumberForKeyString, \
					void*: LuaHashMap_SetValueNumberForKeyPointer, \
					float: LuaHashMap_SetValueNumberForKeyNumber, \
					double: LuaHashMap_SetValueNumberForKeyNumber, \
					long double: LuaHashMap_SetValueNumberForKeyNumber, \
					char: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned char: LuaHashMap_SetValueNumberForKeyInteger, \
					short: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned short: LuaHashMap_SetValueNumberForKeyInteger, \
					int: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned int: LuaHashMap_SetValueNumberForKeyInteger, \
					long: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned long: LuaHashMap_SetValueNumberForKeyInteger, \
					default: LuaHashMap_SetValueNumberForKeyPointer),\
				long double: _Generic((C), \
					const char*: LuaHashMap_SetValueNumberForKeyString, \
					char*: LuaHashMap_SetValueNumberForKeyString, \
					void*: LuaHashMap_SetValueNumberForKeyPointer, \
					float: LuaHashMap_SetValueNumberForKeyNumber, \
					double: LuaHashMap_SetValueNumberForKeyNumber, \
					long double: LuaHashMap_SetValueNumberForKeyNumber, \
					char: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned char: LuaHashMap_SetValueNumberForKeyInteger, \
					short: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned short: LuaHashMap_SetValueNumberForKeyInteger, \
					int: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned int: LuaHashMap_SetValueNumberForKeyInteger, \
					long: LuaHashMap_SetValueNumberForKeyInteger, \
					unsigned long: LuaHashMap_SetValueNumberForKeyInteger, \
					default: LuaHashMap_SetValueNumberForKeyPointer),\
				char: _Generic((C), \
					const char*: LuaHashMap_SetValueIntegerForKeyString, \
					char*: LuaHashMap_SetValueIntegerForKeyString, \
					void*: LuaHashMap_SetValueIntegerForKeyPointer, \
					float: LuaHashMap_SetValueIntegerForKeyNumber, \
					double: LuaHashMap_SetValueIntegerForKeyNumber, \
					long double: LuaHashMap_SetValueIntegerForKeyNumber, \
					char: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
					short: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
					int: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
					long: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
					default: LuaHashMap_SetValueIntegerForKeyPointer), \
				unsigned char: _Generic((C), \
					const char*: LuaHashMap_SetValueIntegerForKeyString, \
					char*: LuaHashMap_SetValueIntegerForKeyString, \
					void*: LuaHashMap_SetValueIntegerForKeyPointer, \
					float: LuaHashMap_SetValueIntegerForKeyNumber, \
					double: LuaHashMap_SetValueIntegerForKeyNumber, \
					long double: LuaHashMap_SetValueIntegerForKeyNumber, \
					char: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
					short: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
					int: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
					long: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
					default: LuaHashMap_SetValueIntegerForKeyPointer), \
				short: _Generic((C), \
					const char*: LuaHashMap_SetValueIntegerForKeyString, \
					char*: LuaHashMap_SetValueIntegerForKeyString, \
					void*: LuaHashMap_SetValueIntegerForKeyPointer, \
					float: LuaHashMap_SetValueIntegerForKeyNumber, \
					double: LuaHashMap_SetValueIntegerForKeyNumber, \
					long double: LuaHashMap_SetValueIntegerForKeyNumber, \
					char: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
					short: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
					int: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
					long: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
					default: LuaHashMap_SetValueIntegerForKeyPointer), \
				unsigned short: _Generic((C), \
					const char*: LuaHashMap_SetValueIntegerForKeyString, \
					char*: LuaHashMap_SetValueIntegerForKeyString, \
					void*: LuaHashMap_SetValueIntegerForKeyPointer, \
					float: LuaHashMap_SetValueIntegerForKeyNumber, \
					double: LuaHashMap_SetValueIntegerForKeyNumber, \
					long double: LuaHashMap_SetValueIntegerForKeyNumber, \
					char: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
					short: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
					int: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
					long: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
					default: LuaHashMap_SetValueIntegerForKeyPointer), \
				int: _Generic((C), \
					const char*: LuaHashMap_SetValueIntegerForKeyString, \
					char*: LuaHashMap_SetValueIntegerForKeyString, \
					void*: LuaHashMap_SetValueIntegerForKeyPointer, \
					float: LuaHashMap_SetValueIntegerForKeyNumber, \
					double: LuaHashMap_SetValueIntegerForKeyNumber, \
					long double: LuaHashMap_SetValueIntegerForKeyNumber, \
					char: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
					short: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
					int: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
					long: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
					default: LuaHashMap_SetValueIntegerForKeyPointer), \
				unsigned int: _Generic((C), \
					const char*: LuaHashMap_SetValueIntegerForKeyString, \
					char*: LuaHashMap_SetValueIntegerForKeyString, \
					void*: LuaHashMap_SetValueIntegerForKeyPointer, \
					float: LuaHashMap_SetValueIntegerForKeyNumber, \
					double: LuaHashMap_SetValueIntegerForKeyNumber, \
					long double: LuaHashMap_SetValueIntegerForKeyNumber, \
					char: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
					short: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
					int: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
					long: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
					default: LuaHashMap_SetValueIntegerForKeyPointer), \
				long: _Generic((C), \
					const char*: LuaHashMap_SetValueIntegerForKeyString, \
					char*: LuaHashMap_SetValueIntegerForKeyString, \
					void*: LuaHashMap_SetValueIntegerForKeyPointer, \
					float: LuaHashMap_SetValueIntegerForKeyNumber, \
					double: LuaHashMap_SetValueIntegerForKeyNumber, \
					long double: LuaHashMap_SetValueIntegerForKeyNumber, \
					char: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
					short: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
					int: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
					long: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
					default: LuaHashMap_SetValueIntegerForKeyPointer), \
				unsigned long: _Generic((C), \
					const char*: LuaHashMap_SetValueIntegerForKeyString, \
					char*: LuaHashMap_SetValueIntegerForKeyString, \
					void*: LuaHashMap_SetValueIntegerForKeyPointer, \
					float: LuaHashMap_SetValueIntegerForKeyNumber, \
					double: LuaHashMap_SetValueIntegerForKeyNumber, \
					long double: LuaHashMap_SetValueIntegerForKeyNumber, \
					char: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned char: LuaHashMap_SetValueIntegerForKeyInteger, \
					short: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned short: LuaHashMap_SetValueIntegerForKeyInteger, \
					int: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned int: LuaHashMap_SetValueIntegerForKeyInteger, \
					long: LuaHashMap_SetValueIntegerForKeyInteger, \
					unsigned long: LuaHashMap_SetValueIntegerForKeyInteger, \
					default: LuaHashMap_SetValueIntegerForKeyPointer), \
				default: LuaHashMap_SetValuePointerForKeyPointer), \
			LuaHashMapIterator*: _Generic((B), \
				const char*: _Generic((C), \
					char: LuaHashMap_SetValueStringAtIteratorWithLength, \
					unsigned char: LuaHashMap_SetValueStringAtIteratorWithLength, \
					short: LuaHashMap_SetValueStringAtIteratorWithLength, \
					unsigned short: LuaHashMap_SetValueStringAtIteratorWithLength, \
					int: LuaHashMap_SetValueStringAtIteratorWithLength, \
					unsigned int: LuaHashMap_SetValueStringAtIteratorWithLength, \
					long: LuaHashMap_SetValueStringAtIteratorWithLength, \
					unsigned long: LuaHashMap_SetValueStringAtIteratorWithLength, \
					default: LuaHashMap_SetValueStringAtIteratorWithLength), \
				char*: _Generic((C), \
					char: LuaHashMap_SetValueStringAtIteratorWithLength, \
					unsigned char: LuaHashMap_SetValueStringAtIteratorWithLength, \
					short: LuaHashMap_SetValueStringAtIteratorWithLength, \
					unsigned short: LuaHashMap_SetValueStringAtIteratorWithLength, \
					int: LuaHashMap_SetValueStringAtIteratorWithLength, \
					unsigned int: LuaHashMap_SetValueStringAtIteratorWithLength, \
					long: LuaHashMap_SetValueStringAtIteratorWithLength, \
					unsigned long: LuaHashMap_SetValueStringAtIteratorWithLength, \
					default: LuaHashMap_SetValueStringAtIteratorWithLength), \
				default: LuaHashMap_SetValueStringAtIteratorWithLength), \
			default: LuaHashMap_SetValuePointerForKeyPointer) \
		(A,B,C)

	#define LUAHASHMAP_SETVALUE_2(A,B) LuaHashMap_SetValueAtIterator(A,B)

	/* The private interim macro that simply strips the excess and ends up with the required macro */
	#define LUAHASHMAP_SETVALUE_N(x,A,B,C,D,E,FUNC, ...) FUNC  
/**
 * LuaHashMap_SetValue
 *
 * This C11 _Generic macro essentially overloads all the LuaHashMap_ExistsKey and LuaHashMap_ExistsAtIterator functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_SetValue(...) \
	LUAHASHMAP_SETVALUE_N(,##__VA_ARGS__, \
		LUAHASHMAP_SETVALUE_5(__VA_ARGS__), \
		LUAHASHMAP_SETVALUE_4(__VA_ARGS__), \
		LUAHASHMAP_SETVALUE_3(__VA_ARGS__), \
		LUAHASHMAP_SETVALUE_2(__VA_ARGS__), \
	)




/**
 * LuaHashMap_GetValueStringForKey
 *
 * This C11 _Generic macro essentially overloads all the 2 parameter GetValueStringForKey<T> functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_GetValueStringForKey(hash_map, key) \
	_Generic((hash_map), LuaHashMap*: _Generic((key), \
		const char*: LuaHashMap_GetValueStringForKeyString, \
		char*: LuaHashMap_GetValueStringForKeyString, \
		void*: LuaHashMap_GetValueStringForKeyPointer, \
		float: LuaHashMap_GetValueStringForKeyNumber, \
		double: LuaHashMap_GetValueStringForKeyNumber, \
		long double: LuaHashMap_GetValueStringForKeyNumber, \
		char: LuaHashMap_GetValueStringForKeyInteger, \
		unsigned char: LuaHashMap_GetValueStringForKeyInteger, \
		short: LuaHashMap_GetValueStringForKeyInteger, \
		unsigned short: LuaHashMap_GetValueStringForKeyInteger, \
		int: LuaHashMap_GetValueStringForKeyInteger, \
		unsigned int: LuaHashMap_GetValueStringForKeyInteger, \
		long: LuaHashMap_GetValueStringForKeyInteger, \
		unsigned long: LuaHashMap_GetValueStringForKeyInteger, \
		default: LuaHashMap_GetValueStringForKeyPointer) \
	) \
	(hash_map, key)

/**
 * LuaHashMap_GetValuePointerForKey
 *
 * This C11 _Generic macro essentially overloads all the 2 parameter GetValuePointerForKey<T> functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_GetValuePointerForKey(hash_map, key) \
	_Generic((hash_map), LuaHashMap*: _Generic((key), \
		const char*: LuaHashMap_GetValuePointerForKeyString, \
		char*: LuaHashMap_GetValuePointerForKeyString, \
		void*: LuaHashMap_GetValuePointerForKeyPointer, \
		float: LuaHashMap_GetValuePointerForKeyNumber, \
		double: LuaHashMap_GetValuePointerForKeyNumber, \
		long double: LuaHashMap_GetValuePointerForKeyNumber, \
		char: LuaHashMap_GetValuePointerForKeyInteger, \
		unsigned char: LuaHashMap_GetValuePointerForKeyInteger, \
		short: LuaHashMap_GetValuePointerForKeyInteger, \
		unsigned short: LuaHashMap_GetValuePointerForKeyInteger, \
		int: LuaHashMap_GetValuePointerForKeyInteger, \
		unsigned int: LuaHashMap_GetValuePointerForKeyInteger, \
		long: LuaHashMap_GetValuePointerForKeyInteger, \
		unsigned long: LuaHashMap_GetValuePointerForKeyInteger, \
		default: LuaHashMap_GetValuePointerForKeyPointer) \
	) \
	(hash_map, key)

/**
 * LuaHashMap_GetValueNumberForKey
 *
 * This C11 _Generic macro essentially overloads all the 2 parameter GetValueNumberForKey<T> functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_GetValueNumberForKey(hash_map, key) \
	_Generic((hash_map), LuaHashMap*: _Generic((key), \
		const char*: LuaHashMap_GetValueNumberForKeyString, \
		char*: LuaHashMap_GetValueNumberForKeyString, \
		void*: LuaHashMap_GetValueNumberForKeyPointer, \
		float: LuaHashMap_GetValueNumberForKeyNumber, \
		double: LuaHashMap_GetValueNumberForKeyNumber, \
		long double: LuaHashMap_GetValueNumberForKeyNumber, \
		char: LuaHashMap_GetValueNumberForKeyInteger, \
		unsigned char: LuaHashMap_GetValueNumberForKeyInteger, \
		short: LuaHashMap_GetValueNumberForKeyInteger, \
		unsigned short: LuaHashMap_GetValueNumberForKeyInteger, \
		int: LuaHashMap_GetValueNumberForKeyInteger, \
		unsigned int: LuaHashMap_GetValueNumberForKeyInteger, \
		long: LuaHashMap_GetValueNumberForKeyInteger, \
		unsigned long: LuaHashMap_GetValueNumberForKeyInteger, \
		default: LuaHashMap_GetValueNumberForKeyPointer) \
	) \
	(hash_map, key)

/**
 * LuaHashMap_GetValueIntegerForKey
 *
 * This C11 _Generic macro essentially overloads all the 2 parameter GetValueIntegerForKey<T> functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_GetValueIntegerForKey(hash_map, key) \
	_Generic((hash_map), LuaHashMap*: _Generic((key), \
		const char*: LuaHashMap_GetValueIntegerForKeyString, \
		char*: LuaHashMap_GetValueIntegerForKeyString, \
		void*: LuaHashMap_GetValueIntegerForKeyPointer, \
		float: LuaHashMap_GetValueIntegerForKeyNumber, \
		double: LuaHashMap_GetValueIntegerForKeyNumber, \
		long double: LuaHashMap_GetValueIntegerForKeyNumber, \
		char: LuaHashMap_GetValueIntegerForKeyInteger, \
		unsigned char: LuaHashMap_GetValueIntegerForKeyInteger, \
		short: LuaHashMap_GetValueIntegerForKeyInteger, \
		unsigned short: LuaHashMap_GetValueIntegerForKeyInteger, \
		int: LuaHashMap_GetValueIntegerForKeyInteger, \
		unsigned int: LuaHashMap_GetValueIntegerForKeyInteger, \
		long: LuaHashMap_GetValueIntegerForKeyInteger, \
		unsigned long: LuaHashMap_GetValueIntegerForKeyInteger, \
		default: LuaHashMap_GetValueIntegerForKeyPointer) \
	) \
	(hash_map, key)


/* Adapted from http://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros */
	/* Private multiple macros for each different number of arguments */
	#define LUAHASHMAP_GETVALUESTRING_2(A,B) LuaHashMap_GetValueStringForKey(A,B)
	#define LUAHASHMAP_GETVALUESTRING_1(A) LuaHashMap_GetValueStringAtIterator(A)

	/* The private interim macro that simply strips the excess and ends up with the required macro */
	#define LUAHASHMAP_GETVALUESTRING_N(x,A,B,FUNC, ...) FUNC  
/**
 * LuaHashMap_GetValueString
 *
 * This C11 _Generic macro essentially overloads all LuaHashMap_GetValueStringForKey and LuaHashMap_GetValueStringAtIterator functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_GetValueString(...) \
	LUAHASHMAP_GETVALUESTRING_N(,##__VA_ARGS__, \
		LUAHASHMAP_GETVALUESTRING_2(__VA_ARGS__), \
		LUAHASHMAP_GETVALUESTRING_1(__VA_ARGS__), \
	)

/* Adapted from http://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros */
	/* Private multiple macros for each different number of arguments */
	#define LUAHASHMAP_GETVALUEPOINTER_2(A,B) LuaHashMap_GetValuePointerForKey(A,B)
	#define LUAHASHMAP_GETVALUEPOINTER_1(A) LuaHashMap_GetValuePointerAtIterator(A)

	/* The private interim macro that simply strips the excess and ends up with the required macro */
	#define LUAHASHMAP_GETVALUEPOINTER_N(x,A,B,FUNC, ...) FUNC  
/**
 * LuaHashMap_GetValuePointer
 *
 * This C11 _Generic macro essentially overloads all LuaHashMap_GetValuePointerForKey and LuaHashMap_GetValuePointerAtIterator functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_GetValuePointer(...) \
	LUAHASHMAP_GETVALUEPOINTER_N(,##__VA_ARGS__, \
		LUAHASHMAP_GETVALUEPOINTER_2(__VA_ARGS__), \
		LUAHASHMAP_GETVALUEPOINTER_1(__VA_ARGS__), \
	)

/* Adapted from http://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros */
	/* Private multiple macros for each different number of arguments */
	#define LUAHASHMAP_GETVALUENUMBER_2(A,B) LuaHashMap_GetValueNumberForKey(A,B)
	#define LUAHASHMAP_GETVALUENUMBER_1(A) LuaHashMap_GetValueNumberAtIterator(A)

	/* The private interim macro that simply strips the excess and ends up with the required macro */
	#define LUAHASHMAP_GETVALUENUMBER_N(x,A,B,FUNC, ...) FUNC  
/**
 * LuaHashMap_GetValueNumber
 *
 * This C11 _Generic macro essentially overloads all LuaHashMap_GetValueNumberForKey and LuaHashMap_GetValueNumberAtIterator functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_GetValueNumber(...) \
	LUAHASHMAP_GETVALUENUMBER_N(,##__VA_ARGS__, \
		LUAHASHMAP_GETVALUENUMBER_2(__VA_ARGS__), \
		LUAHASHMAP_GETVALUENUMBER_1(__VA_ARGS__), \
	)

/* Adapted from http://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros */
	/* Private multiple macros for each different number of arguments */
	#define LUAHASHMAP_GETVALUEINTEGER_2(A,B) LuaHashMap_GetValueIntegerForKey(A,B)
	#define LUAHASHMAP_GETVALUEINTEGER_1(A) LuaHashMap_GetValueIntegerAtIterator(A)

	/* The private interim macro that simply strips the excess and ends up with the required macro */
	#define LUAHASHMAP_GETVALUEINTEGER_N(x,A,B,FUNC, ...) FUNC  
/**
 * LuaHashMap_GetValueInteger
 *
 * This C11 _Generic macro essentially overloads all LuaHashMap_GetValueIntegerForKey and LuaHashMap_GetValueIntegerAtIterator functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_GetValueInteger(...) \
	LUAHASHMAP_GETVALUEINTEGER_N(,##__VA_ARGS__, \
		LUAHASHMAP_GETVALUEINTEGER_2(__VA_ARGS__), \
		LUAHASHMAP_GETVALUEINTEGER_1(__VA_ARGS__), \
	)




/**
 * LuaHashMap_ExistsKey
 *
 * This C11 _Generic macro essentially overloads all the 2 parameter LuaHashMap_ExistsKey<T> functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_ExistsKey(hash_map, key) \
	_Generic((hash_map), LuaHashMap*: _Generic((key), \
		const char*: LuaHashMap_ExistsKeyString, \
		char*: LuaHashMap_ExistsKeyString, \
		void*: LuaHashMap_ExistsKeyPointer, \
		float: LuaHashMap_ExistsKeyNumber, \
		double: LuaHashMap_ExistsKeyNumber, \
		long double: LuaHashMap_ExistsKeyNumber, \
		char: LuaHashMap_ExistsKeyInteger, \
		unsigned char: LuaHashMap_ExistsKeyInteger, \
		short: LuaHashMap_ExistsKeyInteger, \
		unsigned short: LuaHashMap_ExistsKeyInteger, \
		int: LuaHashMap_ExistsKeyInteger, \
		unsigned int: LuaHashMap_ExistsKeyInteger, \
		long: LuaHashMap_ExistsKeyInteger, \
		unsigned long: LuaHashMap_ExistsKeyInteger, \
		default: LuaHashMap_ExistsKeyPointer) \
	) \
	(hash_map, key)

/* Adapted from http://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros */
	/* Private multiple macros for each different number of arguments */
	#define LUAHASHMAP_EXISTS_1(A) LuaHashMap_ExistsAtIterator(A)
	#define LUAHASHMAP_EXISTS_2(A,B) LuaHashMap_ExistsKey(A,B)

	/* The private interim macro that simply strips the excess and ends up with the required macro */
	#define LUAHASHMAP_EXISTS_N(x,A,B,FUNC, ...) FUNC  
/**
 * LuaHashMap_Exists
 *
 * This C11 _Generic macro essentially overloads all the LuaHashMap_ExistsKey and LuaHashMap_ExistsAtIterator functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_Exists(...) \
	LUAHASHMAP_EXISTS_N(,##__VA_ARGS__, \
		LUAHASHMAP_EXISTS_2(__VA_ARGS__), \
		LUAHASHMAP_EXISTS_1(__VA_ARGS__), \
	)


/**
 * LuaHashMap_RemoveKey
 *
 * This C11 _Generic macro essentially overloads all the 2 parameter LuaHashMap_RemoveKey<T> functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_RemoveKey(hash_map, key) \
	_Generic((hash_map), LuaHashMap*: _Generic((key), \
		const char*: LuaHashMap_RemoveKeyString, \
		char*: LuaHashMap_RemoveKeyString, \
		void*: LuaHashMap_RemoveKeyPointer, \
		float: LuaHashMap_RemoveKeyNumber, \
		double: LuaHashMap_RemoveKeyNumber, \
		long double: LuaHashMap_RemoveKeyNumber, \
		char: LuaHashMap_RemoveKeyInteger, \
		unsigned char: LuaHashMap_RemoveKeyInteger, \
		short: LuaHashMap_RemoveKeyInteger, \
		unsigned short: LuaHashMap_RemoveKeyInteger, \
		int: LuaHashMap_RemoveKeyInteger, \
		unsigned int: LuaHashMap_RemoveKeyInteger, \
		long: LuaHashMap_RemoveKeyInteger, \
		unsigned long: LuaHashMap_RemoveKeyInteger, \
		default: LuaHashMap_RemoveKeyPointer) \
	) \
	(hash_map, key)


/* Adapted from http://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros */
	/* Private multiple macros for each different number of arguments */
	#define LUAHASHMAP_REMOVE_1(A) LuaHashMap_RemoveAtIterator(A)
	#define LUAHASHMAP_REMOVE_2(A,B) LuaHashMap_RemoveKey(A,B)

	/* The private interim macro that simply strips the excess and ends up with the required macro */
	#define LUAHASHMAP_REMOVE_N(x,A,B,FUNC, ...) FUNC  
/**
 * LuaHashMap_Remove
 *
 * This C11 _Generic macro essentially overloads all the LuaHashMap_ExistsKey and LuaHashMap_ExistsAtIterator functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_Remove(...) \
	LUAHASHMAP_REMOVE_N(,##__VA_ARGS__, \
		LUAHASHMAP_REMOVE_2(__VA_ARGS__), \
		LUAHASHMAP_REMOVE_1(__VA_ARGS__), \
	)

/**
 * LuaHashMap_GetIteratorForKey
 *
 * This C11 _Generic macro essentially overloads all the 2 parameter LuaHashMap_GetIteratorForKey<T> functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_GetIteratorForKey(hash_map, key) \
	_Generic((hash_map), LuaHashMap*: _Generic((key), \
		const char*: LuaHashMap_GetIteratorForKeyString, \
		char*: LuaHashMap_GetIteratorForKeyString, \
		void*: LuaHashMap_GetIteratorForKeyPointer, \
		float: LuaHashMap_GetIteratorForKeyNumber, \
		double: LuaHashMap_GetIteratorForKeyNumber, \
		long double: LuaHashMap_GetIteratorForKeyNumber, \
		char: LuaHashMap_GetIteratorForKeyInteger, \
		unsigned char: LuaHashMap_GetIteratorForKeyInteger, \
		short: LuaHashMap_GetIteratorForKeyInteger, \
		unsigned short: LuaHashMap_GetIteratorForKeyInteger, \
		int: LuaHashMap_GetIteratorForKeyInteger, \
		unsigned int: LuaHashMap_GetIteratorForKeyInteger, \
		long: LuaHashMap_GetIteratorForKeyInteger, \
		unsigned long: LuaHashMap_GetIteratorForKeyInteger, \
		default: LuaHashMap_GetIteratorForKeyPointer) \
	) \
	(hash_map, key)


/* You may use this to test to see if generics are supported so you don't have to reproduce my test. */
#define LUAHASHMAP_SUPPORTS_GENERICS 1

#endif /* end of C11 _Generic */


/* List Functions (Deprecated) */
/* The iterator functions are much cleaner than these. These are also O(n). 
 * Also, now that mixing key types in the same hash is no longer explictly forbidden/caught, 
 * these functions are not resilient to mixed keys.
 * These functions are now deprecated. 
 */
LUAHASHMAP_EXPORT size_t LuaHashMap_GetKeysString(LuaHashMap* hash_map, const char* keys_array[], size_t max_array_size);
LUAHASHMAP_EXPORT size_t LuaHashMap_GetKeysPointer(LuaHashMap* hash_map, void* keys_array[], size_t max_array_size);
LUAHASHMAP_EXPORT size_t LuaHashMap_GetKeysNumber(LuaHashMap* hash_map, lua_Number keys_array[], size_t max_array_size);
LUAHASHMAP_EXPORT size_t LuaHashMap_GetKeysInteger(LuaHashMap* hash_map, lua_Integer keys_array[], size_t max_array_size);
/* End Experiemental Functions */	



#if defined(__LUAHASHMAP_LUA_NUMBER_DEFINED__)
	#undef lua_Number
	#undef __LUAHASHMAP_LUA_NUMBER_DEFINED__
#endif
    
#if defined(__LUAHASHMAP_LUA_INTEGER_DEFINED__)
	#undef lua_Integer
	#undef __LUAHASHMAP_LUA_INTEGER_DEFINED__
#endif
    
#if defined(__LUAHASHMAP_LUA_STATE_DEFINED__)
	#undef lua_State
	#undef __LUAHASHMAP_LUA_STATE_DEFINED__
#endif

/* You can't undo a typedef */
#if defined(__LUAHASHMAP_LUA_ALLOC_DEFINED__)
/*	#undef lua_Alloc */
	#undef __LUAHASHMAP_LUA_ALLOC_DEFINED__
#endif


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

