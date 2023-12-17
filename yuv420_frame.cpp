#include "yuv420_frame.h"



#include <iostream>
#include <fstream>
#include <vector>
#include<iterator>
YUV420_Frame::YUV420_Frame(uint16_t w,uint16_t h,ifstream* input)
{
//    std::istream_iterator<char>it;
    istream_iterator<uint8_t> tt(*input);
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

void convertByThreadRGBYUV420(YUV420_Frame* frame,BMP_Image* img,int first_line,int last_line)
{
    int width = img->getInfo().width;
    int height = img->getInfo().height;

    int yuv_w = width/2*2;
    int yuv_h = height/2*2;

   int offset = 4-(sizeof(pixel_rgb888)*width)%4;
    // проверяем указатели и размер векторов Y U V для переданного указател на кадр
    if(
       frame!=nullptr && img!= nullptr
//       && (frame->get_Y_Data().size()==width*height
//       && frame->get_V_Data().size()==width*height/4
//       && frame->get_U_Data().size()==width*height/4)
      )
    {
        int w = img->getInfo().width;
        int h = img->getInfo().height;

        // получаем номер обрабатываемых строк
//        int first_line = (h/thread_total_number)*thread_number;
//        int last_line  = (h/thread_total_number)*(thread_number+1);

//        // если высота bmp нацело не делится на число потоков - остаток обрабатываем в последнем потоке
//        if(thread_number == thread_total_number)
//        {
//            last_line+=w%thread_total_number;
//        };

        // переменные
        uint16_t y_temp,u_temp,v_temp;// для возврата значений из функции
        uint16_t y,u,v;               // хранение промежуточных результатов

        y = u = v = 0;

        // указатели на 2 строки пикселей с номерами n и n+1
        pixel_rgb888* p1=img->data()->data() + first_line*w,// переносим указатель на первый элемент строки first_line
                    * p2 =  p1 + w;  // переносим p2 на следующую строку c номером first_line+1


        // получаем указатели на массивы
        uint8_t* Y_ptr = frame->get_Y_Data().data();
        uint8_t* U_ptr = frame->get_U_Data().data() + first_line/2*2*yuv_w/4;
        uint8_t* V_ptr = frame->get_V_Data().data() + first_line/2*2*yuv_w/4;


        for(int i = first_line;i<last_line/2*2;i+=2)
        {
            for(int j = 0;j<w;j++)
            {
                rgb_to_yuv(*p1,y_temp,u_temp,v_temp);
//                rgb_to_yuv(p1[i*width+j],y_temp,u_temp,v_temp);
                // верхний пиксель [i;j]
                if(j<yuv_w)
                Y_ptr[i*yuv_w+j] = y_temp;


                u+=u_temp;
                v+=v_temp;

                //нижний пиксель [i+1;j]
                rgb_to_yuv(*p2,y_temp,u_temp,v_temp);
//                rgb_to_yuv(p2[(i+1)*width+j],y_temp,u_temp,v_temp);
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

//            char* byte_ptr = reinterpret_cast<char*>(p1);
////            byte_ptr--;
//            byte_ptr+=offset;
            //p1 = reinterpret_cast<pixel_rgb888*>(reinterpret_cast<uint8_t*>(p1)+offset-4);
            //p2 = reinterpret_cast<pixel_rgb888*>(reinterpret_cast<uint8_t*>(p2)+offset-4);

        }


    }
    else
    {

    }
}
#include<immintrin.h>
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

            int i0 = ypos;
            int j0 = xpos;

            // колличество пикселей по горизонтали и вертикали, которые влазят в видео
            int dw = min(width-xpos,image->getWidth());
            int dh = min(height-ypos,image->getHeight());

            int image_w = image->getWidth();
            int image_h = image->getHeight();

            const std::vector<uint8_t>& image_Y = image->get_Y_Data();
            const std::vector<uint8_t>& image_U = image->get_U_Data();
            const std::vector<uint8_t>& image_V = image->get_V_Data();

            for(int i = 0;i<dh;i++)
            {
                for(int j=0;j<dw;j++)
                {
                    Luma[(i+i0)*width+(j+j0)] = image_Y[i*image_w+j];

                    if(i%2==0 && j%2==0)
                    {
                        U[((i+i0)*width/4+(j+j0)/2)] = image_U[(i*image_w/4+j/2)];
                        V[((i+i0)*width/4+(j+j0)/2)] = image_V[(i*image_w/4+j/2)];
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
