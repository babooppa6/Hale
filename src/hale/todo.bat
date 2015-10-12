@echo off

echo -------
echo -------

set Wildcard=hale_*.h hale_*.cpp hale_*.inl hale_*.c

echo TODOS FOUND:
findstr -s -n -i -l "TODO" %Wildcard%

echo -------
echo -------
