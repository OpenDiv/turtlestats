local modemSide = "back"

local protocol = "miner"
local ws = http.websocket("ws://127.0.0.1:8083/")
local hostID = os.getComputerID()

local msgBuffer = {}
local idBuffer = -1

local turtles = {} -- roles as turtles["mine"] and turtles["fuel"]

local ordersAmount = 1
local orderList = {}

local miners = {}
local refuelers = {}
turtles["mine"] = miners
turtles["stash"] = refuelers

local role = "null"

local xC, yC, zC = gps.locate()

function wsCast(data) -- printing data + sending to ws client
	if ws then
		ws.send(data)
	end
	print(data)
	return ws
end

function listener() -- receiving messages
	wsCast("[listener] waiting for msg")
	local id, message = rednet.receive(protocol)
		msgBuffer = message
		idBuffer = id
		print("[listener] NEW MSG: "..message[1].." from #"..id)
		print("[listener] orderList size: "..ordersAmount)
		if message[1] == "NEW" then
			parallel.waitForAll(listener, proceedMsgNEW)
		elseif message[1] == "HOLE" then
			parallel.waitForAll(listener, proceedMsgHOLE)
		elseif message[1] == "ORDER" then
			parallel.waitForAll(listener, proceedMsgORDER_CREATE)
		elseif message[1] == "ORDER_PICKED" then
			parallel.waitForAll(listener, proceedMsgORDER_PICKED)
		elseif message[1] == "ORDER_LIST" then
			parallel.waitForAll(listener, proceedMsgORDER_LIST)
		else
			listener()
		end
end

function proceedMsgORDER_LIST()
	print(#orderList)
	for i = 1, #orderList do
		rednet.send(53, ("ord: "..tostring(orderList[i][2])), "order")
	end
	rednet.send(53,"FIN", "order")
end

function proceedMsgNEW()
	local thisMsg = msgBuffer
	local thisID = tostring(idBuffer)
	local role = msgBuffer[2]
	if turtles[role][thisID] == nil then
		turtles[role][thisID] = true
	end
	
	rednet.send(tonumber(thisID), {true, yC}, protocol)
	if turtles[role][thisID] == true then 
		
		turtles[role][thisID] = {-1, -1, -1, {"hole"}}
		wsCast("[proceedMsgNew] added #"..thisID)
	else
		wsCast("[proceedMsgNew] ignored #"..thisID)
	end
	
	if role == "stash" then
		if #orderList>0 then
			for i = 1, #orderList do
				print(i.." order: "..orderList[i][2])
				rednet.send(tonumber(thisID) ,orderList[i], "order")
			end
		end
		
		rednet.send(tonumber(thisID) ,{"ORDERS_LOADED"}, "order")
	end
end
-- holes {holeInfo, x, y, z} => holeInfo {blockInfo} => blockInfo {1,2,3,4}
function proceedMsgHOLE() -- HOLE, x, y, z, holes[#holes](last hole)
	local thisMsg = msgBuffer
	local thisID = idBuffer
	-- thisMsg = "HOLE", xCord, yCord, zCord, holes[#holes]
	-- turtles (1role; 2x; 3y; 4z; 5holes; 5n holeInfo) 
	local data = textutils.serialise(thisMsg[5][1])
	print(data)
	wsCast(data)
	--local holeInfoSize = #turtles["mine"][thisID][5]
	--if turtles["mine"][thisID] then
	--		turtles["mine"][thisID][4][holeInfoSize+1] = thisMsg[5]
	--		turtles["mine"][thisID][1] = thisMsg[2]
	--		turtles["mine"][thisID][2] = thisMsg[3]
	--		turtles["mine"][thisID][3] = thisMsg[4]
	--		wsCast("[proceedMsgData] added #"..thisID) 
	--end
	--wsCast("[proceedMsgData] ignored #"..thisID) 
end


-- {"ORDER", orderID, x, y, z, type, opt}
function proceedMsgORDER_CREATE()
	local thisMsg = msgBuffer
	local thisID = idBuffer
	
	thisMsg[2] = ordersAmount
	ordersAmount = ordersAmount + 1
	
	table.insert(orderList, thisMsg)
	
	rednet.broadcast(thisMsg, "order")
	print("[proceedMsgORDER_CREATE] created ")
end
-- {"ORDER_PICKED", orderID)
function proceedMsgORDER_PICKED()
	local thisMsg = msgBuffer
	local thisID = idBuffer
	local orderWasFound = false
	
	thisMsg[3] = thisID
	-- {"ORDER_PICKED", orderID, stashID)
	print("#orderList "..#orderList)
	if #orderList>1 then
	
		for i = 1, #orderList do -- удаление заказа из списка
			if orderList[i][2] == thisMsg[2] then
					orderWasFound = true
					for k = i, #orderList-1 do
						orderList[k] = orderList[k+1]
					end
					orderList[#orderList] = nil
				break
			end
		end
	else
		if #orderList>0 then
		if orderList[1][2] == thisMsg[2] then
			orderWasFound = true
			orderList[1] = nil
		end
		end
	end
	print("#orderList after clean-up "..#orderList)
	if orderWasFound then
		rednet.broadcast(thisMsg, "order")
	else
		print("#"..thisID.." unable to take order"..thisMsg[2])
	end
end

function main()
	rednet.open(modemSide)
	
end

print("Hosting #"..hostID)

parallel.waitForAll(listener, main)