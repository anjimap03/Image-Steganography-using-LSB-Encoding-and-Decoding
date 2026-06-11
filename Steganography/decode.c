#include <stdio.h>
#include <string.h>
#include "decode.h"
#include"types.h"
#include"common.h"
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *ptr = strstr(argv[2], ".bmp");//checking if the input file has (.bmp) extension
    if ((ptr != NULL) && ((strcmp(ptr, ".bmp")) == 0))
        decInfo->stego_image_fname= argv[2];//storing the file name
    else
    {
        printf("Only (.bmp) files can be used\n");
        return e_failure;
    }
    if (!(argv[3] == NULL))//checking if the output file name is given or not
    {
            decInfo->secret_fname = argv[3];//store output file name
            return e_success;
    }
        else
        {
            decInfo->secret_fname = "secret2";//use default filename
            return e_success;
        }
    
}
Status open_files2(DecodeInfo *decInfo)
{
        printf("INFO: Opening required files for decoding\n");
        // Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }
    return e_success;
}
Status decode_magic_string(const char*magic_string,DecodeInfo *decInfo)
{
    printf("INFO: Decoding magic string\n");
     char imagebuffer[8];            //create an imagebuffer
     int len=strlen(magic_string);//finding the length of magic string
     char decode_string[len+1 ];
    for(int i=0;i<len;i++)              //generate the loop upto the magic string length
    {
        fread(imagebuffer,8,1,decInfo->fptr_stego_image);    //read 8bytes from src image and store into the imagebuffer
        decode_string[i]=decode_byte_from_lsb(imagebuffer);//fwrite image buffer to the destination image
    }
    decode_string[len]='\0';//Null terminate decoded string

    if(strcmp(decode_string,magic_string)==0)//comparing the decoded string and magic string
    {
        printf("INFO: Magic string matched\n");
        return e_success;
    }
    
    else
    return e_failure;
}
Status decode_secret_file_extn_size( DecodeInfo *decInfo)
{
    printf("INFO: Decoding secret file extension size\n");
    char imagebuffer[32];//buffer to store 32 image bytes
    fread(imagebuffer,32,1,decInfo->fptr_stego_image);//reading 32 bytes from image
    decInfo->size_secret_extn=decode_size_from_lsb(imagebuffer);//decode extension size
    printf("INFO: Extension size = %ld\n",decInfo->size_secret_extn);//print extension size
    return e_success;
}
Status decode_secret_file_extn( DecodeInfo *decInfo)
{
        printf("INFO: Decoding secret file extension \n");
    char imagebuffer[8];//buffer to store image bytes
    for(int i=0;i<decInfo->size_secret_extn;i++)
    {
        fread(imagebuffer,8,1,decInfo->fptr_stego_image);//read 8bytes from image
        decInfo->extn_secret_file[i]=decode_byte_from_lsb(imagebuffer);//decode one extension character
    }
    decInfo->extn_secret_file[decInfo->size_secret_extn]='\0';//Null terminate extension string
    return e_success;
}
Status decode_secret_file_size( DecodeInfo *decInfo)//
{
        printf("INFO: Decoding secret file size \n");
    char imagebuffer[32];//buffer to store image bytes
    fread(imagebuffer,32,1,decInfo->fptr_stego_image);//reading 32 bytes from image
    decInfo->size_secret_file=decode_size_from_lsb(imagebuffer);//decode secret file size
    return e_success;
}
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    printf("INFO: Decoding secret file data\n");
     char ch;//variable to store decoded character
     char imagebuffer[8];//buffer to store 8 image bytes
     for(int i=0;i<decInfo->size_secret_file;i++)
    {
        fread(imagebuffer,8,1,decInfo->fptr_stego_image);//read 8 bytes from image
        ch=decode_byte_from_lsb(imagebuffer);//decode one byte
        fputc(ch,decInfo->fptr_secret);//write decoded byte to output file
    }
    return e_success;
}
int decode_size_from_lsb(char *imageBuffer)
{
    int size = 0;//initialize size as zero

    for (int i = 0; i < 32; i++)
    {
        size = size << 1;//left shift size
        size = size | (imageBuffer[i] & 1);//extract LSB and append to size
    }

    return size;//return decoded size
}

char decode_byte_from_lsb( char *image_buffer)
{
     char ch=0;//initialize decoded character to zero
    for(int i=0;i<8;i++)//generate a loop upto 8times
    {
        char get_bit=image_buffer[i] & 1;//get the LSB from image byte
        ch=(ch<<1)|get_bit;//Append bit to character
    }
    return ch;//return the decoded character
}
Status do_decoding(DecodeInfo * decInfo)
{
    if (open_files2(decInfo) == e_success)//open stego image file
        {
            printf("INFO: Skipping BMP header(54 bytes)\n"); 
            fseek(decInfo->fptr_stego_image, 54, SEEK_SET);//skipping the bmp header
            if (decode_magic_string(MAGIC_STRING,decInfo) == e_success)//decode magic string
            {
                if (decode_secret_file_extn_size(decInfo) == e_success)//decode extension size
                {
                    if (decode_secret_file_extn(decInfo) == e_success)//decode extension
                    {
                        char output_fname[100];//buffer for output filename
                        char *token = strtok(decInfo->secret_fname, ".");//extract base name
                        if (token != NULL)
                        strcpy(output_fname, token);//copying base name

                        strcat(output_fname, decInfo->extn_secret_file);//concatinating the file name and file extension
                        // Secret file
                        decInfo->fptr_secret = fopen(output_fname ,"w");//open output file
                        // Do Error handling
                        printf("INFO: Creating output file %s\n",output_fname);
                        if (decInfo->fptr_secret == NULL)//check for file open error
                        {
                            perror("fopen");
                            fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);

                            return e_failure;
                        }
                        printf("INFO: Output file opened successfully\n");
                        if (decode_secret_file_size(decInfo) == e_success)//decode secret file size
                        {
                            if (decode_secret_file_data(decInfo) == e_success)//decode secret data
                            {
                                return e_success;
                            }
                            return e_failure;
                        }
                        else
                            return e_failure;
                    }
                    else
                        return e_failure;
                }
                else
                    return e_failure;
            }
            else
                return e_failure;
        }
        else
            return e_failure;
}