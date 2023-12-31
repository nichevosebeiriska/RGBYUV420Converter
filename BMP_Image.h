#ifndef BMP_IMAGE_H
#define BMP_IMAGE_H

#include<fstream>
using namespace std;
#include<stdint.h>
#include"pixel_rgb888.h"
#include<vector>

// позиции первых байтов нужных значений
enum
{
  Ftype         = 0x0000,
  Fsize         = 0x0002,
  FbitsOffset   = 0x000A,

  FmapInfotype  = 0x000E,
  CoreFwidth    = 0x0012,
  CoreFheight,
  CoreFbitsPerPix=0x0018,

  v3FWidth      = 0x0012,
  v3FHeight     = 0x0016,
  v3FbitsPerPix = 0x001C
};

#include<thread>

ifstream* openIfstream(const string path);
ofstream* openOfstream(const string path);
struct BITMAPFILEHEADER
{
    char buffer[14];

    char signature[2];
    int32_t size;
    int32_t offset;
    BITMAPFILEHEADER(){}
    BITMAPFILEHEADER(ifstream& file)
    {
        file.seekg(0);
        file.read(buffer,14);
        file.seekg(0);
        file.read(signature,2);
        size = *(reinterpret_cast<int32_t*>(&buffer[2]));
        offset=*(reinterpret_cast<int32_t*>(&buffer[FbitsOffset]));
    }
};

struct BITMAPINFO
{
    char buffer[124];

    int32_t version;
    int32_t width,height;
    int32_t bitcount;
    BITMAPINFO(){}
    BITMAPINFO(ifstream& file)
    {
        file.seekg(14);
//        int32_t version;
        file.read((char*)&version,4);
        file.seekg(14);
        file.read(buffer,124);

        width =*(reinterpret_cast<int32_t*>(&buffer[v3FWidth-FmapInfotype]));
        height =*(reinterpret_cast<int32_t*>(&buffer[v3FHeight-FmapInfotype]));
        bitcount =*(reinterpret_cast<int32_t*>(&buffer[v3FbitsPerPix-FmapInfotype]));


    }
};
class BMP_Image
{
    std::vector<pixel_rgb888> pixel_data;

    BITMAPFILEHEADER header;
    BITMAPINFO       info;

    enum Version
    {
        CORE = 12,
        v3 = 24,
        v4 = 108,
        v5 = 124
    };

public:
    BMP_Image(BMP_Image& bmp);
    BMP_Image(const string path_to_file);
    ~BMP_Image();


    void saveAsYUV420(const string path);
    void turnHorizontaly();
    void clamp();// делает стороны четными
    BITMAPFILEHEADER getHeader() const;
    BITMAPINFO getInfo()         const;
    vector<pixel_rgb888>*  data();
    void save(const string path_to_save,BITMAPFILEHEADER Header,BITMAPINFO Info);
};


#endif // BMP_IMAGE_H
