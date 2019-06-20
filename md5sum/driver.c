/*
 * Author: Ming Li(adagio.ming@gmail.com)
 * Date: 2018/12/05
 * Note -
 *	1. code follows C99 standard complilation and has CPP support.
 *	2. code is based on md5 algorithm which's not a cryptographic method for hash.
 *	3. code is comprised of severail functions for md5 utility implementation.
 *
 */

#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "md5.h"

/*
 * Forward decalarations.
 */
/* file to be digested */
static void md5_file(const char *filename);
/* string to be digested */
static void md5_string(const unsigned char* data);



static int8_t is_dir(struct stat buf)
{
	return ((buf.st_mode & S_IFMT) == S_IFDIR)? 1 : 0;
}

static int8_t is_reg(struct stat buf)
{
	return ((buf.st_mode & S_IFMT) == S_IFREG)? 1 : 0;
}


static void md5_usage(){
	printf("md5 usage:\n");
	printf("\tmd5 \t[-f|--file]\t[filename]\n");
	printf("\t\t[-s|--string]\t\"xxx\"\n");
}

static void md5_file(const char *filename){
	struct stat buf;

	stat(filename, &buf);

	if(is_dir(buf)){
		printf("this is directory, plz specify a regular file\n");
		return ;
	}
	if(is_reg(buf) && access(filename, F_OK)){
		printf("file %s exists, but not accessible\n", filename);
		return ;
	}

	FILE *file;
	MD5_CTX context;
	unsigned long file_byte_len;
	uint8_t buffer[1024] = {0};

	if((file = fopen(filename, "r")) == NULL) {
		printf("%s can't be opened\n", filename);
		return ;
	} else {
		fseek(file, 0, SEEK_END);
		file_byte_len = (unsigned long)ftell(file);
		fseek(file, 0, SEEK_SET);
		md5_ctx_init(&context, file_byte_len);

		unsigned int len;

		while((len = fread(buffer, 1, 1024, file))) {
			md5_calc(&context, buffer, len);
			memset(&buffer, 0, 1024);
		}

		printf("MD5 (%s) = ", filename);
		md5_print(context.state);
	}

	fclose(file);
}

static void md5_string(const unsigned char* data)
{
	MD5_CTX context;
	unsigned int len = strlen((const char *)data);

	if(data[0] == '\"' && data[len - 1] == '\"'){
		data += 1;
		len -= 2;
	}

	md5_ctx_init(&context, len);
	md5_calc(&context, data, len);

	md5_print(context.state);
}


/* Main Entry */
int main(int argc, char ** argv)
{
	if(argc == 2 && strlen(argv[1])){
		md5_file(argv[1]);
		return 0;
	}

	if (argc != 3) {
		md5_usage();
		return 0;
	}

	if(!(strcmp(argv[1], "-f") && strcmp(argv[1], "--file")))
		md5_file(argv[2]);
	else if(!(strcmp(argv[1], "-s") && strcmp(argv[1], "--string"))){
		md5_string((unsigned char *)argv[2]);
	} else {
		md5_usage();
	}
	return 0;
}
