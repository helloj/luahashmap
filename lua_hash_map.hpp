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
protected:
	LuaHashMap* luaHashMap;

public:
	typedef std::pair<const char*, const char*> value_type;

	
	lua_hash_map()
	: luaHashMap(NULL)
	{
		luaHashMap = LuaHashMap_Create();
	}
	
	~lua_hash_map()
	{
		LuaHashMap_Free(luaHashMap);
	}
	
	void clear()
	{
		LuaHashMap_Clear(luaHashMap);
	}
	
	bool empty() const
	{
		return LuaHashMap_IsEmpty(luaHashMap);
	}
	
	size_t size() const
	{
		return LuaHashMap_GetKeysString(luaHashMap, NULL, 0);
	}
	
	void insert(const value_type& key_value_pair)
	{
		LuaHashMap_InsertValueStringForKeyString(luaHashMap, key_value_pair.second, key_value_pair.first);
	}
	
	size_t erase(const char* key)
	{
		if(true == LuaHashMap_ExistsKeyString(luaHashMap, key))
		{
			LuaHashMap_RemoveKeyString(luaHashMap, key);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	// This won't work right for assignment like foo[bar] = "fee";
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	const char* operator[](const char* key_string)
	{
		const char* ret_val = LuaHashMap_GetValueStringForKeyString(luaHashMap, key_string);
		return ret_val;
	}
#endif
	
	class iterator : public std::iterator<std::forward_iterator_tag, lua_hash_map<const char*, const char*> >
	{
		LuaHashMapIterator luaHashMapIterator;
		LuaHashMap* luaHashMap;
		friend class lua_hash_map;

		void set_begin()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtBeginForKeyString(luaHashMap);			
		}
		void set_end()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtEndForKeyString(luaHashMap);			
		}
		
		void set_current_key(const char* key)
		{
			luaHashMapIterator = LuaHashMap_GetIteratorForKeyString(luaHashMap, key);
		}
		
	public:
		iterator()
		: luaHashMap(NULL)
		{
			
		}
		
		iterator(LuaHashMap* lua_hash_map)
		: luaHashMap(lua_hash_map)
		{
			
		}
		
		std::pair<const char*, const char*> operator*()
		{
			// User needs to be very careful about the pointer to the strings
			return std::make_pair(luaHashMapIterator.currentKey.keyString, LuaHashMap_GetValueStringAtIterator(&luaHashMapIterator));
		}
		
		bool operator==(const iterator& the_other) const
		{
			return (true == LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		bool operator!=(const iterator& the_other) const
		{
			return (true != LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		const iterator& operator++()
		{
			LuaHashMap_IteratorNext(&this->luaHashMapIterator);
			return *this;
		}
	};
	
	
	iterator find(const char* key_string)
	{
		iterator the_iter(luaHashMap);
		the_iter.set_current_key(key_string);
		return the_iter;
	}
    
    iterator begin()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_begin();
		return the_iter;
    }
	iterator end()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_end();
		return the_iter;
    }
	
	size_t erase(iterator the_iterator)
	{
		return erase((*the_iterator).first);
	}

	
//	LuaHashMap* GetLuaHashMap() { return hashMap; }
	

};

/* This seems stupid, but it seems I must reimplement every single method 
 * for each partial specialization, even if I just want to reuse the base behavior.
 */
template<typename _TValue>
class lua_hash_map<const char*, _TValue*>
{
protected:
	LuaHashMap* luaHashMap;

public:
	typedef std::pair<const char*, _TValue*> value_type;
	
	lua_hash_map()
	: luaHashMap(NULL)
	{
		luaHashMap = LuaHashMap_Create();
	}
	
	~lua_hash_map()
	{
		LuaHashMap_Free(luaHashMap);
	}
	
	void clear()
	{
		LuaHashMap_Clear(luaHashMap);
	}
	
	bool empty() const
	{
		return LuaHashMap_IsEmpty(luaHashMap);
	}
	
	size_t size() const
	{
		return LuaHashMap_GetKeysString(luaHashMap, NULL, 0);
	}
	
	void insert(const value_type& key_value_pair)
	{
		LuaHashMap_InsertValuePointerForKeyString(luaHashMap, key_value_pair.second, key_value_pair.first);
	}
	
	size_t erase(const char* key)
	{
		if(true == LuaHashMap_ExistsKeyString(luaHashMap, key))
		{
			LuaHashMap_RemoveKeyString(luaHashMap, key);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	// This won't work right for assignment like foo[bar] = "fee";
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	_TValue* operator[](const char* key_string)
	{
		_TValue* ret_val = LuaHashMap_GetValuePointerForKeyString(luaHashMap, key_string);
		return ret_val;
	}
#endif
	
	class iterator : public std::iterator<std::forward_iterator_tag, lua_hash_map<const char*, _TValue*> >
	{
		LuaHashMapIterator luaHashMapIterator;
		LuaHashMap* luaHashMap;
		friend class lua_hash_map;

		void set_begin()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtBeginForKeyString(luaHashMap);			
		}
		void set_end()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtEndForKeyString(luaHashMap);			
		}
		
		void set_current_key(const char* key)
		{
			luaHashMapIterator = LuaHashMap_GetIteratorForKeyString(luaHashMap, key);
		}
		
	public:
		iterator()
		: luaHashMap(NULL)
		{
			
		}
		
		iterator(LuaHashMap* lua_hash_map)
		: luaHashMap(lua_hash_map)
		{
			
		}
		
		value_type operator*()
		{
			return std::make_pair(luaHashMapIterator.currentKey.keyString, static_cast<_TValue*>(LuaHashMap_GetValuePointerAtIterator(&luaHashMapIterator)));
		}
		
		bool operator==(const iterator& the_other) const
		{
			return (true == LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		bool operator!=(const iterator& the_other) const
		{
			return (true != LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		const iterator& operator++()
		{
			LuaHashMap_IteratorNext(&this->luaHashMapIterator);
			return *this;
		}
	};
	
	
	iterator find(const char* key_string)
	{
		iterator the_iter(luaHashMap);
		the_iter.set_current_key(key_string);
		return the_iter;
	}
    
    iterator begin()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_begin();
		return the_iter;
    }
	iterator end()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_end();
		return the_iter;
    }
	
	size_t erase(iterator the_iterator)
	{
		return erase((*the_iterator).first);
	}

};

	
/* This seems stupid, but it seems I must reimplement every single method 
 * for each partial specialization, even if I just want to reuse the base behavior.
 */
template<>
class lua_hash_map<const char*, lua_Number>
{
protected:
	LuaHashMap* luaHashMap;

public:
	typedef std::pair<const char*, lua_Number> value_type;
	
	lua_hash_map()
	: luaHashMap(NULL)
	{
		luaHashMap = LuaHashMap_Create();
	}
	
	~lua_hash_map()
	{
		LuaHashMap_Free(luaHashMap);
	}
	
	void clear()
	{
		LuaHashMap_Clear(luaHashMap);
	}
	
	bool empty() const
	{
		return LuaHashMap_IsEmpty(luaHashMap);
	}
	
	size_t size() const
	{
		return LuaHashMap_GetKeysString(luaHashMap, NULL, 0);
	}
	
	void insert(const value_type& key_value_pair)
	{
		LuaHashMap_InsertValueNumberForKeyString(luaHashMap, key_value_pair.second, key_value_pair.first);
	}
	
	size_t erase(const char* key)
	{
		if(true == LuaHashMap_ExistsKeyString(luaHashMap, key))
		{
			LuaHashMap_RemoveKeyString(luaHashMap, key);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	// This won't work right for assignment like foo[bar] = "fee";
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	lua_Number operator[](const char* key_string)
	{
		lua_Number ret_val = LuaHashMap_GetValueNumberForKeyString(luaHashMap, key_string);
		return ret_val;
	}
#endif
	
	class iterator : public std::iterator<std::forward_iterator_tag, lua_hash_map<const char*, lua_Number> >
	{
		LuaHashMapIterator luaHashMapIterator;
		LuaHashMap* luaHashMap;
		friend class lua_hash_map;

		void set_begin()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtBeginForKeyString(luaHashMap);			
		}
		void set_end()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtEndForKeyString(luaHashMap);			
		}
		
		void set_current_key(const char* key)
		{
			luaHashMapIterator = LuaHashMap_GetIteratorForKeyString(luaHashMap, key);
		}
		
	public:
		iterator()
		: luaHashMap(NULL)
		{
			
		}
		
		iterator(LuaHashMap* lua_hash_map)
		: luaHashMap(lua_hash_map)
		{
			
		}
		
		std::pair<const char*, lua_Number> operator*()
		{
			return std::make_pair(luaHashMapIterator.currentKey.keyString, LuaHashMap_GetValueNumberAtIterator(&luaHashMapIterator));
		}
		
		bool operator==(const iterator& the_other) const
		{
			return (true == LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		bool operator!=(const iterator& the_other) const
		{
			return (true != LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		const iterator& operator++()
		{
			LuaHashMap_IteratorNext(&this->luaHashMapIterator);
			return *this;
		}
	};
	
	
	iterator find(const char* key_string)
	{
		iterator the_iter(luaHashMap);
		the_iter.set_current_key(key_string);
		return the_iter;
	}
    
    iterator begin()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_begin();
		return the_iter;
    }
	iterator end()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_end();
		return the_iter;
    }
	
	size_t erase(iterator the_iterator)
	{
		return erase((*the_iterator).first);
	}

};

/* This seems stupid, but it seems I must reimplement every single method 
 * for each partial specialization, even if I just want to reuse the base behavior.
 */
template<>
class lua_hash_map<const char*, lua_Integer>
{
protected:
	LuaHashMap* luaHashMap;

public:
	typedef std::pair<const char*, lua_Integer> value_type;
	
	lua_hash_map()
	: luaHashMap(NULL)
	{
		luaHashMap = LuaHashMap_Create();
	}
	
	~lua_hash_map()
	{
		LuaHashMap_Free(luaHashMap);
	}
	
	void clear()
	{
		LuaHashMap_Clear(luaHashMap);
	}
	
	bool empty() const
	{
		return LuaHashMap_IsEmpty(luaHashMap);
	}
	
	size_t size() const
	{
		return LuaHashMap_GetKeysString(luaHashMap, NULL, 0);
	}
	
	void insert(const value_type& key_value_pair)
	{
		LuaHashMap_InsertValueIntegerForKeyString(luaHashMap, key_value_pair.second, key_value_pair.first);
	}
	
	size_t erase(const char* key)
	{
		if(true == LuaHashMap_ExistsKeyString(luaHashMap, key))
		{
			LuaHashMap_RemoveKeyString(luaHashMap, key);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	// This won't work right for assignment like foo[bar] = "fee";
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	lua_Integer operator[](const char* key_string)
	{
		lua_Integer ret_val = LuaHashMap_GetValueIntegerForKeyString(luaHashMap, key_string);
		return ret_val;
	}
#endif
	
	class iterator : public std::iterator<std::forward_iterator_tag, lua_hash_map<const char*, lua_Integer> >
	{
		LuaHashMapIterator luaHashMapIterator;
		LuaHashMap* luaHashMap;
		friend class lua_hash_map;

		void set_begin()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtBeginForKeyString(luaHashMap);			
		}
		void set_end()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtEndForKeyString(luaHashMap);			
		}
		
		void set_current_key(const char* key)
		{
			luaHashMapIterator = LuaHashMap_GetIteratorForKeyString(luaHashMap, key);
		}
		
	public:
		iterator()
		: luaHashMap(NULL)
		{
			
		}
		
		iterator(LuaHashMap* lua_hash_map)
		: luaHashMap(lua_hash_map)
		{
			
		}
		
		std::pair<const char*, lua_Integer> operator*()
		{
			return std::make_pair(luaHashMapIterator.currentKey.keyString, LuaHashMap_GetValueIntegerAtIterator(&luaHashMapIterator));
		}
		
		bool operator==(const iterator& the_other) const
		{
			return (true == LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		bool operator!=(const iterator& the_other) const
		{
			return (true != LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		const iterator& operator++()
		{
			LuaHashMap_IteratorNext(&this->luaHashMapIterator);
			return *this;
		}
	};
	
	
	iterator find(const char* key_string)
	{
		iterator the_iter(luaHashMap);
		the_iter.set_current_key(key_string);
		return the_iter;
	}
    
    iterator begin()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_begin();
		return the_iter;
    }
	iterator end()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_end();
		return the_iter;
    }
	
	size_t erase(iterator the_iterator)
	{
		return erase((*the_iterator).first);
	}

};



/* This seems stupid, but it seems I must reimplement every single method 
 * for each partial specialization, even if I just want to reuse the base behavior.
 */
template<typename _TKey, typename _TValue>
class lua_hash_map<_TKey*, _TValue*>
{
protected:
	LuaHashMap* luaHashMap;

public:
	typedef std::pair<_TKey*, _TValue*> value_type;
	
	lua_hash_map()
	: luaHashMap(NULL)
	{
		luaHashMap = LuaHashMap_Create();
	}
	
	~lua_hash_map()
	{
		LuaHashMap_Free(luaHashMap);
	}
	
	void clear()
	{
		LuaHashMap_Clear(luaHashMap);
	}
	
	bool empty() const
	{
		return LuaHashMap_IsEmpty(luaHashMap);
	}
	
	size_t size() const
	{
		return LuaHashMap_GetKeysPointer(luaHashMap, NULL, 0);
	}
	
	void insert(const value_type& key_value_pair)
	{
		LuaHashMap_InsertValuePointerForKeyPointer(luaHashMap, key_value_pair.second, key_value_pair.first);
	}
	
	size_t erase(_TKey* key)
	{
		if(true == LuaHashMap_ExistsKeyPointer(luaHashMap, key))
		{
			LuaHashMap_RemoveKeyPointer(luaHashMap, key);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	// This won't work right for assignment like foo[bar] = "fee";
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	_TValue* operator[](_TKey* key)
	{
		_TValue* ret_val = LuaHashMap_GetValuePointerForKeyPointer(luaHashMap, key);
		return ret_val;
	}
#endif
	
	class iterator : public std::iterator<std::forward_iterator_tag, lua_hash_map<_TKey*, _TValue*> >
	{
		LuaHashMapIterator luaHashMapIterator;
		LuaHashMap* luaHashMap;
		friend class lua_hash_map;

		void set_begin()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtBeginForKeyPointer(luaHashMap);			
		}
		void set_end()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtEndForKeyPointer(luaHashMap);			
		}
		
		void set_current_key(_TKey* key)
		{
			luaHashMapIterator = LuaHashMap_GetIteratorForKeyPointer(luaHashMap, key);
		}
		
	public:
		iterator()
		: luaHashMap(NULL)
		{
			
		}
		
		iterator(LuaHashMap* lua_hash_map)
		: luaHashMap(lua_hash_map)
		{
			
		}
		
		value_type operator*()
		{
			return std::make_pair(static_cast<_TKey*>(luaHashMapIterator.currentKey.keyPointer), static_cast<_TValue*>(LuaHashMap_GetValuePointerAtIterator(&luaHashMapIterator)));
		}
		
		bool operator==(const iterator& the_other) const
		{
			return (true == LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		bool operator!=(const iterator& the_other) const
		{
			return (true != LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		const iterator& operator++()
		{
			LuaHashMap_IteratorNext(&this->luaHashMapIterator);
			return *this;
		}
	};
	
	
	iterator find(_TKey* key)
	{
		iterator the_iter(luaHashMap);
		the_iter.set_current_key(key);
		return the_iter;
	}
    
    iterator begin()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_begin();
		return the_iter;
    }
	iterator end()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_end();
		return the_iter;
    }
	
	size_t erase(iterator the_iterator)
	{
		return erase((*the_iterator).first);
	}

};


    


/* This seems stupid, but it seems I must reimplement every single method 
 * for each partial specialization, even if I just want to reuse the base behavior.
 */
template<typename _TKey>
class lua_hash_map<_TKey*, const char*>
{
protected:
	LuaHashMap* luaHashMap;

public:
	typedef const char _TValue;
    typedef std::pair<_TKey*, _TValue*> value_type;

	
	lua_hash_map()
	: luaHashMap(NULL)
	{
		luaHashMap = LuaHashMap_Create();
	}
	
	~lua_hash_map()
	{
		LuaHashMap_Free(luaHashMap);
	}
	
	void clear()
	{
		LuaHashMap_Clear(luaHashMap);
	}
	
	bool empty() const
	{
		return LuaHashMap_IsEmpty(luaHashMap);
	}
	
	size_t size() const
	{
		return LuaHashMap_GetKeysPointer(luaHashMap, NULL, 0);
	}
	
	void insert(const value_type& key_value_pair)
	{
		LuaHashMap_InsertValueStringForKeyPointer(luaHashMap, key_value_pair.second, key_value_pair.first);
	}
	
	size_t erase(_TKey* key)
	{
		if(true == LuaHashMap_ExistsKeyPointer(luaHashMap, key))
		{
			LuaHashMap_RemoveKeyPointer(luaHashMap, key);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	// This won't work right for assignment like foo[bar] = "fee";
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	_TValue* operator[](_TKey* key)
	{
		_TValue* ret_val = LuaHashMap_GetValueStringForKeyPointer(luaHashMap, key);
		return ret_val;
	}
#endif
	
	class iterator : public std::iterator<std::forward_iterator_tag, lua_hash_map<_TKey*, _TValue*> >
	{
		LuaHashMapIterator luaHashMapIterator;
		LuaHashMap* luaHashMap;
		friend class lua_hash_map;

		void set_begin()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtBeginForKeyPointer(luaHashMap);			
		}
		void set_end()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtEndForKeyPointer(luaHashMap);			
		}
		
		void set_current_key(_TKey* key)
		{
			luaHashMapIterator = LuaHashMap_GetIteratorForKeyPointer(luaHashMap, key);
		}
		
	public:
		iterator()
		: luaHashMap(NULL)
		{
			
		}
		
		iterator(LuaHashMap* lua_hash_map)
		: luaHashMap(lua_hash_map)
		{
			
		}
		
		value_type operator*()
		{
			return std::make_pair(static_cast<_TKey*>(luaHashMapIterator.currentKey.keyPointer), static_cast<_TValue*>(LuaHashMap_GetValueStringAtIterator(&luaHashMapIterator)));
		}
		
		bool operator==(const iterator& the_other) const
		{
			return (true == LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		bool operator!=(const iterator& the_other) const
		{
			return (true != LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		const iterator& operator++()
		{
			LuaHashMap_IteratorNext(&this->luaHashMapIterator);
			return *this;
		}
	};
	
	
	iterator find(_TKey* key)
	{
		iterator the_iter(luaHashMap);
		the_iter.set_current_key(key);
		return the_iter;
	}
    
    iterator begin()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_begin();
		return the_iter;
    }
	iterator end()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_end();
		return the_iter;
    }
	
	size_t erase(iterator the_iterator)
	{
		return erase((*the_iterator).first);
	}

};
	

/* This seems stupid, but it seems I must reimplement every single method 
 * for each partial specialization, even if I just want to reuse the base behavior.
 */
template<typename _TKey>
class lua_hash_map<_TKey*, lua_Number>
{
protected:
	LuaHashMap* luaHashMap;

public:
	typedef lua_Number _TValue;
    typedef std::pair<_TKey*, _TValue> value_type;

	
	lua_hash_map()
	: luaHashMap(NULL)
	{
		luaHashMap = LuaHashMap_Create();
	}
	
	~lua_hash_map()
	{
		LuaHashMap_Free(luaHashMap);
	}
	
	void clear()
	{
		LuaHashMap_Clear(luaHashMap);
	}
	
	bool empty() const
	{
		return LuaHashMap_IsEmpty(luaHashMap);
	}
	
	size_t size() const
	{
		return LuaHashMap_GetKeysPointer(luaHashMap, NULL, 0);
	}
	
	void insert(const value_type& key_value_pair)
	{
		LuaHashMap_InsertValueNumberForKeyPointer(luaHashMap, key_value_pair.second, key_value_pair.first);
	}
	
	size_t erase(_TKey* key)
	{
		if(true == LuaHashMap_ExistsKeyPointer(luaHashMap, key))
		{
			LuaHashMap_RemoveKeyPointer(luaHashMap, key);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	// This won't work right for assignment like foo[bar] = "fee";
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	_TValue operator[](_TKey* key)
	{
		_TValue ret_val = LuaHashMap_GetValueNumberForKeyPointer(luaHashMap, key);
		return ret_val;
	}
#endif
	
	class iterator : public std::iterator<std::forward_iterator_tag, lua_hash_map<_TKey*, _TValue*> >
	{
		LuaHashMapIterator luaHashMapIterator;
		LuaHashMap* luaHashMap;
		friend class lua_hash_map;

		void set_begin()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtBeginForKeyPointer(luaHashMap);			
		}
		void set_end()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtEndForKeyPointer(luaHashMap);			
		}
		
		void set_current_key(_TKey* key)
		{
			luaHashMapIterator = LuaHashMap_GetIteratorForKeyPointer(luaHashMap, key);
		}
		
	public:
		iterator()
		: luaHashMap(NULL)
		{
			
		}
		
		iterator(LuaHashMap* lua_hash_map)
		: luaHashMap(lua_hash_map)
		{
			
		}
		
		value_type operator*()
		{
			return std::make_pair(static_cast<_TKey*>(luaHashMapIterator.currentKey.keyPointer), static_cast<_TValue>(LuaHashMap_GetValueNumberAtIterator(&luaHashMapIterator)));
		}
		
		bool operator==(const iterator& the_other) const
		{
			return (true == LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		bool operator!=(const iterator& the_other) const
		{
			return (true != LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		const iterator& operator++()
		{
			LuaHashMap_IteratorNext(&this->luaHashMapIterator);
			return *this;
		}
	};
	
	
	iterator find(_TKey* key)
	{
		iterator the_iter(luaHashMap);
		the_iter.set_current_key(key);
		return the_iter;
	}
    
    iterator begin()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_begin();
		return the_iter;
    }
	iterator end()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_end();
		return the_iter;
    }
	
	size_t erase(iterator the_iterator)
	{
		return erase((*the_iterator).first);
	}

};

/* This seems stupid, but it seems I must reimplement every single method 
 * for each partial specialization, even if I just want to reuse the base behavior.
 */
template<typename _TKey>
class lua_hash_map<_TKey*, lua_Integer>
{
protected:
	LuaHashMap* luaHashMap;

public:
	typedef lua_Integer _TValue;
    typedef std::pair<_TKey*, _TValue> value_type;

	
	lua_hash_map()
	: luaHashMap(NULL)
	{
		luaHashMap = LuaHashMap_Create();
	}
	
	~lua_hash_map()
	{
		LuaHashMap_Free(luaHashMap);
	}
	
	void clear()
	{
		LuaHashMap_Clear(luaHashMap);
	}
	
	bool empty() const
	{
		return LuaHashMap_IsEmpty(luaHashMap);
	}
	
	size_t size() const
	{
		return LuaHashMap_GetKeysPointer(luaHashMap, NULL, 0);
	}
	
	void insert(const value_type& key_value_pair)
	{
		LuaHashMap_InsertValueIntegerForKeyPointer(luaHashMap, key_value_pair.second, key_value_pair.first);
	}
	
	size_t erase(_TKey* key)
	{
		if(true == LuaHashMap_ExistsKeyPointer(luaHashMap, key))
		{
			LuaHashMap_RemoveKeyPointer(luaHashMap, key);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	// This won't work right for assignment like foo[bar] = "fee";
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	_TValue operator[](_TKey* key)
	{
		_TValue ret_val = LuaHashMap_GetValueIntegerForKeyPointer(luaHashMap, key);
		return ret_val;
	}
#endif
	
	class iterator : public std::iterator<std::forward_iterator_tag, lua_hash_map<_TKey*, _TValue*> >
	{
		LuaHashMapIterator luaHashMapIterator;
		LuaHashMap* luaHashMap;
		friend class lua_hash_map;

		void set_begin()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtBeginForKeyPointer(luaHashMap);			
		}
		void set_end()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtEndForKeyPointer(luaHashMap);			
		}
		
		void set_current_key(_TKey* key)
		{
			luaHashMapIterator = LuaHashMap_GetIteratorForKeyPointer(luaHashMap, key);
		}
		
	public:
		iterator()
		: luaHashMap(NULL)
		{
			
		}
		
		iterator(LuaHashMap* lua_hash_map)
		: luaHashMap(lua_hash_map)
		{
			
		}
		
		value_type operator*()
		{
			return std::make_pair(static_cast<_TKey*>(luaHashMapIterator.currentKey.keyPointer), static_cast<_TValue>(LuaHashMap_GetValueIntegerAtIterator(&luaHashMapIterator)));
		}
		
		bool operator==(const iterator& the_other) const
		{
			return (true == LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		bool operator!=(const iterator& the_other) const
		{
			return (true != LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		const iterator& operator++()
		{
			LuaHashMap_IteratorNext(&this->luaHashMapIterator);
			return *this;
		}
	};
	
	
	iterator find(_TKey* key)
	{
		iterator the_iter(luaHashMap);
		the_iter.set_current_key(key);
		return the_iter;
	}
    
    iterator begin()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_begin();
		return the_iter;
    }
	iterator end()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_end();
		return the_iter;
    }
	
	size_t erase(iterator the_iterator)
	{
		return erase((*the_iterator).first);
	}

};
	


    
    
    
    
    
    

/* This seems stupid, but it seems I must reimplement every single method 
 * for each partial specialization, even if I just want to reuse the base behavior.
 */
template<typename _TValue>
class lua_hash_map<lua_Integer, _TValue*>
{
protected:
	LuaHashMap* luaHashMap;

public:
	typedef lua_Integer _TKey;
	typedef std::pair<_TKey, _TValue*> value_type;
	
	lua_hash_map()
	: luaHashMap(NULL)
	{
		luaHashMap = LuaHashMap_Create();
	}
	
	~lua_hash_map()
	{
		LuaHashMap_Free(luaHashMap);
	}
	
	void clear()
	{
		LuaHashMap_Clear(luaHashMap);
	}
	
	bool empty() const
	{
		return LuaHashMap_IsEmpty(luaHashMap);
	}
	
	size_t size() const
	{
		return LuaHashMap_GetKeysInteger(luaHashMap, NULL, 0);
	}
	
	void insert(const value_type& key_value_pair)
	{
		LuaHashMap_InsertValuePointerForKeyInteger(luaHashMap, key_value_pair.second, key_value_pair.first);
	}
	
	size_t erase(_TKey key)
	{
		if(true == LuaHashMap_ExistsKeyInteger(luaHashMap, key))
		{
			LuaHashMap_RemoveKeyInteger(luaHashMap, key);
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	// This won't work right for assignment like foo[bar] = "fee";
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	_TValue* operator[](_TKey key)
	{
		_TValue* ret_val = LuaHashMap_GetValuePointerFoKeyInteger(luaHashMap, key);
		return ret_val;
	}
#endif
	
	class iterator : public std::iterator<std::forward_iterator_tag, lua_hash_map<_TKey, _TValue*> >
	{
		LuaHashMapIterator luaHashMapIterator;
		LuaHashMap* luaHashMap;
		friend class lua_hash_map;

		void set_begin()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtBeginForKeyInteger(luaHashMap);			
		}
		void set_end()
		{
			luaHashMapIterator = LuaHashMap_GetIteratorAtEndForKeyInteger(luaHashMap);			
		}
		void set_current_key(_TKey key)
		{
			luaHashMapIterator = LuaHashMap_GetIteratorForKeyInteger(luaHashMap, key);
		}
		
	public:
		iterator()
		: luaHashMap(NULL)
		{
			
		}
		
		iterator(LuaHashMap* lua_hash_map)
		: luaHashMap(lua_hash_map)
		{
			
		}
		
		value_type operator*()
		{
			return std::make_pair(static_cast<_TKey>(luaHashMapIterator.currentKey.keyInteger), static_cast<_TValue*>(LuaHashMap_GetValuePointerAtIterator(&luaHashMapIterator)));
		}
		
		bool operator==(const iterator& the_other) const
		{
			return (true == LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		bool operator!=(const iterator& the_other) const
		{
			return (true != LuaHashMap_IteratorIsEqual(&this->luaHashMapIterator, &(the_other.luaHashMapIterator)));
		}
		
		const iterator& operator++()
		{
			LuaHashMap_IteratorNext(&this->luaHashMapIterator);
			return *this;
		}
	};
	
	
	iterator find(_TKey key)
	{
		iterator the_iter(luaHashMap);
		the_iter.set_current_key(key);
		return the_iter;
	}
    
    iterator begin()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_begin();
		return the_iter;
    }
	iterator end()
    {
		iterator the_iter(luaHashMap);
		the_iter.set_end();
		return the_iter;
    }
	
	size_t erase(iterator the_iterator)
	{
		return erase((*the_iterator).first);
	}

};



    

} /* end namespace */

#endif /* CPP_LUA_HASH_MAP_H */

