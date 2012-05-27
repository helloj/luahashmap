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

#include "LuaHashMap.h"
#include "lua.h"
#include "lauxlib.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

/* Benchmark:
 
 CFTimeInterval start_time = CACurrentMediaTime();
 void* ret_ptr = NULL;	
 for(i=0; i<40000000; i++)
 {
 LuaHashMap_LuaHashMap_SetValuePointerForKeyPointer(hash_map, (void*)i, (void*)i);
 ret_ptr = LuaHashMap_GetValuePointerForKeyPointer(hash_map, (void*)i);
 LuaHashMap_RemoveKeyPointer(hash_map, ret_ptr);
 }
 CFTimeInterval end_time = CACurrentMediaTime();
 fprintf(stderr, "diff time: %lf", end_time-start_time);
 
 iMac i3, 32-bit with lua_get/settable ~15-16 sec;
 iMac i3, 32-bit with lua_raw/get/set ~14-15 sec;
 iMac i3, 64-bit with lua_get/settable ~13-13.5 sec;
 iMac i3, 64-bit with lua_raw/get/set  ~12.5 sec;
*/ 
/* I don't expect we'll need metamethods so rawget/set is fine. */
#if 1
	#define LUAHASHMAP_SETTABLE lua_rawset
	#define LUAHASHMAP_GETTABLE lua_rawget
#else
	#define LUAHASHMAP_SETTABLE lua_settable
	#define LUAHASHMAP_GETTABLE lua_gettable
#endif

#define LUAHASHMAP_GETGLOBAL_UNIQUESTRING(lua_state, unique_key) lua_rawgeti(lua_state, LUA_GLOBALSINDEX, unique_key)

#define LUAHASHMAP_SETGLOBAL_UNIQUESTRING(lua_state, unique_key) lua_rawseti(lua_state, LUA_GLOBALSINDEX, unique_key)

#define LUAHASHMAP_DEBUG
#ifdef LUAHASHMAP_DEBUG
	#define LUAHASHMAP_ASSERT(e) assert(e)
#else
	#define LUAHASHMAP_ASSERT(e)
#endif

struct LuaHashMap
{
	lua_State* luaState;
	lua_Alloc memoryAllocator;
	void* allocatorUserData;
	lua_Integer uniqueTableNameForSharedState;
};


static void Internal_InitializeInternalTables(LuaHashMap* hash_map)
{
	/* Create a table in Lua to be our hash map */
	 /* According to Lua Gems performance, Lua tables are created with 0 size by default to not waste memory
	 * so creating different tables shouldn't be that bad.
	 * 
	 * I originally did an experiment to see if integer keys on the global table with 
	 * rawset/geti was faster than the rawset/get with light userdata keys (which 
	 * are also effectively integers). (The light userdata keys were static strings.)
	 * My benchmarks are inconclusive, testing with 10000000 insert/removes.
	 * They appear to be the same speed, though I've seen large timing variations in
	 * both tests.
	 * But in the changes for shared state, I realized luaL_ref basically was what I was doing with 
	 * integer keys. So I removed all this for a luaL_ref based implementation which is more
	 * general, but I lose the explicit separate tables for the 4 key types.
	 * luaL_ref is nice in that is also creates unique table numbers automatically and recycles them as they become free.
	 */
	
	lua_newtable(hash_map->luaState);
	/* The idea is that every LuaHashMap instance gets a unique reference id so they can share the same lua_State. */
	/* Even the first/original hash map created gets its own so all hash maps are effectively peers of each other. */
	hash_map->uniqueTableNameForSharedState = luaL_ref(hash_map->luaState, LUA_GLOBALSINDEX);
}

LuaHashMap* LuaHashMap_Create()
{
	LuaHashMap* hash_map;
	lua_State* lua_state = luaL_newstate();
	if(NULL == lua_state)
	{
		return NULL;
	}
	hash_map = (LuaHashMap*)calloc(1, sizeof(LuaHashMap));
	if(NULL == hash_map)
	{
		lua_close(lua_state);
		return NULL;
	}
	hash_map->luaState = lua_state;

	Internal_InitializeInternalTables(hash_map);

	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return hash_map;
}

LuaHashMap* LuaHashMap_CreateWithAllocator(lua_Alloc the_allocator, void* user_data)
{
	LuaHashMap* hash_map;
	lua_State* lua_state = lua_newstate(the_allocator, user_data);
	if(NULL == lua_state)
	{
		return NULL;
	}

	hash_map = (LuaHashMap*)(*the_allocator)(user_data, NULL, 0, sizeof(LuaHashMap));
	if(NULL == hash_map)
	{
		lua_close(lua_state);
		return NULL;
	}
	memset(hash_map, 0, sizeof(LuaHashMap));

	hash_map->luaState = lua_state;
	hash_map->memoryAllocator = the_allocator;
	hash_map->allocatorUserData = user_data;

	Internal_InitializeInternalTables(hash_map);
	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return hash_map;
}


LuaHashMap* LuaHashMap_CreateWithSizeHints(int number_of_array_elements, int number_of_hash_elements)
{
	LuaHashMap* hash_map;
	lua_State* lua_state = luaL_newstate();
	if(NULL == lua_state)
	{
		return NULL;
	}
	hash_map = (LuaHashMap*)calloc(1, sizeof(LuaHashMap));
	if(NULL == hash_map)
	{
		lua_close(lua_state);
		return NULL;
	}
	hash_map->luaState = lua_state;
	
	/* I'm no longer using key_type/value_type. But I wonder if I should reserve it for future changes. */
/*
	switch( 0x0F & key_type)
	{
		case LUAHASHMAP_KEYSTRING_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYPOINTER_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYNUMBER_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYINTEGER_TYPE:
		{
			break;
		}
		default:
		{
			Internal_InitializeInternalTables(hash_map);
		}
	}
*/
	lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);	
	hash_map->uniqueTableNameForSharedState = luaL_ref(hash_map->luaState, LUA_GLOBALSINDEX);

	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return hash_map;
}


