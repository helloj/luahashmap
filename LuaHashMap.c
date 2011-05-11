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

/* Benchmark:
 
 CFTimeInterval start_time = CACurrentMediaTime();
 void* ret_ptr = NULL;	
 for(i=0; i<40000000; i++)
 {
 LuaHashMap_InsertValuePointerForKeyPointer(hash_map, (void*)i, (void*)i);
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

#if 1

	#define LUAHASHMAP_GETGLOBAL_UNIQUESTRING(lua_state, unique_string_pointer) \
		lua_pushlightuserdata(hash_map->luaState, (void*)unique_string_pointer); \
		LUAHASHMAP_GETTABLE(hash_map->luaState, LUA_GLOBALSINDEX);

	/* The lua_insert is used to swap the value and key positions because settable assumes the order:
		lua_pushlightuserdata(hash_map->luaState, (void*)LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
		lua_newtable(hash_map->luaState);	
		LUAHASHMAP_SETTABLE(hash_map->luaState, LUA_GLOBALSINDEX);
	 
		But with this API, the user must have already pushed the newtable before calling this.
	 */

	#define LUAHASHMAP_SETGLOBAL_UNIQUESTRING(lua_state, unique_string_pointer) \
		lua_pushlightuserdata(hash_map->luaState, (void*)unique_string_pointer); \
		lua_insert(lua_state, -2); \
		LUAHASHMAP_SETTABLE(lua_state, LUA_GLOBALSINDEX);

#else
	#define LUAHASHMAP_GETGLOBAL_UNIQUESTRING(lua_state, unique_string_pointer) lua_getglobal(hash_map->luaState, unique_string_pointer)

	#define LUAHASHMAP_SETGLOBAL_UNIQUESTRING(lua_state, unique_string_pointer) lua_setglobal(lua_state, unique_string_pointer)

#endif

#define LUAHASHMAP_DEBUG
#ifdef LUAHASHMAP_DEBUG
	#define LUAHASHMAP_ASSERT(e) assert(e)
#else
	#define LUAHASHMAP_ASSERT(e)
#endif



//static const char* LUAHASHMAP_DEFAULT_TABLE_NAME = "lhm";
static const char* LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING = "lhm.string";
static const char* LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER = "lhm.number";
static const char* LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER = "lhm.integer";
static const char* LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER = "lhm.pointer";

struct LuaHashMap
{
	lua_State* luaState;
	lua_Alloc memoryAllocator;
	void* allocatorUserData;
	/*
	size_t numberOfStringKeys;
	size_t numberOfNumberKeys;
	size_t numberOfIntegerKeys;
	size_t numberOfPointerKeys;
	*/
};


