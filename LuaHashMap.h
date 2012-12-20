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
@mainpage LuaHashMap: An easy to use hash table for C.
LuaHashMap is a hash table library/implementation for use in C. 
Since C lacks a hash table in the standard library, 
this library provides a way to fill that hole.
But instead of reinventing the wheel again to implement a hash table for C, 
LuaHashMap cleverly wraps Lua, providing a friendly C API for hash tables 
without needing to learn/use the low-level Lua C-API.

For those not familar with Lua, 
Lua is a scripting language implemented in pure ANSI C designed to be embedded in larger applications. 
It is the de-facto scripting language used in the video game industry for the past 15 years 
(e.g. Escape from Monkey Island, World of Warcraft, Angry Birds) 
as well as other non-gaming high profile products such as Adobe Lightroom, Wireshark, and MediaWiki (Wikipedia).
Lua is fast, small, lightweight, simple, and under the MIT license.
And tables are the sole data structure in Lua so the underlying hash table is 
fast, reliable, well documented/understood, and proven. 

So rather than reinventing a new hash table implementation,
LuaHashMap simply wraps Lua's to mimimize bugs and leverage known performance and behaviors.
But to you (the end user/developer), LuaHashMap provides a simple/straight-forward C API 
for hash tables and Lua is simply an implementation detail, so if you don't know anything 
about Lua, it is not a problem. 
(But for those wondering about the implementation details, 
LuaHashMap itself is completely written in C and uses Lua's C API 
exclusively without any actual Lua script code.)

LuaHashMap is designed to be easy to use and convenient. 
LuaHashMap can work with keys and values of the following types: strings, numbers, and pointers.
The API provides explicit function names for each permutation of types to 
avoid macro hell and hard to debug situations. 
(Though C11 users, check out the _Generic macros to use C++ like overloading without all the namemangling problems.)
And there are no hashing functions you need to figure-out/write/provide. (All of this was already done/tuned by Lua itself.)


Programming Overview:
LuaHashMap provides explicit interfaces for 4 types: integers, floating point, strings, and pointers. 
There is no macro-hell here. (Yes, I typed-out/implemented all the permutations. You can thank me later.) 
So you get nice straight-forward compiler warnings and more type safety.

The general pattern for all the permutations of the APIs is:
LuaHashMap_SetValue<TV>ForKey<TK>
where TV is the value type and TK is the key type.
The names are "String", "Pointer", "Number" (for floating point), and "Integer".


There are two general ways to access the hash map, either by key or by iterator.

The following example inserts values into the table with string keys.

@code
	LuaHashMap* hash_map = LuaHashMap_Create();

	LuaHashMap_SetValueNumberForKeyString(hash_map, 3.99, "milk");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 4.349, "gas");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 2.99, "bread");

	printf("Price of gas: %lf\n", LuaHashMap_GetValueNumberForKeyString(hash_map, "gas"));

	LuaHashMap_Free(hash_map);
@endcode


Iterators in LuaHashMap conceptually similar to the C++ STL notion of an iterator (but much simpler).
They let you iterator through a hash table and let you get and set values.
Here's a similar example that prints everything in the hash table using an iterator.

@code
	LuaHashMap* hash_map = LuaHashMap_Create();

	LuaHashMap_SetValueNumberForKeyString(hash_map, 3.99, "milk");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 4.349, "gas");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 2.99, "bread");

	// This loop will iterate through the whole table and print out each entry.
	LuaHashMapIterator hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{

		printf("Price of %s: %lf\n", 
			LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
			LuaHashMap_GetCachedValueNumberAtIterator(&hash_iterator));

	} while(LuaHashMap_IteratorNext(&hash_iterator));
	
	LuaHashMap_Free(hash_map);
@endcode

Of course this is just a taste. There are a lot more APIs available to make this a full featured library.

