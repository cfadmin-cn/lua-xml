#define LUA_LIB

#include <core.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

// 设置版本号
#define lua_setlversion(L, k, v) ({lua_pushliteral(L, k); lua_pushliteral(L, v); lua_rawset(L, -3);})
// 确保栈上有4个元素，第四个元素必然是`LUA_TTABLE`类型
#define lua_pushtable(L, name) ({ lua_createtable(L, 16, 16); lua_pushstring(L, (name)); lua_pushvalue(L, -2); lua_rawset(L, -4);})

static int xml_array_encode(lua_State *L, xmlNodePtr node, const char* key);
static int xml_table_encode(lua_State *L, xmlNodePtr node);

static int xml_array_encode(lua_State *L, xmlNodePtr node, const char* key) {
  lua_pushnil(L);
  if (lua_next(L, -2)) {
    if (lua_type(L, -2) != LUA_TNUMBER) {
      lua_pop(L, 2);
      return xml_table_encode(L, node);
    }
    lua_pop(L, 2);
    lua_pushnil(L);
  }
  xmlNodePtr son;
  size_t bsize;
  while (lua_next(L, -2)) {
    // 检查Key的类型是否为可序列化类型
    if (lua_type(L, -2) != LUA_TNUMBER && lua_type(L, -2) != LUA_TSTRING) 
      return luaL_error(L, "[XML ERROR]: Invalid table key, required (number or string).");
    // 根据VALUE类型编码
    switch (lua_type(L, -1)){
      case LUA_TSTRING:    //String类型
        son = xmlNewNode(NULL, BAD_CAST(key));
        xmlAddChild(node, son);
        xmlAddChild(son, xmlNewCDataBlock(son->doc, (const xmlChar *)luaL_checklstring(L, -1, &bsize), bsize));
        break;
      case LUA_TNUMBER:    //Number类型
        xmlNewTextChild(node, NULL, BAD_CAST(key), BAD_CAST(lua_tostring(L, -1)));
        break;
      case LUA_TBOOLEAN:  //Boolean类型
        xmlNewTextChild(node, NULL, BAD_CAST(key), lua_toboolean(L, -1) == 0 ? BAD_CAST("false") : BAD_CAST("true"));
        break;
      case LUA_TUSERDATA: case LUA_TLIGHTUSERDATA: //指针类型
        xmlNewTextChild(node, NULL, BAD_CAST(key), BAD_CAST("NULL"));
        break;
      case LUA_TTABLE:    //LUA_TTABLE类型
        lua_pushnil(L);
        //如果是一张空表, 则直接返回空节点即可;
        if (!lua_next(L, -2)) {
          xmlAddChild(node, xmlNewNode(NULL, BAD_CAST(key)));
          break;
        }
        // 如果是一个字典表
        if (lua_type(L, -2) == LUA_TSTRING) {
          lua_pop(L, 2); lua_checkstack(L, 4); // 防止调用栈过深导致的错误;
          son = xmlNewNode(NULL, BAD_CAST(key));
          xmlAddChild(node, son);
          xml_table_encode(L, son);
        // 如果是个数组表
        } else if (lua_type(L, -2) == LUA_TNUMBER) {
          lua_pop(L, 2); lua_checkstack(L, 4); // 防止调用栈过深导致的错误;
          son = xmlNewNode(NULL, BAD_CAST(key));
          xmlAddChild(node, son);
          xml_array_encode(L, son, "item");
        // 其他类型抛出异常.
        } else {
          luaL_error(L, "Invalid type in lua table key.");
        }
        break;
      default:
        return luaL_error(L, "[XML ERROR]: Invalid table value type, required (`number`/`string`/`boolean`/`table`).");
    }
    lua_pop(L, 1);
  }
  return 1;
}