static void Internal_InitializeInternalTables(LuaHashMap* hash_map)
{
	/* Create a table in Lua to be our hash map */
	/* We could just use one table since the use documents only
	 * one kind of key-value type per instance, but for extra safety,
	 * I will create a table for each key type so lua_next doesn't break
	 * when fetching all keys.
	 *
	 * According to Lua Gems performance, Lua tables are created with 0 size by default to not waste memory
	 * so creating different tables shouldn't be that bad.
	 * 
	 * Using lightuserdata pointers instead of strings because I know my strings are constants 
	 * and unique and expect it to be faster than needing to do strlen()'s and possibly strcmp()'s.
	 */
	
#if 0
	lua_pushlightuserdata(hash_map->luaState, (void*)LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [key] */
	lua_newtable(hash_map->luaState); /* push value: stack: [value_table, key] */ 	
	LUAHASHMAP_SETTABLE(hash_map->luaState, LUA_GLOBALSINDEX);
	
	
	lua_pushlightuserdata(hash_map->luaState, (void*)LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [key] */
	lua_newtable(hash_map->luaState); /* push value: stack: [value_table, key] */ 	
	LUAHASHMAP_SETTABLE(hash_map->luaState, LUA_GLOBALSINDEX);

	lua_pushlightuserdata(hash_map->luaState, (void*)LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [key] */
	lua_newtable(hash_map->luaState); /* push value: stack: [value_table, key] */ 	
	LUAHASHMAP_SETTABLE(hash_map->luaState, LUA_GLOBALSINDEX);

	lua_pushlightuserdata(hash_map->luaState, (void*)LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [key] */
	lua_newtable(hash_map->luaState); /* push value: stack: [value_table, key] */ 	
	LUAHASHMAP_SETTABLE(hash_map->luaState, LUA_GLOBALSINDEX);
#endif

	lua_newtable(hash_map->luaState);
	LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
	lua_newtable(hash_map->luaState);
	LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
	lua_newtable(hash_map->luaState);
	LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
	lua_newtable(hash_map->luaState);
	LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
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


LuaHashMap* LuaHashMap_CreateWithSizeHints(int number_of_array_elements, int number_of_hash_elements, int key_type, int value_type)
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
	
	switch( 0x0F & key_type)
	{
		case LUAHASHMAP_KEYSTRING_TYPE:
		{
			lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
			break;
		}
		case LUAHASHMAP_KEYPOINTER_TYPE:
		{
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
			
			lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
			break;
		}
		case LUAHASHMAP_KEYNUMBER_TYPE:
		{
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
			
			lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
			break;
		}
		case LUAHASHMAP_KEYINTEGER_TYPE:
		{
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);

			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
			
			lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
			break;
		}
		default:
		{
			Internal_InitializeInternalTables(hash_map);
		}
	}
	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return hash_map;
}


LuaHashMap* LuaHashMap_CreateWithAllocatorAndSizeHints(lua_Alloc the_allocator, void* user_data, int number_of_array_elements, int number_of_hash_elements, int key_type, int value_type)
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

	switch( 0x0F & key_type)
	{
		case LUAHASHMAP_KEYSTRING_TYPE:
		{
			lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
			break;
		}
		case LUAHASHMAP_KEYPOINTER_TYPE:
		{
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
			
			lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
			break;
		}
		case LUAHASHMAP_KEYNUMBER_TYPE:
		{
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
			
			lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
			break;
		}
		case LUAHASHMAP_KEYINTEGER_TYPE:
		{
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
			
			lua_newtable(hash_map->luaState);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
			
			lua_createtable(hash_map->luaState, number_of_array_elements, number_of_hash_elements);
			LUAHASHMAP_SETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
			break;
		}
		default:
		{
			Internal_InitializeInternalTables(hash_map);
		}
	}
	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
	return hash_map;
}


void LuaHashMap_Free(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return;
	}
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

void LuaHashMap_InsertValueStringForKeyString(LuaHashMap* hash_map, const char* restrict value_string, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_string)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	lua_pushstring(hash_map->luaState, value_string); /* stack: [value_string, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_InsertValuePointerForKeyString(LuaHashMap* hash_map, void* value_pointer, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_string)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_pointer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
}

void LuaHashMap_InsertValueNumberForKeyString(LuaHashMap* hash_map, lua_Number value_number, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_string)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
}

void LuaHashMap_InsertValueIntegerForKeyString(LuaHashMap* hash_map, lua_Integer value_integer, const char* restrict key_string)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_string)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
	lua_pushstring(hash_map->luaState, key_string); /* stack: [key_string, table] */
	lua_pushinteger(hash_map->luaState, value_integer); /* stack: [value_integer, key_string, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_string]=value_integer; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
}





void LuaHashMap_InsertValueStringForKeyPointer(LuaHashMap* hash_map, const char* restrict value_string, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_pointer)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushstring(hash_map->luaState, value_string); /* stack: [value_string, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_InsertValuePointerForKeyPointer(LuaHashMap* hash_map, void* value_pointer, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_pointer)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_pointer; stack: [table] */
//	lua_rawset(hash_map->luaState, -3);  /* table[key_pointer]=value_pointer; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}


void LuaHashMap_InsertValueNumberForKeyPointer(LuaHashMap* hash_map, lua_Number value_number, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_pointer)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_InsertValueIntegerForKeyPointer(LuaHashMap* hash_map, lua_Integer value_integer, void* key_pointer)
{
	if(NULL == hash_map)
	{
		return;
	}
	if(NULL == key_pointer)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushinteger(hash_map->luaState, value_integer); /* stack: [value_integer, key_pointer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_pointer]=value_integer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}


void LuaHashMap_InsertValueStringForKeyNumber(LuaHashMap* hash_map, const char* restrict value_string, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushstring(hash_map->luaState, value_string); /* stack: [value_string, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}


void LuaHashMap_InsertValuePointerForKeyNumber(LuaHashMap* hash_map, void* value_pointer, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_pointer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}


void LuaHashMap_InsertValueNumberForKeyNumber(LuaHashMap* hash_map, lua_Number value_number, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_InsertValueIntegerForKeyNumber(LuaHashMap* hash_map, lua_Integer value_integer, lua_Number key_number)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [table] */
	lua_pushnumber(hash_map->luaState, key_number); /* stack: [key_number, table] */
	lua_pushinteger(hash_map->luaState, value_integer); /* stack: [value_integer, key_number, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_number]=value_integer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}


void LuaHashMap_InsertValueStringForKeyInteger(LuaHashMap* hash_map, const char* restrict value_string, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushstring(hash_map->luaState, value_string); /* stack: [value_string, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_InsertValuePointerForKeyInteger(LuaHashMap* hash_map, void* value_pointer, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=value_pointer; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_InsertValueNumberForKeyInteger(LuaHashMap* hash_map, lua_Number value_number, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [table] */
	lua_pushinteger(hash_map->luaState, key_integer); /* stack: [key_integer, table] */
	lua_pushnumber(hash_map->luaState, value_number); /* stack: [value_number, key_integer, table] */
	LUAHASHMAP_SETTABLE(hash_map->luaState, -3);  /* table[key_integer]=value_number; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);
}

void LuaHashMap_InsertValueIntegerForKeyInteger(LuaHashMap* hash_map, lua_Integer value_integer, lua_Integer key_integer)
{
	if(NULL == hash_map)
	{
		return;
	}
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [table] */
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

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
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

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
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

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [table] */
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

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
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

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [table] */
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

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
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


	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
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

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [table] */
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [table] */
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

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */

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

	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */

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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER); /* stack: [table] */
	
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
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER); /* stack: [table] */
	
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

void Internal_Clear(LuaHashMap* hash_map, const char* table_name)
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
	/* If we simply replace the current table with a new one, we lose the internal current size of the hash.
	 * So instead, it is probably better to iterate through all keys and remove them.
	 * If the user wants to blow away everything for more speed, they can always destroy the hashmap and create a new one.
     * Lua Gems verifies that removing entries only nils entries and doesn't rehash so 
     * it is safe to iterate via lua_next while removing items.
	 */

	Internal_Clear(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
	Internal_Clear(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
	Internal_Clear(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
	Internal_Clear(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
	
	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);	
	LUAHASHMAP_ASSERT(true == LuaHashMap_IsEmpty(hash_map));
}

static bool Internal_IsEmpty(LuaHashMap* hash_map, const char* table_name)
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

	is_empty = Internal_IsEmpty(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING)
		&&  Internal_IsEmpty(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER)
		&&  Internal_IsEmpty(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER)
		&&  Internal_IsEmpty(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER)
		;

	LUAHASHMAP_ASSERT(lua_gettop(hash_map->luaState) == 0);		

	return is_empty;
}

static bool Internal_IteratorNext(LuaHashMapIterator* hash_iterator)
{
	LuaHashMap* hash_map = hash_iterator->hashMap;
	const char* table_name = hash_iterator->whichTable;
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, table_name); /* stack: [table] */
	
	 /* first key */
	if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == table_name)
	{
		lua_pushstring(hash_map->luaState, hash_iterator->currentKey.keyString);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == table_name)
	{
		lua_pushlightuserdata(hash_map->luaState, hash_iterator->currentKey.keyPointer);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == table_name)
	{
		lua_pushnumber(hash_map->luaState, hash_iterator->currentKey.keyNumber);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == table_name)
	{
		lua_pushinteger(hash_map->luaState, hash_iterator->currentKey.keyInteger);		
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
				/* pointer comparison should be fine in this case */
				if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == table_name)
				{
					hash_iterator->currentKey.keyNumber = lua_tonumber(hash_map->luaState, -2);
				}
				else
				{
					hash_iterator->currentKey.keyInteger = lua_tointeger(hash_map->luaState, -2);
				}
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

static LuaHashMapIterator Internal_GetIteratorBegin(LuaHashMap* hash_map, const char* table_name)
{
	LuaHashMapIterator the_iterator;
	the_iterator.hashMap = hash_map;
	the_iterator.whichTable = table_name;
	
	LUAHASHMAP_GETGLOBAL_UNIQUESTRING(hash_map->luaState, table_name); /* stack: [table] */
	
	lua_pushnil(hash_map->luaState);  /* first key */
	if(lua_next(hash_map->luaState, -2) != 0) /* use index of table */
	{
		the_iterator.atEnd = false;
		switch(lua_type(hash_map->luaState, -2))
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
				/* pointer comparison should be fine in this case */
				if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == table_name)
				{
					the_iterator.currentKey.keyNumber = lua_tonumber(hash_map->luaState, -2);
				}
				else
				{
					the_iterator.currentKey.keyInteger = lua_tointeger(hash_map->luaState, -2);
				}
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

static LuaHashMapIterator Internal_GetIteratorEnd(LuaHashMap* hash_map, const char* table_name)
{
	LuaHashMapIterator the_iterator;
	the_iterator.hashMap = hash_map;
	the_iterator.whichTable = table_name;
	the_iterator.atEnd = true;
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
	the_iterator.atEnd = true;
	return the_iterator;
}

LuaHashMapIterator LuaHashMap_GetIteratorAtBeginForKeyString(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	return Internal_GetIteratorBegin(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
}

LuaHashMapIterator LuaHashMap_GetIteratorAtEndForKeyString(LuaHashMap* hash_map)
{
	return Internal_GetIteratorEnd(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
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
		the_iterator.whichTable = LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING;
		the_iterator.atEnd = false;
		the_iterator.currentKey.keyString = key_string;
		return the_iterator;
	}
	else
	{
		return Internal_CreateBadIterator();
	}
}


LuaHashMapIterator LuaHashMap_GetIteratorAtBeginForKeyPointer(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	return Internal_GetIteratorBegin(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
}

LuaHashMapIterator LuaHashMap_GetIteratorAtEndForKeyPointer(LuaHashMap* hash_map)
{
	return Internal_GetIteratorEnd(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
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
		the_iterator.whichTable = LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER;
		the_iterator.atEnd = false;
		the_iterator.currentKey.keyPointer = key_pointer;
		return the_iterator;
	}
	else
	{
		return Internal_CreateBadIterator();
	}
}


LuaHashMapIterator LuaHashMap_GetIteratorAtBeginForKeyNumber(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	return Internal_GetIteratorBegin(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
}

LuaHashMapIterator LuaHashMap_GetIteratorAtEndForKeyNumber(LuaHashMap* hash_map)
{
	return Internal_GetIteratorEnd(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
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
		the_iterator.whichTable = LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER;
		the_iterator.atEnd = false;
		the_iterator.currentKey.keyNumber = key_number;
		return the_iterator;
	}
	else
	{
		return Internal_CreateBadIterator();
	}
}


LuaHashMapIterator LuaHashMap_GetIteratorAtBeginForKeyInteger(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return Internal_CreateBadIterator();
	}
	return Internal_GetIteratorBegin(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
}

LuaHashMapIterator LuaHashMap_GetIteratorAtEndForKeyInteger(LuaHashMap* hash_map)
{
	return Internal_GetIteratorEnd(hash_map, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
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
		the_iterator.whichTable = LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER;
		the_iterator.atEnd = false;
		the_iterator.currentKey.keyInteger = key_integer;
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
		 
		
		if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == hash_iterator1->whichTable)
		{
			return (0 == Internal_safestrcmp(hash_iterator1->currentKey.keyString, hash_iterator2->currentKey.keyString));
		}
		else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == hash_iterator1->whichTable)
		{
			return (hash_iterator1->currentKey.keyPointer == hash_iterator2->currentKey.keyPointer);
		}
		else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == hash_iterator1->whichTable)
		{
			return (hash_iterator1->currentKey.keyNumber == hash_iterator2->currentKey.keyNumber);
		}
		else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == hash_iterator1->whichTable)
		{
			return (hash_iterator1->currentKey.keyInteger == hash_iterator2->currentKey.keyInteger);
		}
		else
		{
			/* shouldn't get here */
			LUAHASHMAP_ASSERT(false);
			return false;
		}
	}
}

void LuaHashMap_InsertValueStringAtIterator(LuaHashMapIterator* hash_iterator, const char* restrict value_string)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	
	
	if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueStringForKeyString(hash_iterator->hashMap, value_string, hash_iterator->currentKey.keyString);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueStringForKeyPointer(hash_iterator->hashMap, value_string, hash_iterator->currentKey.keyPointer);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueStringForKeyNumber(hash_iterator->hashMap, value_string, hash_iterator->currentKey.keyNumber);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueStringForKeyInteger(hash_iterator->hashMap, value_string, hash_iterator->currentKey.keyInteger);
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return;
	}
}