More on Iterators:
Iterators are the way to iterate through a hash table. 
To keep things simple, notice that the GetIterator family of functions all return a full copy of a struct (LuaHashMapIterator). 

A major concept with iterators to notice is they are simply a struct that are intended to live on the stack when you use them.
This allows you to use iterators without worrying about leaking.
They are also generally seen as short term, inexpensive objects you should be able to refetch at will (an O(1) lookup by key).

Functions that operate on Iterators like IteratorNext() operate on the iterator by reference so it can change the data in the struct. The pattern is to pass your struct in by reference:
LuaHashMap_IteratorNext(&hash_iterator)

In Lua (and thus LuaHashMap), it is safe to remove a key/value entry from the hash table while iterating through it. Note that Lua does not reshuffle entries or compact the table when clearing entries.

Unlike removing, it is not safe to add new entries to the table while iterating because the table may reshuffle.


Also of note, there are two ways to extract values from iterators:
LuaHashMap_GetCachedValue<TV>AtIterator
LuaHashMap_GetValue<TV>AtIterator

The "Cached" value is the value copied in the struct when the iterator was last created/iterated/updated.
The non-cached version incurs a full lookup in the hash to find the value.
Generally, if you have the iterator and haven't modified the hash table behind the back of the iterator, the cached value is correct and will be faster.
Proper programming style of the LuaHashMap iterator functions should keep the "Cached" value up to date.


Iterator Patterns:

This demonstrates a typical pattern for using iterators to traverse an entire hash map. You get an iterator at the begin position and another at the end position. You increment your begin position iterator with ItereratorNext() as long as it does not equal the end iterator.
@code
	// Showing a different way to loop 
	for(LuaHashMapIterator hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map), LuaHashMapIterator hash_iterator_end = LuaHashMap_GetIteratorAtEnd(hash_map);
		! LuaHashMap_IteratorIsEqual(&hash_iterator, &hash_iterator_end);
		LuaHashMap_IteratorNext(&hash_iterator)
	)
	{
		fprintf(stderr, "Price of %s: %lf\n", 
				LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
				LuaHashMap_GetCachedValueNumberAtIterator(&hash_iterator));
		
	}
@endcode


Alternatively, if you know you have at least one entry in the hash, this is a slightly more succinct pattern which uses a do-while loop and the return value of IteratorNext.
@code
	LuaHashMapIterator hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Price of %s: %lf\n", 
				LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
				LuaHashMap_GetCachedValueNumberAtIterator(&hash_iterator));
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));
@endcode






