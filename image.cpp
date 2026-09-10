#include "image.h"
#include <algorithm>
#include <cmath>
#include <cstring>

bool ImageLoader::initialize() {
    return SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory_)));
}

void ImageLoader::shutdown() { if (factory_) { factory_->Release(); factory_ = nullptr; } }

bool ImageLoader::loadCell(const std::wstring& file, int columns, int rows, int index, int targetSize, PngFrame& out) {
    out.reset();
    if (!factory_ || columns < 1 || rows < 1 || index < 0 || index >= columns * rows || targetSize < 1) return false;
    IWICBitmapDecoder* decoder{}; IWICBitmapFrameDecode* source{}; IWICBitmapClipper* clipper{}; IWICBitmapScaler* scaler{}; IWICFormatConverter* converter{};
    HRESULT hr = factory_->CreateDecoderFromFilename(file.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
    if (SUCCEEDED(hr)) hr = decoder->GetFrame(0, &source);
    UINT sw=0, sh=0; if (SUCCEEDED(hr)) hr = source->GetSize(&sw, &sh);
    const UINT cw = sw / static_cast<UINT>(columns), ch = sh / static_cast<UINT>(rows);
    WICRect rect{ static_cast<INT>((index % columns) * cw), static_cast<INT>((index / columns) * ch), static_cast<INT>(cw), static_cast<INT>(ch) };
    if (SUCCEEDED(hr)) hr = factory_->CreateBitmapClipper(&clipper);
    if (SUCCEEDED(hr)) hr = clipper->Initialize(source, &rect);
    if (SUCCEEDED(hr)) hr = factory_->CreateBitmapScaler(&scaler);
    if (SUCCEEDED(hr)) hr = scaler->Initialize(clipper, targetSize, targetSize, WICBitmapInterpolationModeFant);
    if (SUCCEEDED(hr)) hr = factory_->CreateFormatConverter(&converter);
    if (SUCCEEDED(hr)) hr = converter->Initialize(scaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    BITMAPINFO bi{}; bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER); bi.bmiHeader.biWidth=targetSize; bi.bmiHeader.biHeight=-targetSize; bi.bmiHeader.biPlanes=1; bi.bmiHeader.biBitCount=32; bi.bmiHeader.biCompression=BI_RGB;
    HDC dc = GetDC(nullptr); void* bits{}; HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0); ReleaseDC(nullptr, dc);
    if (!bmp) hr = E_OUTOFMEMORY;
    if (SUCCEEDED(hr)) hr = converter->CopyPixels(nullptr, targetSize * 4, targetSize * targetSize * 4, static_cast<BYTE*>(bits));
    if (SUCCEEDED(hr)) { out.bitmap=bmp; out.pixels=bits; out.width=out.height=targetSize; } else if (bmp) DeleteObject(bmp);
    if (converter) converter->Release(); if (scaler) scaler->Release(); if (clipper) clipper->Release(); if (source) source->Release(); if (decoder) decoder->Release();
    return SUCCEEDED(hr);
}

bool PresentLayered(HWND hwnd, const PngFrame& frame, POINT pos, BYTE alpha) {
    if (!hwnd || !frame.bitmap) return false;
    HDC screen=GetDC(nullptr), mem=CreateCompatibleDC(screen); HGDIOBJ old=SelectObject(mem, frame.bitmap);
    SIZE size{frame.width,frame.height}; POINT src{}; BLENDFUNCTION blend{AC_SRC_OVER,0,alpha,AC_SRC_ALPHA};
    BOOL ok=UpdateLayeredWindow(hwnd,screen,&pos,&size,mem,&src,0,&blend,ULW_ALPHA);
    SelectObject(mem,old); DeleteDC(mem); ReleaseDC(nullptr,screen); return ok!=FALSE;
}

void MirrorFrame(PngFrame& f){if(!f.pixels)return;auto*p=(DWORD*)f.pixels;for(int y=0;y<f.height;++y)for(int x=0;x<f.width/2;++x)std::swap(p[y*f.width+x],p[y*f.width+f.width-1-x]);}

bool CreateCaveFrame(int size,PngFrame& out){out.reset();BITMAPINFO bi{};bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=size;bi.bmiHeader.biHeight=-size;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;HDC dc=GetDC(nullptr);void*bits{};HBITMAP bmp=CreateDIBSection(dc,&bi,DIB_RGB_COLORS,&bits,nullptr,0);ReleaseDC(nullptr,dc);if(!bmp)return false;auto*p=(DWORD*)bits;memset(p,0,size*size*4);double cx=size*.5,cy=size*.58,rx=size*.45,ry=size*.42;for(int y=0;y<size;++y)for(int x=0;x<size;++x){double q=std::sqrt(((x-cx)*(x-cx))/(rx*rx)+((y-cy)*(y-cy))/(ry*ry));BYTE a=0,r=0,g=0,b=0;if(q<1){a=255;if(q>.78){r=157;g=130;b=116;}else if(q>.64){r=225;g=190;b=174;}else{r=102;g=225;b=238;}if(q<.55){r=31;g=53;b=72;}}if(a)p[y*size+x]=(a<<24)|(r<<16)|(g<<8)|b;}out.bitmap=bmp;out.pixels=bits;out.width=out.height=size;return true;}
