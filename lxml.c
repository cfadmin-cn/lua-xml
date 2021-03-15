#define LUA_LIB

#include <core.h>
#include <libxml/parser.h>

static int lencode(lua_State *L) {
  return 1;
}

static int ldecode(lua_State *L) {
  return 1;
}

LUAMOD_API int luaopen_lz(lua_State *L){
  luaL_checkversion(L);
  luaL_Reg xml_libs[] = {
    {"encode", lencode},
    {"decode", ldecode},
    {NULL, NULL}
  };
  luaL_newlib(L, xml_libs);
  return 1;
}