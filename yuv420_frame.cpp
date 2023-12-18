#include "yuv420_frame.h"



#include <iostream>
#include <fstream>
#include <vector>
#include<iterator>
YUV420_Frame::YUV420_Frame(uint16_t w,uint16_t h,ifstream* input)
{

    if(input!=nullptr && w>0 && h>0)
    {
        width = w;
        height = h;

        Luma = std::move(std::vector<uint8_t>(width*height,0));
        U = std::move(std::vector<uint8_t>(width*height/4,0));
        V = std::move(std::vector<uint8_t>(width*height/4,0));

        input->read((char*)Luma.data(),width*height);
        input->read((char*)U.data(),width*height/4);
        input->read((char*)V.data(),width*height/4-1);
    }
}
YUV420_Frame::YUV420_Frame(BMP_Image& bmp)
{
    int w = bmp.getInfo().width;
    int h = bmp.getInfo().height;

    width = w;
    height = h;

    Luma = std::move(std::vector<uint8_t>(w*h,0));
    U    = std::move(std::vector<uint8_t>(w*h/4,0));
    V    = std::move(std::vector<uint8_t>(w*h/4,0));

    uint16_t y_temp,u_temp,v_temp;// для возврата значений из функции
    uint16_t y,u,v;               // хранение промежуточных результатов

    y = u = v = 0;

    pixel_rgb888* p1,*p2;// указатели на 2 строки пикселей с номерами n и n+1

    p1 = bmp.data()->data();
    p2 = p1 + width;

    for(int i = 0;i<h;i+=2)
    {
        for(int j = 0;j<w;j++)
        {
            rgb_to_yuv(*p1,y_temp,u_temp,v_temp);

            // верхний пиксель [i;j]
            Luma[i*width+j] = y_temp;

            u+=u_temp;
            v+=v_temp;

            //нижний пиксель [i+1;j]
            rgb_to_yuv(*p2,y_temp,u_temp,v_temp);
            Luma[(i+1)*width+j] = y_temp;

            u+=u_temp;
            v+=v_temp;

            // на каждом четном пикселе усредняем u и v, записываем их в массивы
            if((j+1)%2==0)
            {
                U[i*width/4+j/2] = u>>2;
                V[i*width/4+j/2] = v>>2;

                u = v = 0;
            }

            p1++;
            p2++;

        }
        // шаг через строку
        p1+=width;
        p2+=width;
    }

}
YUV420_Frame::YUV420_Frame(YUV420_Frame&& frame_to_move)
{
    //frame_to_move.get_Y_Data();

    Luma = std::move(frame_to_move.get_Y_Data());
    U = std::move(frame_to_move.get_U_Data());
    V = std::move(frame_to_move.get_V_Data());

    width = frame_to_move.getWidth();
    height = frame_to_move.getHeight();

}
YUV420_Frame::YUV420_Frame(int16_t w,int16_t h)// создание пустого кадра
{
    width =w;
    height= h;

    Luma = std::move(std::vector<uint8_t>(w*h,0));
    U    = std::move(std::vector<uint8_t>(w*h/4,0));
    V    = std::move(std::vector<uint8_t>(w*h/4,0));
}
#define AVX_OPTIMIZATION
void convertByThreadRGBYUV420(YUV420_Frame* frame,BMP_Image* img,int first_line,int last_line)
{
    int width = img->getInfo().width;
    int height = img->getInfo().height;

    int yuv_w = width/2*2;
    int yuv_h = height/2*2;

   int offset = 4-(sizeof(pixel_rgb888)*width)%4;
    // проверяем указатели и размер векторов Y U V для переданного указател на кадр
    if(frame!=nullptr && img!= nullptr)
    {
        int w = img->getInfo().width;
        int h = img->getInfo().height;

        // переменные
        uint16_t y_temp,u_temp,v_temp;// для возврата значений из функции
        uint16_t y,u,v;               // хранение промежуточных результатов

        uint8_t* y_arr = new uint8_t[4];
        uint8_t avx_u,avx_v;

        y = u = v = 0;

        // указатели на 2 строки пикселей с номерами n и n+1
        pixel_rgb888* p1=img->data()->data() + first_line*w,// переносим указатель на первый элемент строки first_line
                    * p2 =  p1 + w;  // переносим p2 на следующую строку c номером first_line+1


        // получаем указатели на массивы
        uint8_t* Y_ptr = frame->get_Y_Data().data();
        uint8_t* U_ptr = frame->get_U_Data().data() + first_line/2*2*yuv_w/4;
        uint8_t* V_ptr = frame->get_V_Data().data() + first_line/2*2*yuv_w/4;

// если скомпилируется с флагом -mavx
#ifdef AVX_OPTIMIZATION
        pixel_rgb888* p1_ptr,*p2_ptr;
        for(int i = first_line;i<last_line/2*2;i+=2)
        {
            for(int j = 0;j<w;j+=2)
            {
                p1_ptr = &p1[i*width+j];
                p2_ptr = &p2[i*width+j];

                auto p11 = p1_ptr[0];
                auto p12 = p1_ptr[1];
                auto p21 = p2_ptr[0];
                auto p22 = p2_ptr[1];

                rgb_to_yuv_avx(p1_ptr,p2_ptr,y_arr,&avx_u,&avx_v);

                Y_ptr[i*width+j] = y_arr[0]+16;
                Y_ptr[i*width+j+1] = y_arr[1]+16;
                Y_ptr[i*width+j+width] = y_arr[2]+16;
                Y_ptr[i*width+j+width+1] = y_arr[3]+16;

                U_ptr[i*width/4+j/2] = avx_u;
                V_ptr[i*width/4+j/2] = avx_v;
            }
        }

#else
                for(int i = first_line;i<last_line/2*2;i+=2)
                {
                    for(int j = 0;j<w;j++)
                    {
                        rgb_to_yuv(*p1,y_temp,u_temp,v_temp);
                        // верхний пиксель [i;j]
                        if(j<yuv_w)
                        Y_ptr[i*yuv_w+j] = y_temp;


                        u+=u_temp;
                        v+=v_temp;

                        //нижний пиксель [i+1;j]
                        rgb_to_yuv(*p2,y_temp,u_temp,v_temp);

                        if(j<yuv_w)
                        Y_ptr[(i+1)*yuv_w+j] = y_temp;

                        u+=u_temp;
                        v+=v_temp;

                        // на каждом четном пикселе усредняем u и v, записываем их в массивы
                        if((j+1)%2==0)
                        {
                            *U_ptr++ = u/4;
                            *V_ptr++ = v/4;
                            u = v = 0;
                        }

                        p1++;
                        p2++;

                    }
                    u=v=0;
                    // шаг через строку
                    p1+=width;
                    p2+=width;
                }
#endif
    }

}
YUV420_Frame* multithreadConvertRGBYUV420(const string path_to_bmp)
{

    BMP_Image* image = new BMP_Image(path_to_bmp);
    image->clamp();
    image->save("/home/bebrou/Converter/test_bmp_save.bmp",image->getHeader(),image->getInfo());

    // вертикально отражаем bmp
    image->turnHorizontaly();

    int width = image->getInfo().width;
    int height = image->getInfo().height;

    int yuv_w = width/2*2;
    int yuv_h = height/2*2;
    YUV420_Frame* frame = new YUV420_Frame(yuv_w,yuv_h);

    std::vector<std::thread> threads;

    int N = 1;

    int step = (height/N)/2*2;

    int first=0,last;
    for(int i = 0;i<N-1;i++)
    {
        int _i=i;
        first = first;
        last = first+step;

        threads.push_back(std::thread([=]{convertByThreadRGBYUV420(std::ref(frame),std::ref(image),first,last);}));

        first+=step;
    }

    threads.push_back(std::thread([=]{convertByThreadRGBYUV420(std::ref(frame),std::ref(image),first,height);}));

    for(std::thread& thr:threads)
    {
        thr.join();
    }

    return frame;
}

