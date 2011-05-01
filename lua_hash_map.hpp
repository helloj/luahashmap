#ifndef CPP_LUA_HASH_MAP_H
#define CPP_LUA_HASH_MAP_H

extern "C" {
#include "LuaHashMap.h"
}
#include <utility>
namespace lhm
{

template<class _Key, class _Tp >
//	template<class _Key, class _Tp, class _Alloc = allocator<_Tp> >
class lua_hash_map
{
private:
	LuaHashMap* hashMap;

public:
	typedef _Key                                          key_type;
	typedef _Tp                                           mapped_type;
	typedef std::pair<const _Key, _Tp>                    value_type;
	
	lua_hash_map()
		: hashMap(NULL)
	{
		hashMap = LuaHashMap_Create();
	}

	~lua_hash_map()
	{
		LuaHashMap_Free(hashMap);
	}

	void clear()
	{
		LuaHashMap_Clear(hashMap);
	}

	bool empty() const
	{
		return LuaHashMap_IsEmpty(hashMap);
	}
	
	// FIXME: 
//	std::pair<iterator, bool>
	void insert(const value_type& key_value_pair) = 0;
	
	mapped_type& operator[](const key_type& __k) = 0;
	
};

/* This seems stupid, but it seems I must reimplement every single method 
 * for each partial specialization, even if I just want to reuse the base behavior.
 */
template<>
//	template<class _Key, class _Tp, class _Alloc = allocator<_Tp> >
class lua_hash_map<const char*, const char*>
{
private:
	LuaHashMap* hashMap;

public:
//	typedef _Key                                          const char*;
//	typedef _Tp                                           const char*;
	typedef std::pair<const char*, const char*> value_type;
//	typedef std::pair<const _Key, _Tp>                    value_type;
	
	lua_hash_map()
	: hashMap(NULL)
	{
		hashMap = LuaHashMap_Create();
	}
	
	~lua_hash_map()
	{
		LuaHashMap_Free(hashMap);
	}
	
	void clear()
	{
		LuaHashMap_Clear(hashMap);
	}
	
	bool empty() const
	{
		return LuaHashMap_IsEmpty(hashMap);
	}
	
	void insert(const value_type& key_value_pair)
	{
		LuaHashMap_InsertValueStringForKeyString(hashMap, key_value_pair.second, key_value_pair.first);
	}
	
	// This won't work right for assignment like foo[bar] = "fee";
	const char* operator[](const char* key_string)
	{
		const char* ret_val = LuaHashMap_GetValueStringForKeyString(hashMap, key_string);
		return ret_val;
	}

};
	
	
	

} /* end namespace */

#endif /* CPP_LUA_HASH_MAP_H */

