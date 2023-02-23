#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

Status read_and_validate_encode_args(char *argv[],EncodeInfo *encinfo) //function to read and validate the command line arguments
{
	char *ptr;
	if(strstr(argv[2],".bmp"))
	{
		encinfo->src_image_fname=argv[2];
		
		if(ptr=strchr(argv[3],'.'))
	      {
		  encinfo->secret_fname=argv[3];
		  strcpy(encinfo->extn_secret_file,ptr);
		  		    
		         if(argv[4]!=NULL)
	             {
	                 if(strstr(argv[4],".bmp"))
	                 {
		                 encinfo->stego_image_fname=argv[4];
				         return e_success;
	                 }
					 else
					 {
						 return e_failure;
					 }
		     }
	             else
	             {
					 encinfo->stego_image_fname="default.bmp";
					 return e_success;
	             }
		  }
	             
	      else
	      {
	         return e_failure;
	      }
	}
	else
	{
		return e_failure;
	}             

	
}



uint get_image_size_for_bmp(FILE *fptr_image) //function to get the size of the image
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

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo) //function to opening all the neccessary files
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo) //function to check if the image has the required capacity to store the secret data
{
int secret_file1;
fseek(encInfo->fptr_secret,0,SEEK_END);
secret_file1=ftell(encInfo->fptr_secret);
rewind(encInfo->fptr_secret);

if(get_image_size_for_bmp(encInfo->fptr_src_image)>(54+16+32+(sizeof(encInfo->extn_secret_file)*8)+32+(secret_file1*8))) //to check size of image
{
	return e_success;
}
}

Status copy_bmp_header(FILE *fptr_src,FILE *fptr_dest) //function to copy the image header content
{
	rewind(fptr_src);
	char buff[54];
	fread(buff,54,1,fptr_src);
	fwrite(buff,54,1,fptr_dest);
	return e_success;
}

Status encode_magic_string(char *magic_string,EncodeInfo *encInfo) //function to enode the magic string
{
	if(encode_data_to_image(magic_string,2,encInfo->fptr_src_image,encInfo->fptr_stego_image,encInfo)==e_success)
	{
		return e_success;
	}
}

Status encode_data_to_image(char *data,int size,FILE *src_image,FILE *dest_image,EncodeInfo *encInfo) 
{
	for(int i=0;i<size;i++)
	{
      fread(encInfo->image_data,8,1,src_image);
	  encode_byte_to_lsb(data[i],encInfo->image_data);
	  fwrite(encInfo->image_data,8,1,dest_image);
	}
	return e_success;
}

Status encode_byte_to_lsb(char data,char *image_buffer)
{
	unsigned int mask=0x80;
	for(int i=0;i<8;i++)
	{
		image_buffer[i]=(image_buffer[i] & 0xFE) | ((data & mask)>>(7-i));
	    mask=mask>>1;	
	}
	return e_success;
}

Status encode_secret_file_extn_size(int size,FILE *src_image,FILE *dest_image) //function to encode the secret file extension
{
	char str[32];
	fread(str,32,1,src_image);
	encode_size_to_lsb(size,str);
	fwrite(str,32,1,dest_image);
	return e_success;
}

Status encode_size_to_lsb(int data,char *str) //function to encode sizes(integer values)
{
	unsigned int mask=1<<31;
	for(int i=0;i<32;i++)
	{
		str[i]=(str[i] & 0xFE) | ((data & mask) >> (31-i));
		mask=mask>>1;

	}
}

Status encode_secret_file_extn(char *file_extn,EncodeInfo *encInfo) //function to return success if file extension encoded successfully
{
if(encode_data_to_image(file_extn,strlen(file_extn),encInfo->fptr_src_image,encInfo->fptr_stego_image,encInfo)==e_success)
{
	return e_success;
}
}

Status encode_secret_file_size(long file_size,EncodeInfo *encInfo) //encoding the secret file size 
{

	fseek(encInfo->fptr_secret,0,SEEK_END);
	int size=ftell(encInfo->fptr_secret);
	rewind(encInfo->fptr_secret);
	char str[32];
	fread(str,32,1,encInfo->fptr_src_image);
	encode_size_to_lsb(size,str);
	fwrite(str,32,1,encInfo->fptr_stego_image);
	return e_success;

}

Status encode_secret_file_data(EncodeInfo *encInfo) //encoding the secret file data
{
fseek(encInfo->fptr_secret,0,SEEK_END);
int size1=ftell(encInfo->fptr_secret);
rewind(encInfo->fptr_secret);
char str1[size1];
fread(str1,size1,1,encInfo->fptr_secret);
if(encode_data_to_image(str1,size1,encInfo->fptr_src_image,encInfo->fptr_stego_image,encInfo)==e_success) //returning success if data is encoded to image
{
	return e_success;
}
}

Status copy_remaining_img_data(FILE *src_image,FILE *dest_image) //coping the remaining data
{
char ch;
while(fread(&ch,1,1,src_image)>0)
{
	fwrite(&ch,1,1,dest_image);
}
return e_success;
}

