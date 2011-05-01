#include "LuaHashMap.h"
#include "lua.h"
#include "lauxlib.h"
#include <stdlib.h>

#include <assert.h>

//static const char* LUAHASHMAP_DEFAULT_TABLE_NAME = "lhm";
static const char* LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING = "lhm.string";
static const char* LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER = "lhm.number";
static const char* LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER = "lhm.integer";
static const char* LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER = "lhm.pointer";

struct LuaHashMap
{
	lua_State* luaState;
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
	 */
	lua_newtable(hash_map->luaState);
	lua_setglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING);
	lua_newtable(hash_map->luaState);
	lua_setglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYNUMBER);
	lua_newtable(hash_map->luaState);
	lua_setglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYINTEGER);
	lua_newtable(hash_map->luaState);
	lua_setglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER);
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

	assert(lua_gettop(hash_map->luaState) == 0);
	return hash_map;
}

void LuaHashMap_Free(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return;
	}
	free(hash_map->luaState);
	free(hash_map);
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

	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
	lua_pushstring(hash_map->luaState, value_string); /* stack: [value_string, table] */
	lua_setfield(hash_map->luaState, -2, key_string); /* table[key_string]=value_string; stack: [table] */
	
	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	assert(lua_gettop(hash_map->luaState) == 0);
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

	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, table] */
	lua_setfield(hash_map->luaState, -2, key_string);  /* table[key_string]=value_pointer; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	assert(lua_gettop(hash_map->luaState) == 0);	
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

	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushlightuserdata(hash_map->luaState, value_pointer); /* stack: [value_pointer, key_pointer, table] */
	lua_settable(hash_map->luaState, -3);  /* table[key_pointer]=value_pointer; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	assert(lua_gettop(hash_map->luaState) == 0);
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

	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
	lua_getfield(hash_map->luaState, -1, key_string); /* table[key_string]; stack: [value_string, table] */
	ret_val = lua_tostring(hash_map->luaState, -1);

//	fprintf(stderr, "ret_val=%s, %d\n", ret_val, lua_gettop(hash_map->luaState));

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	assert(lua_gettop(hash_map->luaState) == 0);	
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

	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
	lua_getfield(hash_map->luaState, -1, key_string); /* table[key_string]; stack: [value_string, table] */
	ret_val = lua_touserdata(hash_map->luaState, -1);

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	assert(lua_gettop(hash_map->luaState) == 0);	
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

	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_gettable(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */
	ret_val = lua_touserdata(hash_map->luaState, -1);

	/* return value and table are still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 2);
	assert(lua_gettop(hash_map->luaState) == 0);
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

	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
	lua_pushnil(hash_map->luaState); /* stack: [nil, table] */
	lua_setfield(hash_map->luaState, -2, key_string);  /* table[key_string]=nil ; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	assert(lua_gettop(hash_map->luaState) == 0);	
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

	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_pushnil(hash_map->luaState); /* stack: [nil, key_pointer, table] */
	lua_settable(hash_map->luaState, -3);  /* table[key_pointer]=nil; stack: [table] */

	/* table is still on top of stack. Don't forget to pop it now that we are done with it */
	lua_pop(hash_map->luaState, 1);
	assert(lua_gettop(hash_map->luaState) == 0);
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

	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */
	lua_getfield(hash_map->luaState, -1, key_string); /* table[key_string]; stack: [value_string, table] */

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
	assert(lua_gettop(hash_map->luaState) == 0);	
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


	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */
	lua_pushlightuserdata(hash_map->luaState, key_pointer); /* stack: [key_pointer, table] */
	lua_gettable(hash_map->luaState, -2);  /* table[key_pointer]; stack: [value_pointer, table] */

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
	assert(lua_gettop(hash_map->luaState) == 0);
	return ret_val;
}

size_t LuaHashMap_GetKeysString(LuaHashMap* hash_map, const char* keys_array[], size_t max_array_size)
{
	size_t total_count = 0;
	if(NULL == hash_map)
	{
		return 0;
	}

	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYSTRING); /* stack: [table] */

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
	assert(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}

size_t LuaHashMap_GetKeysPointer(LuaHashMap* hash_map, void* keys_array[], size_t max_array_size)
{
	size_t total_count = 0;
	if(NULL == hash_map)
	{
		return 0;
	}

	lua_getglobal(hash_map->luaState, LUAHASHMAP_DEFAULT_TABLE_NAME_KEYPOINTER); /* stack: [table] */

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
	assert(lua_gettop(hash_map->luaState) == 0);	
	return total_count;
}

void LuaHashMap_Clear(LuaHashMap* hash_map)
{
	if(NULL == hash_map)
	{
		return;
	}
	Internal_InitializeInternalTables(hash_map);
	assert(lua_gettop(hash_map->luaState) == 0);	
}

static bool Internal_IsEmpty(LuaHashMap* hash_map, const char* table_name)
{
	bool is_empty;
	lua_getglobal(hash_map->luaState, table_name); /* stack: [table] */

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

	assert(lua_gettop(hash_map->luaState) == 0);		

	return is_empty;
}


