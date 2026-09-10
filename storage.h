#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <chrono>

struct Settings {
    int desktopPetSize=180, obsPetSize=180;
    std::wstring activity=L"ふつう";
    int coinMinMinutes=15, coinMaxMinutes=30;
    int speechMinMinutes=8, speechMaxMinutes=15, maybeFrequency=30;
    bool silentMode=false;
    int focusMinutes=25, breakMinutes=5, pomodoroLoops=4;
    bool effectsEnabled=true; int effectsVolume=65;
    std::wstring alarmTime; bool alarmEnabled=false;
    bool autoStart=true; int obsPort=47831;
};

struct Profile {
    int coins=0;
    std::vector<std::wstring> unlocked;
    std::map<std::wstring,int> counts;
    long long nextCoinUnix=0, nextSpeechUnix=0;
    double lastX=-1, lastY=-1;
};

struct CharacterDef {
    std::wstring id=L"chibidaful", displayName=L"ちび堕ふる";
    std::wstring sprite=L"sprites-v5.png", coin=L"coin-v2.png";
    std::wstring sitWalk=L"animations-sit-walk-v7.png";
    std::wstring petClimb=L"animations-pet-climb-v6.png";
    std::wstring sleepLookTrip=L"animations-sleep-look-trip-v6.png";
    std::wstring dig=L"animations-dig-v3.png";
    std::wstring accessory=L"items-accessories-v3.png", sweet=L"items-sweets-v3.png", toy=L"items-toys-v3.png";
    std::vector<std::wstring> speech, maybeSpeech;
};

std::wstring ExecutableDirectory();
bool LoadSettings(const std::wstring& path, Settings& out);
bool SaveSettings(const std::wstring& path, const Settings& value);
bool LoadProfile(const std::wstring& path, Profile& out);
bool SaveProfile(const std::wstring& path, const Profile& value);
bool LoadCharacter(const std::wstring& path, CharacterDef& out);
void EnsureInitialItems(Profile& value);
void LogLine(const std::wstring& root, const std::wstring& text);

