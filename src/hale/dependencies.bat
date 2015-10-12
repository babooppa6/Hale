@echo off

echo -------
echo -------

set Wildcard=hale_*.h hale_*.cpp hale_*.inl hale_*.c

findstr -s -n "Q[A-Za-z0-9]*" %Wildcard%

echo -------
echo -------
