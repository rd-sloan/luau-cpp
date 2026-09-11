--!strict

-- AI Player that randomly places a piece in any open column
function getMove(myPlayerNumber: number) : number
    local validColumns = {}
    local board = getBoard()
    local emptyMarker: number = getEmptyMarker()


    -- find all valid columns
    for col = 1, #board[1] do
        if board[1][col] == emptyMarker then
            table.insert(validColumns, col)
        end
    end

    -- return random valid column
    -- todo could error check here if no valid columns
    return validColumns[math.random(#validColumns)]   
end