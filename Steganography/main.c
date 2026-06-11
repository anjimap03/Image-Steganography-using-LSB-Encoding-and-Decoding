#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"
OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    if(argc<3)//checking the argument count
    {
        printf("Insufficent arguments\n");
        return 0;
    }
        if(check_operation_type(argv[1])==e_encode)//checking if it is encoding
        {
            if(argc<4)//checking arguments count for encoding
            {
                printf("Insufficent arguments for encoding\n");
                return 0;
            }
            EncodeInfo encInfo;//declaring the structure
            if(read_and_validate_encode_args(argv,&encInfo)==e_success)//validating the encoding arguments
            {
                if(do_encoding(&encInfo)==e_success)//calling the encoding function
                {
                    printf("INFO: ##Encoding Done Successfully ##\n");
                }
                else
                {
                    printf("Encoding failed\n");
                }
            }
            else
            {
                printf("Invalid encoding arguments\n");
                return 0;
            }
        }
        else if(check_operation_type(argv[1])==e_decode)//checking if it is decoding
        {
            DecodeInfo decInfo;//declaring the structure 
            if(read_and_validate_decode_args(argv,&decInfo)==e_success)//validating the decoding arguments
            {
                if(do_decoding(&decInfo)==e_success)//calling the decoding function
                {
                    printf("INFO: ##Decoding Done Successfully ##\n");
                }
                else
                {
                    printf("Decoded failed\n");
                }
            }
            else
            {
                printf("Invalid decoding arguments\n");
                return 0;
            }
        }
        else
        {
            printf("%s is an invalid option\n",argv[1]);
            return 0;
        }
}
OperationType check_operation_type(char *symbol)
{
    if((strcmp(symbol,"-e"))==0)// checking the argument for encoding
        return e_encode;
    else if((strcmp(symbol,"-d"))==0)//checking the argument for decoding
        return e_decode;
    else
        return e_unsupported;
}
