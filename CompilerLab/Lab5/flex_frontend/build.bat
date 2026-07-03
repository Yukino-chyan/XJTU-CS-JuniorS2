@echo off
REM Build the flex + bison compiler frontend (the Lab5 optional part).
REM Run this from the Lab5\flex_frontend directory.
REM win_flex.exe / win_bison.exe live in ..\..\flex_bison (they need their data\ skeletons).

set TOOLS=..\..\flex_bison

echo [1/3] bison: generating parser...
%TOOLS%\win_bison.exe -d lab5.y
if errorlevel 1 goto :err

echo [2/3] flex: generating lexer...
%TOOLS%\win_flex.exe lab5.l
if errorlevel 1 goto :err

echo [3/3] g++: compiling...
g++ -std=c++17 lab5.tab.c lex.yy.c -o lab5_frontend.exe
if errorlevel 1 goto :err

echo.
echo Build OK: lab5_frontend.exe
echo Usage: .\lab5_frontend.exe ^<path-to-source.src^>
goto :eof

:err
echo Build failed.
exit /b 1
