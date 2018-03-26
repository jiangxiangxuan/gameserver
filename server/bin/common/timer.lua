
local timer = {}

timer_list	= {}

function timer.add( expire, times, func )

    local timerID = addTimer( expire, times )
    timer_list[timerID] = func
    return timerID

end

function timer.del( id )

    delTimer( id );
    if timer_list[id] ~= nil then
        timer_list[id] = nil
    end

end

function timer.handle( id )

    if timer_list[id] ~= nil then
        timer_list[id]( id )
    end

end

return timer