LuaHashMap* LuaHashMap_CreateWithAllocatorAndSizeHints(lua_Alloc the_allocator, void* user_data, int number_of_array_elements, int number_of_hash_elements)
{
	LuaHashMap* hash_map;
	lua_State* lua_state = lua_newstate(the_allocator, user_data);
	if(NULL == lua_state)
	{
		return NULL;
	}

	hash_map = (LuaHashMap*)(*the_allocator)(user_data, NULL, 0, sizeof(LuaHashMap));
	if(NULL == hash_map)
	{
		lua_close(lua_state);
		return NULL;
	}
	memset(hash_map, 0, sizeof(LuaHashMap));

	hash_map->luaState = lua_state;
	hash_map->memoryAllocator = the_allocator;
	hash_map->allocatorUserData = user_data;
	/* I'm no longer using key_type/value_type. But I wonder if I should reserve it for future changes. */
/*
	switch( 0x0F & key_type)
	{
		case LUAHASHMAP_KEYSTRING_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYPOINTER_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYNUMBER_TYPE:
		{
			break;
		}
		case LUAHASHMAP_KEYINTEGER_TYPE:
		{
			break;
		}
		default:
		{
			Internal_InitializeInternalTables(hash_map);
		}
	}
*/
	lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);	
	hash_map->uniqueTableNameForSharedState = luaL_ref(hash_map->luaState, LUA_GLOBALSINDEX);

	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return hash_map;
}


LuaHashMap* LuaHashMap_CreateShare(LuaHashMap* original_hash_map)
{
	LuaHashMap* hash_map;
	if(NULL == original_hash_map->memoryAllocator)
	{
		hash_map = (LuaHashMap*)calloc(1, sizeof(LuaHashMap));
	}	
	else
	{
		hash_map = (LuaHashMap*)(*original_hash_map->memoryAllocator)(original_hash_map->allocatorUserData, NULL, 0, sizeof(LuaHashMap));
	}
	if(NULL == hash_map)
	{
		return NULL;
	}
	memset(hash_map, 0, sizeof(LuaHashMap));

	hash_map->luaState = original_hash_map->luaState;
	hash_map->memoryAllocator = original_hash_map->memoryAllocator;
	hash_map->allocatorUserData = original_hash_map->allocatorUserData;

	Internal_InitializeInternalTables(hash_map);

	return hash_map;
}


LuaHashMap* LuaHashMap_CreateShareWithSizeHints(LuaHashMap* original_hash_map, int number_of_array_elements, int number_of_hash_elements)
{
	LuaHashMap* hash_map;
	if(NULL == original_hash_map->memoryAllocator)
	{
		hash_map = (LuaHashMap*)calloc(1, sizeof(LuaHashMap));
	}	
	else
	{
		hash_map = (LuaHashMap*)(*original_hash_map->memoryAllocator)(original_hash_map->allocatorUserData, NULL, 0, sizeof(LuaHashMap));
	}
	if(NULL == hash_map)
	{
		return NULL;
	}
	memset(hash_map, 0, sizeof(LuaHashMap));

	hash_map->luaState = original_hash_map->luaState;
	hash_map->memoryAllocator = original_hash_map->memoryAllocator;
	hash_map->allocatorUserData = original_hash_map->allocatorUserData;

	lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);	
	hash_map->uniqueTableNameForSharedState = luaL_ref(hash_map->luaState, LUA_GLOBALSINDEX);

	return hash_map;
}




/* This version does not close the Lua state since it is shared */
void LuaHashMap_FreeShare(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return;
	}
	luaL_unref(hash_map->luaState, LUA_GLOBALSINDEX, hash_map->uniqueTableNameForSharedState);
	/* Seems like a good time to force the garbage collector */
	lua_gc(hash_map->luaState, LUA_GCCOLLECT, 0);
	if(NULL != hash_map->memoryAllocator)
	{
		(*hash_map->memoryAllocator)(hash_map->allocatorUserData, hash_map, sizeof(LuaHashMap), 0);
	}
	else
	{
		free(hash_map);
	}
}

void LuaHashMap_Free(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return;
	}
	/* Since we close the lua_State, we don't need to call luaL_unref */
/*	luaL_unref(hash_map->luaState, LUA_GLOBALSINDEX, hash_map->uniqueTableNameForSharedState); */
	lua_close(hash_map->luaState);
	if(NULL != hash_map->memoryAllocator)
	{
		(*hash_map->memoryAllocator)(hash_map->allocatorUserData, hash_map, sizeof(LuaHashMap), 0);
	}
	else
	{
		free(hash_map);
	}
}

lua_State* LuaHashMap_GetLuaState(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return NULL;
	}
	return hash_map->luaState;
}

