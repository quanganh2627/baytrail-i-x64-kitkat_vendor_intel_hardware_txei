#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "txei.h"

void print_usage(void)
{
	printf("Following arguments are needed\n");
	printf("	1. Name of guid file (text file)\n");
	printf("	2. Size of input message (hex number)\n");
	printf("	3. Size of output message (hex number)\n");
	printf("		Following files are not needed. If you do not\n");
	printf("		want one, mark it with - and it will not be opened\n");
	printf("		You must have a - or file name for each of the four\n");
	printf("		file parameters below.\n");
	printf("		Missing input files will be zero filled; missing output\n");
	printf("		files will have output printed to standard output\n");
	printf("		please note that all files are binary, not text\n");
	printf("		Please also note that we will try to read up to the\n");
	printf("		buffer size from the file. If there are less bytes in the\n");
	printf("		file, then the remainder of the buffer will be zeros.\n\n");
	printf("	4. Name of input message file (binary file) - means no file\n");
	printf("	5. Name of output message file (binary file) - means no file\n");
}

int main(int argc, char **argv)
{
	MEI_HANDLE *my_handle_p;
	ssize_t input_snd_count = 0;
	ssize_t input_rcv_count = 0;

	ssize_t file_ops_result = 0;
	
	int result;

	int snd_count;
	int rcv_count;

	FILE *fp1;

	int fd_in = 0;
	int fd_out = 0;

	uint8_t *snd_msg_buf = NULL;
	uint8_t *rcv_msg_buf = NULL;

	GUID my_guid;
	
	if (argc != 6) {
		printf("Incorrected number of arguments %d\n", argc);
		print_usage();
		return -1;
	}

	printf("opening guid file%s\n", (char *)argv[1]);

	/* Grab the contents of the guid file */
	fp1 = fopen((char *)argv[1], "r");
	if (fp1 == NULL) {
		printf("cannot open guid file %s\n", (char *)argv[1]);
		print_usage();
		return -1;
	}

	fscanf(fp1, "%x %x %x %x %x %x %x %x %x %x %x\n",
		(unsigned int *)&my_guid.data1,
		(unsigned int *)&my_guid.data2,
		(unsigned int *)&my_guid.data3,
		(unsigned int *)&my_guid.data4[0],
		(unsigned int *)&my_guid.data4[1],
		(unsigned int *)&my_guid.data4[2],
		(unsigned int *)&my_guid.data4[3],
		(unsigned int *)&my_guid.data4[4],
		(unsigned int *)&my_guid.data4[5],
		(unsigned int *)&my_guid.data4[6],
		(unsigned int *)&my_guid.data4[7]);

	printf("guid %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x\n",
		(unsigned int)my_guid.data1,
		(unsigned int)my_guid.data2,
		(unsigned int)my_guid.data3,
		(unsigned int)my_guid.data4[0],
		(unsigned int)my_guid.data4[1],
		(unsigned int)my_guid.data4[2],
		(unsigned int)my_guid.data4[3],
		(unsigned int)my_guid.data4[4],
		(unsigned int)my_guid.data4[5],
		(unsigned int)my_guid.data4[6],
		(unsigned int)my_guid.data4[7]);
	fclose(fp1);

	/* Grab the input buffer size */
	result = sscanf(argv[2], "%x", (unsigned int *)&input_snd_count);
	if (result != 1) {
		printf("could not read snd count value\n");
		print_usage();
		return -1;
	}
	printf("Will be setting up send message buffer of %x bytes\n",
		(unsigned int)input_snd_count);

	/* Grab the output buffer size */
	result = sscanf(argv[3], "%x", (unsigned int *)&input_rcv_count);
	if (result != 1) {
		printf("could not read rcv count value\n");
		print_usage();
		return -1;
	}
	printf("Will be setting up receive message buffer of %x bytes\n",
		(unsigned int)input_snd_count);

	/* Now try to allocate the message buffers */
	snd_msg_buf = calloc(input_snd_count, 1);
	if (snd_msg_buf == NULL) {
		printf("cannot allocate send message buffer\n");
		print_usage();
		return -1;
	}

	rcv_msg_buf = calloc(input_rcv_count, 1);
	if (rcv_msg_buf == NULL) {
		printf("cannot allocate receive message buffer\n");
		print_usage();
		free(snd_msg_buf);
		return -1;
	}

	/**
	 * Now try to connect to the device using the guid
	 * that we got from the guid file
	 */
	
	my_handle_p = mei_connect(&my_guid);
	if (my_handle_p  == NULL) {
		printf("cannot connect to device\n");
		print_usage();
		free(snd_msg_buf);
		free(rcv_msg_buf);
		return -1;
	}

	printf("After open/init\n");
	mei_print_buffer("fd", (uint8_t *)&my_handle_p->fd, sizeof(my_handle_p->fd));
	mei_print_buffer("guid", (uint8_t *)&my_handle_p->guid, sizeof(my_handle_p->guid));
	mei_print_buffer("client_properties", (uint8_t *)&my_handle_p->client_properties,
		sizeof(my_handle_p->client_properties));
	mei_print_buffer("mei_version", (uint8_t *)&my_handle_p->mei_version,
		sizeof(my_handle_p->mei_version));


	/* We're connected - now try to read the input buffer file (if any) */
	if (strncmp((char *)argv[4], "-", strlen("-")) != 0) {
		printf("We have input message file %s\n", (char *)argv[6]);
		fd_in = open((char *)argv[4], O_RDONLY);
		if (fd_in <= 0) {
			printf("cannot open input message file\n");
			print_usage();
			free(snd_msg_buf);
			free(rcv_msg_buf);
			mei_disconnect(my_handle_p);
			return -1;
		}

		file_ops_result = read(fd_in, snd_msg_buf, input_snd_count);
		printf("we read %x bytes into input send buffer\n",
			(unsigned int)file_ops_result);
		close(fd_in);
	} else {
		printf("There is no input buffer file; buffer will be all zeros\n");
	}

	printf("Sending output buffer\n");
	mei_print_buffer("first 10 chrs of snd_msg_buf", snd_msg_buf, 10);

	snd_count = mei_sndmsg(my_handle_p, snd_msg_buf, input_snd_count);
	if (snd_count != input_snd_count) {
		printf("incorrect size sent %x instead of %x\n", snd_count,
			(unsigned int)input_snd_count);
	}

	rcv_count = mei_rcvmsg(my_handle_p, rcv_msg_buf, input_rcv_count);
	if (rcv_count != input_rcv_count) {
		printf("incorrect size received %x instead of %x\n", rcv_count,
			(unsigned int)input_rcv_count);
	}
	
	printf("Received input buffer\n");
	mei_print_buffer("first 10 chrs rcv_msg_buf", rcv_msg_buf, 10);

	/* We are done. Now we only process output. Close device */
	mei_disconnect(my_handle_p);

	/* Now copy the rcv_msg_buf to a file if a file was  provided */
	if (strncmp((char *)argv[5], "-", strlen("-")) != 0) {
		printf("We have output message file %s\n", (char *)argv[5]);
		fd_out = open((char *)argv[5], O_WRONLY| O_CREAT | O_TRUNC,
			S_IRWXU | S_IRWXG);
		if (fd_out <= 0) {
			printf("cannot open output message file\n");
			print_usage();
			free(snd_msg_buf);
			free(rcv_msg_buf);
			return -1;
		}

		file_ops_result = write(fd_out, rcv_msg_buf, rcv_count);
		printf("we wrote %x bytes out of output rcv buffer\n",
			(unsigned int)rcv_count);
		close(fd_out);
	} else {
		printf("There is no output buffer file\n");
		mei_print_buffer("entire rcv_msg_buf", rcv_msg_buf,
			input_rcv_count);
	}

	return 0;
}
