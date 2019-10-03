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

static pthread_t thread_data;
static pthread_attr_t thread_attr;
static struct sched_param param;
static int ret;

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
    
    //logging(ERROR, "image_handler", "Error while reading file");
    exit(ERROR);
}

static void compress_file(const char* file_name_in, const char* file_name_out, int c_level) {

    size_t ret;

    FILE* file_in = fopen(file_name_in, "rb");
    if(file_in==NULL){
        printf("Error file open file_in");
    }
    FILE* file_out = fopen(file_name_out, "wb");
    if(file_out==NULL){
        printf("Error file open file_out");
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
        //logging(ERROR, "Img Handler", "Failed to set workers to 0, %d", ret);
    }

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

            if(ZSTD_isError(remaining)){
                //logging(ERROR, "Img Handler", "compressStream2 failed, %d", remaining);
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

}

/* compression_stream:
 * Compresses a file.
 *
 * input:
 * in_filename: filepath of file to be compressed.
 * out_filename: filepath for storage location
 */
int compression_stream(const char* in_filename, const char* out_filename) {
    /*
    size_t const in_length = strlen(in_filename);
    size_t const out_length = in_length + 5;
    char* const out_filename = (char *) malloc(out_length);
    memset(out_filename, 0, out_length);
    strcat(out_filename, in_filename);
    strcat(out_filename, ".zst");
    */
    compress_file(in_filename, out_filename, 3);
    //free(out_filename);

    return SUCCESS;

}

int init_image_handler(void* args) {

    /*
     *  --Thread
     */

    ret = pthread_attr_init(&thread_attr);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_init for e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setstacksize(&thread_attr, PTHREAD_STACK_MIN);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setstacksize of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setschedpolicy of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    param.sched_priority = 19;
    ret = pthread_attr_setschedparam(&thread_attr, &param);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setschedparam of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_attr_setinheritsched of e_link component. "
            "Return value: %d\n", ret);
        return ret;
    }

    ret = pthread_create(&thread_data, &thread_attr, thread_func, NULL);
    if( ret != 0 ){
        fprintf(stderr,
            "Failed pthread_create of socket component. "
            "Return value: %d\n", ret);
        return ret;
    }

    return SUCCESS;

}

static void* thread_func(void* param){

    char out_name[100];
    struct node temp;

    struct tm date_time;
    time_t epoch_time;
    
    while(1){

        temp = read_data_queue();

        time(&epoch_time);
        gmtime_r(&epoch_time, &date_time);

        if(temp.type==IMAGE_MAIN){
            
            sprintf(out_name, "/tmp/IMG_MAIN/IMG_MAIN_%02d:%02d:%02d_.fit.zst",
                    date_time.tm_hour, date_time.tm_min, date_time.tm_sec);
            //img_main_counter++;

        } else if (temp.type==IMAGE_STARTRACKER){

            sprintf(out_name, "/tmp/IMG_START/IMG_START_%02d:%02d:%02d_.fit.zst",
                    date_time.tm_hour, date_time.tm_min, date_time.tm_sec);
            //img_startracker_counter++;

        }

        compression_stream(temp.filepath, out_name);
        //remove(temp.filepath);
        send_telemetry(out_name, temp.priority, 1, 0);
        //check_downlink_list();

        sleep(10);


    }

    return SUCCESS;
}
