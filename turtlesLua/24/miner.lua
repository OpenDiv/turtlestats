local thisID = os.getComputerID()
local hostID = 21
local protocol = "miner"

local yMainCord = -1
local xCord = -1
local xCord = -1
local yCord = -1
local zCord = -1

local lastFuel = -1
local isFuelEnough = false
local fuelTask = {}
fuelTask["hole"] = (yMainCord+64)*2

requiedBlocks = {} 
requiedBlocks["minecraft:white_wool"] = true-- filtered blocks to mine
holes = {} -- holeInfo, x, y, z

function connectHost()
	rednet.open("left")
	rednet.send(hostID, {"NEW", "mine"}, protocol)
	repeat
		local id, message = rednet.receive(protocol)
		if message[2] then
			yMainCord = message[2]
		end
	until id == hostID and message[1] == true
	print("[connectHost] #"..thisID.." connected to #"..hostID)
end

function listener()
	print("[listener] started")
	local id, message = rednet.receive(protocol)
	msgBuffer = message
	idBuffer = id
	
end

function sendHoleInfo()
	local holeInfoMsg = {"HOLE", xCord, yCord, zCord, holes[#holes]}
	print("#holes "..#holes)
	rednet.send(hostID, holeInfoMsg, protocol)
end

function getRequiedResource(levelInfo)
	local hasReq = false
	for i = 1, 4 do -- blockList - hash table
		if requiedBlocks[levelInfo[i]] then
			hasReq = true
			break
		end
	end
	
	if hasReq then
		if requiedBlocks[levelInfo[3]] or 
		(requiedBlocks[levelInfo[2]] and requiedBlocks[levelInfo[4]])
		and not requiedBlocks[levelInfo[1]] then -- DELETE LATER INFO1
			for i = 1, 4 do -- blockList - hash table
				if requiedBlocks[levelInfo[i]] then
					turtle.dig()
					holes[#holes][yCord][i] = "minecraft:air"
				end
				turtle.turnLeft()
			end
		else
			
			if requiedBlocks[levelInfo[4]] then
				turtle.turnRight()
				turtle.dig()
				holes[#holes][yCord][4] = "minecraft:air"
				turtle.turnLeft()
			end
			
			if requiedBlocks[levelInfo[2]] then
				turtle.turnLeft()
				turtle.dig()
				holes[#holes][yCord][2] = "minecraft:air"
				turtle.turnRight()
			end
			
			if requiedBlocks[levelInfo[1]] then
			holes[#holes][yCord][1] = "minecraft:air"
				turtle.dig()
			end
		end
		
	end
end

function holeCycleDown()
	local holeInfo = {}
	repeat
		local blockInfo = {}
		turtle.digDown()
		turtle.down()
		yCord = yCord - 1
		
		local has_block, data
		for i = 1, 4 do
			has_block, data = turtle.inspect()
			if has_block then
				blockInfo[i] = data.name
			else
				blockInfo[i] = "air"
			end
			turtle.turnLeft()
		end
		
		has_block, data = turtle.inspectDown()
		holeInfo[yCord] = blockInfo
	until data.name == "minecraft:bedrock"
	holes[#holes+1] = {holeInfo, xCord, yCord, zCord}
end

function holeCycleUp()
	repeat
		getRequiedResource(holes[#holes][1][yCord])
-- holes => current hole => holeInfo[1] {blockInfo} =>yCord
			print(yCord.." :cur ! main: "..yMainCord)
			if not turtle.up() then
				turtle.digUp()
				turtle.up()
			end
			yCord = yCord + 1
	until yCord == yMainCord
end

function sortInventory() -- returns amount of cleared slots
	local slotsCleared = 0
	for i = 1, 16 do
		local slot = turtle.getItemDetail(i)
			if not requiedBlocks[slot["name"]] then
				turtle.dropDown()
				slotsCleared = slotsCleared + 1
			end
	end
	return slotsCleared()
end

function fuelCheck(task) -- hole / 
	local fuelLevel = turtle.getFuelLevel()
	lastFuel = fuelLevel
	if fuelLevel<fuelTask[task] then
		isFuelEnough = false
		waitForRefuel()
	else
		isFuelEnough = true
	end
end

function waitForRefuel()
	local availableSlots = amountOfAvailableSlots()
	local forcedDown = false
	if availableSlots == 0 then
		availableSlots = sortInventory()
		if availableSlots == 0 then -- if no options than drop item
			local has_block, data = turtle.inspect()
				if data["name"] ~= "computercraft:turtle_advanced" then
					turtle.dig()
				else
					forcedDown = true
					turtle.digDown()
					if turtle.down() then
						turtle.dig()
					else
						print("smth break")
						while true do
						end
					end
				end -- if cant break other turtle then dig down as 
				-- other turtles cant be lower
			turtle.drop()
			turtle.up()
			availableSlots = 1
			
			local curX, curY, curZ = gps.locate()
			local cords = {curX, curY, curZ}
			
			rednet.send(hostID, {"REFUEL", cords}, fuelLevel, protocol)
		end
	end
	
	for i = 1, 16 do
		select(i)
		turtle.refuel()
		
	end
end

function amountOfAvailableSlots()
	slotCounter = 0
	for i = 1, 16 do
		if turtle.getItemCount(i) == 0 then
			slotCounter = slotCounter + 1
		end
	end
	return slotCounter
end

function main()
	 local x, y, z = gps.locate()
	 if x then
		xCord = x
		yCord = y
		zCord = z
		print("yMain:"..yMainCord.."; x:"..x.."; y:"..y.."; z:"..z)
		-- TEMP
		
		-- TEMP
	 end
	 
	 holeCycleDown()
	 holeCycleUp()
	 sendHoleInfo()
end

connectHost()
parallel.waitForAll(listener, main)
