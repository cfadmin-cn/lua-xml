require "utils"


local t = {
  -- a = {
  --   a1 = {
  --     a2 = {
  --       a3 = {
  --       }
  --     }
  --   }
  -- },
  -- b = { "我", "是", "谁" },
  -- c = { { 1, 2, 3 }, { 4, 5, 6} },
  -- d = { a = 1, b = 2, c = 3 },
  -- e = {true, false, null},
  -- f = function () end,
  -- key = "key", value = "value",
}


-- 使用 lxml
local lxml = require "lxml"
-- 编码
local xml = lxml.encode(t)
print(xml)
-- 解码
local tab = lxml.decode(xml)
var_dump(tab)


-- -- 使用 xml2lua
-- local xml2lua = require "xml2lua"
-- -- 编码
-- local xml = xml2lua.toxml(t, "xml")
-- print(xml)
-- -- 解码
-- local xml = xml2lua.parser(xml)
-- var_dump(xml)