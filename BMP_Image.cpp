#include"BMP_Image.h"
#include <cstring>
#include"xassert.h"

ifstream* openIfstream(const string path)
{
    ifstream* file = new ifstream(path,ios::binary);
    xassert(file->is_open(),"ifstream* openIfstream(const string path)::error::can`t open file");
    return file;
}
ofstream* openOfstream(const string path)
{
    ofstream* file = new ofstream(path,ios::binary);
    xassert(file->is_open(),"ofstream* openOfstream(const string path)::error::can`t open file");
    return file;
}
BMP_Image::BMP_Image(BMP_Image& bmp)
{
    header = bmp.header;
    info = bmp.info;
    pixel_data = bmp.pixel_data;
}
BMP_Image::BMP_Image(const string path_to_file)
{
    ifstream input(path_to_file,ios::binary);
    if(input.is_open())
    {

        header = BITMAPFILEHEADER(input);
        info = BITMAPINFO(input);

        if(header.buffer[0]=='B' && header.buffer[1]=='M')// первые два байта должы быть равны "BM"
        {
                input.seekg(v3FWidth);
                if(info.bitcount==24)
                {
                    int n_pix = info.width*info.height;
                    pixel_data = std::move(std::vector<pixel_rgb888>(n_pix,pixel_rgb888()));
                    input.seekg(header.offset);// переносим указатель на начало пиксельных данных
                    input.read((char*)pixel_data.data(),n_pix*3);
                }
                else
                {
                    // ошибка, предполагается только rgb888
                    xassert(false,"BMP_Image(const string path_to_file)::error::bits per pixel != 24 ; format != rgb888");
                }
        }
        else
        {
            // сигнатура не совпала - файл не BMP
            xassert(false,"BMP_Image(const string path_to_file)::error::file signature != 'BM' ");
        }
        input.close();
    }
    else
    {
        xassert(false,"BMP_Image(const string path_to_file)::error::can`t open BMP file");
    }
}

BMP_Image::~BMP_Image()
{

}
void BMP_Image::clamp()
{
    int w=info.width;
    int h=info.height;

    if(h%2 || w%2)
    {
        int new_w = w/2*2;
        int new_h = h/2*2;

        info.width = new_w;
        info.height = new_h;

        std::vector<pixel_rgb888> new_data(new_w*new_h,pixel_rgb888());

        pixel_rgb888* data_ptr = &pixel_data[0];

        // для выравнивания границы по 4 байта
        int offset = 4-(sizeof(pixel_rgb888)*w)%4;
//        char* byte_ptr = reinterpret_cast<char*>(data_ptr);
//        byte_ptr+=offset;
//        data_ptr = reinterpret_cast<pixel_rgb888*>(byte_ptr);
        for(int i = 0;i<new_h;i++)
        {
            for(int j = 0;j<new_w;j++)
            {
                new_data[i*new_w+j] = data_ptr[i*w+j];

            }
//            std::cerr<<data_ptr<<"\n";
            char* byte_ptr = reinterpret_cast<char*>(data_ptr);
//            byte_ptr--;
            byte_ptr+=offset;
            data_ptr = reinterpret_cast<pixel_rgb888*>(byte_ptr);

//            std::cerr<<data_ptr<<"\n";
//            data_ptr =(pixel_rgb888*)(((uint8_t*)(data_ptr))+offset);
        }

        pixel_data = std::move(new_data);
    }
}
void BMP_Image::turnHorizontaly()
{

    int h = info.height;
    int w = info.width;

    std::vector<pixel_rgb888> new_img(w*h,pixel_rgb888());

    for(int i = 0;i<h;i++)
    {
        for(int j = 0;j<w;j++)
        {
            new_img[i*w+j] = pixel_data[(h-i-1)*w+j];
        }
    }
    pixel_data = std::move(new_img);

}

void BMP_Image::saveAsYUV420(const string path)
{
    //auto file = openIfstream("/home/bebrou/Converter/sample_640_426.bmp");
    if(true)
    {
        vector<uint8_t> Y,U,V;
        Y.reserve(info.width*info.height);
        U.reserve(info.width*info.height/4);
        V.reserve(info.width*info.height/4);

        int r,g,b;
        uint8_t y,u,v;
        uint16_t u_temp,v_temp;
        for(int i = 0;i<info.height;i++)
        {
            for(int j = 0;j<info.width;j++)
            {
                auto pix = pixel_data[i*info.width+j];
                r = pix.r;
                g = pix.g;
                b = pix.b;
                rgb_to_yuv(r,g,b,y,u,v);
                u_temp+=u;
                v_temp+=v;
                Y.push_back(y);
                if((j+1)%2==0 && (i)%2==0)
                {

                    U.push_back(u);
                    V.push_back(v);
                    u_temp = 0;
                    v_temp = 0;
                }
            }

//                for(int j = 0;j<info.width;j++)
//                {
////                    U.push_back(U[j+i*info.width]);
////                    V.push_back(V[j+i*info.width]);
//                    U.push_back(0);
//                    V.push_back(0);
//                }
        }
        auto out = openOfstream(path);
        if(out!=nullptr)
        {
            out->write((char*)Y.data(),info.width*info.height);
            out->write((char*)V.data(),info.width*info.height/4);
            out->write((char*)U.data(),info.width*info.height/4);

            out->close();
        }
    }

}

BITMAPFILEHEADER BMP_Image::getHeader()     const{return header;};
BITMAPINFO BMP_Image::getInfo()             const{return info;}
vector<pixel_rgb888>*  BMP_Image::data()         {return &pixel_data;}

void BMP_Image::save(const string path_to_save,BITMAPFILEHEADER Header,BITMAPINFO Info)
{
    ofstream* output_file=openOfstream(path_to_save);
    memcpy(&Info.buffer[4],&Info.width,4);
    memcpy(&Info.buffer[8],&Info.height,4);
    output_file->write(Header.buffer,14);
    output_file->write(Info.buffer,124);
    output_file->write((char*)pixel_data.data(),Info.width*Info.height*sizeof(pixel_rgb888));
    output_file->close();
}
