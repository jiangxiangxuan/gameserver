
package.path = package.path .. ";../common/?.lua;"
local util  = require "util"
local timer = require "timer"
local dba   = require "dba"

function timerTest( id )
    print("timer test  "..id)
    local eventid = dba.execute("select * from test", dbaTest);
    print("eventid = " .. eventid)
end

function dbaTest( eventid, dbRes )
    while( dbRes:nextRow() )
    do
        print( "event id ="..eventid .. "  id="..dbRes:getData(0).."  name="..dbRes:getData(1) )
    end
end


function main()

    local timerID = timer.add( 100, -1, timerTest )
    print("timer id = " .. timerID)
    --local eventid = dba.execute("select * from test", dbaTest);
    --print("eventid = " .. eventid)

    data = {
	    a="a",
	    t={1,2,3}
	}

    local sz = util.serialize(data)
    print(sz)
    print(#sz)

end

main()

function handleMsg( session, clientID, data, datalen )
    sendClientMsg( session, clientID, data, datalen )
    print( data );
    local eventid = executeSql("select * from test");
    print("eventid = " .. eventid)

    local msg = "platform data to game test"
    sendGameMsg( 0, msg, #msg )
end

function handleDBAMsg( error, eventid, row, col, data, datalen )
    dba.handle( error, eventid, row, col, data, datalen )
end

function handleGameMsg( serverID, data, datalen )
    print( "handleGameMsg data="..data );
end

function handleTimerMsg( id )
    timer.handle( id )
    timer.del( id );
end