void LuaHashMap_LuaHashMap_SetValueStringForKeyString(LuaHashMap* hash_map, const char* restrict value_string, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_string)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	lua_pushstring(hash_map->luaState, value_string); /* stack: [value_string, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_LuaHashMap_SetValuePointerForKeyString(LuaHashMap* hash_map, void* value_pointer, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_string)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_pointer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
}

void LuaHashMap_LuaHashMap_SetValueNumberForKeyString(LuaHashMap* hash_map, lua_Number value_number, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_string)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
}

void LuaHashMap_LuaHashMap_SetValueIntegerForKeyString(LuaHashMap* hash_map, lua_Integer value_integer, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_string)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	lua_pushinteger(hash_map->luaState, value_integer); /* stack: [value_integer, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_integer; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
}





void LuaHashMap_LuaHashMap_SetValueStringForKeyPointer(LuaHashMap* hash_map, const char* restrict value_string, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_pointer)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushstring(hash_map->luaState, value_string); /* stack: [value_string, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_LuaHashMap_SetValuePointerForKeyPointer(LuaHashMap* hash_map, void* value_pointer, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_pointer)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_pointer; stack: [table] */
//	lua_rawset(hash_map->luaState, -3);  /* table[key_pointer]=value_pointer; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}


void LuaHashMap_LuaHashMap_SetValueNumberForKeyPointer(LuaHashMap* hash_map, lua_Number value_number, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_pointer)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_LuaHashMap_SetValueIntegerForKeyPointer(LuaHashMap* hash_map, lua_Integer value_integer, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_pointer)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushinteger(hash_map->luaState, value_integer); /* stack: [value_integer, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_integer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}


void LuaHashMap_LuaHashMap_SetValueStringForKeyNumber(LuaHashMap* hash_map, const char* restrict value_string, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushstring(hash_map->luaState, value_string); /* stack: [value_string, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}


void LuaHashMap_LuaHashMap_SetValuePointerForKeyNumber(LuaHashMap* hash_map, void* value_pointer, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_pointer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}


void LuaHashMap_LuaHashMap_SetValueNumberForKeyNumber(LuaHashMap* hash_map, lua_Number value_number, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_LuaHashMap_SetValueIntegerForKeyNumber(LuaHashMap* hash_map, lua_Integer value_integer, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushinteger(hash_map->luaState, value_integer); /* stack: [value_integer, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_integer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}


void LuaHashMap_LuaHashMap_SetValueStringForKeyInteger(LuaHashMap* hash_map, const char* restrict value_string, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushstring(hash_map->luaState, value_string); /* stack: [value_string, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_LuaHashMap_SetValuePointerForKeyInteger(LuaHashMap* hash_map, void* value_pointer, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=value_pointer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_LuaHashMap_SetValueNumberForKeyInteger(LuaHashMap* hash_map, lua_Number value_number, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_LuaHashMap_SetValueIntegerForKeyInteger(LuaHashMap* hash_map, lua_Integer value_integer, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushinteger(hash_map->luaState, value_integer); /* stack: [value_integer, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=value_integer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}




const char* LuaHashMap_GetValueStringForKeyString(LuaHashMap* hash_map, const char* restrict key_string)
{
	const char* ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_string, table] */
	
	ret_val = lua_tostring(hash_map->luaState, -1);

//	fprintf(stderr, "ret_val=%s, %d\n", ret_val, lua_gettop(hash_map->luaState));

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}

void* LuaHashMap_GetValuePointerForKeyString(LuaHashMap* hash_map, const char* restrict key_string)
{
	void* ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_string)
	{
		return NULL;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_pointer, table] */

	ret_val = lua_touserdata(hash_map->luaState, -1);

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}