void LuaHashMap_InsertValuePointerAtIterator(LuaHashMapIterator* hash_iterator, void* value_pointer)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	
	if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValuePointerForKeyString(hash_iterator->hashMap, value_pointer, hash_iterator->currentKey.keyString);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValuePointerForKeyPointer(hash_iterator->hashMap, value_pointer, hash_iterator->currentKey.keyPointer);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValuePointerForKeyNumber(hash_iterator->hashMap, value_pointer, hash_iterator->currentKey.keyNumber);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValuePointerForKeyInteger(hash_iterator->hashMap, value_pointer, hash_iterator->currentKey.keyInteger);
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return;
	}
}

void LuaHashMap_InsertValueNumberAtIterator(LuaHashMapIterator* hash_iterator, lua_Number value_number)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	
	
	if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueNumberForKeyString(hash_iterator->hashMap, value_number, hash_iterator->currentKey.keyString);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueNumberForKeyPointer(hash_iterator->hashMap, value_number, hash_iterator->currentKey.keyPointer);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueNumberForKeyNumber(hash_iterator->hashMap, value_number, hash_iterator->currentKey.keyNumber);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueNumberForKeyInteger(hash_iterator->hashMap, value_number, hash_iterator->currentKey.keyInteger);
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return;
	}
}

