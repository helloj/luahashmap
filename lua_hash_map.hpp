/*
 lua_hash_map - An STL-like adator for LuaHashMap
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

#ifndef CPP_LUA_HASH_MAP_H
#define CPP_LUA_HASH_MAP_H

extern "C" {
#include "LuaHashMap.h"
}
#include <utility>
#include <iterator>

namespace lhm
{
template<class _Key, class _Tp >
	class lua_hash_map;
	
template<class Container_>
class luahashmap_iterator : public std::iterator<std::forward_iterator_tag, Container_>
{
protected:
//	friend class Container_;
	Container_& hashMap;
    LuaHashMapIterator luaHashMapIterator;
	const char* currentKeyString;
	typedef std::pair<const char*, const char*> value_type;

public:
	explicit luahashmap_iterator(Container_& hash_map)
		: hashMap(hash_map),
	currentKeyString(NULL)
	{
	}
	
	~luahashmap_iterator()
	{
	}
	
	// The assignment and relational operators are straightforward
	luahashmap_iterator& operator=(const luahashmap_iterator& value)
	{
		hashMap.insert(value);
		return (*this);
	}
	
//	luahashmap_iterator<Container_>& operator*()
	std::pair<const char*, const char*> operator*()
	{
//		std::pair<const char*, const char*> value_type
//		LuaHashMap* luahash = hashMap.GetLuaHashMap();
        
        // User needs to be very careful about the pointer to the strings
		return std::make_pair(luaHashMapIterator.currentKey.keyString, LuaHashMap_GetValueStringAtIterator(&luaHashMapIterator));
//		value_type ret_val;
//		return *this;
	}
	void setCurrentKey(const char* key)
	{
        luaHashMapIterator = LuaHashMap_GetIteratorForKeyString(hashMap.GetLuaHashMap(), key);

	}
/*	
	bool operator==(const Iterator& other)
	{
		return(node_ == other.node_);
	}
	
	bool operator!=(const Iterator& other)
	{
		return(node_ != other.node_);
	}
	
	//public: T& operator*();
	const mylist_iterator<T>& operator++();
	bool operator!=(const mylist_iterator<T>& other) const;
	
private: mylist_node<T> *pointee; mylist_iterator(mylist_node<T> *pointee) : pointee(pointee) {}
template <typename T> T& mylist_iterator<T>::operator*() {
return pointee->elem;
}
template <typename T> const mylist_iterator<T>& mylist_iterator<T>::operator++() {
assert(pointee != NULL); pointee = pointee->next; return *this;
}
template <typename T> bool mylist_iterator<T>:: operator!=(const mylist_iterator<T>& other) const {
return this->pointee != other.pointee;
*/ 
};
	
template<class _Key, class _Tp >
//	template<class _Key, class _Tp, class _Alloc = allocator<_Tp> >
class lua_hash_map
{
private:
	LuaHashMap* hashMap;
	const char* currentKeyString;

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
	
	luahashmap_iterator<lua_hash_map> find(const char* key_string)
	{
	//	const char* ret_val = LuaHashMap_GetValueStringForKeyString(hashMap, key_string);
		luahashmap_iterator<lua_hash_map> iter(*this);
		iter.setCurrentKey(key_string);
		return iter;
	}
	
	LuaHashMap* GetLuaHashMap() { return hashMap; }
	
//	luahashmap_iterator<Container_>& operator*()


};
	
	
	

} /* end namespace */

#endif /* CPP_LUA_HASH_MAP_H */