lua_Number LuaHashMap_GetValueNumberForKeyString(LuaHashMap* hash_map, const char* restrict key_string)
{
	lua_Number ret_val;
	if(NULL == hash_map)
	{
		return (lua_Number)0.0;
	}
	if(NULL == key_string)
	{
		return (lua_Number)0.0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_number, table] */
	ret_val = lua_tonumber(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}

lua_Integer LuaHashMap_GetValueIntegerForKeyString(LuaHashMap* hash_map, const char* restrict key_string)
{
	lua_Integer ret_val;
	if(NULL == hash_map)
	{
		return 0;
	}
	if(NULL == key_string)
	{
		return 0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_integer, table] */
	ret_val = lua_tointeger(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}


const char* LuaHashMap_GetValueStringForKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	const char* ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_pointer)
	{
		return NULL;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */
	ret_val = lua_tostring(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

void* LuaHashMap_GetValuePointerForKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	void* ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_pointer)
	{
		return NULL;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */
//	lua_rawget(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */
	ret_val = lua_touserdata(hash_map->luaState, -1);

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Number LuaHashMap_GetValueNumberForKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	lua_Number ret_val;
	if(NULL == hash_map)
	{
		return 0.0;
	}
	if(NULL == key_pointer)
	{
		return 0.0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */
	ret_val = lua_tonumber(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Integer LuaHashMap_GetValueIntegerForKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	lua_Integer ret_val;
	if(NULL == hash_map)
	{
		return 0;
	}
	if(NULL == key_pointer)
	{
		return 0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */
	ret_val = lua_tointeger(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}


const char* LuaHashMap_GetValueStringForKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	const char* ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_pointer, table] */
	ret_val = lua_tostring(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

void* LuaHashMap_GetValuePointerForKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	void* ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_pointer, table] */
	ret_val = lua_touserdata(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Number LuaHashMap_GetValueNumberForKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	lua_Number ret_val;
	if(NULL == hash_map)
	{
		return 0.0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_number, table] */
	ret_val = lua_tonumber(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Integer LuaHashMap_GetValueIntegerForKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	lua_Integer ret_val;
	if(NULL == hash_map)
	{
		return 0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_integer, table] */
	ret_val = lua_tointeger(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}


const char* LuaHashMap_GetValueStringForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	const char* ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_string, table] */
	ret_val = lua_tostring(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

void* LuaHashMap_GetValuePointerForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	void* ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_integer]; stack: [value_pointer, table] */
	ret_val = lua_touserdata(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Number LuaHashMap_GetValueNumberForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	lua_Number ret_val;
	if(NULL == hash_map)
	{
		return 0.0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_integer]; stack: [value_number, table] */
	ret_val = lua_tonumber(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

lua_Integer LuaHashMap_GetValueIntegerForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	lua_Integer ret_val;
	if(NULL == hash_map)
	{
		return 0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_integer]; stack: [value_integer, table] */
	ret_val = lua_tointeger(hash_map->luaState, -1);
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}





void LuaHashMap_RemoveKeyString(LuaHashMap* hash_map, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_string)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	lua_pushnil(hash_map->luaState); /* stack: [nil, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=nil; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
}

void LuaHashMap_RemoveKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_pointer)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushnil(hash_map->luaState); /* stack: [nil, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=nil; stack: [table] */
//	lua_rawset(hash_map->luaState, -3);  /* table[key_pointer]=nil; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_RemoveKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushnil(hash_map->luaState); /* stack: [nil, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=nil; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_RemoveKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushnil(hash_map->luaState); /* stack: [nil, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=nil; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

bool LuaHashMap_ExistsKeyString(LuaHashMap* hash_map, const char* restrict key_string)
{
	bool ret_val;
	if(NULL == hash_map)
	{
		return false;
	}
	if(NULL == key_string)
	{
		return false;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value, table] */
	
	if(LUA_TNIL==lua_type(hash_map->luaState, -1))
	{
		ret_val = false;
	}
	else
	{
		ret_val = true;
	}
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}

bool LuaHashMap_ExistsKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	bool ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	if(NULL == key_pointer)
	{
		return false;
	}


	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */

	if(LUA_TNIL==lua_type(hash_map->luaState, -1))
	{
		ret_val = false;
	}
	else
	{
		ret_val = true;
	}

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

bool LuaHashMap_ExistsKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	bool ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_number]; stack: [value_pointer, table] */

	if(LUA_TNIL==lua_type(hash_map->luaState, -1))
	{
		ret_val = false;
	}
	else
	{
		ret_val = true;
	}

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

bool LuaHashMap_ExistsKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	bool ret_val;
	if(NULL == hash_map)
	{
		return NULL;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_integer]; stack: [value_pointer, table] */
	
	if(LUA_TNIL==lua_type(hash_map->luaState, -1))
	{
		ret_val = false;
	}
	else
	{
		ret_val = true;
	}
	
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

size_t LuaHashMap_GetKeysString(LuaHashMap* hash_map, const char* keys_array[], size_t max_array_size)
{
	size_t total_count = 0;
	if(NULL == hash_map)
	{
		return 0;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */

	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		
		/* Remember that lua_tolstring modifies the string and confuses lua_next, so be sure these are all strings */
		if( (NULL != keys_array) && (total_count < max_array_size) )
		{
			keys_array[total_count] = lua_tostring(hash_map->luaState, -2);
		}

		total_count++;

		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(hash_map->luaState, 1);
	}

	/* Pop the global table */
	lua_pop(hash_map->luaState, 1);	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}

size_t LuaHashMap_GetKeysPointer(LuaHashMap* hash_map, void* keys_array[], size_t max_array_size)
{
	size_t total_count = 0;
	if(NULL == hash_map)
	{
		return 0;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */

	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		
		/* Remember that lua_tolstring modifies the string and confuses lua_next, so be sure these are all strings */
		if( (NULL != keys_array) && (total_count < max_array_size) )
		{
			keys_array[total_count] = lua_touserdata(hash_map->luaState, -2);
		}

		total_count++;

		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(hash_map->luaState, 1);
	}

	/* Pop the global table */
	lua_pop(hash_map->luaState, 1);	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}

size_t LuaHashMap_GetKeysNumber(LuaHashMap* hash_map, lua_Number keys_array[], size_t max_array_size)
{
	size_t total_count = 0;
	if(NULL == hash_map)
	{
		return 0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	
	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		
		/* Remember that lua_tolstring modifies the string and confuses lua_next, so be sure these are all strings */
		if( (NULL != keys_array) && (total_count < max_array_size) )
		{
			keys_array[total_count] = lua_tonumber(hash_map->luaState, -2);
		}
		
		total_count++;
		
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(hash_map->luaState, 1);
	}
	
	/* Pop the global table */
	lua_pop(hash_map->luaState, 1);	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}

size_t LuaHashMap_GetKeysInteger(LuaHashMap* hash_map, lua_Integer keys_array[], size_t max_array_size)
{
	size_t total_count = 0;
	if(NULL == hash_map)
	{
		return 0;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
	
	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		
		/* Remember that lua_tolstring modifies the string and confuses lua_next, so be sure these are all strings */
		if( (NULL != keys_array) && (total_count < max_array_size) )
		{
			keys_array[total_count] = lua_tointeger(hash_map->luaState, -2);
		}
		
		total_count++;
		
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(hash_map->luaState, 1);
	}
	
	/* Pop the global table */
	lua_pop(hash_map->luaState, 1);	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}

void Internal_Clear(LuaHashMap* hash_map, LuaHashMap_InternalGlobalKeyType table_name)
{
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, table_name); /* stack: [table] */
	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		
		lua_pop(hash_map->luaState, 1); /* pop the value because we don't need it; stack: [key table] */
		/* duplicate the key because we want to save a copy of it to be used for the next round of lua_next */
		lua_pushvalue(hash_map->luaState, -1); /* stack: [key key table] */
		lua_pushnil(hash_map->luaState); /* stack: [nil, key, key, table] */
		LUAHASHMAP_SETTABLE(hash_map->luaState, -4);  /* table[key_pointer]=nil; stack: [key table] */
		
		/* key is at the top of the stack, ready for next round of lua_next() */
	}
	lua_pop(hash_map->luaState, 1); /* pop the table */
}

void LuaHashMap_Clear(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return;
	}
/*	Internal_InitializeInternalTables(hash_map); */
	/* If we simply replace the current table with a new one, we lose the internal current size of the hash
	 * which might be painful to reallocate if the user intends to refill the hash with the same amount of data.
	 * So instead, it is probably better to iterate through all keys and remove them.
	 * If the user wants to blow away everything for more speed, they can always destroy the hashmap and create a new one.
     * Lua Gems verifies that removing entries only nils entries and doesn't rehash so 
     * it is safe to iterate via lua_next while removing items.
	 * See Purge for clearing that reclaims memory.
	 */

	Internal_Clear(hash_map, hash_map->uniqueTableNameForSharedState);
	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	LUAHASHMAP_ASSERT(true == LuaHashMap_IsEmpty(hash_map));
}


void LuaHashMap_Purge(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return;
	}
	/* Unlike Clear, this version replaces the existing table with a completely new one.
	 * This effectively purges the memory since Lua normally doesn't reclaim memory when nil-ing an entry.
	 * The presumption here is you really want the memory back.
	 */
	lua_newtable(hash_map->luaState);
	lua_rawseti(hash_map->luaState, LUA_GLOBALSINDEX, hash_map->uniqueTableNameForSharedState);
	/* Now seems to be a reasonable time to invoke garbage collection. */
	lua_gc(hash_map->luaState, LUA_GCCOLLECT, 0);

	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	LUAHASHMAP_ASSERT(true == LuaHashMap_IsEmpty(hash_map));
}


static bool Internal_IsEmpty(LuaHashMap* hash_map, LuaHashMap_InternalGlobalKeyType table_name)
{
	bool is_empty;
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, table_name); /* stack: [table] */

	lua_pushnil(hash_map->luaState);  /* first key */
	if(lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		is_empty = false;
		/* pop key, value, and table */
		lua_pop(hash_map->luaState, 3);
	}
	else
	{
		is_empty = true;
		/* pop table */
		lua_pop(hash_map->luaState, 1);
	}

	return is_empty;
}

bool LuaHashMap_IsEmpty(LuaHashMap* hash_map)
{
	bool is_empty = true;
	if(NULL == hash_map)
	{
		return true;
	}

	is_empty = Internal_IsEmpty(hash_map, hash_map->uniqueTableNameForSharedState);

	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);		

	return is_empty;
}

