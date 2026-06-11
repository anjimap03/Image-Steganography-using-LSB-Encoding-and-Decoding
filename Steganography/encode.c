#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include"common.h"
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
     fseek(fptr, 0, SEEK_END);//moving the file pointer to end of the file
     uint len=ftell(fptr);//get current position
     return len; //returning file size

}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char*ptr=strstr(argv[2],".bmp");//checking if source image has(.bmp)
    if((ptr!=NULL) && ((strcmp(ptr,".bmp"))==0))
        encInfo->src_image_fname=argv[2];//storing source image filename
    else
    {
        printf("Only (.bmp) files can be used\n");
        return e_failure;
    }
    char*str=strstr(argv[3],".");//checking the file extension of the secret file
    if (str != NULL && (strcmp(str,".txt")==0 || strcmp(str,".c")==0 || strcmp(str,".h")==0 || strcmp(str,".sh")==0))
    encInfo->secret_fname=argv[3];//storing it
    else
    {
        return e_failure;
    }
    if(!(argv[4]==NULL))
    {
        char*qtr=strstr(argv[4],".bmp");//checking if the output file has(.bmp)
        if((strcmp(qtr,".bmp"))==0)
        {
            encInfo->stego_image_fname=argv[4];//storing it
            return e_success;
        }
        else
        {
            printf("Enter the correct format(.bmp)\n");
            return e_failure;
        }
    }
    else 
    {
        printf("INFO: Output File not mentioned.Creating steged_img.bmp as default\n");
       encInfo->stego_image_fname="steged_img.bmp";//giving default name for the output
        return e_success;
    }

}

