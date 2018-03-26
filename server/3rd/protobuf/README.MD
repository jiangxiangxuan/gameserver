protoc-gen-lua的使用

a)将下载好的文件解压C:\protoc-gen-lua，里面有三个文件夹：example，plugin，protobuf。在plugin文件夹下创建protoc-gen-lua.bat文件，写入 @python "%~dp0protoc-gen-lua"，将最开始编译好的protoc.exe文件拷贝到C:\protoc-gen-lua目录下，创建build_for_lua.bat文件，写入（proto_demo是用来存放 proto文件的，方便转化成xx_pb.lua文件）

rem 切换到.proto协议所在的目录
cd C:\protoc-gen-lua\proto_demo
rem 将当前文件夹中的所有协议文件转换为lua文件
for %%i in (*.proto) do (  
echo %%i
"..\protoc.exe" --plugin=protoc-gen-lua="..\plugin\protoc-gen-lua.bat" --lua_out=. %%i

)
echo end
pause


b)执行build_for_lua.bat文件将会得到xx_pb.lua文件，将该lua文件加入到cocos2dx的Resources文件加下，另外将C:\protoc-gen-lua\protobuf目录下的所有lua文件（共9个）全部加入到Resources文件夹下，将C:\protoc-gen-lua\protobuf目录下的pb.c文件加入到class文件夹下。修改pb.c文件的 #include <endian.h>为#ifndef _WIN32? #include <endian.h>? #endif。 函数struct_unpack中修改switch(format)之前的代码为

uint8_t format = luaL_checkinteger(L, 1);
size_t len;
const uint8_t* buffer = (uint8_t*)luaL_checklstring(L, 2, &len);
size_t pos = luaL_checkinteger(L, 3);
uint8_t out[8];   
buffer += pos;


c)lua中使用
	int luaopen_pb(lua_State *L);

	require "person_pb"
	local person_pbeee=person_pb.Person()  
	person_pbeee:ParseFromString(str);  
	print(person_pbeee.id..person_pbeee.name..person_pbeee.email)

#########################################################
编译protoc-gen-lua
#########################################################
cd protobuf && make

#########################################################
编译protobuf
#########################################################
yum install install autoconf automake libtool unzip zip
./autogen.sh
./configure
make
make check
sudo make install
sudo ldconfig # refresh shared library cache





protoc.exe --lua_out=./ --plugin=protoc-gen-lua=../plugin/protoc-gen-lua person.proto

