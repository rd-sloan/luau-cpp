print("Hello World!")
local x = 10 + 32
print("x =", x)

local board = getBoard();
-- print(board[6][4]);

-- debug print the board
-- #board here gives the length of the array
for row = 1, #board do
    local line = ""
    for col = 1, #board[row] do
        line = line .. tostring(board[row][col]) .. " "
    end
    print(line)
end