static bool Internal_IteratorNext(LuaHashMapIterator* hash_iterator)
{
	LuaHashMap* hash_map = hash_iterator->hashMap;
	LuaHashMap_InternalGlobalKeyType table_name = hash_iterator->whichTable;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, table_name); /* stack: [table] */
	
	 /* first key */
	if(LUA_TSTRING == hash_iterator->keyType)
	{
		lua_pushstring(hash_map->luaState, hash_iterator->currentKey.keyString);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
		lua_pushlightuserdata(hash_map->luaState, hash_iterator->currentKey.keyPointer);
	}
	/* Note: This could be either a number or integer since they are the same to Lua.
	 * Since this is just for internal use, I don't need to distinguish.
	 */
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		lua_pushnumber(hash_map->luaState, hash_iterator->currentKey.keyNumber);
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		lua_pop(hash_map->luaState, 1);
		return false;
	}

	if(lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		hash_iterator->atEnd = false;
		switch(lua_type(hash_map->luaState, -2))
		{
			case LUA_TSTRING:
			{
				hash_iterator->currentKey.keyString = lua_tostring(hash_map->luaState, -2);
				break;
			}
			case LUA_TLIGHTUSERDATA:
			case LUA_TUSERDATA:
			{
				hash_iterator->currentKey.keyPointer = lua_touserdata(hash_map->luaState, -2);
				break;
			}
			case LUA_TNUMBER:
			{
				/* Note: Like above, I can't easily distinguish between a number and integer.
				 * So I will treat things as a number since that is the Lua default and cast later as necessary.
				 */
				hash_iterator->currentKey.keyNumber = lua_tonumber(hash_map->luaState, -2);
				break;
			}
			default:
			{
				/* Clear the largest field to make sure every thing is cleared. */
				memset(&hash_iterator->currentKey, 0, sizeof(union LuaHashMapKeyType));
			}
		}
		
		/* pop key, value, and table */
		lua_pop(hash_map->luaState, 3);
	}
	else
	{
		hash_iterator->atEnd = true;
		/* Clear the largest field to make sure every thing is cleared. */
		memset(&hash_iterator->currentKey, 0, sizeof(union LuaHashMapKeyType));
		
		/* pop table */
		lua_pop(hash_map->luaState, 1);
	}
	
	/* return true if more keys follow, false if we're at the end */
	return (false == hash_iterator->atEnd);
}

