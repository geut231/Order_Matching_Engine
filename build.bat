@echo off
set "PATH=C:\msys64\ucrt64\bin;%PATH%"

echo Building Order Matching Engine with Native HTTP API...
g++ main.cpp Order.cpp OrderBook.cpp MatchingEngine.cpp Database.cpp HttpServer.cpp -I"C:\msys64\ucrt64\include\mysql" -L"C:\msys64\ucrt64\lib" -lmysqlclient -lws2_32 -o engine.exe

if %ERRORLEVEL% equ 0 (
    echo.
    echo =======================================================
    echo  Build Successful: engine.exe
    echo =======================================================
) else (
    echo.
    echo [ERROR] Build failed! Check compiler output above.
)

