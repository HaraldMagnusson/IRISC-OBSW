/* -----------------------------------------------------------------------------
 * Component Name: Image Handler
 * Parent Component: Img Processing
 * Author(s): William Eriksson
 * Purpose: Process and store images along with housekeeping data, as well as
 *          sending it to ground.
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zstd.h>
#include <errno.h>
#include <sys/stat.h>

static size_t fread_return_size(void* buffer, size_t sizeToRead, FILE* file)
{
    size_t const readSize = fread(buffer, 1, sizeToRead, file);
    if (readSize == sizeToRead) return readSize;
    if (feof(file)) return readSize;
    
    logging(ERROR, "image_handler",
    "Error while reading file");
    exit(ERROR);
}

static void compress_file(const char* file_name_in, const char* file_name_out, int c_level) {

    FILE* file_in = fopen(file_name_in, "rb");
    FILE* file_out = fopen(file_name_out, "wb");

    size_t const buff_in_size = ZSTD_CStreamInSize();
    void* const buff_in = malloc(buff_in_size);
    size_t const buff_out_size = ZSTD_CStreamInSize();
    void* const buff_out = malloc(buff_out_size);

    ZSTD_CCtx* const cctx = ZSTD_createCCtx();

    ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, c_level);
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_checksumFlag, 1);

    size_t const to_read = buff_in_size;
    size_t read;
    
    while ((read = fread_return_size(buff_in, to_read, file_in))) {

        int const last_chunk = (read < to_read);
        
        ZSTD_EndDirective const mode = last_chunk ? ZSTD_e_end : ZSTD_e_continue;
        ZSTD_inBuffer input = { buff_in, read, 0 };
        int finished;
        
        do {
            ZSTD_outBuffer output = { buff_out, buff_out_size, 0 };
            size_t const remaining = ZSTD_compressStream2(cctx, &output, &input, mode);

            fwrite(buff_out, 1, output.pos, file_out);

            finished = last_chunk ? (remaining == 0) : (input.pos == input.size);

        } while (!finished);

    }

    ZSTD_freeCCtx(cctx);
    fclose(file_out);
    fclose(file_in);
    free(buff_in);
    free(buff_out);
}

/* compression_stream:
 * Compresses a file.
 *
 * input:
 * in_filename: filepath of file to be compressed.
 */
int compression_stream(const char* in_filename) {

    size_t const in_length = strlen(in_filename);
    size_t const out_length = in_length + 5;
    char* const out_filename = (char *) malloc(out_length);
    memset(out_filename, 0, out_length);
    strcat(out_filename, in_filename);
    strcat(out_filename, ".zst");

    compress_file(in_filename, out_filename, 1);
    free(out_filename);

    return SUCCESS;

}

int init_image_handler(void) {

    return SUCCESS;

}
