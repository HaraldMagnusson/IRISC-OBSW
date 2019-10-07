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
#include <pthread.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

#include "data_queue.h"
#include "img_processing.h"
#include "telemetry.h"

int img_main_counter = 0;
int img_startracker_counter = 0;

/* prototypes declaration */
static void* thread_func(void*);
int compression_stream(const char* in_filename, const char* out_filename);

static size_t fread_return_size(void* buffer, size_t sizeToRead, FILE* file)
{
    size_t const readSize = fread(buffer, 1, sizeToRead, file);
    if (readSize == sizeToRead) return readSize;
    if (feof(file)) return readSize;
    logging(ERROR, "image_handler", "fread_return_size failed");
    return FAILURE;
}

static int compress_file(const char* file_name_in, const char* file_name_out, int c_level) {

    size_t ret;

    FILE* file_in = fopen(file_name_in, "rb");
    if(file_in==NULL){
        logging(ERROR, "image_handler", "Could not open in file: %m");
        return FAILURE;
    }
    FILE* file_out = fopen(file_name_out, "wb");
    if(file_out==NULL){
        logging(ERROR, "image_handler", "Could not open out file");
        return FAILURE;
    }

    
    size_t const buff_in_size = ZSTD_CStreamInSize();
    void* const buff_in = malloc(buff_in_size);
    size_t const buff_out_size = ZSTD_CStreamInSize();
    void* const buff_out = malloc(buff_out_size);

    ZSTD_CCtx* const cctx = ZSTD_createCCtx();

    ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, c_level);
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_checksumFlag, 1);
    ret = ZSTD_CCtx_setParameter(cctx, ZSTD_c_nbWorkers, 0);
    if(ZSTD_isError(ret)){
        logging(ERROR, "Img Handler", "Failed to set workers to 0, %d", ret);
    }

    size_t const to_read = buff_in_size;
    size_t read;
    
    while ((read = fread_return_size(buff_in, to_read, file_in))) {
        if(read==-1){
            return FAILURE;
        }

        int const last_chunk = (read < to_read);
        
        ZSTD_EndDirective const mode = last_chunk ? ZSTD_e_end : ZSTD_e_continue;
        ZSTD_inBuffer input = { buff_in, read, 0 };
        int finished;
        
        do {
            ZSTD_outBuffer output = { buff_out, buff_out_size, 0 };
            size_t const remaining = ZSTD_compressStream2(cctx, &output, &input, mode);

            if(ZSTD_isError(remaining)){
                logging(ERROR, "Img Handler", "compressStream2 failed, %d", remaining);
                return FAILURE;
            }

            fwrite(buff_out, 1, output.pos, file_out);

            finished = last_chunk ? (remaining == 0) : (input.pos == input.size);

        } while (!finished);

    }

    ZSTD_freeCCtx(cctx);
    fclose(file_out);
    fclose(file_in);
    free(buff_in);
    free(buff_out);

    return SUCCESS;

}

/* compression_stream:
 * Compresses a file.
 *
 * input:
 * in_filename: filepath of file to be compressed.
 * out_filename: filepath for storage location
 */
int compression_stream(const char* in_filename, const char* out_filename) {

    return compress_file(in_filename, out_filename, COMPRESSION_LEVEL);

}

char st_fp[100];
char nir_fp[100];

int init_image_handler(void* args) {

    strcpy(st_fp, get_top_dir());
    strcat(st_fp, "output/guiding/");

    strcpy(nir_fp, get_top_dir());
    strcat(nir_fp, "output/nir/");

    return create_thread("image_handler", thread_func, 19);

}

static void* thread_func(void* param){

    char out_name[100];
    struct node temp;

    struct tm date_time;
    time_t epoch_time;

    while(1){

        temp = read_data_queue();

        time(&epoch_time);
        localtime_r(&epoch_time, &date_time);

        if(temp.type==IMAGE_MAIN){

            sprintf(out_name, "%sIMG_MAIN_%02d:%02d:%02d.fit.zst", nir_fp,
                    date_time.tm_hour, date_time.tm_min, date_time.tm_sec);

        } else if (temp.type==IMAGE_STARTRACKER){

            sprintf(out_name, "%sIMG_ST_%02d:%02d:%02d.fit.zst", st_fp,
                    date_time.tm_hour, date_time.tm_min, date_time.tm_sec);
        }

        if(compression_stream(temp.filepath, out_name)){
            queue_image(temp.filepath, temp.type);
        } else {
            send_telemetry(out_name, temp.priority, 1, 0);
            remove(temp.filepath);
        }
    }

    return SUCCESS;
}
