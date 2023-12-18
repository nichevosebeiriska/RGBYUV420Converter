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

    width = w;
    height = h;

    ifstream* input = openIfstream(path);

    xassert(input!=nullptr && input->is_open(),"YUV420_Video::YUV420_Video(const string path,int w,int h,int n_frames)::error::input file can`t be opened");


    input->seekg(0, ios::end);
    int fileSize = (int)input->tellg();
    input->seekg(0);

    nFrames = min(n_frames,((int)fileSize/(int)(width*height*1.5)));

    for(int i = 0;i<nFrames;i++)
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
    std::cout<<"video saved:: "<<path_to_save<<'\n';


}


#define multithread_rgb_parsing

#include<sstream>

int main(int argc,char** argv)
{
   int n_frames;
   int yuv_width;
   int yuv_height;
   string yuv_path;
   string bmp_path;
   int xpos,ypos;
   string result_path;
   if(argc>1)
   {
       std::cout<<"launch parameters: \n";

       yuv_path = std::string(argv[1]);
       yuv_width = stoi(std::string(argv[2]));
       yuv_height =stoi(std::string(argv[3]));
       n_frames =stoi(std::string(argv[4]));
       bmp_path = std::string(argv[5]);
       xpos=stoi(std::string(argv[6]));;
       ypos=stoi(std::string(argv[7]));;
       result_path=std::string(argv[8]);
   }
   else
   {
       n_frames = 1;
       yuv_width = 352;
       yuv_height = 288;
       yuv_path = "../yuv_samples/akiyo_cif.yuv";
       bmp_path = "../bmp_samples/bmp_save.bmp";
       xpos    = 50;
       ypos    = -150;
       result_path  = "../results/result.yuv";
   }

   std::cerr<<"yuv path = "<<yuv_path<<"\n";
   std::cerr<<"yuv w = "<<yuv_width<<"\n";
   std::cerr<<"yuv h = "<<yuv_height<<"\n";
   std::cerr<<"yuv nf = "<<n_frames<<"\n";
   std::cerr<<"bmp path = "<<bmp_path<<"\n";
   std::cerr<<"xpos = "<<xpos<<"\n";
   std::cerr<<"ypos = "<<ypos<<"\n";
   std::cerr<<"result path = "<<result_path<<"\n";

#ifdef multithread_rgb_parsing
    // выполнение в многопоточном режиме( по умолчанию 8 потоков, можно поменять в самой функции)
    YUV420_Frame* img_to_add = multithreadConvertRGBYUV420(bmp_path);// передаем путь до bmp, конвертируем в N потоков rgb->yuv420
#else

    BMP_Image img(bmp_path);
    img.turnHorizontaly();
    YUV420_Frame* img_to_add = new YUV420_Frame(img);
#endif

    YUV420_Video* video = new YUV420_Video(yuv_path,yuv_width,yuv_height,n_frames);
    video->addYUV420Image(img_to_add,xpos,ypos);
    video->save(result_path);

    return 0;
}
