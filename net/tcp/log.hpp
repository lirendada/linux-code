#pragma once
#include <iostream>
#include <string>
using namespace std;

enum Level{
    DEBUG = 0,
    NORMAL,
    WARING, 
    ERROR,
    FATAL
};

void logMessage(int level, const string& message)
{
    cout << "[" << level << "] [" << message << "]" << endl;
}