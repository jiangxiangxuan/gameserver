# jephy  SERVER
=========================================================

运行时需要执行以下命令设置动态库所在路径:
vim ~/.bash_profile
export LD_LIBRARY_PATH=~/gameserver/server/Lib:$LD_LIBRARY_PATH
export LUA_CPATH=/home/workspace/game-code/server/Lib/?.so
#LUA_PATH

=========================================================

Game/GatewayServer:网关服务器
Game/GameServer:游戏服务器
Game/DBServer:数据库服务器
Kernal:核心
Idbc:数据库接口
External:第三方库头/源文件
Lib:库文件
