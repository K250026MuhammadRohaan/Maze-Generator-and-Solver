@echo off
g++ -std=c++17 -O2 -Wall -Wextra main.cpp -o maze.exe
if %errorlevel%==0 (echo Build OK - run maze.exe) else (echo Build failed)
