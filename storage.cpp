#include "storage.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <iomanip>
#include <algorithm>

static std::string ReadUtf8(const std::wstring& path) {
    std::ifstream f(path, std::ios::binary); if (!f) return {};
    return std::string(std::istreambuf_iterator<char>(f), {});
}
static std::wstring Widen(const std::string& s) {
    if (s.empty()) return {};
    int n=MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),nullptr,0); std::wstring w(n,L'\0');
    MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),w.data(),n); return w;
}
static std::string Narrow(const std::wstring& w) {
    if (w.empty()) return {};
    int n=WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),nullptr,0,nullptr,nullptr); std::string s(n,'\0');
    WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),s.data(),n,nullptr,nullptr); return s;
}
static std::string Escape(const std::wstring& w) {
    std::string s=Narrow(w), o; for(char c:s){ if(c=='\\'||c=='"')o+='\\'; if(c=='\n'){o+="\\n";continue;} o+=c;} return o;
}
static bool WriteUtf8(const std::wstring& path,const std::string& s){std::ofstream f(path,std::ios::binary|std::ios::trunc);if(!f)return false;f.write(s.data(),s.size());return !!f;}
static std::string Capture(const std::string& j,const char* key,const std::string& def=""){
    std::regex r(std::string("\\\"")+key+"\\\"\\s*:\\s*\\\"([^\\\"]*)\\\"");std::smatch m;return std::regex_search(j,m,r)?m[1].str():def;
}
static long long Number(const std::string& j,const char* key,long long def){std::regex r(std::string("\\\"")+key+"\\\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?)");std::smatch m;return std::regex_search(j,m,r)?std::stoll(m[1].str()):def;}
static double Real(const std::string& j,const char* key,double def){std::regex r(std::string("\\\"")+key+"\\\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?)");std::smatch m;return std::regex_search(j,m,r)?std::stod(m[1].str()):def;}
static bool Flag(const std::string& j,const char* key,bool def){std::regex r(std::string("\\\"")+key+"\\\"\\s*:\\s*(true|false)",std::regex::icase);std::smatch m;return std::regex_search(j,m,r)?m[1].str()[0]=='t'||m[1].str()[0]=='T':def;}
static long long NowUnix(){return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();}
static long long DateUnix(const std::string& j,const char* key,long long def){std::regex r(std::string("\\\"")+key+"\\\"\\s*:\\s*\\\"?\\\\?/Date\\((-?[0-9]+)");std::smatch m;return std::regex_search(j,m,r)?std::stoll(m[1].str())/1000:def;}

std::wstring ExecutableDirectory(){wchar_t b[MAX_PATH*4]{};GetModuleFileNameW(nullptr,b,(DWORD)std::size(b));std::wstring p=b;size_t x=p.find_last_of(L"\\/");return x==std::wstring::npos?L".":p.substr(0,x);}

bool LoadSettings(const std::wstring& path,Settings& v){std::string j=ReadUtf8(path);if(j.empty())return false;
    v.desktopPetSize=(int)Number(j,"DesktopPetSize",v.desktopPetSize);v.obsPetSize=(int)Number(j,"ObsPetSize",v.obsPetSize);v.activity=Widen(Capture(j,"ActivityLevel","ふつう"));
    v.coinMinMinutes=(int)Number(j,"CoinMinMinutes",15);v.coinMaxMinutes=(int)Number(j,"CoinMaxMinutes",30);v.speechMinMinutes=(int)Number(j,"SpeechMinMinutes",8);v.speechMaxMinutes=(int)Number(j,"SpeechMaxMinutes",15);v.maybeFrequency=(int)Number(j,"MaybeFrequency",30);v.silentMode=Flag(j,"SilentMode",false);
    v.focusMinutes=(int)Number(j,"FocusMinutes",25);v.breakMinutes=(int)Number(j,"BreakMinutes",5);v.pomodoroLoops=(int)Number(j,"PomodoroLoops",4);v.effectsEnabled=Flag(j,"EffectsEnabled",true);v.effectsVolume=(int)Number(j,"EffectsVolume",65);
    v.alarmTime=Widen(Capture(j,"AlarmTime"));v.alarmEnabled=Flag(j,"AlarmEnabled",false);v.autoStart=Flag(j,"AutoStart",true);v.obsPort=(int)Number(j,"ObsPort",47831);return true;}

