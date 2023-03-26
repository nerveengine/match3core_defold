// myextension.cpp
// Extension lib defines
#define LIB_NAME "MyExtension"
#define MODULE_NAME "myextension"



#include "match3Core/library.include.h"
#include "match3Communicator/library.include.h"

#include "waitable_queue.h"

#include <dmsdk/sdk.h> // the Defold SDK

#include <atomic>
#include <chrono>
#include <iostream>
#include <functional>
#include <future>
#include <mutex>
#include <random>
#include <thread>
#include <filesystem>

using namespace std::chrono_literals;

static int setupUpdateFieldCallback(lua_State* L);
static int setupSetFieldCallback(lua_State* L);
static void asyncCallback();
static void setGameArea();
static void updateGameArea();
static int needPull(lua_State* L);
static int getGameArea(lua_State* L);
static int trySwap(lua_State* L);

GameArea game_area;

static dmScript::LuaCallbackInfo* update_field_callback = nullptr;
static dmScript::LuaCallbackInfo* set_field_callback = nullptr;

static int width = 7;
static int height = 5;
static std::atomic_bool need_set = false;
static std::atomic_bool need_update = false;
static std::atomic_bool need_pull = false;
static auto iteration_time = 250ms;
// static std::mutex game_area_mutex;

// using SimpleCallback = std::function<void()>;
// WaitableQueue<SimpleCallback> thread_safe_queue;
// 
// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{
    {"getGameArea", getGameArea },
    {"setupUpdateFieldCallback", setupUpdateFieldCallback },
    {"setupSetFieldCallback", setupSetFieldCallback },
    {"trySwap", trySwap },
    {"needPull", needPull },
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

