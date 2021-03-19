-- key->value的空表
local t = {
  a = {},
  b = {},
  c = {},
}

-- 嵌套数组
local t = {
  query = {
    { a = { 1, 2, 3 } },
    { b = { 4, 5, 6 } },
    { c = { 7, 8, 9 } },
  }
}

-- 多层嵌套数组
local t = {
  query = {
    { { { 1 }, { 2 }, { 3 } } },
    { { 4, 5, 6 } },
    { { 7, 8, 9 } },
  }
}

local lxml = require "lxml"
local xml = lxml.encode(t)
print(xml)

local xml2lua = require "xml2lua"
local xml = xml2lua.toxml(t, "xml")
print(xml)