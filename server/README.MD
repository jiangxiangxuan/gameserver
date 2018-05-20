# server
运行时需要执行以下命令设置动态库所在路径:  
vim ~/.bash_profile  
export LD_LIBRARY_PATH=~/gameserver/server/Lib:$LD_LIBRARY_PATH  
export LUA_CPATH=~/gameserver/server/Lib/?.so  #LUA_PATH  

## 模块介绍
Engine/Kernal:核心  
Engine/Idbc:数据库接口  
External:第三方库头/源文件  
Lib:库文件  
CenterServer:中心服务器  
DBAServer:数据库服务器  
GameServer:游戏服务器  
GatewayServer:网关服务器  
LoginServer:登录服务器  
