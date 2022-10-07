#include <iostream>
#include <time.h>
#include <cstdlib>
#include <cuda.h>

using namespace std;
__host__ __device__ unsigned char bgr2y(int R, int G, int B){
  int Y = ((66 * R + 129 * G + 25 * B + 128) >> 8) + 16;
  return (unsigned char)((Y<0)? 0 : ((Y > 255) ? 255 : Y));}
__host__ __device__ int bgr2u(int R, int G, int B){
  int U = ((-38 * R - 74 * G + 112 * B + 128) >> 8) + 128;
  return (unsigned char)((U<0)? 0 : ((U > 255) ? 255 : U));}
__host__ __device__ int bgr2v(int R, int G, int B){
  int V = ((112 * R - 94 * G - 18 * B + 128) >> 8) + 128;
  return (unsigned char)((V<0)? 0 : ((V > 255) ? 255 : V));}

void bgr_to_yuv420sp(unsigned  char* yuv420p, unsigned char* bgr, int width, int height)
{
    if (yuv420p == NULL || bgr== NULL)
        return;
    int frameSize = width*height;

    int yIndex = 0;
    int uIndex = frameSize;

    int R, G, B;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            B = bgr[(i * width + j) * 3 + 0];
            G = bgr[(i * width + j) * 3 + 1];
            R = bgr[(i * width + j) * 3 + 2];

            //BGR to YUV
            yuv420p[yIndex++] = bgr2y(R,G,B);
            if (i % 2 == 0 && j % 2 == 0)
            {
                yuv420p[uIndex] = bgr2u(R,G,B);
                yuv420p[uIndex+1] = bgr2v(R,G,B);
                uIndex+=2;
            }
        }
    }
}
void bgr_to_yuv420p(unsigned  char* yuv420p, unsigned char* bgr, int width, int height)
{
    if (yuv420p == NULL || bgr== NULL)
        return;
    int frameSize = width*height;
    int chromaSize = frameSize / 4;

    int yIndex = 0;
    int uIndex = frameSize;
    int vIndex = frameSize + chromaSize;

    int R, G, B, Y, U, V;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            B = bgr[(i * width + j) * 3 + 0];
            G = bgr[(i * width + j) * 3 + 1];
            R = bgr[(i * width + j) * 3 + 2];

            //BGR to YUV
            Y = ((66 * R + 129 * G + 25 * B + 128) >> 8) + 16;
            U = ((-38 * R - 74 * G + 112 * B + 128) >> 8) + 128;
            V = ((112 * R - 94 * G - 18 * B + 128) >> 8) + 128;

            yuv420p[yIndex++] = (unsigned char)((Y < 0) ? 0 : ((Y > 255) ? 255 : Y));
            if (i % 2 == 0 && j % 2 == 0)
            {
                yuv420p[uIndex++] = (unsigned char)((U < 0) ? 0 : ((U > 255) ? 255 : U));
                yuv420p[vIndex++] = (unsigned char)((V < 0) ? 0 : ((V > 255) ? 255 : V));
            }
        }
    }
}
//kernel function to convert bgr to yuv420sp
__global__ void bgr2yuv420sp(uchar3 *  d_in, unsigned char * d_out,
                               uint imgheight, uint imgwidth)
{

    int col_num = blockIdx.x*blockDim.x+threadIdx.x;
    int row_num = blockIdx.y*blockDim.y+threadIdx.y;

    if ((row_num < imgheight) && (col_num < imgwidth))
    {
//        uint32_t a = *((uint32_t *)&dinput[global_offset*3]);
        int global_offset = row_num*imgwidth+col_num;

        int r,g,b;
        r = int(d_in[global_offset].z);
        g = int (d_in[global_offset].y);
        b = int (d_in[global_offset].x);


        d_out[row_num * imgwidth + col_num] = bgr2y(r,g,b);
        if(((threadIdx.x & 1) == 0)  && ((threadIdx.y & 1) == 0)){
            int uv_offset = imgwidth*imgheight+((row_num>>1)*imgwidth)+col_num;
            d_out[uv_offset] = bgr2u(r,g,b);
            d_out[uv_offset+1] = bgr2v(r,g,b);

        }

    }
}
//kernel function to convert bgr to yuv420p
__global__ void bgr2yuv420p(uchar3 *  d_in, unsigned char * d_out,
                               uint imgheight, uint imgwidth)
{

    int col_num = blockIdx.x*blockDim.x+threadIdx.x;
    int row_num = blockIdx.y*blockDim.y+threadIdx.y;

    if ((row_num < imgheight) && (col_num < imgwidth))
    {
//        uint32_t a = *((uint32_t *)&dinput[global_offset*3]);
        int global_offset = row_num*imgwidth+col_num;

        int r,g,b;
        r = int(d_in[global_offset].z);
        g = int (d_in[global_offset].y);
        b = int (d_in[global_offset].x);


        d_out[row_num * imgwidth + col_num] = bgr2y(r,g,b);
        if(((threadIdx.x & 1) == 0)  && ((threadIdx.y & 1) == 0)){
            int u_offset = imgwidth*imgheight+((row_num>>1)*(imgwidth>>1))+(col_num>>1);
            d_out[u_offset] = bgr2u(r,g,b);
            int v_offset = u_offset+((imgheight>>1)*(imgwidth>>1));
            d_out[v_offset] = bgr2v(r,g,b);

        }
    }
}


int main(void)
{

    const uint imgheight = 1000;
    const uint imgwidth = 1500;

    //input and output
    uchar3 *d_in;
    unsigned char *d_out;
    uchar3 *idata = new uchar3[imgheight*imgwidth];
    unsigned char *odata = new unsigned char[imgheight*imgwidth*3/2];
    unsigned char *cdata = new unsigned char[imgheight*imgwidth*3/2];
    uchar3 pix;
    for (int i = 0; i < imgheight*imgwidth; i++){
      pix.x = (rand()%30)+40;
      pix.y = (rand()%30)+40;
      pix.z = (rand()%30)+40;
      idata[i] = pix;}
    for (int i = 0; i < imgheight*imgwidth; i++) idata[i] = pix;
    bgr_to_yuv420p(cdata, (unsigned char*) idata, imgwidth, imgheight);
    // malloc memo in gpu
    cudaMalloc((void**)&d_in, imgheight*imgwidth*sizeof(uchar3));
    cudaMalloc((void**)&d_out, imgheight*imgwidth*sizeof(unsigned char) * 3 / 2);

    //copy image from cpu to gpu
    cudaMemcpy(d_in, idata, imgheight*imgwidth*sizeof(uchar3), cudaMemcpyHostToDevice);

    dim3 threadsPerBlock(32, 32);
    dim3 blocksPerGrid((imgwidth + threadsPerBlock.x - 1) / threadsPerBlock.x,
                       (imgheight + threadsPerBlock.y - 1) / threadsPerBlock.y);

    //run kernel function
    bgr2yuv420p<<<blocksPerGrid, threadsPerBlock>>>(d_in, d_out, imgheight, imgwidth);

    cudaDeviceSynchronize();

    //copy yuv420p from gpu to cpu
    cudaMemcpy(odata, d_out, imgheight*imgwidth*sizeof(unsigned char) * 3 / 2, cudaMemcpyDeviceToHost);
    for (int i = 0; i < (imgwidth*imgheight*3/2); i++) if (odata[i] != cdata[i]) {std::cout << "mismatch at: " << i << " was: " << (int)odata[i] << " should be: " << (int)cdata[i] << std::endl; return 0;}
    cudaFree(d_in);
    cudaFree(d_out);


    return 0;

}