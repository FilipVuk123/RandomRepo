#include "video_decoder.h"
#include "video_encoder.h"
#include "orqa_clock.h"

int main()
{
    int width = 1280, height = 720;
    FILE *inputfptr = fopen("out720.yuv", "rb");
    if (inputfptr == NULL){
        printf("Could not open file 1\n");
        return 1;  
    } 
    FILE *outputfptr = fopen("totestdecoder.h264", "w+");
    if (outputfptr == NULL) {
        printf("Could not open file 2\n");
        return 1;
    }
    FILE *fp_yuv = fopen("output.yuv", "w+");
    if (fp_yuv == NULL) {
        printf("Could not open file 3\n");
        return 1;
    }
    decoder_t dec;
    encoder_t enc;
    
    
    uint8_t *inputbuffer = malloc(width*height * 3 / 2);
    int ret = fread(inputbuffer, width*height * 3 / 2, 1, inputfptr);
    fclose(inputfptr);

    decoder_init(&dec, width, height);
    encoder_init(&enc, width, height);

    orqa_clock_t start = orqa_time_now();

    for (int i = 0; i < 10; i++)
    {
        int size = encode(&enc, inputbuffer);
        if(size < 0) goto destroy;
        if(enc.encoder_flag){
            fwrite(enc.encoded_buffer, enc.size, 1, outputfptr);
            size = decode(&dec, enc.encoded_buffer, enc.size);
            if(size < 0) goto destroy;
            enc.encoder_flag = 0;
        }
        if (dec.decoder_flag)
        {
            fwrite(dec.decoded_buffer, dec.size, 1, fp_yuv);
            dec.decoder_flag = 0;
        }
    }

    printf("ms %f\n", orqa_get_time_diff_msec(start, orqa_time_now()));
destroy:
    decoder_destroy(&dec);
    encoder_destroy(&enc);
    free(inputbuffer);
    fclose(fp_yuv);
    fclose(outputfptr);
    return 0;
}

