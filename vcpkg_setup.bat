@echo off
pushd %~dp0

echo Running vcpkg bootstrap...
call .\vcpkg\bootstrap-vcpkg.bat

echo Updating repository...
call git pull

echo Integrating vcpkg with Visual Studio...
call .\vcpkg\vcpkg integrate install

echo Installing GameNetworkingSockets...
call .\vcpkg\vcpkg install GameNetworkingSockets --triplet=x64-windows
call .\vcpkg\vcpkg install GameNetworkingSockets --triplet=x86-windows

echo Installing Protobuf...
call .\vcpkg\vcpkg install protobuf --triplet=x64-windows
call .\vcpkg\vcpkg install protobuf --triplet x86-windows

popd