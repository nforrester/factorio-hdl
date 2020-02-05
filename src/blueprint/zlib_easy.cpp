/* Adapted from https://zlib.net/zlib_how.html */

#include "zlib_easy.h"
#include "src/util.h"

#include <cstring>
#include <array>
#include <zlib.h>

namespace
{
    size_t constexpr chunk_size = 256 * 1024;
}

std::string zlib_decompress(std::string const & compressed)
{
    int zlib_return_code = Z_STREAM_END;
    size_t zlib_bytes_produced;
    z_stream z;
    std::array<unsigned char, chunk_size> in_buffer;
    std::array<unsigned char, chunk_size> out_buffer;
    std::string plain;

    /* allocate inflate state */
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;
    z.avail_in = 0;
    z.next_in = Z_NULL;
    assert(inflateInit(&z) == Z_OK);

    /* decompress until deflate stream ends or end of string */
    size_t current_index = 0;
    char const * current_location = compressed.data();
    do {
        z.avail_in = std::min(chunk_size, compressed.size() - current_index);
        memcpy(in_buffer.data(), current_location, z.avail_in);
        z.next_in = in_buffer.data();

        current_index += z.avail_in;
        current_location += z.avail_in;

        /* run inflate() on input until output buffer not full */
        do {
            z.avail_out = chunk_size;
            z.next_out = out_buffer.data();
            zlib_return_code = inflate(&z, Z_NO_FLUSH);
            assert(zlib_return_code != Z_STREAM_ERROR);  /* state not clobbered */
            switch (zlib_return_code) {
            case Z_NEED_DICT:
                zlib_return_code = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&z);
                assert(false);
            }
            zlib_bytes_produced = chunk_size - z.avail_out;
            plain.insert(plain.size(),
                         reinterpret_cast<char *>(out_buffer.data()),
                         zlib_bytes_produced);
        } while (z.avail_out == 0);

        /* done when inflate() says it's done */
    } while (zlib_return_code != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&z);
    assert(zlib_return_code == Z_STREAM_END);
    return plain;
}

std::string zlib_compress(std::string const & plain)
{
    int zlib_return_code;
    int zlib_should_flush;
    unsigned zlib_bytes_produced;
    z_stream z;
    std::array<unsigned char, chunk_size> in_buffer;
    std::array<unsigned char, chunk_size> out_buffer;
    std::string compressed;

    /* allocate deflate state */
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;
    assert(deflateInit(&z, 9) == Z_OK);

    /* compress until end of string */
    size_t current_index = 0;
    char const * current_location = plain.data();
    do {
        z.avail_in = std::min(chunk_size, plain.size() - current_index);
        memcpy(in_buffer.data(), current_location, z.avail_in);
        z.next_in = in_buffer.data();

        current_index += z.avail_in;
        current_location += z.avail_in;

        zlib_should_flush = current_index == plain.size() ? Z_FINISH : Z_NO_FLUSH;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            z.avail_out = chunk_size;
            z.next_out = out_buffer.data();
            zlib_return_code = deflate(&z, zlib_should_flush);    /* no bad return value */
            assert(zlib_return_code != Z_STREAM_ERROR);  /* state not clobbered */
            zlib_bytes_produced = chunk_size - z.avail_out;
            compressed.insert(compressed.size(),
                              reinterpret_cast<char *>(out_buffer.data()),
                              zlib_bytes_produced);
        } while (z.avail_out == 0);
        assert(z.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (zlib_should_flush != Z_FINISH);
    assert(zlib_return_code == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&z);
    return compressed;
}
