#include "tinylibdeflate.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1000

int main(){
    // data to be compressed
    char data[BUFFER_SIZE] = "This is an example for DEFLATE compression and decompression!\0";
    char compressed[BUFFER_SIZE];
    char extracted[BUFFER_SIZE];
    int data_len = strlen(data);

    // prepare environment
    deflate_set_memory_allocator((void * (*)(int))malloc, free);
    struct libdeflate_compressor * compressor = deflate_alloc_compressor(1);
    struct libdeflate_decompressor * decompressor = deflate_alloc_decompressor();

    // do compress
    int out_nbytes = deflate_deflate_compress(compressor, data, data_len, compressed, BUFFER_SIZE);
    printf("original: %d\n", data_len);
    printf("compressed: %d\n", out_nbytes);

    // do decompress
    int restore_nbytes = 0;
    if(deflate_deflate_decompress(decompressor, compressed, out_nbytes, extracted, data_len, &restore_nbytes)){
        printf("An error occurred during decompression.\n");
        exit(1);
    }
    printf("decompressed: %d\n%s\n", restore_nbytes, extracted);

    return 0;
}