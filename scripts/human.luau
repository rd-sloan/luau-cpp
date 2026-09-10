--print("Hello World!")
--local x = 10 + 32
--print("x =", x)

--local board = getBoard();
-- print(board[6][4]);

-- debug print the board
-- #board here gives the length of the array
--for row = 1, #board do
--    local line = ""
--    for col = 1, #board[row] do
--        line = line .. tostring(board[row][col]) .. " "
--    end
--    print(line)
--end


function getMove(myPlayerNumber: number) : number
    ---- logic ----
    local board = getBoard();
    local column = 5
    print(`fake human playing column {column}`)
    return column
end