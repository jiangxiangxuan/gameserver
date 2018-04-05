
pkill -9 centerserver
kill -s 9 `ps -aux | grep centerserver | awk '{print $2}'`

pkill -9 dbaserver
kill -s 9 `ps -aux | grep dbaserver | awk '{print $2}'`

pkill -9 gateway
kill -s 9 `ps -aux | grep gateway | awk '{print $2}'`

pkill -9 loginserver
kill -s 9 `ps -aux | grep loginserver | awk '{print $2}'`

pkill -9 gameserver
kill -s 9 `ps -aux | grep gameserver | awk '{print $2}'`