static LuaHashMapIterator Internal_GetIteratorBegin(LuaHashMap* hash_map, LuaHashMap_InternalGlobalKeyType table_name)
{
	LuaHashMapIterator the_iterator;
	the_iterator.hashMap = hash_map;
	the_iterator.whichTable = table_name;
	the_iterator.keyType = LUA_TNIL;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, table_name); /* stack: [table] */
	
	lua_pushnil(hash_map->luaState);  /* first key */
	if(lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		the_iterator.atEnd = false;
		the_iterator.keyType = lua_type(hash_map->luaState, -2);
		switch(the_iterator.keyType)
		{
			case LUA_TSTRING:
			{
				the_iterator.currentKey.keyString = lua_tostring(hash_map->luaState, -2);
				break;
			}
			case LUA_TLIGHTUSERDATA:
			case LUA_TUSERDATA:
			{
				the_iterator.currentKey.keyPointer = lua_touserdata(hash_map->luaState, -2);
				break;
			}
			case LUA_TNUMBER:
			{
				/* Note: Like above, I can't easily distinguish between a number and integer.
				 * So I will treat things as a number since that is the Lua default and cast later as necessary.
				 */
				the_iterator.currentKey.keyNumber = lua_tonumber(hash_map->luaState, -2);
				break;
			}
			default:
			{
				the_iterator.currentKey.keyPointer = NULL;
			}
		}

		/* pop key, value, and table */
		lua_pop(hash_map->luaState, 3);
	}
	else
	{
		the_iterator.atEnd = true;
		the_iterator.currentKey.keyPointer = NULL;

		/* pop table */
		lua_pop(hash_map->luaState, 1);
	}
	
	return the_iterator;
}

static LuaHashMapIterator Internal_GetIteratorEnd(LuaHashMap* hash_map, LuaHashMap_InternalGlobalKeyType table_name)
{
	LuaHashMapIterator the_iterator;
	the_iterator.hashMap = hash_map;
	the_iterator.whichTable = table_name;
	the_iterator.atEnd = true;
	the_iterator.keyType = LUA_TNIL;	
	/* Clear the largest field to make sure every thing is cleared. */
	memset(&the_iterator.currentKey, 0, sizeof(union LuaHashMapKeyType));
	return the_iterator;
}

bool LuaHashMap_IteratorNext(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return false;
	}
	return Internal_IteratorNext(hash_iterator);
}

static LuaHashMapIterator Internal_CreateBadIterator()
{
	LuaHashMapIterator the_iterator;
	memset(&the_iterator, 0, sizeof(LuaHashMapIterator));
	the_iterator.whichTable = LUA_NOREF;
	the_iterator.atEnd = true;
	the_iterator.keyType = LUA_TNIL;
	return the_iterator;
}

LuaHashMapIterator LuaHashMap_GetIteratorAtBegin(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	return Internal_GetIteratorBegin(hash_map, hash_map->uniqueTableNameForSharedState);
}

LuaHashMapIterator LuaHashMap_GetIteratorAtEnd(LuaHashMap* hash_map)
{
	return Internal_GetIteratorEnd(hash_map, hash_map->uniqueTableNameForSharedState);
}

LuaHashMapIterator LuaHashMap_GetIteratorForKeyString(LuaHashMap* hash_map, const char* key_string)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	if(LuaHashMap_ExistsKeyString(hash_map, key_string))
	{
		LuaHashMapIterator the_iterator;
		the_iterator.hashMap = hash_map;
		the_iterator.whichTable = hash_map->uniqueTableNameForSharedState;
		the_iterator.atEnd = false;
		the_iterator.keyType = LUA_TSTRING;
		the_iterator.currentKey.keyString = key_string;
		return the_iterator;
	}
	else
	{
		return Internal_CreateBadIterator();
	}
}

LuaHashMapIterator LuaHashMap_GetIteratorForKeyPointer(LuaHashMap* hash_map, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	if(LuaHashMap_ExistsKeyPointer(hash_map, key_pointer))
	{
		LuaHashMapIterator the_iterator;
		the_iterator.hashMap = hash_map;
		the_iterator.whichTable = hash_map->uniqueTableNameForSharedState;
		the_iterator.atEnd = false;
		the_iterator.keyType = LUA_TLIGHTUSERDATA;		
		the_iterator.currentKey.keyPointer = key_pointer;
		return the_iterator;
	}
	else
	{
		return Internal_CreateBadIterator();
	}
}

LuaHashMapIterator LuaHashMap_GetIteratorForKeyNumber(LuaHashMap* hash_map, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	if(LuaHashMap_ExistsKeyNumber(hash_map, key_number))
	{
		LuaHashMapIterator the_iterator;
		the_iterator.hashMap = hash_map;
		the_iterator.whichTable = hash_map->uniqueTableNameForSharedState;
		the_iterator.atEnd = false;
		the_iterator.keyType = LUA_TNUMBER;
		the_iterator.currentKey.keyNumber = key_number;
		return the_iterator;
	}
	else
	{
		return Internal_CreateBadIterator();
	}
}

