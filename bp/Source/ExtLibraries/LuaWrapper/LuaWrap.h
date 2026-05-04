/******************************************************************************
* Name: LuaWrapper for C++
*
* Author: Yong Lin
* Date: 2004-07-19 17:15
* E-Mail: ly4cn@21cn.com
*
* Copyright (C) 2004 Yong Lin.  All rights reserved.
* HomePage：第二人生 http://www.d2-life.com
*　　　http://www.d2-life.com/LBS/blogview.asp?logID=41
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#pragma once

#include <stdlib.h>
#include <string>

#include "Support_Lua.h"
#include "luaStack.h"
#include ".\luafunction.h"


class LuaWrap
	: public luaStack
{
public:
	LuaWrap()
		: luaStack(lua_open())
	{
		// initialize lua standard library functions
      ::luaopen_base(m_pluaVM);
      ::luaopen_table(m_pluaVM);
		// don't give access to file routines like execute/delete etc.
      //lua_iolibopen(m_pluaVM);
      ::luaopen_string(m_pluaVM);
      ::luaopen_math(m_pluaVM);
      ::luaopen_debug(m_pluaVM);
	}

	~LuaWrap()
	{
		if(m_pluaVM)
			lua_close(m_pluaVM);
	}
public:
	bool LoadFile(const char* filename)
	{
		return lua_dofile(m_pluaVM, filename) == 0;
	}
	bool LoadString(const char* buffer)
	{
		return LoadBuffer(buffer, strlen(buffer));
	}
	bool LoadBuffer(const char* buffer, size_t size)
	{
		return lua_dobuffer(m_pluaVM, buffer, size, "LuaWrap") == 0;
	}

	void Register(const char* func, lua_CFunction f)
	{
		lua_register(m_pluaVM, func, f);
	}

	template<class LUATYPE>
	void Register()
	{
		LUATYPE::LuaOpenLibMember(m_pluaVM);
		LUATYPE::LuaOpenLib(m_pluaVM);
	}

	//operator lua_State* ()
	//{
	//	return m_pluaVM;
	//}
};

template<class T>
int LuaNewObject (lua_State *L) 
{
	T* obj = new T;

	T** pobj = static_cast<T**>( lua_newuserdata((L), sizeof(T*)) );
	*pobj = obj;

	luaL_getmetatable(L, T::GetLuaTypeName());
	lua_setmetatable(L, -2);

	return 1;  /* new userdatum is already on the stack */
}

template<class T>
int LuaDelObject (lua_State *L) 
{
	T** pobj = static_cast<T**>( luaL_checkudata(L, -1, T::GetLuaTypeName()) );
	luaL_argcheck(L, pobj != NULL, -1, "Ojbect type missing");

	delete *pobj;
	*pobj = NULL;

	return 0;
}

template<const char* GetLuaTypeName()>
int Lua_ImplementIndex (lua_State *L)	
{	
	int narg = lua_gettop(L);
	if(lua_isstring(L, -1))
	{
		const char* index = lua_tostring(L, -1);	
		luaL_getmetatable(L, GetLuaTypeName());	
		lua_pushstring(L, index);	
		lua_gettable(L, -2);	
		lua_remove(L, -2);
		if(! lua_isnil(L, -1) )
			return 1;
		lua_settop(L, -2);	
	}
	int narg2 = lua_gettop(L);

	luaL_getmetatable(L, GetLuaTypeName());	
	lua_pushstring(L, "__getindex");	
	lua_gettable(L, -2);	
	lua_remove(L, -2);
	if(! lua_isnil(L, -1) )
	{
		for(int i = 1; i <= narg; ++i)
			lua_pushvalue(L, i);
		lua_pcall(L, narg, LUA_MULTRET, 0);
		int nnewtop = lua_gettop(L);
		return lua_gettop(L) - narg;
	}
	return 1;
}


#define DEFINE_TYPENAME(type_name)	\
inline static const char* GetLuaTypeName(){return (type_name);}

#define BEGIN_REGLUALIB(lib_name)	\
static void LuaOpenLib(lua_State* L)	\
{	\
	const char* __libname = (lib_name);	\
	static const struct luaL_reg table[] = { {NULL, NULL} };	\
	luaL_openlib(L, __libname, table, 0);	

#define LUALIB_ITEM(funcname, func)	\
	lua_pushstring(L, (funcname));	\
	lua_pushcclosure(L, (func), 0);	\
	lua_settable(L, -3);

#define LUALIB_ITEM_CREATE(funcname, classname)	\
	LUALIB_ITEM(funcname, LuaNewObject<classname>)

#define LUALIB_ITEM_DESTROY(funcname, classname)	\
	LUALIB_ITEM(funcname, LuaDelObject<classname>)

// 此处用了一个 Microsoft 的扩展常量 __COUNTER__， 目的是为了能在编译期得到每次不同的常量序列
// 非Microsoft的编译器，可尝试用 __LINE__ （似乎不够，因可能在不同文件中__LINE__会相同）

#ifndef __COUNTER__

#define RegisterFunc(funcname, type, func)	\
	Register(funcname, ( LuaCFunctionWrap< __LINE__ , type>(func) ) )

#define LUALIB_ITEM_FUNC(funcname, type, func)	\
	LUALIB_ITEM(funcname, ( LuaCFunctionWrap< __LINE__ , type>(func) ) )

#else

#define RegisterFunc(funcname, type, func)	\
	Register(funcname, ( LuaCFunctionWrap< __COUNTER__ , type>(func) ) )

#define LUALIB_ITEM_FUNC(funcname, type, func)	\
	LUALIB_ITEM(funcname, ( LuaCFunctionWrap< __COUNTER__ , type>(func) ) )

#endif


#define END_REGLUALIB()	\
}

#define BEGIN_REGLUALIB_MEMBER()	\
static void LuaOpenLibMember(lua_State* L) \
{	\
	luaL_newmetatable(L, GetLuaTypeName());	\
	static const struct luaL_reg table[] = { {"__index", Lua_ImplementIndex<GetLuaTypeName>}, {NULL, NULL} };	\
	luaL_openlib(L, NULL, table, 0);	

//#define LUALIB_ITEM(funcname, func)	...

#define END_REGLUALIB_MEMBER()	\
}