bool SaveSettings(const std::wstring& path,const Settings& v){std::ostringstream o;o<<"{\r\n"
<<"  \"DesktopPetSize\": "<<v.desktopPetSize<<",\r\n  \"ObsPetSize\": "<<v.obsPetSize<<",\r\n  \"ActivityLevel\": \""<<Escape(v.activity)<<"\",\r\n"
<<"  \"CoinMinMinutes\": "<<v.coinMinMinutes<<",\r\n  \"CoinMaxMinutes\": "<<v.coinMaxMinutes<<",\r\n  \"SpeechMinMinutes\": "<<v.speechMinMinutes<<",\r\n  \"SpeechMaxMinutes\": "<<v.speechMaxMinutes<<",\r\n  \"MaybeFrequency\": "<<v.maybeFrequency<<",\r\n  \"SilentMode\": "<<(v.silentMode?"true":"false")<<",\r\n"
<<"  \"FocusMinutes\": "<<v.focusMinutes<<",\r\n  \"BreakMinutes\": "<<v.breakMinutes<<",\r\n  \"PomodoroLoops\": "<<v.pomodoroLoops<<",\r\n  \"EffectsEnabled\": "<<(v.effectsEnabled?"true":"false")<<",\r\n  \"EffectsVolume\": "<<v.effectsVolume<<",\r\n"
<<"  \"AlarmTime\": \""<<Escape(v.alarmTime)<<"\",\r\n  \"AlarmEnabled\": "<<(v.alarmEnabled?"true":"false")<<",\r\n  \"AlarmDueLocal\": null,\r\n  \"AutoStart\": "<<(v.autoStart?"true":"false")<<",\r\n  \"ObsPort\": "<<v.obsPort<<"\r\n}\r\n";return WriteUtf8(path,o.str());}

bool LoadProfile(const std::wstring& path,Profile& v){std::string j=ReadUtf8(path);if(j.empty()){v.nextCoinUnix=NowUnix()+900;v.nextSpeechUnix=NowUnix()+480;return false;}v.coins=(int)Number(j,"Coins",0);v.lastX=Real(j,"LastPetX",-1);v.lastY=Real(j,"LastPetY",-1);v.nextCoinUnix=DateUnix(j,"NextCoinAtUtc",NowUnix()+900);v.nextSpeechUnix=DateUnix(j,"NextSpeechAtUtc",NowUnix()+480);
    std::regex ar("\\\"UnlockedItems\\\"\\s*:\\s*\\[([^\\]]*)\\]");std::smatch m;if(std::regex_search(j,m,ar)){std::regex q("\\\"([^\\\"]+)\\\"");auto b=std::sregex_iterator(m[1].first,m[1].second,q),e=std::sregex_iterator();for(;b!=e;++b)v.unlocked.push_back(Widen((*b)[1].str()));}
    std::regex pair("\\{\\s*\\\"Key\\\"\\s*:\\s*\\\"([^\\\"]+)\\\"\\s*,\\s*\\\"Value\\\"\\s*:\\s*([0-9]+)\\s*\\}");for(auto b=std::sregex_iterator(j.begin(),j.end(),pair),e=std::sregex_iterator();b!=e;++b)v.counts[Widen((*b)[1].str())]=std::stoi((*b)[2].str());return true;}

