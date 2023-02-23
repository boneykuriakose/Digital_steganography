#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
#include "decode.h"
int main(int argc , char *argv[])
{

	int option=check_operation_type(argv); //function call to check the operation type

	if(option==e_encode){ //if the selected option is encode
	
		EncodeInfo encInfo; //declaring the structure variable as encInfo 
		printf("Encoding selected\n");
	    if((read_and_validate_encode_args(argv,&encInfo))==e_success) //reading and validating all the arguments
		{
			printf("Successfully read and validated\n");
			if(open_files(&encInfo)==e_success) //openning all the neccessary files
			{
				printf("Files opened successfully\n");

				if(check_capacity(&encInfo)==e_success) //checking if the bmp file is compactable to store the secret file
				{
					printf("File has the neccessary capacity\n");
					if(copy_bmp_header(encInfo.fptr_src_image,encInfo.fptr_stego_image)==e_success) //coping the heaer file of original image to new image
					{
						printf("header contents copied successfully\n");
                        if(encode_magic_string(MAGIC_STRING,&encInfo)==e_success) //encoded the magic string to the new image
						{
							printf("Magic string copied succesfully\n");

							if(encode_secret_file_extn_size(strlen(encInfo.extn_secret_file),encInfo.fptr_src_image,encInfo.fptr_stego_image)==e_success) //size of secret file extension encoded
							{
								printf("Secret file extention size encoded succesfully\n");

								if(encode_secret_file_extn(encInfo.extn_secret_file,&encInfo)==e_success) //secret file extension encoded
								{
									printf("Secret file extention encoded successfully\n");

                                    if(encode_secret_file_size(encInfo.size_secret_file,&encInfo)==e_success) //secret file size encoded
	                                {
										printf("Secret file size encoded succesfully\n");
										if(encode_secret_file_data(&encInfo)==e_success) //secret file data encoded to the new image
										{
											printf("Secret file data encoded succesfully\n");

											if(copy_remaining_img_data(encInfo.fptr_src_image,encInfo.fptr_stego_image)==e_success) //the remaining data is copied as it is to the new image
											{
												printf("Reamaining data copied\n");
											}
											else
											{
												printf("Error\n");
											}

										}
										else
										{
											printf("Error\n");
										}
										
									}
									else
									{
										printf("error\n");
									}									
								}
								else
								{
									printf("error\n");
								}
								
							}
							else
							{
								printf("error\n");
							}
						}
						else
						{
							printf("error \n");
						}
					}
					else
					{
						printf("error\n");
					}
					

				}
			
				else
				{
					printf("File capacity check failed\n");
				}
				
			}
		}
		else
		{
			printf("Read and validation failed\n");
		}

	}
	else if(option ==e_decode) //checking if decode is selected
	{
		DecodeInfo decInfo; //declaring the structure variable as decInfo
		printf("selected decoding");
		if(read_and_validate_decode_args(argv,&decInfo)==e_success) //reading and validating the arguments for decoding
		{
			printf("Read and validate success\n");
			do_decoding(&decInfo); //function call to start the decoding
		}
	}
	else //if the entered option is neither encoding or decoding
	{
		printf("please pass the valid option");
	}

}

OperationType check_operation_type(char *argv[]) //function to check the operation type
{
	
	if(!strcmp(argv[1],"-e"))
	{
      return e_encode;
	}
    
	else if(!strcmp(argv[1],"-d"))
	{
		return e_decode;
	}
	else
	{
		return e_unsupported;
	}

}