Audience:
LuaHashMap is ideal for projects that may agree with one the following:
- Need a portable hash table for C
- Need a hash table for C++ but can't use the STL or templates (yes, this happens)
- Want a really friendly, simple to use API/interface (you don't need to know anything about Lua)
- Want an explicit, type safe API that doesn't use macro-hell
- Already using Lua elsewhere in your project or are considering using Lua
- Not already using Lua but don't think the ~100-200KB library (disk) size of Lua is a big deal. Lua is ~100KB for the core, and ~100KB for the standard library which is not needed by LuaHashMap. (pfft! My icon takes more space.)
- Don't mind the ~4KB overhead (RAM size) of creating a Lua state instance. (Seriously, Apple Mac icons are full color 1024x1024 now.)
- Want a time proven, heavily tested/used, and documented implementation (i.e. Lua)
- Need a library under a permissive license (MIT)
- Like overall good performance (but doesn't necessarily need to be the best)
- Want something with minimal dependencies and easy to embed in your application

LuaHashMap may not be ideal for projects that:
- Need to fine-tune every single possible detail for the utmost performance (speed/memory)
- Not already using Lua and bothered by the ~100-200KB disk space of including Lua
- Can't afford the ~4KB overhead for a Lua state instance
- Have alternative hash table libraries that better suit your specific needs



Portability Notes:
LuaHashMap supports both Lua 5.1 and Lua 5.2.
LuaHashMap was written to be portable to all platforms with at least a C89 compiler (because Microsoft Visual Studio refuses to update their pathetic C compiler).
LuaHashMap was developed with clang, gcc, and Visual Studio on Mac, iOS, Linux, Android, and Windows.
However, there is an underlying desire to modernize and utilize C99 and C11 features when available so there are conditionals in the code.


Extra Lua Details:
I said you didn't need to know anything about Lua. Well, that's mostly true, 
but there are some interesting details you should know about in Lua which may be useful.


lua_Number and lua_Integer
Canonical (unmodified) Lua only has one number type, which by default is double.
Double has enough bits to store a 32-bit integer without any loss of precision which is why this works.
But you may notice that LuaHashMap has APIs to deal with both lua_Number (floating point) and lua_Integer (integer).
Canonical Lua converts everything to lua_Number so if you provide a lua_Number key that happens to be the same value as a lua_Integer key,
these may in fact be the same key. 
(But if you are using a patched version of Lua (e.g. LNUM)  Lua that may avoid conversion and keep types separated. 
Please note that some of the iterator based functions in LuaHashMap may still convert due to the lack of a formal way to detect integer types.) 


Lua internalizes strings
As an implementation detail, all strings in Lua are internalized which means there is only one immutable instance of a string within Lua for duplicate strings. 
This potentially can be exploited to yield further optimizations in your code.
LuaHashMap does not rely on this fact, 
but do note that the APIs return the internalized string pointer when you add a key (which may be different than the one you fed the API).
This was intended to allow you to exploit this fact if you choose in your own code. 
However, if you use alternative implementations of Lua, they are not guaranteed to behave in this same way.

Memory management for strings:
Related to Lua's string internalization, for LuaHashMap, this means when you add a string into the hash table, 
Lua creates its own copy so you don't need to hold on to the original. 
But be careful: if you Get a string from LuaHashMap and then completely remove the entries that are holding it, 
the memory could be deleted out from under you if you still are holding on to the string pointer.


Lua does not have a moving garbage collector nor does it move its memory around invalidating pointers.
LuaHashMap does exploit an implementation detail of Lua for functions that return strings, 
such as const char* LuaHashMap_GetValueStringForKeyString. 
In the Lua API, you communicate between C and Lua using a stack API provided by Lua.
When you are finished with an operation, the stack should generally be returned to the state you started in.
That means when this function finishes, the Lua stack should be reset. 
Technically, the pointer to string that is returned is not guaranteed by the Lua specification to still be valid and the pointer could be dangling.
But as an implementation detail, the Lua authors have stated that this pointer is still valid as long as the string is still in Lua (e.g. not removed).
It is likely that all implementations of Lua share this behavior (since moving collectors are hard to deal with in C for this exact reason of pointers),
so this implementation detail is probably safe. However, if you do use a completely different implementation of Lua that is not from PUC-Rio, you may want to verify this behavior.

Lua tables have an array optimization
If you create a hash with sequential, continuous integer keys, you may trigger an optimization in Lua which treats that an array. 
This will yield very fast performance as Lua is working with C arrays instead of hashing.


It is safe/supported to remove keys from a hash while iterating it. (But adding keys while iterating is not supported.)
This is a behavior of Lua which LuaHashMap keeps. (It is related to the fact that Lua does not recompact tables when keys are removed.)

Lua does not recompact/reshuffle tables when you remove keys.
Also note that the LuaHashMap_Clear() function will only clear the entries from the table. The number of allocated buckets will not shrink.
If you want to clear all the entries and free the memory, use LuaHashMap_Purge().

The book "Lua Gems" gives wonderful insights to how tables are designed and work in Lua, and what their behavior and performance characteristics are.
The free sample chapter tells you everything you need to know.
http://www.lua.org/gems/sample.pdf



Advanced tricks:

LuaHashMap_CreateShare:
Unfortunately, using the standard API, each LuaHashMap instance also creates an entirely new Lua virtual machine instance.
On a 64-bit machine, this amounts to about 4-5KB of RAM (measured on a Mac). For large data sets, you will not notice, 
but for small data sets and a lot of separate instances of LuaHashMap, this might eventually add up.
So an addition to the standard API, a CreateShare API exists to allow you to create a new instance of LuaHashMap
without creating an entirely new virtual machine instance. Instead it will simply create a new Lua table instance
in the existing virtual machine and not incur any more RAM overhead.
Technically speaking, the original and shared maps are peers of each other. The implementation does not make a distinction 
about which one the original is so any hash map with the lua_State you want to share may be passed in as the parameter.
So to the rest of the LuaHashMap API (everything except CreateShare and FreeShare), the hash map instance looks like a separate instance with its own hash and you may think about them in those terms.  

Every CreateShare should be balanced by FreeShare. Free should balance the original Create and should be the very last thing to be called. 
(Free will destroy the entire virtual machine instance as it calls lua_close().)



Mixed Types in the same hash map:
Lua supports mixed types (i.e. numbers, strings, pointers) in the same table. 
However you need to be careful about doing this with LuaHashMap and support is still considered experimental as of this writing 
(though the tests seem to work well enough). 

First and most importantly, you must understand that Lua only has one number type and thus integers and numbers 
(doubles in stock Lua) are the same. So an integer key of say 100 would be the same as a number key of 100.00, 
so be very careful about understanding which of your keys are unique. 
In addition, when you Get a number, there is no identifying information (in stock Lua) about whether the number was originally an 
integer or number. It is up to you to figure it out.
 
To help support mixed types, two (experimental) functions are provided:
int LuaHashMap_GetKeyTypeAtIterator(LuaHashMapIterator* hash_iterator);
int LuaHashMap_GetValueTypeAtIterator(LuaHashMapIterator* hash_iterator);

These return the ints defined by Lua for LUA_TSTRING, LUA_TNUMBER, LUA_TLIGHTUSERDATA for strings, numbers, and pointers respectively. 
Remember that there is no distinction between number and integer in this case.

So while possible to mix types in a single hash map instance, you may find it cumbersome to deal with. 
Keeping separate hash map instances may be easier to work with.




C++ STL like interface:
For kicks and giggles, there is also a STL (C++ Standard Template Library) like template interface wrapper 
included which gives an STL hash_map-like interface for LuaHashMap. 
Theoretically you might be able to switch between implementations fairly easily using a strategic typedef. 
(Though if you can use the STL, I'm not sure why you would be here. And yes, the STL interface was a colossal waste of time.)
This probably won't be maintained over the long haul.


C11 _Generics (Experimental):
Yes, I said there is "no macro-hell" earlier. But trust me; this isn't really that bad and it's optional. (And it's really quite cool.)
C11 introduces _Generic which extends the C preprocessor to allow you to map different functions to a single macro function name based on their parameter types.
Essentially this brings C++-like function overloading to C, but without all the name-mangling problems and other C++ baggage.
And this was a lot less code than the C++ parital template specialization mentioned above.
So for example, these 16 following functions:
@code
LuaHashMap_SetValueStringForKeyString
LuaHashMap_SetValueStringForKeyPointer
LuaHashMap_SetValueStringForKeyNumber
LuaHashMap_SetValueStringForKeyInteger
LuaHashMap_SetValuePointerForKeyString
LuaHashMap_SetValuePointerForKeyPointer
LuaHashMap_SetValuePointerForKeyNumber
LuaHashMap_SetValuePointerForKeyInteger
LuaHashMap_SetValueNumberForKeyString
LuaHashMap_SetValueNumberForKeyPointer
LuaHashMap_SetValueNumberForKeyNumber
LuaHashMap_SetValueNumberForKeyInteger
LuaHashMap_SetValueIntegerForKeyString
LuaHashMap_SetValueIntegerForKeyPointer
LuaHashMap_SetValueIntegerForKeyNumber
LuaHashMap_SetValueIntegerForKeyInteger
@endcode
can now be called with just this one macro:
@code
	LuaHashMap_SetValueForKey(hash_map, value, key);
@endcode

The preprocessor/compiler will look at your types and automatically inject the correct version of the function to call.

And with a few more (C99) macro tricks, we can also do variable number of arguments 
to also unify the WithLength versions of the String permutations 
as well as the iterator versions. So this can be further reduced down to just:
@code
LuaHashMap_SetValue
@endcode

@code
LuaHashMap_SetValue(hash_map, 1, 2.2); // LuaHashMap_SetValueIntegerForKeyNumber
LuaHashMap_SetValue(hash_map, (const char*)"hello", 0xABCD); // LuaHashMap_SetValueStringForKeyInteger
LuaHashMap_SetValue(hash_map, (const char*)"hello", (void*)0xABCDE, strlen("hello")); // LuaHashMap_SetValueStringForKeyPointerWithLength
LuaHashMap_SetValue(hash_map, (const char*)"cat", (const char*)"animal", strlen("cat"), strlen("animal")); // LuaHashMap_SetValueStringForKeyStringWithLength
LuaHashMap_SetValue(&iterator, (const char*)"goodbye"); // LuaHashMap_SetValueStringAtIterator
@endcode

@note Please watch out for string literals. They technically are not const char*, but an array type which I can't seem to express. So you should use an explict cast for those or they get mapped to the void* (Pointer) rule.

For convenience, if C11 _Generic support is detected on your compiler, the following #define will be defined to 1.
LUAHASHMAP_SUPPORTS_GENERICS

Refer to the documentation to see all the macros available.


Benchmarks:
Yes, I did performance benchmarks. More information coming soon. 


References:
Lua Performance Tips (explains tables)
http://www.lua.org/gems/sample.pdf

Mailing List Reference: How is string passed from Lua to C
http://lua-users.org/lists/lua-l/2011-12/msg00222.html

Programming in Lua
http://www.lua.org/pil/

Lua Reference Manual
http://www.lua.org/manual/5.1/manual.html

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
#define LUAHASHMAP_PATCHLEVEL			1

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
LUAHASHMAP_EXPORT int LuaHashMap_GetValueTypeAtIterator(LuaHashMapIterator* hash_iterator);


#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
/* Use the C99 inline if available to hint the compiler that this might benefit from inlining. 
 * Since these functions are merely pulling data out of an already exposed struct, inlining seems reasonable.
 */

/* I don't know if I really want to support mixed types in a single hashmap. But if I do, then you need to be able to figure out the type. */
LUAHASHMAP_EXPORT inline int LuaHashMap_GetKeyTypeAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return LUA_TNONE;
	}
	return hash_iterator->keyType;
}

/* Gets the value contained (copied) in the iterator as opposed to doing an actual hash map lookup.
 * If you have just retrieved the iterator and have not altered anything in the hash map behind the back of the iterator,
 * then this value should be the same as what is in the actual hash map.
 */
LUAHASHMAP_EXPORT inline int LuaHashMap_GetCachedValueTypeAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return LUA_TNONE;
	}
	if(true == hash_iterator->atEnd)
	{
		return LUA_TNONE;
	}
	if(true == hash_iterator->isNext)
	{
		return LUA_TNONE;
	}
	
	return hash_iterator->valueType;
}

