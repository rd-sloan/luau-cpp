--!strict
-- AI Player that wins if it can see a win,
-- Blocks if it can see the other player is about to win
-- or plays randomly if no obvious move.

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

-- Deep-copy the board so we can simulate without touching the real one
local function copyBoard(b, numCols: number, numRows: number, emptyMarker: number)
    local copy = {}
    for row = 1, numRows do
        copy[row] = {}
        for col = 1, numCols do
            copy[row][col] = b[row][col]
        end
    end
    return copy
end

-- Simulate dropping a piece into a column (mimics gravity)
-- TODO: We already have this logic in C++, could rework and expose a function here.
local function drop(b, numCols: number, numRows: number, emptyMarker: number, col: number, player: number) : boolean
    for row = numRows, 1, -1 do
        if b[row][col] == emptyMarker then
            b[row][col] = player
            return true
        end
    end
    return false -- column was full, shouldn't happen if col came from getValidColumns
end

-- Check if `player` has a 4-in-a-row anywhere on the 
-- TODO: We already have this logic in C++, could rework and expose a function here.
local function checkWin(b, numCols: number, numRows: number, player: number) : boolean
    -- Horizontal
    for row = 1, numRows do
        for col = 1, numCols - 3 do
            if b[row][col] == player and b[row][col+1] == player
                and b[row][col+2] == player and b[row][col+3] == player then
                return true
            end
        end
    end

    -- Vertical
    for col = 1, numCols do
        for row = 1, numRows - 3 do
            if b[row][col] == player and b[row+1][col] == player
                and b[row+2][col] == player and b[row+3][col] == player then
                return true
            end
        end
    end

    -- Diagonal down-right (\)
    for row = 1, numRows - 3 do
        for col = 1, numCols - 3 do
            if b[row][col] == player and b[row+1][col+1] == player
                and b[row+2][col+2] == player and b[row+3][col+3] == player then
                return true
            end
        end
    end

    -- Diagonal down-left (/)
    for row = 1, numRows - 3 do
        for col = 4, numCols do
            if b[row][col] == player and b[row+1][col-1] == player
                and b[row+2][col-2] == player and b[row+3][col-3] == player then
                return true
            end
        end
    end

    return false
end

local function getOpponent(player: number) : number
    return (player == 1) and 2 or 1
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
        local simulated = copyBoard(board, numCols, numRows, emptyMarker)
        drop(simulated, numCols, numRows, emptyMarker, col, myPlayerNumber)
        if checkWin(simulated, numCols, numRows, myPlayerNumber) then
            return col
        end
    end

    -- 2. Can opponent win next turn? Block it.
    for _, col in validColumns do
        local simulated = copyBoard(board, numCols, numRows, emptyMarker)
        drop(simulated, numCols, numRows, emptyMarker, col, opponent)
        if checkWin(simulated, numCols, numRows, opponent) then
            return col
        end
    end

    -- 3. Otherwise, just play randomly among valid columns
    return validColumns[math.random(#validColumns)]
end