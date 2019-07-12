/* -----------------------------------------------------------------------------
 * Component Name: Image Handler
 * Parent Component: Img Processing
 * Author(s):
 * Purpose: Process and store images along with housekeeping data, as well as
 *          sending it to ground.
 * -----------------------------------------------------------------------------
 */

#include "global_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zstd.h>

static size_t fread_orDie(void* buffer, size_t sizeToRead, FILE* file)
{
    size_t const readSize = fread(buffer, 1, sizeToRead, file);
    if (readSize == sizeToRead) return readSize;   /* good */
    if (feof(file)) return readSize;   /* good, reached end of file */
    /* error */
    perror("fread");
    exit(4);        // --------fixa
}

static size_t fwrite_orDie(const void* buffer, size_t sizeToWrite, FILE* file)
{
    size_t const writtenSize = fwrite(buffer, 1, sizeToWrite, file);
    if (writtenSize == sizeToWrite) return sizeToWrite;   /* good */
    /* error */
    perror("fwrite");
    exit(5);        // --------fixa
}

static void compress_file(const char* file_name_in, const char* file_name_out, int c_level) {

    FILE* const file_in = fopen(file_name_in, "rb");
    FILE* const file_out = fopen(file_name_out, "wb");

    size_t const buff_in_size = ZSTD_CStreamInSize();
    void* const buff_in = malloc(buff_in_size);
    size_t const buff_out_size = ZSTD_CStreamInSize();
    void* const buff_out = malloc(buff_out_size);

    ZSTD_CCtx* const cctx = ZSTD_createCCtx();
    /* check? */

    ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, c_level);
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_checksumFlag, 1);

    size_t const to_read = buff_in_size;
    size_t read;
    while ((read = fread_orDie(buff_in, to_read, file_in))) {

        int const last_chunk = (read < to_read);
        ZSTD_EndDirective const mode = last_chunk ? ZSTD_e_end : ZSTD_e_continue;

        ZSTD_inBuffer input = { buff_in, read, 0 };
        int finished;
        do {
            ZSTD_outBuffer output = { buff_out, buff_out_size, 0 };
            size_t const remaining = ZSTD_compressStream2(cctx, &output, &input, mode);
            /*check_zstd ??*/
            fwrite_orDie(buff_out, output.pos, file_out);

            finished = last_chunk ? (remaining == 0) : (input.pos == input.size);

        } while (!finished);
            /*check input.pos??*/

    }

    ZSTD_freeCCtx(cctx);
    fclose(file_out);
    fclose(file_in);
    free(buff_in);
    free(buff_out);
}


static char* create_outputfilename(const char* filename)
{
    size_t const inL = strlen(filename);
    size_t const outL = inL + 5;
    char* const outSpace = (char *) malloc(outL);
    memset(outSpace, 0, outL);
    strcat(outSpace, filename);
    strcat(outSpace, ".zst");
    return (char*)outSpace;
}

int compression_stream(const char* argv) {

    const char* const in_filename = argv;
    char* const out_filename = create_outputfilename(in_filename);
    compress_file(in_filename, out_filename, 1);

    free(out_filename);

    return 0;

}

int test() {


    char intput_file[] = "/irisc-obsw/src/img_processing/image_handler/test.FIT";
        compression_stream(intput_file);

    return 2;
}

int init_image_handler(void) {

    test();

    return SUCCESS;
}