LUAHASHMAP_EXPORT inline const char* LuaHashMap_GetCachedValueStringAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		return NULL;
	}
	if(LUA_TSTRING != hash_iterator->valueType)
	{
		return NULL;
	}
	return hash_iterator->currentValue.theString.stringPointer;
}

LUAHASHMAP_EXPORT inline const char* LuaHashMap_GetCachedValueStringAtIteratorWithLength(LuaHashMapIterator* hash_iterator, size_t* value_string_length_return)
{
	if(NULL == hash_iterator)
	{
		if(NULL != value_string_length_return)
		{
			*value_string_length_return = 0;
		}
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		if(NULL != value_string_length_return)
		{
			*value_string_length_return = 0;
		}
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		if(NULL != value_string_length_return)
		{
			*value_string_length_return = 0;
		}
		return NULL;
	}
	if(LUA_TSTRING != hash_iterator->valueType)
	{
		if(NULL != value_string_length_return)
		{
			*value_string_length_return = 0;
		}
		return NULL;
	}

	if(NULL != value_string_length_return)
	{
		*value_string_length_return = hash_iterator->currentKey.theString.stringLength;
	}

	return hash_iterator->currentValue.theString.stringPointer;
}

LUAHASHMAP_EXPORT inline size_t LuaHashMap_GetCachedValueStringLengthAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return 0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0;
	}
	if(true == hash_iterator->isNext)
	{
		return 0;
	}
	if(LUA_TSTRING != hash_iterator->valueType)
	{
		return 0;
	}
	return hash_iterator->currentValue.theString.stringLength;
}

