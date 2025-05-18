/*
 * libdeflate.h - public header for libdeflate
 */

#ifndef TINYLIBDEFLATE_H
#define TINYLIBDEFLATE_H

#ifdef __cplusplus
extern "C" {
#endif

struct libdeflate_compressor;

/*
 * libdeflate_alloc_compressor() allocates a new compressor that supports
 * DEFLATE, zlib, and gzip compression.  'compression_level' is the compression
 * level on a zlib-like scale but with a higher maximum value (1 = fastest, 6 =
 * medium/default, 9 = slow, 12 = slowest).  Level 0 is also supported and means
 * "no compression", specifically "create a valid stream, but only emit
 * uncompressed blocks" (this will expand the data slightly).
 *
 * The return value is a pointer to the new compressor, or NULL if out of memory
 * or if the compression level is invalid (i.e. outside the range [0, 12]).
 *
 * Note: for compression, the sliding window size is defined at compilation time
 * to 32768, the largest size permissible in the DEFLATE format.  It cannot be
 * changed at runtime.
 *
 * A single compressor is not safe to use by multiple threads concurrently.
 * However, different threads may use different compressors concurrently.
 */
struct libdeflate_compressor *
deflate_alloc_compressor(int compression_level);

/*
 * libdeflate_deflate_compress() performs raw DEFLATE compression on a buffer of
 * data.  It attempts to compress 'in_nbytes' bytes of data located at 'in' and
 * write the result to 'out', which has space for 'out_nbytes_avail' bytes.  The
 * return value is the compressed size in bytes, or 0 if the data could not be
 * compressed to 'out_nbytes_avail' bytes or fewer (but see note below).
 *
 * If compression is successful, then the output data is guaranteed to be a
 * valid DEFLATE stream that decompresses to the input data.  No other
 * guarantees are made about the output data.  Notably, different versions of
 * libdeflate can produce different compressed data for the same uncompressed
 * data, even at the same compression level.  Do ***NOT*** do things like
 * writing tests that compare compressed data to a golden output, as this can
 * break when libdeflate is updated.  (This property isn't specific to
 * libdeflate; the same is true for zlib and other compression libraries too.)
 *
 * Note: due to a performance optimization, libdeflate_deflate_compress()
 * currently needs a small amount of slack space at the end of the output
 * buffer.  As a result, it can't actually report compressed sizes very close to
 * 'out_nbytes_avail'.  This doesn't matter in real-world use cases, and
 * libdeflate_deflate_compress_bound() already includes the slack space.
 * However, it does mean that testing code that redundantly compresses data
 * using an exact-sized output buffer won't work as might be expected:
 *
 *	out_nbytes = libdeflate_deflate_compress(c, in, in_nbytes, out,
 *						 libdeflate_deflate_compress_bound(in_nbytes));
 *	// The following assertion will fail.
 *	assert(libdeflate_deflate_compress(c, in, in_nbytes, out, out_nbytes) != 0);
 *
 * To avoid this, either don't write tests like the above, or make sure to
 * include at least 9 bytes of slack space in 'out_nbytes_avail'.
 */
int
deflate_deflate_compress(struct libdeflate_compressor *compressor,
			    const void *in, int in_nbytes,
			    void *out, int out_nbytes_avail);

/*
 * libdeflate_free_compressor() frees a compressor that was allocated with
 * libdeflate_alloc_compressor().  If a NULL pointer is passed in, no action is
 * taken.
 */
void
deflate_free_compressor(struct libdeflate_compressor *compressor);

/* ========================================================================== */
/*                             Decompression                                  */
/* ========================================================================== */

struct libdeflate_decompressor;

/*
 * libdeflate_alloc_decompressor() allocates a new decompressor that can be used
 * for DEFLATE, zlib, and gzip decompression.  The return value is a pointer to
 * the new decompressor, or NULL if out of memory.
 *
 * This function takes no parameters, and the returned decompressor is valid for
 * decompressing data that was compressed at any compression level and with any
 * sliding window size.
 *
 * A single decompressor is not safe to use by multiple threads concurrently.
 * However, different threads may use different decompressors concurrently.
 */
struct libdeflate_decompressor *
deflate_alloc_decompressor(void);

/*
 * libdeflate_deflate_decompress() decompresses a DEFLATE stream from the buffer
 * 'in' with compressed size up to 'in_nbytes' bytes.  The uncompressed data is
 * written to 'out', a buffer with size 'out_nbytes_avail' bytes.  If
 * decompression succeeds, then 0 (LIBDEFLATE_SUCCESS) is returned.  Otherwise,
 * a nonzero result code such as LIBDEFLATE_BAD_DATA is returned, and the
 * contents of the output buffer are undefined.
 *
 * Decompression stops at the end of the DEFLATE stream (as indicated by the
 * BFINAL flag), even if it is actually shorter than 'in_nbytes' bytes.
 *
 * libdeflate_deflate_decompress() can be used in cases where the actual
 * uncompressed size is known (recommended) or unknown (not recommended):
 *
 *   - If the actual uncompressed size is known, then pass the actual
 *     uncompressed size as 'out_nbytes_avail' and pass NULL for
 *     'actual_out_nbytes_ret'.  This makes libdeflate_deflate_decompress() fail
 *     with LIBDEFLATE_SHORT_OUTPUT if the data decompressed to fewer than the
 *     specified number of bytes.
 *
 *   - If the actual uncompressed size is unknown, then provide a non-NULL
 *     'actual_out_nbytes_ret' and provide a buffer with some size
 *     'out_nbytes_avail' that you think is large enough to hold all the
 *     uncompressed data.  In this case, if the data decompresses to less than
 *     or equal to 'out_nbytes_avail' bytes, then
 *     libdeflate_deflate_decompress() will write the actual uncompressed size
 *     to *actual_out_nbytes_ret and return 0 (LIBDEFLATE_SUCCESS).  Otherwise,
 *     it will return LIBDEFLATE_INSUFFICIENT_SPACE if the provided buffer was
 *     not large enough but no other problems were encountered, or another
 *     nonzero result code if decompression failed for another reason.
 */
int
deflate_deflate_decompress(struct libdeflate_decompressor *decompressor,
			      const void *in, int in_nbytes,
			      void *out, int out_nbytes_avail,
			      int *actual_out_nbytes_ret);

/*
 * libdeflate_free_decompressor() frees a decompressor that was allocated with
 * libdeflate_alloc_decompressor().  If a NULL pointer is passed in, no action
 * is taken.
 */
void
deflate_free_decompressor(struct libdeflate_decompressor *decompressor);

/* ========================================================================== */
/*                           Custom memory allocator                          */
/* ========================================================================== */

/*
 * Install a custom memory allocator which libdeflate will use for all memory
 * allocations by default.  'malloc_func' is a function that must behave like
 * malloc(), and 'free_func' is a function that must behave like free().
 *
 * The per-(de)compressor custom memory allocator that can be specified in
 * 'struct libdeflate_options' takes priority over this.
 *
 * This doesn't affect the free() function that will be used to free
 * (de)compressors that were already in existence when this is called.
 */
void
deflate_set_memory_allocator(void *(*malloc_func)(int),
				void (*free_func)(void *));

#ifdef __cplusplus
}
#endif

#endif /* LIBDEFLATE_H */
