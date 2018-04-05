
curdir=$(pwd)

cd $curdir/center
pkill -9 centerserver
kill -s 9 `ps -aux | grep centerserver | awk '{print $2}'`
setsid ./centerserver &

cd $curdir/dba
pkill -9 dbaserver
kill -s 9 `ps -aux | grep dbaserver | awk '{print $2}'`
setsid ./dbaserver &

cd $curdir/gateway
pkill -9 gateway
kill -s 9 `ps -aux | grep gateway | awk '{print $2}'`
setsid ./gateway &

cd $curdir/login
pkill -9 loginserver
kill -s 9 `ps -aux | grep loginserver | awk '{print $2}'`
setsid ./loginserver &

cd $curdir/game
pkill -9 gameserver
kill -s 9 `ps -aux | grep gameserver | awk '{print $2}'`
setsid ./gameserver &
