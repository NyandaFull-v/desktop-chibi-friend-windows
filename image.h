#pragma once
#include <windows.h>
#include <wincodec.h>
#include <string>

struct PngFrame {
    HBITMAP bitmap{};
    void* pixels{};
    int width{};
    int height{};
    PngFrame() = default;
    PngFrame(const PngFrame&) = delete;
    PngFrame& operator=(const PngFrame&) = delete;
    PngFrame(PngFrame&& other) noexcept { *this = std::move(other); }
    PngFrame& operator=(PngFrame&& other) noexcept {
        if (this != &other) { reset(); bitmap=other.bitmap; pixels=other.pixels; width=other.width; height=other.height; other.bitmap=nullptr; other.pixels=nullptr; }
        return *this;
    }
    ~PngFrame() { reset(); }
    void reset() { if (bitmap) DeleteObject(bitmap); bitmap=nullptr; pixels=nullptr; width=height=0; }
};

class ImageLoader {
public:
    bool initialize();
    void shutdown();
    bool loadCell(const std::wstring& file, int columns, int rows, int index, int targetSize, PngFrame& out);
private:
    IWICImagingFactory* factory_{};
};

bool PresentLayered(HWND hwnd, const PngFrame& frame, POINT screenPosition, BYTE alpha = 255);
void MirrorFrame(PngFrame& frame);
bool CreateCaveFrame(int size, PngFrame& out);