LUAHASHMAP_EXPORT inline void* LuaHashMap_GetCachedValuePointerAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	if(true == hash_iterator->isNext)
	{
		return NULL;
	}
	if(LUA_TLIGHTUSERDATA != hash_iterator->valueType)
	{
		return NULL;
	}
	return hash_iterator->currentValue.thePointer;
}

LUAHASHMAP_EXPORT inline lua_Number LuaHashMap_GetCachedValueNumberAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return 0.0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0.0;
	}
	if(true == hash_iterator->isNext)
	{
		return 0.0;
	}
	if(LUA_TNUMBER != hash_iterator->valueType)
	{
		return 0.0;
	}
	return hash_iterator->currentValue.theNumber;
}

LUAHASHMAP_EXPORT inline lua_Integer LuaHashMap_GetCachedValueIntegerAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return 0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0;
	}
	if(true == hash_iterator->isNext)
	{
		return 0;
	}
	if(LUA_TNUMBER != hash_iterator->valueType)
	{
		return 0;
	}
	return (lua_Integer)hash_iterator->currentValue.theNumber;
}

#else /* We are not using C99 inline so the standard declaration is here. */

/* I don't know if I really want to support mixed types in a single hashmap. But if I do, then you need to be able to figure out the type. */
LUAHASHMAP_EXPORT int LuaHashMap_GetKeyTypeAtIterator(LuaHashMapIterator* hash_iterator);

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
#endif /* defined(__STDC_VERSION__) && (__STDC_VERSION__ > 199901L) */



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
 *
 * @note I've been trying to pull the remaining default clauses out of this, but can't figure out how to do it without causing compile problems.
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
				unsigned long: LuaHashMap_SetValueStringForKeyPointerWithLength), \
			float: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			double: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			long double: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			char: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned char: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			short: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned short: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			int: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned int: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			long: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned long: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
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
				unsigned long: LuaHashMap_SetValueStringForKeyPointerWithLength), \
			float: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			double: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			long double: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				long: LuaHashMap_SetValueStringForKeyNumberWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyNumberWithLength), \
			char: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned char: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			short: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned short: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			int: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned int: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			long: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
			unsigned long: _Generic((strlength), \
				char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned char: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned short: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned int: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				long: LuaHashMap_SetValueStringForKeyIntegerWithLength, \
				unsigned long: LuaHashMap_SetValueStringForKeyIntegerWithLength), \
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
				unsigned long: LuaHashMap_SetValuePointerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				short: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				int: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				long: LuaHashMap_SetValuePointerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValuePointerForKeyStringWithLength), \
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
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength), \
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
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength), \
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
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				long: LuaHashMap_SetValueNumberForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueNumberForKeyStringWithLength), \
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
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
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
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
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
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
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
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
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
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
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
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
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
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
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
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			char*: _Generic((strlength), \
				char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned char: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned short: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned int: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				long: LuaHashMap_SetValueIntegerForKeyStringWithLength, \
				unsigned long: LuaHashMap_SetValueIntegerForKeyStringWithLength), \
			default: LuaHashMap_SetValueIntegerForKeyStringWithLength) \
		) \
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
 * LuaHashMap_GetValueStringForKeyWithLength
 *
 * This C11 _Generic macro essentially overloads all the 3 parameter GetValueStringForKey<T>WithLength functions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_GetValueStringForKeyWithLength(hash_map, key, strlengthoutptr) \
	_Generic((hash_map), LuaHashMap*: _Generic((key), \
		void*: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyPointerWithLength), \
		float: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyNumberWithLength), \
		double: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyNumberWithLength), \
		long double: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyNumberWithLength), \
		char: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyIntegerWithLength), \
		unsigned char: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyIntegerWithLength), \
		short: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyIntegerWithLength), \
		unsigned short: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyIntegerWithLength), \
		int: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyIntegerWithLength), \
		unsigned int: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyIntegerWithLength), \
		long: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyIntegerWithLength), \
		unsigned long: _Generic((strlengthoutptr), \
			size_t*: LuaHashMap_GetValueStringForKeyIntegerWithLength) \
		) \
	) \
	(hash_map, key, strlengthoutptr)



