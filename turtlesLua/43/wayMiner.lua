local thisID = os.getComputerID()
local hostID = 21
local protocol = "order"

local x = -1
local y = -1
local z = -1

local msgBuffer = {}
local idBuffer = -1

local completingOrder = false
local slotsCapacity = -1

local yMainCord = -1

local cur = {x, z} -- 0; 0
local curOrder
local curDestination = {-1, -1}
local orderList = {}

local cordGet = {"zMinus", "xMinus", "zPlus", "xPlus"}
local cordSwitch -- checkDirection() initializes

local curDir

local ordersLoaded = false

function ready()
	while true do
		print(completingOrder)
		sleep(5)
	end
end

function connectHost()
	rednet.open("left")
	rednet.send(hostID, {"NEW", "stash"}, "miner")
	repeat
		local id, message = rednet.receive("miner")
		if message[2] then
			yMainCord = message[2]
		end
	until id == hostID and message[1] == true
	print("[connectHost] #"..thisID.." connected to #"..hostID)
end

function listener()
	--print("[listener] started")
	local id, message = rednet.receive(protocol)
	--print("[listener] New msg: "..message[1])
	
	
	msgBuffer = message
		idBuffer = id
		
	if message[1] == "ORDER" then
		parallel.waitForAll(listener, proceedMsgORDER)
	elseif message[1] == "ORDER_PICKED" then
		parallel.waitForAll(listener, proceedMsgORDER_PICKED)
	elseif message[1] == "ORDERS_LOADED" then
		ordersLoaded = true
		selectOrder()
		listener()
	else
		listener()
	end
end

function proceedMsgORDER()
	local thisMsg = msgBuffer
	table.insert(orderList, thisMsg)
	print("[proceedMsgORDER] New order, amount: "..#orderList)
	if ordersLoaded then
		selectOrder()
	end
end

function proceedMsgORDER_PICKED()
	local thisMsg = msgBuffer
	
	-- {"ORDER_PICKED", orderID, stashID)
	
	for i = 1, #orderList do -- удаление заказа из списка
		if orderList[i][2] == thisMsg[2] then
				if thisMsg[3] == thisID then
					curOrder = orderList[i]
				end
				
				for k = i, #orderList-1 do
					orderList[k] = orderList[k+1]
				end
				orderList[#orderList] = nil
			break
		end
	end
	if thisMsg[3] == thisID then
		print("[proceedMsgORDER_PICKED] order picked #"..thisMsg[2])
		print("FROM: x="..x.."; z="..z.." to x1=")
		completingOrder = true
		orderCompleteMovement()
	else
		print("[proceedMsgORDER_PICKED] order was stolen by #"..thisMsg[3])
		selectOrder()
	end
end

function selectOrder() -- выбирает заказ
	local closestOrderID = -1
	local closestRange = -1
	
	print("[selectOrder] selecting order")
	
	if not completingOrder then -- если ничем не занят
		if #orderList>0 then -- если есть заказы
			for i = 1, #orderList do -- orderList type = {"STASH", orderID, x, y, z, "type", {opt}"
					local currentRange = abs(orderList[i][3]-x) + abs(orderList[i][5]-z)
					if currentRange<closestRange or closestRange == -1 then
								closestOrderID = orderList[i][2]
								closestRange = currentRange
					end
			end -- когда заказ выбран, уведомляет хост
			print("[selectOrder] requesting host about order #"..closestOrderID)
			rednet.send(hostID, {"ORDER_PICKED", closestOrderID}, "miner")
		else
			print("[selectOrder] order list is empty")
		end
	else 
		print("[selectOrder] Already completing order #"..curOrder[2])
	end
end

function waitForOrder() -- protocol order
	completingOrder = false
	if #orderList>0 then
		selectOrder()
	end
end

function changeDir(cmd)
	if cmd == "left" then
		cordSwitch = cordSwitch + 1
		if cordSwitch > 4 then
			cordSwitch = 1
		end
		turtle.turnLeft()
	end
	
	if cmd == "right" then
		cordSwitch = cordSwitch - 1
		if cordSwitch < 1 then
			cordSwitch = 4
		end
		turtle.turnRight()
	end
	curDir = cordGet[cordSwitch]

end

function moveForward(amount)
	for i = 1, amount do
		turtle.forward()
		if curDir == xPlus then
			x = x + 1
		elseif curDir == xMinus then
			x = x - 1
		elseif curDir == zPlus then
			z = z + 1
		elseif curDir == zMinus then
			z = z - 1
		end
	end
end

function abs(num)
	if num<0 then
		num = num * -1
	end
	return num
end

function orderCompleteMovement()
	if x<curOrder[3] then
		repeat
			changeDir("left")
		until curDir == "xPlus"
	elseif x>curOrder[3] then
		repeat
			changeDir("left")
		until curDir == "xMinus"
	end
	print("go")
	moveForward(abs(x-curOrder[3]))
	
	waitForOrder()
end

function checkDirection()
	local xFir, yFir, zFir = gps.locate()
	
	if not turtle.forward() then
		turtle.dig()
		turtle.forward()
	end
	
	local xSnd, ySnd, zSnd = gps.locate()
	x = xSnd - xFir
	z = zSnd - zFir
	
	turtle.turnLeft()
	turtle.turnLeft()
	
	turtle.forward()
	
	turtle.turnLeft()
	turtle.turnLeft()

	if z == 1 then
		cordSwitch = 3
	elseif z == -1 then 
		cordSwitch = 1
	elseif x == 1 then
		cordSwitch = 4
	elseif x == -1 then
		cordSwitch = 2
	end
	
	curDir = cordGet[cordSwitch]
	print("Current direction: "..curDir)

	x = xFir
	y = yFir
	z = zFir
	
		print("Current cords: x="..x.." z="..z)
end

checkDirection()

connectHost()

parallel.waitForAll(listener, ready)