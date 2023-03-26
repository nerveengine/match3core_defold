-- Put functions in this file to use them in several other scripts.
-- To get access to the functions, you need to put:
-- require "my_directory.my_file"
-- in any script using the functions.

local M = {}

-- private
local scale = vmath.vector3(1)

local chip_textures = { hash("000"), hash("001"), hash("002"), hash("003"), hash("004"), hash("005"), hash("006"), hash("007"), hash("008"), hash("009"), hash("010")}
local chip_empty_texture = hash("empty_cell")

function M.setScale(new_scale)
	scale = new_scale
	print("New scale set", scale)
end

function M.create_chip(chip, pos)
	-- print(message)
	local match_id = chip.match_id
	local chip_uid = chip.chip_uid
	local sprite = chip_empty_texture
	if match_id >= 0 then
		sprite = chip_textures[match_id]
	end
	local id = factory.create("#blockfactory", pos, nil, {color = sprite, chip_uid = chip_uid}, scale)
	return id
end

return M