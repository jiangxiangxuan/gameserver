
package.path = package.path .. ";../protobuf/?.lua;"

local protobuf = require "protobuf"

-- pbc protobuf 测试
function test()
    addr = io.open("../protocol/person.pb","rb")
    buffer = addr:read "*a"
    addr:close()    
    protobuf.register(buffer)
    person = {
        name = "name_test",
        id = 12345,
        email="email_test",
    }
    
    code = protobuf.encode("test.Person", person)    
    decode = protobuf.decode("test.Person" , code)
    
    print(decode.name)
    print(decode.id)
    
end

function main()
    --[[
    local locker = Locker()
    locker:lock()
    locker:unlock()
    DestoryLocker( locker )
    locker = nil     
    --]]  
    test()
    
    local timerID = addTimer( 100, -1 )
    print("timer id = " .. timerID)   
end

main()

function handleMsg( session, clientID, data, datalen )
    sendClientMsg( session, clientID, data, datalen )
    print( data );
end

function handlePlatformMsg( serverID, data, datalen )
    print( "handlePlatformMsg data="..data );
end

function handleTimerMsg( id )
    print( id );
        
    delTimer( id );
end
