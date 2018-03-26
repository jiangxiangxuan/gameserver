
package.path = package.path .. ";common/?.lua;"
local util = require "util"

function main()
        data = {
	        a="a",
	        t={1,2,3}
	}
	
        local sz = util.serialize(data)
        print(sz)
        print(#sz)

end

main()

function handleMsg( data, datalen, time )
        print( "Client::handleMsg "..time.."  "..datalen.."   ==="..data.."" );
end

function handleTimerMsg( id )
        print( "Client::handleTimerMsg"..id );

        delTimer( id );
end
