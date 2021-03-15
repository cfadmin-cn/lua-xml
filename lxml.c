#define LUA_LIB

#include <core.h>
#include <libxml/parser.h>

/***

    //设置根节点

    // xmlDocSetRootElement(doc,root_node);

    // //在根节点中直接创建节点

    // xmlNewTextChild(root_node, NULL, BAD_CAST "newNode1", BAD_CAST "newNode1 content");

    // xmlNewTextChild(root_node, NULL, BAD_CAST "newNode2", BAD_CAST "newNode2 content");

    // xmlNewTextChild(root_node, NULL, BAD_CAST "newNode3", BAD_CAST "newNode3 content");

    // //创建一个节点，设置其内容和属性，然后加入根结点

    // xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"node2");

    // xmlNodePtr content = xmlNewText(BAD_CAST"NODE CONTENT");

    // xmlAddChild(root_node,node);

    // xmlAddChild(node,content);

    // xmlNewProp(node,BAD_CAST"attribute",BAD_CAST "yes");

    // //创建一个儿子和孙子节点

    // node = xmlNewNode(NULL, BAD_CAST "son");

    // xmlAddChild(root_node,node);

    // xmlNodePtr grandson = xmlNewNode(NULL, BAD_CAST "grandson");

    // xmlAddChild(node,grandson);

    // xmlAddChild(grandson, xmlNewText(BAD_CAST "This is a grandson node"));

***/

// #define lua_tailcall(L, T) {T == LUA_TNUMBER ? ({lua_pop(L, 2); xml_array_encode(L, son);}) : ({lua_pop(L, 2); xml_table_encode(L, son);})}

static int xml_array_encode(lua_State *L, xmlNodePtr node);
static int xml_table_encode(lua_State *L, xmlNodePtr node);

static inline void lua_tailcall(lua_State *L, int TYPE, xmlNodePtr node) {
  if (TYPE == LUA_TNUMBER) {
    lua_pop(L, 2);
    xml_array_encode(L, node);
  } else {
    lua_pop(L, 2);
    xml_table_encode(L, node);
  }
}

static int xml_array_encode(lua_State *L, xmlNodePtr node) {
  lua_pushnil(L);
  // if (lua_next(L, -2)) {
  //   if (lua_type(L, -2) == LUA_TNUMBER) {
  //     lua_pop(L, 2);
  //     xml_array_encode(L, node);
  //     return 1;
  //   }
  //   lua_pop(L, 2);
  // }
  xmlNodePtr son;
  size_t bsize;
  while (lua_next(L, -2)) {
    // 检查Key的类型是否为可序列化类型
    if (lua_type(L, -2) != LUA_TNUMBER && lua_type(L, -2) != LUA_TSTRING) 
      return luaL_error(L, "Invalid table key, required (number or string).");
    // 根据VALUE类型编码
    switch (lua_type(L, -1)){
      case LUA_TSTRING:    //String类型
        son = xmlNewNode(NULL, BAD_CAST("items"));
        xmlAddChild(node, son);
        xmlAddChild(son, xmlNewCDataBlock(son->doc, (const xmlChar *)luaL_checklstring(L, -1, &bsize), bsize));
        break;
      case LUA_TNUMBER:    //Number类型
        xmlNewTextChild(node, NULL, BAD_CAST("items"), BAD_CAST(lua_tostring(L, -1)));
        break;
      case LUA_TBOOLEAN:  //Boolean类型
        son = xmlNewNode(NULL, BAD_CAST("items"));
        // 设置属性字段boolean = "1"
        xmlNewProp(son, BAD_CAST"boolean", BAD_CAST "1");
        // 设置内容为字符串类型的`true`或`false`
        xmlAddChild(son, xmlNewText(lua_toboolean(L, -1) == 0 ? BAD_CAST("false") : BAD_CAST("true")));
        // 绑定到父节点.
        xmlAddChild(node, son);
        break;
      case LUA_TTABLE:    //LUA_TTABLE类型
        son = xmlNewNode(NULL, BAD_CAST("items"));
        xmlAddChild(node, son);
        lua_pushnil(L);
        if (lua_next(L, -2)) {
          lua_checkstack(L, 10); // 防止调用栈过深导致的错误;
          lua_tailcall(L, lua_type(L, -2), son);
        }
        break;
      default:
        return luaL_error(L, "Invalid table value type, required (`number`/`string`/`boolean`/`table`).");
    }
    lua_pop(L, 1);
  }
  return 1;
}

