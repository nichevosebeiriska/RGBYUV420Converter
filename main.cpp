#include <iostream>
#include <vector>
#include <string>
#include<fstream>
#include<stdint.h>
#include <cstring>
using namespace std;
#include"yuv420_frame.h"


#include"BMP_Image.h"
#include<immintrin.h>

#include"xassert.h"


// конвертация в многопоточном режиме
void convertBMPtoYUV420(BMP_Image& img,YUV420_Frame*& frame,int n_threads/* общее число потоков */,int num/* номер потока */)
{
    int shift = n_threads;
    int line = num;
    int first_line;
    int last_line;

    int width = img.getInfo().width;
    int height = img.getInfo().height;

    first_line = (height/n_threads)*num;
    last_line = (height/n_threads)*(num+1);

    // проверяем, что frame имеет достаточно места, чтобы туда выгрузить bmp

        pixel_rgb888* p1,*p2;//указатели на строки с номерами n и n+1

        uint8_t* Y_ptr,*U_ptr,*V_ptr;

        YUV420_Frame* f = frame;

        uint16_t y_temp,u_temp,v_temp;
        uint16_t y,v,u;
        auto pix_data = img.data();
        p1 = (pixel_rgb888*)pix_data->data();
        p2 = (pixel_rgb888*)(pix_data->data()+width);


        Y_ptr = f->get_Y_Data().data();
        V_ptr = f->get_V_Data().data();
        U_ptr = f->get_U_Data().data();


        p1+=first_line*width;
        p2+=first_line*width;

        Y_ptr += first_line*width+1;
        V_ptr += first_line*width/4+1;
        U_ptr += first_line*width/4+1;

        for(int i = first_line;i<last_line;i++)
        {

            for(int j = 0;j<width ;j++)
            {
                rgb_to_yuv(*p1++,y_temp,u_temp,v_temp);

                *Y_ptr++ = y_temp;

                u+=u_temp;
                v+=v_temp;

                if((j-1)%2==0 && i%2 == 0)
                {
                    int a = u>>2;
                    *U_ptr++ = u>>1;
                    *V_ptr++ = v>>1;
                    u = 0;
                    v = 0;
                }
            }
        }
}
#include<thread>
#include<functional>
#include<set>



class YUV420_Video
{

public:

    YUV420_Video(const string path,int width,int heigth,int n_frames);

    void addYUV420Image(YUV420_Frame* image,int xpos=0,int ypos=0);
    void save(const string path_to_save);

protected:

    std::vector<YUV420_Frame> frames;
    int nFrames;
    int width,height;
};

YUV420_Video::YUV420_Video(const string path,int w,int h,int n_frames)
{
    ifstream* input = openIfstream(path);

    xassert(input!=nullptr && input->is_open(),"YUV420_Video::YUV420_Video(const string path,int w,int h,int n_frames)::error::input file can`t be opened");
    nFrames = n_frames;
    width = w;
    height = h;
    char c[4];
    for(int i = 0;i<n_frames;i++)
    {
            frames.push_back(std::move(YUV420_Frame(w,h,input)));
    }
}
void YUV420_Video::addYUV420Image(YUV420_Frame* image,int xpos,int ypos)
{
    for(auto& frame:frames)
    {
        frame.addYUV420_Image(image,xpos,ypos);
    }
}
void YUV420_Video::save(const string path_to_save)
{
    ofstream* output = openOfstream(path_to_save);

    xassert(output->is_open(),"void YUV420_Video::save(const string path_to_save)::can`t open output file; output file path = "+std::string(path_to_save));

    for(auto& frame:frames)
    {
        uint8_t*  Y = frame.get_Y_Data().data();
        uint8_t*  U = frame.get_U_Data().data();
        uint8_t*  V = frame.get_V_Data().data();

        output->write((char*)Y,width*height);
        output->write((char*)U,width*height/4);
        output->write((char*)V,width*height/4);
    }


}


#define multithread_rgb_parsing

#include<sstream>

int main(int argc,char** argv)
{

   int nFrames;
   int video_width;
   int video_height;
   string video_path;
   string bmp_path;
   int bmp_x_pos,bmp_y_pos;
   string result_path;
   if(argc>1)
   {
       std::cout<<"launch parameters: \n";

       video_path = std::string(argv[1]);
       video_width = stoi(std::string(argv[2]));
       video_height =stoi(std::string(argv[3]));
       nFrames =stoi(std::string(argv[4]));
       bmp_path = std::string(argv[5]);
       bmp_x_pos=stoi(std::string(argv[6]));;
       bmp_y_pos=stoi(std::string(argv[7]));;
       result_path=std::string(argv[8]);
   }
   else
   {
       nFrames = 10;
       video_width = 1920;
       video_height = 1080;
       video_path = "/home/bebrou/Converter/tv1920.yuv";
       bmp_path = "/home/bebrou/Converter/dragon_1245_1059.bmp";
       bmp_x_pos    = 100;
       bmp_y_pos    = 200;
       result_path  = "result.yuv";
   }

   std::cerr<<"yuv path = "<<video_path<<"\n";
   std::cerr<<"yuv w = "<<video_width<<"\n";
   std::cerr<<"yuv h = "<<video_height<<"\n";
   std::cerr<<"yuv nf = "<<nFrames<<"\n";
   std::cerr<<"bmp path = "<<bmp_path<<"\n";
   std::cerr<<"xpos = "<<bmp_x_pos<<"\n";
   std::cerr<<"ypos = "<<bmp_y_pos<<"\n";
   std::cerr<<"result path = "<<result_path<<"\n";

   std::cerr<<video_path<<" "<<bmp_path<<"\n";
// 1) конвертация BMP в формате 24бит/пиксель(rgb888) в yuv420 кадр

#ifdef multithread_rgb_parsing

    YUV420_Frame* img_to_add = multithreadConvertRGBYUV420(bmp_path);// передаем путь до bmp, конвертируем в N потоков rgb->yuv420
#else
    YUV420_Frame* frame2 = new YUV420_Frame(640,426,path);
#endif

    YUV420_Video* video = new YUV420_Video(video_path,video_width,video_height,nFrames);
    video->addYUV420Image(img_to_add,bmp_x_pos,bmp_y_pos);
    video->save(result_path);

    return 0;
}
