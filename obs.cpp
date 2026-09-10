#include <winsock2.h>
#include <ws2tcpip.h>
#include "obs.h"
#include <tlhelp32.h>
#include <fstream>
#include <sstream>
#include <algorithm>

static std::string Mime(const std::string& p){if(p.ends_with(".png"))return "image/png";if(p.ends_with(".json"))return "application/json; charset=utf-8";return "text/html; charset=utf-8";}
static std::string FileBytes(const std::wstring& p){std::ifstream f(p,std::ios::binary);return f?std::string(std::istreambuf_iterator<char>(f),{}):std::string();}
static std::wstring WidenPath(const std::string& s){int n=MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),nullptr,0);std::wstring w(n,L'\0');MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),w.data(),n);return w;}

ObsServer::~ObsServer(){stop();}
bool ObsServer::start(int preferredPort,const std::wstring& root,int petSize,int coinMinMinutes,int coinMaxMinutes){if(running_)return true;WSADATA w{};if(WSAStartup(MAKEWORD(2,2),&w))return false;root_=root;petSize_=petSize;coinMinMinutes_=coinMinMinutes;coinMaxMinutes_=coinMaxMinutes;
    SOCKET s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);if(s==INVALID_SOCKET)return false;BOOL yes=TRUE;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(yes));
    for(int p=preferredPort;p<preferredPort+20;++p){sockaddr_in a{};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);a.sin_port=htons((u_short)p);if(bind(s,(sockaddr*)&a,sizeof(a))==0){port_=p;break;}}
    if(!port_||listen(s,4)){closesocket(s);return false;}listener_=(uintptr_t)s;running_=true;thread_=std::thread(&ObsServer::run,this);return true;}
void ObsServer::stop(){if(!running_.exchange(false))return;SOCKET s=(SOCKET)listener_;shutdown(s,SD_BOTH);closesocket(s);if(thread_.joinable())thread_.join();listener_=0;WSACleanup();}
void ObsServer::run(){while(running_){SOCKET c=accept((SOCKET)listener_,nullptr,nullptr);if(c==INVALID_SOCKET)break;char b[4096]{};int n=recv(c,b,sizeof(b)-1,0);if(n>0){std::string r=responseFor(std::string(b,n));send(c,r.data(),(int)r.size(),0);}shutdown(c,SD_BOTH);closesocket(c);}}
std::string ObsServer::responseFor(const std::string& request){std::string path="/";size_t a=request.find(' '),b=a==std::string::npos?a:request.find(' ',a+1);if(a!=std::string::npos&&b!=std::string::npos)path=request.substr(a+1,b-a-1);if(path=="/"||path.rfind("/?",0)==0){
    std::ostringstream h;h<<"<!doctype html><meta charset=utf-8><style>html,body{margin:0;overflow:hidden;background:transparent}#p{position:fixed;width:"<<petSize_<<"px;height:"<<petSize_<<"px;background:url('/characters/chibidaful/sprites-v5.png') 0 0/300% 300% no-repeat;left:20px;bottom:0;animation:w 18s linear infinite}@keyframes w{0%{left:2%;bottom:0;transform:scaleX(1)}46%{left:calc(98% - "<<petSize_<<"px);bottom:0;transform:scaleX(1)}49%{bottom:28%}52%{bottom:0;transform:scaleX(-1)}96%{left:2%;bottom:0;transform:scaleX(-1)}99%{bottom:28%}100%{bottom:0;transform:scaleX(1)}}.coin{position:fixed;width:48px;height:48px;object-fit:contain;animation:f 7s linear forwards}@keyframes f{from{top:-60px}to{top:calc(100vh - 50px)}}</style><div id=p></div><script>const lo="<<coinMinMinutes_*60000<<",hi="<<coinMaxMinutes_*60000<<";function coin(){let c=document.createElement('img');c.className='coin';c.src='/characters/chibidaful/coin-v2.png';c.style.left=Math.random()*90+'%';document.body.append(c);setTimeout(()=>c.remove(),8000);setTimeout(coin,lo+Math.random()*(hi-lo))}setTimeout(coin,lo+Math.random()*(hi-lo))</script>";
    std::string body=h.str(),head="HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nCache-Control: no-store\r\nContent-Length: "+std::to_string(body.size())+"\r\n\r\n";return head+body;}
    while(!path.empty()&&path[0]=='/')path.erase(path.begin());if(path.find("..")!=std::string::npos)return "HTTP/1.1 403 Forbidden\r\nContent-Length:0\r\n\r\n";std::replace(path.begin(),path.end(),'/', '\\');std::string body=FileBytes(root_+L"\\"+WidenPath(path));if(body.empty())return "HTTP/1.1 404 Not Found\r\nContent-Length:0\r\n\r\n";return "HTTP/1.1 200 OK\r\nContent-Type: "+Mime(path)+"\r\nCache-Control: public,max-age=3600\r\nContent-Length: "+std::to_string(body.size())+"\r\n\r\n"+body;}

bool IsObsRunning(){bool found=false;HANDLE h=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(h==INVALID_HANDLE_VALUE)return false;PROCESSENTRY32W e{sizeof(e)};if(Process32FirstW(h,&e)){do{if(_wcsicmp(e.szExeFile,L"obs64.exe")==0||_wcsicmp(e.szExeFile,L"obs32.exe")==0){found=true;break;}}while(Process32NextW(h,&e));}CloseHandle(h);return found;}
