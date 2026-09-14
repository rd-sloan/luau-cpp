--!strict
-- AI Player that wins if it can see a win,
-- Blocks if it can see the other player is about to win
-- or if there's no win/block, it plays a random move, weighted more towards the center column

--------------------------------------------------------------
------------------------ HELPERS -----------------------------
--------------------------------------------------------------

-- Find all columns that aren't full (top row is empty)
local function getValidColumns(b, numCols: number, emptyMarker: number) : {number}
    local valid = {}
    for col = 1, numCols do
        if b[1][col] == emptyMarker then
            table.insert(valid, col)
        end
    end
    return valid
end

local function getOpponent(player: number) : number
    return (player == 1) and 2 or 1
end

local function centerWeightedMove(validColumns: {number}, numCols: number): number
    -- Assign each column a weight based on distance from center
    -- Center columns get higher weight since they're part of more
    -- possible winning lines (horizontal, both diagonals)

    local centerCol = math.ceil(numCols / 2)  -- e.g. column 4 for a 7-wide board

    local weightedPool = {}  -- will hold column numbers, with center columns repeated more

    for _, col in validColumns do
        local distanceFromCenter = math.abs(col - centerCol)
        local weight = numCols - distanceFromCenter  -- closer to center = bigger number

        -- Add this column to the pool `weight` times,
        -- so it has a proportionally higher chance of being picked
        local count = 1
        repeat
            table.insert(weightedPool, col)
            count = count + 1
        until count > weight
    end

    return weightedPool[math.random(#weightedPool)]
end


--------------------------------------------------------------
--------------actual decision making logic--------------------
--------------------------------------------------------------

function getMove(myPlayerNumber: number) : number
    local board = getBoard()
    local emptyMarker: number = getEmptyMarker()
    local numRows: number = #board
    local numCols: number = #board[1]

    local validColumns = getValidColumns(board, numCols, emptyMarker)
    local opponent = getOpponent(myPlayerNumber)

    -- 1. Can I win this turn?
    for _, col in validColumns do
        if wouldWin(board, col, myPlayerNumber) then
            return col
        end
    end

    -- 2. Can opponent win next turn? Block it.
    for _, col in validColumns do
        if wouldWin(board, col, opponent) then
            return col
        end
    end

    -- 3. Otherwise, try a semi-random move, its weighted more towards center columns
    return centerWeightedMove(validColumns, numCols)
end