LuaHashMapIterator LuaHashMap_GetIteratorForKeyInteger(LuaHashMap* hash_map, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	if(LuaHashMap_ExistsKeyInteger(hash_map, key_integer))
	{
		LuaHashMapIterator the_iterator;
		the_iterator.hashMap = hash_map;
		the_iterator.whichTable = hash_map->uniqueTableNameForSharedState;
		the_iterator.atEnd = false;
		/* Note: I might be able to benefit from the information that this is an integer,
		 * but since this is not the only entry point to create an iterator,
		 * its usefulness is limited.
		 */
		the_iterator.keyType = LUA_TNUMBER;
		the_iterator.currentKey.keyNumber = (lua_Number)key_integer;		
/*		the_iterator.currentKey.keyInteger = key_integer; */
		return the_iterator;
	}
	else
	{
		return Internal_CreateBadIterator();
	}
}



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

bool LuaHashMap_IteratorIsEqual(const LuaHashMapIterator* hash_iterator1, const LuaHashMapIterator* hash_iterator2)
{
	if(NULL == hash_iterator1 && NULL == hash_iterator2)
	{
		return true;
	}
	else if(NULL == hash_iterator1)
	{
		return false;
	}
	else if(NULL == hash_iterator2)
	{
		return false;
	}
	else
	{
		/* Note: Don't test against keyType because when I create an End iterator, I don't know the type. */
		bool ret_flag = (hash_iterator1->hashMap == hash_iterator2->hashMap)
			&& (hash_iterator1->whichTable == hash_iterator2->whichTable)
			&& (hash_iterator1->atEnd == hash_iterator2->atEnd)
		;
		if(false == ret_flag)
		{
			return false;
		}
		/* If the iterators are both atEnd, we can consider them equal. */
		if(true == hash_iterator1->atEnd)
		{
			return true;
		}
		 
		
		if(LUA_TSTRING == hash_iterator1->keyType)
		{
			return (0 == Internal_safestrcmp(hash_iterator1->currentKey.keyString, hash_iterator2->currentKey.keyString));
		}
		else if(LUA_TLIGHTUSERDATA == hash_iterator1->keyType)
		{
			return (hash_iterator1->currentKey.keyPointer == hash_iterator2->currentKey.keyPointer);
		}
		else if(LUA_TNUMBER == hash_iterator1->keyType)
		{
			return (hash_iterator1->currentKey.keyNumber == hash_iterator2->currentKey.keyNumber);
		}
		else
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			return false;
		}
	}
}

void LuaHashMap_LuaHashMap_SetValueStringAtIterator(LuaHashMapIterator* hash_iterator, const char* restrict value_string)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
		LuaHashMap_LuaHashMap_SetValueStringForKeyString(hash_iterator->hashMap, value_string, hash_iterator->currentKey.keyString);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
		LuaHashMap_LuaHashMap_SetValueStringForKeyPointer(hash_iterator->hashMap, value_string, hash_iterator->currentKey.keyPointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_LuaHashMap_SetValue*ForKeyInteger..
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
		LuaHashMap_LuaHashMap_SetValueStringForKeyNumber(hash_iterator->hashMap, value_string, hash_iterator->currentKey.keyNumber);
		/* LuaHashMap_LuaHashMap_SetValueStringForKeyInteger(hash_iterator->hashMap, value_string, hash_iterator->currentKey.keyInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return;
	}
}

void LuaHashMap_LuaHashMap_SetValuePointerAtIterator(LuaHashMapIterator* hash_iterator, void* value_pointer)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
		LuaHashMap_LuaHashMap_SetValuePointerForKeyString(hash_iterator->hashMap, value_pointer, hash_iterator->currentKey.keyString);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
		LuaHashMap_LuaHashMap_SetValuePointerForKeyPointer(hash_iterator->hashMap, value_pointer, hash_iterator->currentKey.keyPointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_LuaHashMap_SetValue*ForKeyInteger..
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
		LuaHashMap_LuaHashMap_SetValuePointerForKeyNumber(hash_iterator->hashMap, value_pointer, hash_iterator->currentKey.keyNumber);
		/* LuaHashMap_LuaHashMap_SetValuePointerForKeyInteger(hash_iterator->hashMap, value_pointer, hash_iterator->currentKey.keyInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return;
	}
}

void LuaHashMap_LuaHashMap_SetValueNumberAtIterator(LuaHashMapIterator* hash_iterator, lua_Number value_number)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
		LuaHashMap_LuaHashMap_SetValueNumberForKeyString(hash_iterator->hashMap, value_number, hash_iterator->currentKey.keyString);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
		LuaHashMap_LuaHashMap_SetValueNumberForKeyPointer(hash_iterator->hashMap, value_number, hash_iterator->currentKey.keyPointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_LuaHashMap_SetValue*ForKeyInteger.
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
		LuaHashMap_LuaHashMap_SetValueNumberForKeyNumber(hash_iterator->hashMap, value_number, hash_iterator->currentKey.keyNumber);
		/* LuaHashMap_LuaHashMap_SetValueNumberForKeyInteger(hash_iterator->hashMap, value_number, hash_iterator->currentKey.keyInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return;
	}
}

void LuaHashMap_LuaHashMap_SetValueIntegerAtIterator(LuaHashMapIterator* hash_iterator, lua_Integer value_integer)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
		LuaHashMap_LuaHashMap_SetValueIntegerForKeyString(hash_iterator->hashMap, value_integer, hash_iterator->currentKey.keyString);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
		LuaHashMap_LuaHashMap_SetValueIntegerForKeyPointer(hash_iterator->hashMap, value_integer, hash_iterator->currentKey.keyPointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_LuaHashMap_SetValue*ForKeyInteger.
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
		LuaHashMap_LuaHashMap_SetValueIntegerForKeyNumber(hash_iterator->hashMap, value_integer, hash_iterator->currentKey.keyNumber);
		/* LuaHashMap_LuaHashMap_SetValueIntegerForKeyInteger(hash_iterator->hashMap, value_integer, hash_iterator->currentKey.keyInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return;
	}
}




const char* LuaHashMap_GetValueStringAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
        return LuaHashMap_GetValueStringForKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
        return LuaHashMap_GetValueStringForKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_GetValue*ForKeyInteger.
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
        return LuaHashMap_GetValueStringForKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
        /* return LuaHashMap_GetValueStringForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return NULL;
	}
}

void* LuaHashMap_GetValuePointerAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return NULL;
	}
	if(true == hash_iterator->atEnd)
	{
		return NULL;
	}
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
        return LuaHashMap_GetValuePointerForKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
        return LuaHashMap_GetValuePointerForKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_GetValue*ForKeyInteger.
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
        return LuaHashMap_GetValuePointerForKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
        /* return LuaHashMap_GetValuePointerForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return NULL;
	}
}

lua_Number LuaHashMap_GetValueNumberAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return 0.0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0.0;
	}
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
        return LuaHashMap_GetValueNumberForKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
        return LuaHashMap_GetValueNumberForKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_GetValue*ForKeyInteger.
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
        return LuaHashMap_GetValueNumberForKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
        /* return LuaHashMap_GetValueNumberForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return 0.0;
	}
}


lua_Integer LuaHashMap_GetValueIntegerAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return 0;
	}
	if(true == hash_iterator->atEnd)
	{
		return 0;
	}
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
        return LuaHashMap_GetValueIntegerForKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
        return LuaHashMap_GetValueIntegerForKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 * If this is supposed to be an integer, I technically should be calling LuaHashMap_GetValue*ForKeyInteger.
		 * As of this writing, the only difference between the functions is whether I call pushnumber or pushinteger.
		 * In stock Lua cases, I don't think there will be a real difference.
		 * To solve this, I would need to know the intention either by an explicit function, 
		 * or I need to track the intention using a declaration/hint in creation, 
		 * or I need to flag the first use of a type and save it.
		 */
        return LuaHashMap_GetValueIntegerForKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
        /* return LuaHashMap_GetValueIntegerForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return 0;
	}
}


bool LuaHashMap_ExistsAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return false;
	}
	if(true == hash_iterator->atEnd)
	{
		return false;
	}
	
	if(LUA_TSTRING == hash_iterator->keyType)
	{
		return LuaHashMap_ExistsKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
		return LuaHashMap_ExistsKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 */
		return LuaHashMap_ExistsKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
		/* return LuaHashMap_ExistsKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return false;
	}
}

void LuaHashMap_RemoveAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	
	if(LUA_TSTRING == hash_iterator->whichTable)
	{
		LuaHashMap_RemoveKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->whichTable)
	{
		LuaHashMap_RemoveKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUA_TNUMBER == hash_iterator->whichTable)
	{
		/* Warning: This might be a problem. I can't easily distinguish between a number and integer.
		 */
		LuaHashMap_RemoveKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
		/* LuaHashMap_RemoveKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger); */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return;
	}
}

