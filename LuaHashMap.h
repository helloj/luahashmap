#ifndef C_LUA_HASH_MAP_H
#define C_LUA_HASH_MAP_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct LuaHashMap LuaHashMap;
/*
typedef double LuaHashMapNumber;
typedef int LuaHashMapInteger;
*/
#include <stdbool.h>
#include <stddef.h>

LuaHashMap* LuaHashMap_Create(void);
/* Note: If created with an external Lua state, it will not delete the underlying Lua state. */
void LuaHashMap_Free(LuaHashMap* hash_map);

//LuaHashMap* LuaHashMap_CreateWithLuaState(struct lua_State* lua_state);

//LuaHashMap* LuaHashMap_CreateInNamespace(const char* name_space);
//LuaHashMap* LuaHashMap_CreateWithLuaStateInNamespace(struct lua_State* lua_state, const char* restrict name_space);
//LuaHashMap* LuaHashMap_Free(LuaHashMap* hash_map);

//struct lua_State* LuaHashMap_GetLuaState(void);

/* string, string */
void LuaHashMap_InsertValueStringForKeyString(LuaHashMap* hash_map, const char* restrict value_string, const char* restrict key_string);



/* string, number */
//void LuaHashMap_InsertKeyStringValueNumber(LuaHashMap* hash_map, const char* restrict key_string, LuaHashMapNumber value_number); 

/* number, number */
//void LuaHashMap_InsertKeyNumberValueNumber(LuaHashMap* hash_map, LuaHashMapNumber key_number, LuaHashMapNumber value_number); 

/* number, string */
//void LuaHashMap_InsertKeyNumberValueString(LuaHashMap* hash_map, LuaHashMapNumber key_number, const char* restrict value_string); 

/* pointer, pointer */
void LuaHashMap_InsertValuePointerForKeyPointer(LuaHashMap* hash_map, void* value_pointer, void* key_pointer);

/* pointer, string */
/* string, pointer */
void LuaHashMap_InsertValuePointerForKeyString(LuaHashMap* hash_map, void* value_pointer, const char* restrict key_string);


/* pointer, number */
/* number, pointer */

/* string, integer */
/* integer, string */
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


/* Remove functions */
void LuaHashMap_RemoveKeyString(LuaHashMap* hash_map, const char* restrict key_string);
void LuaHashMap_RemoveKeyPointer(LuaHashMap* hash_map, void* key_pointer);


/* Clear List */

/* List Functions */
size_t LuaHashMap_KeysString(LuaHashMap* hash_map, const char* keys_array[], size_t max_array_size);
size_t LuaHashMap_KeysPointer(LuaHashMap* hash_map, void* keys_array[], size_t max_array_size);

#ifdef __cplusplus
}
#endif



#endif /* C_LUA_HASH_MAP_H */

