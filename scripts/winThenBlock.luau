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

    -- 3. Otherwise, just play randomly among valid columns
    return validColumns[math.random(#validColumns)]
end