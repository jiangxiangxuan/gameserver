
projectDir=$(dirname $(pwd))
if [[ "$1" = "build" ]]
then
	echo "build start ..."
	cd $projectDir/Engine/Kernal
	make
    cd $projectDir/Engine/Idbc
    make
    cd $projectDir/Common
    make
    cd $projectDir/CenterServer
    make
    cd $projectDir/DBAServer
    make
    cd $projectDir/GameServer
    make
    cd $projectDir/GatewayServer
    make
    cd $projectDir/LoginServer
    make
	echo "build end ..."	
elif [[ "$1" = "clean" ]]
then
    echo "clean start ..."
    cd $projectDir/Engine/Kernal
    make clean
    cd $projectDir/Engine/Idbc
    make clean
    cd $projectDir/Common
    make clean
    cd $projectDir/CenterServer
    make clean
    cd $projectDir/DBAServer
    make clean
    cd $projectDir/GameServer
    make clean
    cd $projectDir/GatewayServer
    make clean
    cd $projectDir/LoginServer
    make clean
    echo "clean end ..."
else
  	echo "arg error"
fi

