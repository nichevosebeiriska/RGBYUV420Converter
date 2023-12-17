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
pixel_rgb888* rgb_to_yuv(pixel_rgb888* pix_line1,pixel_rgb888* pix_line2,uint8_t* y,uint8_t* u,uint8_t* v)
{
    // в векторном виде:
    // {y} = {a1}{r} + {b1}{g} + {c1}{b}
    // {u} = {a2}*0.25*({b_ij} - {y_ij}) ; (i,j = 0..1)
    // {v} = {a3}*0.25*({r_ij} - {y_ij})

//    uint8_t* first_line_pix_data  = (uint8_t*)pix_line1;
//    uint8_t* second_line_pix_data = (uint8_t*)pix_line2;

//    uint8_t input2[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
//    __m128i packed_data = _mm_loadu_si128((__m128i_u*)input2);
//    auto p = _mm_cvtepi8_epi32(packed_data);

//    _mm_add_epi32(p,p);

////    __m128i  l1 = _mm_loadu_si128((__m128i*)pix_line1)
////            ,l2 = _mm_loadu_si128((__m128i*)pix_line2);

////    __m256i l1_32;



//    //l1_32 = _mm256_cvtepi8_epi32(l2);


//    int subres[8];
//    //_mm256_storeu_epi32(subres,l1);
////    _mm256_storeu_epi32(subres,l1_32);
////    for(int i = 0;i<8;i++)
////    {
////        std::cerr<<subres[i]<<" ";
////    }

//    //_mm256_add_epi32(l1_32,l2_32);

//    ///

//    int inp[]{10,20,30,40};
//    __m128i inp_arr = _mm_load_epi32(inp);
//    __m128i arr8bit = _mm_cvtepi32_epi8(inp_arr);

//    uint8_t* trunc_res = new uint8_t[16];
//    _mm_storeu_si128((__m128i*)trunc_res,arr8bit);

//    // загружаем 16 uint8_t значений y, u и v
//    __m128i y_arr = _mm_load_si128((__m128i*)y);
//    __m128i u_arr = _mm_load_si128((__m128i*)y);
//    __m128i v_arr = _mm_load_si128((__m128i*)y);

//    float a1 = 0.299,
//          b1 = 0.587,
//          c1 = 0.114,
//          a2 = 0.492,
//          a3 = 0.877;

//    __m128 a1_v = {a1,a1,a1,a1};
//    __m128 b1_v = {b1,b1,b1,b1};
//    __m128 c1_v = {c1,c1,c1,c1};

//    __m128 a2_v = {a2,a2,a2,a2};
//    __m128 a3_v = {a3,a3,a3,a3};

//    __m128i tt  =_mm_load_si128((__m128i*)y);
//    __m128i tt2 =_mm_load_si128((__m128i*)u);
//    __m128i yi = _mm_cvtepi8_epi16(tt);
//    __m128i yi2 = _mm_cvtepi8_epi16(tt2);

//    yi = _mm_hadd_epi16(yi,yi2);
//    auto bit16 = _mm_packus_epi16(yi,yi);

//    uint16_t output[16];

//    _mm_storeu_si128((__m128i*)output, yi);
////        for (int i = 0; i < 4; i++) {
////            std::cerr<<(int)output[i]<<" ";
////        }

////    //uint8_t r1_line[8]{pix_line1[0].r,};
////    uint8_t g1_line[8];
////    uint8_t b1_line[8];

////    uint8_t r2_line[8];
////    uint8_t g2_line[8];
////    uint8_t b2_line[8];

//    __m128i r1_line,g1_line,b1_line;
//    __m128i r2_line,g2_line,b2_line;

//    //_mm_loadu_si128((__m128i*)y);
//    r1_line = _mm_loadu_si128((__m128i*)y);
//    r1_line = _mm_loadu_si128((__m128i*)y);
//    r1_line = _mm_loadu_si128((__m128i*)y);

//    auto r1_16_line = _mm_cvtepi8_epi16(r1_line);
//    auto res = _mm_hadd_epi16(r1_16_line,r1_16_line);


//   // __m128i r_line  = _mm_load_ps(pix_line1)



}