static dmExtension::Result AppInitializeMyExtension(dmExtension::AppParams* params)
{
    dmLogInfo("AppInitializeMyExtension");
    // подготавливаем уровень по умолчанию
    // тут луа ещё нет, просто делаем уровень
    

    setup_on_game_area([](const GameArea& area)
    {
        game_area = area;
        game_area.remap(area.game_field_mapping());
        // spawn_chips(game_area);
        need_set = true;
    });
    
     run_update_loop();
// 
    game_area.resize(width, height);
    game_area.setChipTypesRange(1, 5);

    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeMyExtension(dmExtension::Params* params)
{
    // Init Lua
    LuaInit(params->m_L);
    dmLogInfo("Registered %s Extension", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeMyExtension(dmExtension::AppParams* params)
{
    dmLogInfo("AppFinalizeMyExtension");
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeMyExtension(dmExtension::Params* params)
{
    dmLogInfo("FinalizeMyExtension");
    return dmExtension::RESULT_OK;
}

static dmExtension::Result OnUpdateMyExtension(dmExtension::Params* params)
{
    // dmLogInfo("OnUpdateMyExtension");

    // thread_safe_queue.invoke([](SimpleCallback& cb) 
    // {
    //     cb();
    // });

    if (need_set) {
        setGameArea();
    }
    if (need_update) {
        updateGameArea();
    }

    return dmExtension::RESULT_OK;
}

static void OnEventMyExtension(dmExtension::Params* params, const dmExtension::Event* event)
{
    switch(event->m_Event)
    {
        case dmExtension::EVENT_ID_ACTIVATEAPP:
            dmLogInfo("OnEventMyExtension - EVENT_ID_ACTIVATEAPP");
            break;
        case dmExtension::EVENT_ID_DEACTIVATEAPP:
            dmLogInfo("OnEventMyExtension - EVENT_ID_DEACTIVATEAPP");
            break;
        case dmExtension::EVENT_ID_ICONIFYAPP:
            dmLogInfo("OnEventMyExtension - EVENT_ID_ICONIFYAPP");
            break;
        case dmExtension::EVENT_ID_DEICONIFYAPP:
            dmLogInfo("OnEventMyExtension - EVENT_ID_DEICONIFYAPP");
            break;
        default:
            dmLogWarning("OnEventMyExtension - Unknown event id");
            break;
    }
}

// Установка колбэка
static int setupUpdateFieldCallback(lua_State* L) // called from Lua
{
    dmLogInfo("+setupUpdateFieldCallback");

    if (!update_field_callback) {
        update_field_callback = dmScript::CreateCallback(L, 1);
    } else {
        dmLogWarning("callback already registered please wait");
        return 1;
    }

    dmLogInfo("-setupUpdateFieldCallback");
    return 1;
}

static int setupSetFieldCallback(lua_State* L) // called from Lua
{
    dmLogInfo("+setupSetFieldCallback");

    if (!set_field_callback) {
        set_field_callback = dmScript::CreateCallback(L, 1);
    } else {
        dmLogWarning("callback already registered please wait");
        return 1;
    }

    dmLogInfo("-setupSetFieldCallback");
    return 1;
}


static int needPull(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushboolean(L, need_pull);
    return 1;
}

// пушит в луа
static void setGameArea()
{
    dmLogInfo("setGameArea()");
    // need_pull = true; // взводим флаг что нужно пулить со стороны луа. Пуш не сработал пока.
    // return;

    if (set_field_callback == 0)
    {
        dmLogWarning("No listener was set!");
        return;
    }

    lua_State* L = dmScript::GetCallbackLuaContext(set_field_callback);

    if (!dmScript::SetupCallback(set_field_callback))
    {
        dmLogError("failed SetupCallback");
        return;
    }

    const auto new_size = game_area.size();
    width  = new_size.x;
    height = new_size.y;
    int res = getGameArea(L);
    dmScript::PCall(L, res+1, 0); // self + # user arguments
    dmLogInfo("called lua");

    dmScript::TeardownCallback(set_field_callback); // Это нужно обязательно, симметрично к dmScript::SetupCallback

    // Удалить колбэк. Если предполагается взывать ещё и ещё, то удалять не нужно.
    // dmScript::DestroyCallback(update_field_callback);
    // update_field_callback = 0;
    need_set = false;
}

static void updateGameArea()
{
    dmLogInfo("updateGameArea()");
    // need_pull = true; // взводим флаг что нужно пулить со стороны луа. Пуш не сработал пока.
    // return;

    if (update_field_callback == 0)
    {
        dmLogWarning("No listener was set!");
        return;
    }

    lua_State* L = dmScript::GetCallbackLuaContext(update_field_callback);

    if (!dmScript::SetupCallback(update_field_callback))
    {
        dmLogError("failed SetupCallback");
        return;
    }
    int res = getGameArea(L);
    dmScript::PCall(L, res+1, 0); // self + # user arguments
    dmLogInfo("called lua");

    dmScript::TeardownCallback(update_field_callback); // Это нужно обязательно, симметрично к dmScript::SetupCallback

    // Удалить колбэк. Если предполагается взывать ещё и ещё, то удалять не нужно.
    // dmScript::DestroyCallback(update_field_callback);
    // update_field_callback = 0;
    need_update = false;
}

static void dumpStack (lua_State *L) {
    dmLogInfo("+dumpStack");
    int top=lua_gettop(L);
    for (int i=1; i <= top; i++) {
        dmLogInfo("%d\t%s\t", i, luaL_typename(L,i));
        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
            dmLogInfo("%g",lua_tonumber(L,i));
            break;
            case LUA_TSTRING:
            dmLogInfo("%s",lua_tostring(L,i));
            break;
            case LUA_TBOOLEAN:
            dmLogInfo("%s", (lua_toboolean(L, i) ? "true" : "false"));
            break;
            case LUA_TNIL:
            dmLogInfo("%s", "nil");
            break;
            default:
            dmLogInfo("%p",lua_topointer(L,i));
            break;
        }
    }
    dmLogInfo("-dumpStack");
}

static int getGameArea(lua_State* L)
{
    // const std::lock_guard<std::mutex> lock(game_area_mutex);
    dmLogInfo("getGameArea");
    DM_LUA_STACK_CHECK(L, 3); // number of arguments on stack
  
    // conversion of game_area to lua data
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    /**
     * Разные слои — разные поля
    */
    lua_createtable(L, 0, 3); // 3 аргумент - количество слоёв
    {
        // структура поля
        lua_createtable(L, width, 0);
        for (int w = 0; w < width; ++w) { // Или наоборот ширина и высота
            lua_pushinteger(L, w);

            lua_createtable(L, height, 0);
            {
                for (int h = 0; h < height; ++h) {
                    lua_pushinteger(L, h);
                    lua_createtable(L, 0, 1);
                    {
                        const auto& cell = game_area.base.at(w).at(h); // тут если что из complex_field можно брать
                        lua_pushinteger(L, cell.enabled);
                        lua_setfield(L, -2, "enabled");
                    }
                    lua_settable(L, -3);
                }
            }
            lua_settable(L, -3);
        }
        lua_setfield(L, -2, "cells");
    }

    {
        // структура порталов
        lua_createtable(L, width, 0);
        for (int w = 0; w < width; ++w) { // Или наоборот ширина и высота
            lua_pushinteger(L, w);

            lua_createtable(L, height, 0);
            {
                for (int h = 0; h < height; ++h) {
                    lua_pushinteger(L, h);
                    lua_createtable(L, 0, 2);
                    {
                        const auto& portal_cell = game_area.portals.at(w).at(h);
                        lua_pushboolean(L, portal_cell.portal_in);
                        // lua_pushboolean(L, (w+h)%2);
                        lua_setfield(L, -2, "portal_in");
                        lua_pushinteger(L, portal_cell.portal_id);
                        // lua_pushnumber(L, (w*height + h)%4);
                        lua_setfield(L, -2, "portal_id");
                    }
                    lua_settable(L, -3);
                }
            }
            lua_settable(L, -3);
        }
        lua_setfield(L, -2, "portals");
    }
    
    {
        // структура фишек
        lua_createtable(L, width, 0);
        for (int w = 0; w < width; ++w) { // Или наоборот ширина и высота
            lua_pushinteger(L, w); //NOTE: this will be 'indexed' from 0. Add +1 if need from 1
            lua_createtable(L, 0, height);   
            for (int h = 0; h < height; ++h) {
                lua_pushinteger(L, h); //NOTE: this will be 'indexed' from 0. Add +1 if need from 1
                const Match3Element* cell = game_area.game_field.at(w).at(h);

                lua_createtable(L, 0, 2);
                if (cell) {
                    lua_pushinteger(L, cell->match_id);
                    lua_setfield(L, -2, "match_id");
                    lua_pushinteger(L, cell->uid);
                    lua_setfield(L, -2, "chip_uid");
                } else {
                    // std::cerr << "no chip in position " << w << ' ' << h << "chip_uid will be 0\n";
                    lua_pushinteger(L, -1); // нет там ничего
                    lua_setfield(L, -2, "match_id");
                    lua_pushinteger(L, size_t(0));
                    lua_setfield(L, -2, "chip_uid");
                }
                
                lua_settable(L, -3);
            }
            lua_settable(L, -3);
        }
        lua_setfield(L, -2, "chips");
    }
    need_pull = false;
    return 3;
}
    
static int trySwap(lua_State* L)
{
    const int x1 = luaL_checknumber(L, 1);
    const int y1 = luaL_checknumber(L, 2);
    const int x2 = luaL_checknumber(L, 3);
    const int y2 = luaL_checknumber(L, 4);
    dmLogInfo("trySwap (%d, %d) - (%d, %d)", x1, y1, x2, y2);
    // прям тут!
    std::swap(game_area.game_field.at(x1).at(y1), game_area.game_field.at(x2).at(y2));

    // тут вся анимация шага играется за одну итерацию, вообще не наглядно
    // std::thread([] () {
    //     need_update = true;
    //     std::this_thread::sleep_for(iteration_time);
    //     while (true) {
    //         bool res = evaluate_field(game_area, true);
    //         if (!res) {
    //             break;
    //         }
    //         std::this_thread::sleep_for(iteration_time);
    //         need_update = true;
    //     }
    // } ).detach();


    std::thread([] () {
        int iteration = 0;
        need_update = true;
        // setGameArea();
        std::this_thread::sleep_for(iteration_time);
        while (true) {
            bool processing = false;

            while (true) {
                // game_area_mutex.lock();
                bool res = try_clear_matches(game_area);
                // game_area_mutex.unlock();
                if (!res) {
                    break;
                }
                processing = true;
                std::this_thread::sleep_for(iteration_time);
                dmLogInfo("iteration %d", iteration++);
                // setGameArea();
                need_update = true;
            }

            while (true) {
                // game_area_mutex.lock();
                bool res = apply_mechanics(game_area);
                // game_area_mutex.unlock();
                if (!res) {
                    break;
                }
                processing = true;
                std::this_thread::sleep_for(iteration_time);
                dmLogInfo("iteration %d", iteration++);
                // setGameArea();
                need_update = true;
            }

            if (!processing) {
                break;
            }
        }
    } ).detach();
    
    return 0;
}

// Defold SDK uses a macro for setting up extension entry points:
//
// DM_DECLARE_EXTENSION(symbol, name, app_init, app_final, init, update, on_event, final)

// MyExtension is the C++ symbol that holds all relevant extension data.
// It must match the name field in the `ext.manifest`
DM_DECLARE_EXTENSION(MyExtension, LIB_NAME, AppInitializeMyExtension, AppFinalizeMyExtension, InitializeMyExtension, OnUpdateMyExtension, OnEventMyExtension, FinalizeMyExtension)