bool SaveProfile(const std::wstring& path,const Profile& v){std::ostringstream o;o<<"{\r\n  \"Coins\": "<<v.coins<<",\r\n  \"UnlockedItems\": [";for(size_t i=0;i<v.unlocked.size();++i){if(i)o<<",";o<<"\""<<Escape(v.unlocked[i])<<"\"";}o<<"],\r\n  \"ItemCounts\": [";size_t i=0;for(auto& p:v.counts){if(i++)o<<",";o<<"{\"Key\":\""<<Escape(p.first)<<"\",\"Value\":"<<p.second<<"}";}o<<"],\r\n  \"NextCoinAtUtc\": \"\\/Date("<<v.nextCoinUnix*1000<<")\\/\",\r\n  \"NextSpeechAtUtc\": \"\\/Date("<<v.nextSpeechUnix*1000<<")\\/\",\r\n  \"LastPetX\": "<<v.lastX<<",\r\n  \"LastPetY\": "<<v.lastY<<"\r\n}\r\n";return WriteUtf8(path,o.str());}

static std::vector<std::wstring> ArrayStrings(const std::string& j,const char* key){std::vector<std::wstring> out;std::regex ar(std::string("\\\"")+key+"\\\"\\s*:\\s*\\[([^\\]]*)\\]");std::smatch m;if(!std::regex_search(j,m,ar))return out;std::regex q("\\\"([^\\\"]*)\\\"");for(auto b=std::sregex_iterator(m[1].first,m[1].second,q),e=std::sregex_iterator();b!=e;++b)out.push_back(Widen((*b)[1].str()));return out;}
static std::string ObjectBody(const std::string& j,const char* key){std::regex r(std::string("\\\"")+key+"\\\"\\s*:\\s*\\{([^}]*)\\}");std::smatch m;return std::regex_search(j,m,r)?m[1].str():std::string();}
bool LoadCharacter(const std::wstring& path,CharacterDef& v){std::string j=ReadUtf8(path);if(j.empty())return false;v.id=Widen(Capture(j,"id","chibidaful"));v.displayName=Widen(Capture(j,"displayName","ちび堕ふる"));v.sprite=Widen(Capture(j,"spriteSheet","sprites-v5.png"));v.coin=Widen(Capture(j,"coinFile","coin-v2.png"));v.sitWalk=Widen(Capture(ObjectBody(j,"sitWalk"),"file",Narrow(v.sitWalk)));v.petClimb=Widen(Capture(ObjectBody(j,"petClimb"),"file",Narrow(v.petClimb)));v.sleepLookTrip=Widen(Capture(ObjectBody(j,"sleepLookTrip"),"file",Narrow(v.sleepLookTrip)));v.dig=Widen(Capture(ObjectBody(j,"dig"),"file",Narrow(v.dig)));v.accessory=Widen(Capture(j,"accessory",Narrow(v.accessory)));v.sweet=Widen(Capture(j,"sweet",Narrow(v.sweet)));v.toy=Widen(Capture(j,"toy",Narrow(v.toy)));v.speech=ArrayStrings(j,"normalSpeech");v.maybeSpeech=ArrayStrings(j,"maybeSpeech");return true;}
void EnsureInitialItems(Profile& v){const wchar_t* ids[]={L"aqua_ribbon",L"cookie",L"cat_teaser"};for(auto id:ids){if(std::find(v.unlocked.begin(),v.unlocked.end(),id)==v.unlocked.end())v.unlocked.push_back(id);if(!v.counts.count(id))v.counts[id]=1;}}
void LogLine(const std::wstring& root,const std::wstring& text){SYSTEMTIME t{};GetLocalTime(&t);std::wofstream f(root+L"\\DesktopChibiFriend.log",std::ios::app);if(f)f<<std::setfill(L'0')<<std::setw(4)<<t.wYear<<L"-"<<std::setw(2)<<t.wMonth<<L"-"<<std::setw(2)<<t.wDay<<L" "<<std::setw(2)<<t.wHour<<L":"<<std::setw(2)<<t.wMinute<<L":"<<std::setw(2)<<t.wSecond<<L" "<<text<<L"\n";}
