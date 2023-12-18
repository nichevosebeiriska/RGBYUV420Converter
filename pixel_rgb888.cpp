#include"pixel_rgb888.h"
pixel_rgb888::pixel_rgb888()
{
    r = g = b = 0;
}
void rgb_to_yuv(uint8_t r,uint8_t g,uint8_t b,uint8_t&y ,uint8_t&u,uint8_t&v)
{
    y = 0.299 * (float)r + 0.587 * (float)g + 0.114 * (float)b;//+16;
    u = 0.492*(float)((int)b-(int)y)+128;
    v = 0.877*(float)((int)r-(int)y)+128;
}
void rgb_to_yuv(pixel_rgb888 p,uint16_t&y ,uint16_t&u,uint16_t&v)
{
    uint8_t r,g,b;
    r = p.b;
    g = p.g;
    b = p.r;


    y = 0.299 * (float)r + 0.587*(float)g + 0.114*(float)b;//+16;
    u = 0.492*(b-y)+128;

    // коэффициент снижен с 0.887 до 0.8, т.к. для r g b близких к 255
    // значение v уходит за 255*4, следовательно при урезании до uint8_t
    // происходит переполнение -> получаются нулевые значения
    // пока не ясно почему
    v = 0.8*(r-y)+128;


}

#include<immintrin.h>
void print_4_int(int* arr)
{
    for(int i = 0;i<4;i++)
    std::cerr<<arr[i]<<" ";
}
void print_4_float(float* arr)
{
    for(int i = 0;i<4;i++)
    std::cerr<<arr[i]<<" ";
}
void print_4_m128(__m128* arr)
{
    auto ptr = reinterpret_cast<float*>(arr);
    for(int i = 0;i<4;i++)
    std::cerr<<ptr[i]<<" ";
}
void rgb_to_yuv_avx(pixel_rgb888* pix_line1,pixel_rgb888* pix_line2,uint8_t* y,uint8_t* u,uint8_t* v)
{
    // в векторном виде:
    // {y} = {a1}{r} + {b1}{g} + {c1}{b}
    // {u} = {a2}*0.25*({b_ij} - {y_ij}) ; (i,j = 0..1)
    // {v} = {a3}*0.25*({r_ij} - {y_ij})

    __m128i p11,p12,p21,p22;

    // по 2 пикселя из верхней и нижней строки
    p11 = _mm_loadu_si128((__m128i*)pix_line1);
    p12 = _mm_loadu_si128((__m128i*)(pix_line1+1));
    p21 = _mm_loadu_si128((__m128i*)pix_line2);
    p22 = _mm_loadu_si128((__m128i*)(pix_line2+1));

    // float значения rgb компонент соответствующих пикселей
    __m128 p11f,p12f,p21f,p22f;


    p11f = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(p11));
    p12f = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(p12));
    p21f = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(p21));
    p22f = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(p22));

    // среднее значение компонент rgb для 4 пикселей
    __m128 rgb_sum;

    //суммирование
    rgb_sum = _mm_add_ps(p11f,p12f);
    rgb_sum = _mm_add_ps(rgb_sum,p21f);
    rgb_sum = _mm_add_ps(rgb_sum,p22f);

    float a1,a2,a3,b1,c1;
    a1 = 0.299;
    b1 = 0.587;
    c1 = 0.114;
    a2 = 0.492;
    a3 = 0.8;

    __m128 y_mask{c1,b1,a1,0};

    // умножение rgb компонент на a1 b1 c1 для получения y11 y12 y21 y22
    p11f = _mm_mul_ps(y_mask,p11f);
    p12f = _mm_mul_ps(y_mask,p12f);
    p21f = _mm_mul_ps(y_mask,p21f);
    p22f = _mm_mul_ps(y_mask,p22f);

    // сложение r+b для y11 y12
    p11f = _mm_hadd_ps(p11f,p12f);
    // сложение r+b для y21 y22
    p21f = _mm_hadd_ps(p21f,p22f);

    // сложение (r+g)+b для y11 y12
    p11f = _mm_hadd_ps(p11f,p11f);
    // сложение (r+g)+b для y21 y22
    p21f = _mm_hadd_ps(p21f,p21f);

    // запись результата в y[]
    y[0] = (reinterpret_cast<float*>(&p11f))[0];
    y[1] = (reinterpret_cast<float*>(&p11f))[1];
    y[2] = (reinterpret_cast<float*>(&p21f))[0];
    y[3] = (reinterpret_cast<float*>(&p21f))[1];

    // сложение (y11 + y12)
    p11f = _mm_hadd_ps(p11f,p11f);

    // сложение (y21 + y22)
    p21f = _mm_hadd_ps(p21f,p21f);

    // получаем сумму y11+y12
    float* y_sum_res = reinterpret_cast<float*>(&p11f);
    // (y11+y12) + (y21+y22)
    y_sum_res[0]+= (reinterpret_cast<float*>(&p21f))[0];

    // сумма y11 y12 y21 y22

    float r,b;

    // получаем сумму r и b для всех пикселей
    r = reinterpret_cast<float*>(&rgb_sum)[0];
    b = reinterpret_cast<float*>(&rgb_sum)[2];

    // вычисление компонент u v по 4 пикселям
    *u = a2*0.25*(r - y_sum_res[0]) + 128;
    *v = a3*0.25*(b - y_sum_res[0]) + 120;
}