void LuaHashMap_InsertValueIntegerAtIterator(LuaHashMapIterator* hash_iterator, lua_Integer value_integer)
{
	if(NULL == hash_iterator)
	{
		return;
	}
	if(true == hash_iterator->atEnd)
	{
		return;
	}
	
	
	if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueIntegerForKeyString(hash_iterator->hashMap, value_integer, hash_iterator->currentKey.keyString);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueIntegerForKeyPointer(hash_iterator->hashMap, value_integer, hash_iterator->currentKey.keyPointer);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueIntegerForKeyNumber(hash_iterator->hashMap, value_integer, hash_iterator->currentKey.keyNumber);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == hash_iterator->whichTable)
	{
		LuaHashMap_InsertValueIntegerForKeyInteger(hash_iterator->hashMap, value_integer, hash_iterator->currentKey.keyInteger);
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
	
	if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueStringForKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueStringForKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueStringForKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueStringForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger);
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
	
	if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValuePointerForKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValuePointerForKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValuePointerForKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValuePointerForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger);
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
	
	if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueNumberForKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueNumberForKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueNumberForKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueNumberForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger);
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
	
	if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueIntegerForKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueIntegerForKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueIntegerForKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == hash_iterator->whichTable)
	{
        return LuaHashMap_GetValueIntegerForKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger);
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
	
	if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == hash_iterator->whichTable)
	{
		return LuaHashMap_ExistsKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == hash_iterator->whichTable)
	{
		return LuaHashMap_ExistsKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == hash_iterator->whichTable)
	{
		return LuaHashMap_ExistsKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == hash_iterator->whichTable)
	{
		return LuaHashMap_ExistsKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger);
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
	
	if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING == hash_iterator->whichTable)
	{
		LuaHashMap_RemoveKeyString(hash_iterator->hashMap, hash_iterator->currentKey.keyString);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER == hash_iterator->whichTable)
	{
		LuaHashMap_RemoveKeyPointer(hash_iterator->hashMap, hash_iterator->currentKey.keyPointer);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER == hash_iterator->whichTable)
	{
		LuaHashMap_RemoveKeyNumber(hash_iterator->hashMap, hash_iterator->currentKey.keyNumber);
	}
	else if(LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER == hash_iterator->whichTable)
	{
		LuaHashMap_RemoveKeyInteger(hash_iterator->hashMap, hash_iterator->currentKey.keyInteger);
	}
	else
	{
		/* shouldn't get here */
		LUAHASHMAP_ASSERT(false);
		return;
	}
}