static int xml_table_encode(lua_State *L, xmlNodePtr node) {
  lua_pushnil(L);
  if (lua_next(L, -2)) {
    if (lua_type(L, -2) == LUA_TNUMBER) {
      lua_pop(L, 2);
      return xml_array_encode(L, node, "XML");
    }
    lua_pop(L, 2);
    lua_pushnil(L);
  }
  xmlNodePtr son;
  size_t bsize;
  while (lua_next(L, -2)) {
    // 检查Key的类型是否为可序列化类型
    if (lua_type(L, -2) != LUA_TNUMBER && lua_type(L, -2) != LUA_TSTRING) 
      return luaL_error(L, "[XML ERROR]: Invalid table key, required (number or string).");
    // 根据VALUE类型编码
    switch (lua_type(L, -1)){
      case LUA_TSTRING:    //String类型(始终为CDATA)
        son = xmlNewNode(NULL, BAD_CAST(lua_tostring(L, -2)));
        xmlAddChild(node, son);
        xmlAddChild(son, xmlNewCDataBlock(son->doc, BAD_CAST(luaL_checklstring(L, -1, &bsize)), bsize));
        break;
      case LUA_TNUMBER:    //Number类型
        xmlNewTextChild(node, NULL, BAD_CAST(lua_tostring(L, -2)), BAD_CAST(lua_tostring(L, -1)));
        break;
      case LUA_TBOOLEAN:  //Boolean类型
        xmlNewTextChild(node, NULL, BAD_CAST(lua_tostring(L, -2)), lua_toboolean(L, -1) == 0 ? BAD_CAST("false") : BAD_CAST("true"));
        break;
      case LUA_TUSERDATA: case LUA_TLIGHTUSERDATA: //指针类型
        xmlNewTextChild(node, NULL, BAD_CAST(lua_tostring(L, -2)), BAD_CAST("NULL"));
        break;
      case LUA_TTABLE:    //LUA_TTABLE类型
        lua_pushnil(L);
        // 如果是一张空表, 则直接返回空节点即可;
        if (!lua_next(L, -2)) {
          xmlAddChild(node, xmlNewNode(NULL, BAD_CAST(lua_tostring(L, -2))));
          break;
        }
        // 如果是一个字典表
        if (lua_type(L, -2) == LUA_TSTRING) {
          lua_pop(L, 2); lua_checkstack(L, 4); // 防止调用栈过深导致的错误;
          son = xmlNewNode(NULL, BAD_CAST(lua_tostring(L, -2)));
          xmlAddChild(node, son);
          xml_table_encode(L, son);
        // 如果是个数组表
        } else if (lua_type(L, -2) == LUA_TNUMBER) {
          lua_pop(L, 2); lua_checkstack(L, 4); // 防止调用栈过深导致的错误;
          xml_array_encode(L, node, lua_tostring(L, -2));
        // 其他类型抛出异常.
        } else {
          luaL_error(L, "Invalid type in lua table key.");
        }
        break;
      default:
        return luaL_error(L, "[XML ERROR]: Invalid table value type, required (`number`/`string`/`boolean`/`table`).");
    }
    lua_pop(L, 1);
  }
  return 1;
}

// 编码
static int lencode(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);

  // 检查是否有指定表名
  size_t len;
  const char *tn = lua_tolstring(L, 2, &len);
  if (!tn || len < 1){
    len = 3; tn = "xml";
  }
  // 解决GC引用的潜在问题
  char tabname[len + 1];
  memset(tabname, 0x0, len + 1);
  memcpy(tabname, tn, len);

  // 检查是否取消格式化
  int format = lua_toboolean(L, 3);
  if (!format)  format = 1; else format = 0;

  lua_settop(L, 1);
  lua_pushnil(L);
  // 如果是空表直接给出空表记录
  if(lua_next(L, -2) == 0) {
    lua_pushfstring(L, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<%s></%s>", tabname, tabname);
    return 1;
  }
  lua_settop(L, 1);

  // 创建文档对象与根节点
  xmlDocPtr doc = xmlNewDoc(BAD_CAST("1.0"));
  xmlNodePtr root = xmlNewNode(NULL, BAD_CAST(tabname));
  xmlDocSetRootElement(doc, root);
  // LOG("DEBUG", "开始");
  // 开始编码
  xml_table_encode(L, root);
  // LOG("DEBUG", "结束");
  // 编码结束
  int xsize = 0;;
  xmlChar *xmlbuff = NULL;
  xmlDocDumpFormatMemoryEnc(doc, &xmlbuff, &xsize, "UTF-8", format);
  lua_pushlstring(L, (const char *)xmlbuff, xsize);
  // 释放XML结构申请的所有内存
  xmlFree(xmlbuff); xmlFreeDoc(doc);
  return 1;
}