void YUV420_Frame::addBMP_Image(BMP_Image bmp,int pos_x,int pos_y)
{
    int bmp_w = bmp.getInfo().width;
    int bmp_h = bmp.getInfo().height;
    vector<pixel_rgb888>* bmp_data = bmp.data();

    int s = bmp.data()->size();

    int i_uv,j_uv;

    if(width>=bmp_w && height >=bmp_h)
    {

    }
    else
    {
        //ошибка
    }

    int y,u,v;
    int8_t r,g,b;
    float kr = 0.2126;
    float kb = 0.0722;

    uint16_t u_temp,v_temp,y_temp;

        for(int i = 0;i<bmp_h/2;i++)
        {
            for(int j = 0;j<bmp_w/2;j++)
            {
                auto p0 = bmp_data->data()[(i*2*bmp_w+j*2)];
                auto p1 = bmp_data->data()[(i*2*bmp_w+(j+1)*2)];
                auto p2 = bmp_data->data()[(i*2*bmp_w+(j)*2)+bmp_w];
                auto p3 = bmp_data->data()[(i*2*bmp_w+(j+1)*2)+bmp_w];

                r = p0.r;
                g = p0.g;
                b = p0.b;

                rgb_to_yuv(p0,y_temp,u_temp,v_temp);

                Luma[i*width*2 + j*2]               =  y_temp ;

                U[((i%bmp_w)*width/2+ j)] = ((u_temp));

                V[((i%bmp_w)*width/2+ j)] = ((v_temp));

                rgb_to_yuv(p1,y_temp,u_temp,v_temp);
                Luma[i*width*2 + j*2 + 1]           = y_temp;
                rgb_to_yuv(p2,y_temp,u_temp,v_temp);
                Luma[i*width*2 + j*2 + width]       = y_temp ;
                rgb_to_yuv(p3,y_temp,u_temp,v_temp);
                Luma[i*width*2 + j*2 + width + 1]   = y_temp;

            }

        }


}