static int xml_table_encode(lua_State *L, xmlNodePtr node) {
  lua_pushnil(L);
  xmlNodePtr son;
  size_t bsize;
  while (lua_next(L, -2)) {
    // 检查Key的类型是否为可序列化类型
    if (lua_type(L, -2) != LUA_TNUMBER && lua_type(L, -2) != LUA_TSTRING) 
      return luaL_error(L, "Invalid table key, required (number or string).");
    // 根据VALUE类型编码
    switch (lua_type(L, -1)){
      case LUA_TSTRING:    //String类型(始终为CDATA)
        son = xmlNewNode(NULL, BAD_CAST(lua_tostring(L, -2)));
        xmlAddChild(node, son);
        xmlAddChild(son, xmlNewCDataBlock(son->doc, (const xmlChar *)luaL_checklstring(L, -1, &bsize), bsize));
        break;
      case LUA_TNUMBER:    //Number类型
        xmlNewTextChild(node, NULL, BAD_CAST(lua_tostring(L, -2)), BAD_CAST(lua_tostring(L, -1)));
        break;
      case LUA_TBOOLEAN:  //Boolean类型
        son = xmlNewNode(NULL, BAD_CAST(lua_tostring(L, -2)));
        // 设置属性字段boolean = "1"
        xmlNewProp(son, BAD_CAST"boolean", BAD_CAST "1");
        // 设置内容为字符串类型的`true`或`false`
        xmlAddChild(son, xmlNewText(lua_toboolean(L, -1) == 0 ? BAD_CAST("false") : BAD_CAST("true")));
        // 绑定到父节点.
        xmlAddChild(node, son);
        break;
      case LUA_TTABLE:    //LUA_TTABLE类型
        son = xmlNewNode(NULL, BAD_CAST(lua_tostring(L, -2)));
        xmlAddChild(node, son);
        lua_pushnil(L);
        if (lua_next(L, -2)) {
          lua_checkstack(L, 10); // 防止调用栈过深导致的错误;
          lua_tailcall(L, lua_type(L, -2), son);
        }
        break;
      default:
        return luaL_error(L, "Invalid table value type, required (`number`/`string`/`boolean`/`table`).");
    }
    lua_pop(L, 1);
  }
  return 1;
}

// 编码
static int lencode(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  size_t nlen;
  const char *tabname = luaL_checklstring(L, 2, &nlen);
  if (!tabname || nlen < 1) 
    return luaL_error(L, "Invalid xml root name.");

  // 创建对象
  xmlDocPtr doc = xmlNewDoc(BAD_CAST("1.0"));
  xmlNodePtr root = xmlNewNode(NULL, BAD_CAST(tabname));
  xmlDocSetRootElement(doc, root);

  lua_settop(L, 1);
  xml_table_encode(L, root);

  int bsize = 0;;
  xmlChar *xmlbuff = NULL;
  xmlDocDumpFormatMemory(doc, &xmlbuff, &bsize, 1);
  lua_pushlstring(L, (const char *)xmlbuff, bsize);
  // 释放XML结构申请的所有内存
  xmlFree(xmlbuff); xmlFreeDoc(doc);
  return 1;
}

// 解码
static int ldecode(lua_State *L) {
  return 1;
}

LUAMOD_API int luaopen_lxml(lua_State *L){
  luaL_checkversion(L);
  luaL_Reg xml_libs[] = {
    {"encode", lencode},
    {"decode", ldecode},
    {NULL, NULL}
  };
  luaL_newlib(L, xml_libs);
  return 1;
}