static size_t Internal_Count(LuaHashMap* hash_map)
{
	size_t total_count = 0;

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */

	lua_pushnil(hash_map->luaState);  /* first key */
	while (lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		/* lua_next puts 'key' (at index -2) and 'value' (at index -1) */
		total_count++;
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(hash_map->luaState, 1);
	}

	/* Pop the global table */
	lua_pop(hash_map->luaState, 1);	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}

size_t LuaHashMap_Count(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return 0;
	}
	return Internal_Count(hash_map);
}

int LuaHashMap_GetKeyTypeAtIterator(LuaHashMapIterator* hash_iterator)
{
	if(NULL == hash_iterator)
	{
		return LUA_TNIL;
	}
	return hash_iterator->keyType;
}

int LuaHashMap_GetValueTypeAtIterator(LuaHashMapIterator* hash_iterator)
{
	int ret_val;
	if(NULL == hash_iterator)
	{
		return LUA_TNIL;
	}
	if(true == hash_iterator->atEnd)
	{
		return LUA_TNIL;
	}
	
	LuaHashMap* hash_map = hash_iterator->hashMap;
	if(NULL == hash_map)
	{
		return LUA_TNIL;
	}

	if(LUA_TSTRING == hash_iterator->keyType)
	{
		LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
		lua_pushstring(hash_map->luaState, hash_iterator->currentKey.keyString); /* stack: [key_string, table] */
		LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_string, table] */
	}
	else if(LUA_TLIGHTUSERDATA == hash_iterator->keyType)
	{
		LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
		lua_pushlightuserdata(hash_map->luaState, hash_iterator->currentKey.keyPointer); /* stack: [key_string, table] */
		LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_string, table] */
	}
	else if(LUA_TNUMBER == hash_iterator->keyType)
	{
		/* Warning: This might be a problem. I can't distinguish between a number and integer. */		
		LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, hash_map->uniqueTableNameForSharedState); /* stack: [table] */
		lua_pushnumber(hash_map->luaState, hash_iterator->currentKey.keyNumber); /* stack: [key_string, table] */
		LUAHASHMAP_GETTABLE(hash_map->luaState, -2);  /* table[key_string]; stack: [value_string, table] */
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return LUA_TNIL;
	}
	ret_val = lua_type(hash_map->luaState, -1);
	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	return ret_val;
}


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