void YUV420_Frame::addYUV420_Image(YUV420_Frame* image,int xpos,int ypos)
{
        if(image!=nullptr)
        {
            if(xpos>=width || ypos>=height)
            {
                // ошибка - левый верхний угол выходит за экран
            }

            int image_w = image->width,
                image_h = image->height;

            // индексы левого верхнего пикселя изображения в координатах видео
            int i0,j0;

            i0 = min(max(0,ypos),(int)height);
            j0 = min(max(0,xpos),(int)width);

            // индекс первого верхнего левого пикселя картинки(в координатах картинки), который попадает в видео
            int img_i0,img_j0;

            img_i0 = max(0,-ypos);
            img_j0 = max(0,-xpos);

            // число линий пикселей по вертикали и горизонтали, по которым картингка и видео перекрывают друг друга
            int dh,dw;

            dh = max(0,min(height-i0,min(image_h,image_h-img_i0)));
            dw = max(0,min(width-j0,min(image_w,image_w-img_j0)));


//            std::cerr<<"i0 = "<<i0<<" j0 = "<<j0<<"\n img_i0 = "<<img_i0<<" img_j0 = "<<img_j0<<"\n";
//            std::cerr<<"dw = "<<dw<<" dh = "<<dh<<"\n";

            const std::vector<uint8_t>& image_Y = image->get_Y_Data();
            const std::vector<uint8_t>& image_U = image->get_U_Data();
            const std::vector<uint8_t>& image_V = image->get_V_Data();

            for(int i = 0;i<dh;i++)
            {
                for(int j=0;j<dw;j++)
                {
                    Luma[(i+i0)*width+(j+j0)] = image_Y[(i+img_i0)*image_w+(j+img_j0)];
                    // на каждом четном шаге копируем uv
                    if(i%2==0 && j%2==0)
                    {
                        U[((i+i0)*width/4+(j+j0)/2)] = image_U[((i+img_i0)*image_w/4+(j+img_j0)/2)];
                        V[((i+i0)*width/4+(j+j0)/2)] = image_V[((i+img_i0)*image_w/4+(j+img_j0)/2)];
                    }
                }
            }
        }
}

void YUV420_Frame::save(const string path)
{
    auto out = openOfstream(path);
    if(out->is_open())
    {
        out->write((char*)Luma.data(),width*height);
        out->write((char*)U.data(),width*height/4);
        out->write((char*)V.data(),width*height/4);
        out->close();
    }

}


int YUV420_Frame::getWidth() const {return width;};
int YUV420_Frame::getHeight() const {return height;};
std::vector<uint8_t>& YUV420_Frame::get_Y_Data(){return Luma;};
std::vector<uint8_t>& YUV420_Frame::get_V_Data(){return V;};
std::vector<uint8_t>& YUV420_Frame::get_U_Data(){return U;};
