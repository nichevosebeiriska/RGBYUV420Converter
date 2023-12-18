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
pixel_rgb888* rgb_to_yuv(pixel_rgb888* pix_line1,pixel_rgb888* pix_line2,uint8_t* y,uint8_t* u,uint8_t* v)
{
    // в векторном виде:
    // {y} = {a1}{r} + {b1}{g} + {c1}{b}
    // {u} = {a2}*0.25*({b_ij} - {y_ij}) ; (i,j = 0..1)
    // {v} = {a3}*0.25*({r_ij} - {y_ij})

    __m128i p11,p12,p21,p22;

    p11 = _mm_loadu_si128((__m128i*)pix_line1);
    p12 = _mm_loadu_si128((__m128i*)(pix_line1+1));
    p21 = _mm_loadu_si128((__m128i*)pix_line2);
    p22 = _mm_loadu_si128((__m128i*)(pix_line2+1));


    uint8_t* p11res  = new uint8_t[16];
    p11res = reinterpret_cast<uint8_t*>(&p11);
    _mm_storeu_si128((__m128i*)p11res,p11);

    std::cerr<<"p11 = \n";
    for(int i = 0;i<4;i++)
    {
        std::cerr<<(int)p11res[i]<<" ";
    }

    int m[4]{1,1,1,0};
    __m128i mask;

    mask = _mm_loadu_si128((__m128i*)m);
    __m128 fmask = _mm_cvtepi32_ps(mask);
    __m128 p11f,p12f,p21f,p22f;

    std::cerr<<"mask = \n";
    print_4_int(reinterpret_cast<int*>(&mask));


    p11f = _mm_cvtepi32_ps(_mm_cvtepi8_epi32(p11));
    p12f = _mm_cvtepi32_ps(_mm_cvtepi8_epi32(p12));
    p21f = _mm_cvtepi32_ps(_mm_cvtepi8_epi32(p21));
    p22f = _mm_cvtepi32_ps(_mm_cvtepi8_epi32(p22));

    p11f = _mm_mul_ps(p11f,fmask);
    _mm_mul_ps(p12f,fmask);
    _mm_mul_ps(p21f,fmask);
    _mm_mul_ps(p22f,fmask);




    float* res =new float[4];

    res = reinterpret_cast<float*>(&p11f);


    //_mm_storeu_ps(res,p11f);

    std::cerr<<"res = \n";

    for(int i = 0;i<4;i++)
    {
        std::cerr<<res[i]<<' ';
    }

    float a1,a2,a3,b1,c1;
    a1 = 0.299;
    b1 = 0.587;
    c1 = 0.114;
    a2 = 0.492;
    a3 = 0.877;

    __m128 y_mask{a1,b1,c1,0};

    p11f = _mm_mul_ps(y_mask,p11f);
    p12f = _mm_mul_ps(y_mask,p12f);
    p21f = _mm_mul_ps(y_mask,p21f);
    p22f = _mm_mul_ps(y_mask,p22f);

    std::cerr<<"\np11f after mult on coef\n";
    print_4_m128(&p11f);
    std::cerr<<"\np11f after mult on coef\n";
    print_4_m128(&p12f);

    std::cerr<<" \np11 p12 hadd\n";
    p11f = _mm_hadd_ps(p11f,p12f);
        print_4_m128(&p11f);
    std::cerr<<" \n p11 p12 hadd 2\n";
    p11f = _mm_hadd_ps(p11f,p11f);
    print_4_m128(&p11f);

    std::cerr<<" y sum = \n";
    p11f = _mm_hadd_ps(p11f,p11f);
    float* y_sum_res = reinterpret_cast<float*>(&p11f);

    // сумма y11 y12 y21 y22
    std::cerr<<y_sum_res[0];

    // сложение aR + bG + cB для y11 y12
    __m128 hadd1 = _mm_hadd_ps(p11f,p12f);
    // сложение y11 y12
    hadd1 = _mm_hadd_ps(hadd1,hadd1);
    // сложение aR + bG + cB для y21 y22
    __m128 hadd2 = _mm_hadd_ps(p21f,p22f);
    // сложение y21 y22
    hadd2 = _mm_hadd_ps(hadd2,hadd2);





    pixel_rgb888* result;

    return result;


    //_mm128 a_arr,b_arr,c_arr;

}
