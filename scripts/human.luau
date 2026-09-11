--!strict

-- Luau can't read from stdin, so we actually just call back into C++ to let it handle input.
function getMove(myPlayerNumber: number) : number
    local column: number = getHumanMove()
    return column
end