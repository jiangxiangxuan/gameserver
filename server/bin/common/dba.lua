
local dba = { }

dba_execute_list	= {}

function dba.execute( sql, func )
    local eventid = executeSql( sql );
    dba_execute_list[eventid] = func;
    return eventid;
end

function dba.handle( error, eventid, row, col, data, datalen )

    local dbRes = DBResult( row, col, data, datalen );

    if dba_execute_list[eventid] ~= nil then
        dba_execute_list[eventid]( eventid, dbRes )
        dba_execute_list[eventid] = nil
    end

    DestoryDBResult( dbRes )
    dbRes = nil

end

return dba
