#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>
#include "types.h"

typedef struct _DecodeInfo
{
    char *secret_fname;//to store secret file name
    FILE *fptr_secret;//to store secret file address
    char extn_secret_file[5];//to store the secret file extension
    long size_secret_extn;//to store the secret file extension size
    long size_secret_file;//to store the secret file size

    char *stego_image_fname;// to store the file name of the file to be decoded
    FILE *fptr_stego_image;// to store the file address 

} DecodeInfo;

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);//to read and validate decode args
Status do_decoding(DecodeInfo *decoInfo);//perform decoding
Status open_files2(DecodeInfo *decInfo);//to open the files
Status decode_magic_string(const char*magic_string,DecodeInfo *decInfo);//to decoding the magic string
Status decode_secret_file_extn_size( DecodeInfo *decInfo);// to decoding the secret file extenson size
Status decode_secret_file_extn( DecodeInfo *decInfo);// to decoding the secret file extenson 
Status decode_secret_file_size( DecodeInfo *decInfo);// to decoding the secret file size
Status decode_secret_file_data(DecodeInfo *decInfo);// to decoding the secret file data
char decode_byte_from_lsb( char *image_buffer);//decode a byte into LSB 
int decode_size_from_lsb(char *imageBuffer);//decode a size into LSB 
#endif