/* Adapted from http://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros */
	/* Private multiple macros for each different number of arguments */
	#define LUAHASHMAP_GETVALUESTRING_4(A,B,C,D) LuaHashMap_GetValueStringForKeyStringWithLength(A,B,C,D)
	#define LUAHASHMAP_GETVALUESTRING_3(A,B,C) LuaHashMap_GetValueStringForKeyWithLength(A,B,C)
/*
	#define LUAHASHMAP_GETVALUESTRING_2(A,B) \
		_Generic((A), \
			LuaHashMap*: _Generic((B), \
				default: LuaHashMap_GetValueStringForKey), \
			LuaHashMapIterator*: _Generic((B), \
				default: LuaHashMap_GetValueStringAtIteratorWithLength) \
		) \
		(A, B)
*/	
	#define LUAHASHMAP_GETVALUESTRING_2(A,B) \
		_Generic((A), \
			LuaHashMap*: _Generic((B), \
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
				default: LuaHashMap_GetValueStringForKeyPointer), \
			LuaHashMapIterator*: _Generic((B), \
				default: LuaHashMap_GetValueStringAtIteratorWithLength) \
		) \
		(A,B)

	#define LUAHASHMAP_GETVALUESTRING_1(A) LuaHashMap_GetValueStringAtIterator(A)

	/* The private interim macro that simply strips the excess and ends up with the required macro */
	#define LUAHASHMAP_GETVALUESTRING_N(x,A,B,C,D,FUNC, ...) FUNC  
/**
 * LuaHashMap_GetValueString
 *
 * This C11 _Generic macro essentially overloads all LuaHashMap_GetValueStringForKey and LuaHashMap_GetValueStringAtIterator functions including the WithLength varitions so you can call
 * any permutation with this single macro.
 *
 * @warning String literals are technically of type const char[] and not const char* so you must explicitly cast
 * or the fallback/default case will resolve to the Pointer version instead of the String version.
 */
#define LuaHashMap_GetValueString(...) \
	LUAHASHMAP_GETVALUESTRING_N(,##__VA_ARGS__, \
		LUAHASHMAP_GETVALUESTRING_4(__VA_ARGS__), \
		LUAHASHMAP_GETVALUESTRING_3(__VA_ARGS__), \
		LUAHASHMAP_GETVALUESTRING_2(__VA_ARGS__), \
		LUAHASHMAP_GETVALUESTRING_1(__VA_ARGS__), \
	)



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

