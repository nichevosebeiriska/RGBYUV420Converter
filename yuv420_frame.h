#ifndef YUV420_FRAME_H
#define YUV420_FRAME_H
#include"BMP_Image.h"


class YUV420_Frame
{

public:
    YUV420_Frame(int16_t w,int16_t h);// создание пустого кадра
    YUV420_Frame(uint16_t w,uint16_t h,ifstream* input);
    YUV420_Frame(YUV420_Frame&& frame_to_move);
    YUV420_Frame(BMP_Image& bmp);

    void addBMP_Image(BMP_Image bmp,int pos_x,int pos_y);
    void addYUV420_Image(YUV420_Frame* frame,int pos_x=0,int pos_y=0);

    void save(const string path);
    ~YUV420_Frame() = default;

    int getWidth() const;
    int getHeight() const;

    std::vector<uint8_t>& get_Y_Data();
    std::vector<uint8_t>& get_V_Data();
    std::vector<uint8_t>& get_U_Data();

protected:
    std::vector<uint8_t>Luma;// Y
    std::vector<uint8_t>U;
    std::vector<uint8_t>V;

    int16_t width,height;
};

// функция конвертации в несколько потоков
YUV420_Frame* multithreadConvertRGBYUV420(const string path_to_bmp);

#endif // YUV420_FRAME_H