Status open_files(EncodeInfo *encInfo)
{
    printf("INFO: Opening required files\n");
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }
    printf("INFO: Opened SkeletonCode/beautiful.bmp\n");
    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }
    printf("INFO: Opened secret.txt\n");
    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }
    printf("INFO: Opened steged_img.bmp\n");
    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    printf("INFO: Checking for secret.txt size\n");
    encInfo->image_capacity=get_image_size_for_bmp(encInfo->fptr_src_image);//getting source image size
    printf("INFO: Done. Not Empty\n");
    encInfo->size_secret_file=get_file_size(encInfo->fptr_secret);//getting secret file size
    int capacity=16+32+32+32+(encInfo->size_secret_file*8);
    printf("INFO: Checking for SkeletonCode/beautiful.bmp capacity to handle secret.txt\n");
    if(encInfo->image_capacity>capacity)//checking if the image can hold the secret data
    {
        printf("INFO: Done. Found OK\n");
        return e_success;
    }
    else
    return e_failure;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    printf("INFO: Copying Image Header\n");
    char imagebuffer[54];
    rewind(fptr_src_image);//rewinding the source image
    fread(imagebuffer,54,1,fptr_src_image);//reading 54 byte bmp header
    fwrite(imagebuffer,54,1,fptr_dest_image);//write header to destination file 
    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    printf("INFO: Encoding Magic String Signature\n");
     char imagebuffer[8];            //create an imagebuffer
    int len =strlen(magic_string);     //find the length of the magic string
    for(int i=0;i<len;i++)              //generate the loop upto the magic string length
    {
        fread(imagebuffer,8,1,encInfo->fptr_src_image);    //read 8bytes from src image and store into the imagebuffer
        encode_byte_to_lsb(magic_string[i],imagebuffer);//call the function encode byte to lsb
        fwrite(imagebuffer,8,1,encInfo->fptr_stego_image);//write image buffer to the destination image
    }
    long offset_1=ftell(encInfo->fptr_src_image);//check the offset of both the file pointer is 70
    long offset_2=ftell(encInfo->fptr_stego_image);
    if(offset_1==offset_2)
    return e_success;
    else
    return e_failure;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    printf("INFO: Encoding Secret File Extension Size\n");
    char imagebuffer[32];//buffer to store image bytes
    fread(imagebuffer,32,1,encInfo->fptr_src_image);//reading 32 bytes from source image
    encode_size_to_lsb(size,imagebuffer);//ecnoding the extension size to lsb
    fwrite(imagebuffer,32,1,encInfo->fptr_stego_image);//writing the modified bytes to stego image
    return e_success;
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
        printf("INFO: Encoding Secret File Extension\n");
    char imagebuffer[8];//buffer to store image bytes
    int len =strlen(file_extn);//get length of file extension
    for(int i=0;i<len;i++)//running loop upto length
    {
        fread(imagebuffer,8,1,encInfo->fptr_src_image);//reading 8 bytes from source image
        encode_byte_to_lsb(file_extn[i],imagebuffer);//encoding one extension character
        fwrite(imagebuffer,8,1,encInfo->fptr_stego_image);//writing modified bytes to stego image
    }
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    printf("INFO: Encoding Secret File Size\n");
    char imagebuffer[32];//buffer to store image bytes
    fread(imagebuffer,32,1,encInfo->fptr_src_image);//reading 32 bytes from source image
    encode_size_to_lsb(file_size,imagebuffer);//encode file size into lsb
    fwrite(imagebuffer,32,1,encInfo->fptr_stego_image);//write modified bytes to stego image
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    printf("INFO: Encoding Secret File Data\n");
     char secret_file_data[encInfo->size_secret_file];//buffer to store secret file data
     rewind(encInfo->fptr_secret);//move secret file pointer to start
     fread(secret_file_data,encInfo->size_secret_file,1,encInfo->fptr_secret);//read secret file data
     char imagebuffer[8];//byffer to store image bytes
     for(int i=0;i<encInfo->size_secret_file;i++)
    {
        fread(imagebuffer,8,1,encInfo->fptr_src_image);//read 8 bytes from source image
        encode_byte_to_lsb(secret_file_data[i],imagebuffer);//encode one data byte
        fwrite(imagebuffer,8,1,encInfo->fptr_stego_image);//write modified bytes to stego image
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    printf("INFO: Copying Left Over Data\n");
    char ch;//variable to store one byte
    while(fread(&ch,1,1,fptr_src)>0)//read remaining  bytes from source
    {
        fwrite(&ch,1,1,fptr_dest);//write bytes to destination
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=0;i<8;i++)//generate a loop upto 8times
    {
        char get_bit=data>>((7-i))&1;//get the bit from data
        image_buffer[i]=image_buffer[i]&(~1);//clear the lsb of image buffer[i];
        image_buffer[i]=image_buffer[i]|get_bit;//set the get bit into the lsb of imagebuffer[i]
    }
    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for(int i=0;i<32;i++)//running loop upto 32
    {
        char get_bit=(size>>(31-i))&1;//extract each bit from MSB to LSB
        imageBuffer[i]=imageBuffer[i]&(~1);//Clear the LSB of image byte
        imageBuffer[i]=imageBuffer[i]|get_bit;//Set the extracted bit into LSB
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo)==e_success)//opening required files
    {
        printf("INFO: Done\n");
        printf("INFO: ##Encoding Procedure Started ##\n");
        if(check_capacity(encInfo)==e_success)//checking image capacity to hold secret data
        {
            if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)//copying bmp header
            {
                printf("INFO: DONE\n");
                if(encode_magic_string(MAGIC_STRING,encInfo)==e_success)//encoding magic string
                {
                     printf("INFO: DONE\n");
                    int secret_file_extn_size=strlen(strstr(encInfo->secret_fname,"."));//get exyension length
                    if(encode_secret_file_extn_size(secret_file_extn_size,encInfo)==e_success)//encoding extension size
                    {
                                        printf("INFO: DONE\n");
                        if((encode_secret_file_extn((strstr(encInfo->secret_fname,".")),encInfo))==e_success)//encoding extension
                        {
                                        printf("INFO: DONE\n");
                            if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_success)//encoding file size
                            {
                                        printf("INFO: DONE\n");
                                if(encode_secret_file_data(encInfo)==e_success)//encoding file data
                                {
                                        printf("INFO: DONE\n");
                                    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)//copy rest of image
                                    {
                                        printf("INFO: DONE\n");
                                        return e_success;
                                    }
                                    else
                                    return e_failure;
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
        else
        {
            printf("No Capacity\n");
            return e_failure;
        }
    }
    else
    return e_failure;
}
