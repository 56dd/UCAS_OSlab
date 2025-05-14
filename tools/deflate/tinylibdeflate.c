#include "tinylibdeflate.h"
#include "libdeflate.h"

struct libdeflate_compressor *
deflate_alloc_compressor(int compression_level){
    return libdeflate_alloc_compressor(compression_level);
}

int
deflate_deflate_compress(struct libdeflate_compressor *compressor,
			    const void *in, int in_nbytes,
			    void *out, int out_nbytes_avail){
    return libdeflate_deflate_compress(compressor, in, in_nbytes, out, out_nbytes_avail);
}

void
deflate_free_compressor(struct libdeflate_compressor *compressor){
    libdeflate_free_compressor(compressor);
}

struct libdeflate_decompressor *
deflate_alloc_decompressor(void){
    return libdeflate_alloc_decompressor();
}

int
deflate_deflate_decompress(struct libdeflate_decompressor *decompressor,
			      const void *in, int in_nbytes,
			      void *out, int out_nbytes_avail,
			      int *actual_out_nbytes_ret){
    return libdeflate_deflate_decompress(decompressor, in, in_nbytes, out, out_nbytes_avail, (size_t*)actual_out_nbytes_ret);
}

void
deflate_free_decompressor(struct libdeflate_decompressor *decompressor){
    libdeflate_free_decompressor(decompressor);
}

void
deflate_set_memory_allocator(void *(*malloc_func)(int),
				void (*free_func)(void *)){
    libdeflate_set_memory_allocator((void * (*)(size_t))malloc_func, free_func);
}

