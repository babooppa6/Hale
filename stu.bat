@echo off
pushd src\hale
..\..\include.exe . hale_*.cpp _stu.cpp -r -q
popd