static inline void xml_node_dump(lua_State *L, xmlNodePtr node) {
  xmlNodePtr cur_node = NULL;
  for (cur_node = node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
      // 如果没有子节点
      if (!cur_node->children) {
        lua_pushstring(L, (const char *)cur_node->name);
        lua_newtable(L); lua_rawset(L, -3);
        return ;
      }
      // 如果有字节点但是子节点也是`数组`或者`字典`
      if (cur_node->children->type == XML_ELEMENT_NODE){
        lua_pushtable(L, (const char *)cur_node->name);
        xml_node_dump(L, cur_node->children);
        lua_pop(L, 1);
        continue;
      }
      // printf("not.\n");
    } 
    // 如果子节点类型数TEXT或者CDATA需要检查是`数组`表示法或是`字典`表示
    // 规则是：多个同名节点为`数组`节点, 只有一个数据节点或多个不同名节点则为`字典`节点
    if (cur_node->next && xmlStrEqual(cur_node->next->name, cur_node->name)) {
      // printf("开始\n");
      int index = 1;
      xmlNodePtr e;
      xmlNodePtr p = cur_node;
      const xmlChar *name = p->name;
      lua_pushtable(L, (const char *)p->name);
      for (; p && xmlStrEqual(p->name, name); p = p->next) {
        // printf("array [%s] = [%s]\n", p->name, xmlNodeGetContent(p->children));
        lua_pushstring(L, (const char *)xmlNodeGetContent(p->children));
        lua_rawseti(L, -2, index++);
        e = p;
      }
      cur_node = e;
      lua_pop(L, 1);
      // printf("结束\n");
    } else {
      // printf("table [%s] = [%s]\n", cur_node->name, xmlNodeGetContent(cur_node->children));
      lua_pushstring(L, (const char *)cur_node->name);
      lua_pushstring(L, (const char *)xmlNodeGetContent(cur_node));
      lua_rawset(L, -3);
    }
  }
}

static inline int xml_decoder(lua_State *L, xmlDocPtr doc) {
  // 文档对象
  if (!doc) {
    lua_pushnil(L);
    lua_pushstring(L, "[XML ERROR]: Invalid string buffer.");
    return 2;
  }
  // 根节点
  xmlNodePtr root = xmlDocGetRootElement(doc);
  if (!root) {
    xmlFreeDoc(doc);
    lua_pushnil(L);
    lua_pushstring(L, "[XML ERROR]: can't find root node element.");
    return 2;
  }
  lua_settop(L, 0);
  // 构建根节点
  lua_createtable(L, 0, 1);
  lua_pushtable(L, (const char *)root->name);
  // 构建根节点
  xml_node_dump(L, root->children);
  lua_settop(L, 1);
  return 1;
}

// 解码
static int ldecode(lua_State *L) {
  size_t xlen;
  const char *xmlbuffer = luaL_checklstring(L, 1, &xlen);
  if (!xmlbuffer || xlen < 7)
    return luaL_error(L, "[XML ERROR]: Invalid string buffer.");

  // 消除无用的空格与换行
  int i;
  for (i = 0; i < xlen; i++){
    if (xmlbuffer[i] == '<')
      break;
  }
  if (xlen < 7)
    return 0;
  return xml_decoder(L, xmlReadMemory(xmlbuffer + i, xlen - i, NULL, NULL, XML_PARSE_HUGE));
}

LUAMOD_API int luaopen_lxml(lua_State *L){
  luaL_checkversion(L);
  // 去除空白符
  xmlKeepBlanksDefault(0);
  // 注册方法
  luaL_Reg xml_libs[] = {
    {"encode", lencode},
    {"decode", ldecode},
    {NULL, NULL}
  };
  luaL_newlib(L, xml_libs);
  // lxml 版本
  lua_setlversion(L, "__VERSION__", "0.1");
  // libxml2 版本
  lua_setlversion(L, "__XML_VERSION__", LIBXML_DOTTED_VERSION);
  return 1;
}