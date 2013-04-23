/**********************************************************************
 * Copyright 2009 - 2012 (c) Intel Corporation. All rights reserved.

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **********************************************************************/

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "chaabi_error_codes.h"
#include "umip_access.h"
#include "txei.h"

/* define LOG related macros before including the log header file */
#define LOG_STYLE    LOG_STYLE_PRINTF
#define LOG_TAG      "ACD-app"
#include "sepdrm-log.h"

void *ptrHandle;
const GUID guid = {0xafa19346, 0x7459, 0x4f09, {0x9d, 0xad, 0x36, 0x61, 0x1f, 0xe4, 0x28, 0x60}};


static void usage(void)
{
    static const char *usage_string =
	{
	    "Usage:\n"
	    "       read  <index> <file name>                              -- read an entry\n"
	    "       write <index> <file name> <data size> <data max size>  -- write an entry\n"
	    "       lock                                                   -- lock ACD table\n"
	    "       prov  <provisioning scheme> <input file> [ output file ] -- key provisioning\n"
	    "       pmdb-read  [WO | WM] <file>                            -- read PMDB write-once or write-many\n"
	    "       pmdb-write [WO | WM] <file>                            -- write PMDB write-once or write-many\n"
	    "       pmdb-lock                                              -- lock PMDB\n"
	};
    LOGERR("%s", usage_string);
}

/*
 * @brief Reads data from an input file and copies it into a memory buffer
 * @param pFilename Input filename to read the data from
 * @param pDataBuffer Pointer to the allocated memory buffer that will store the file data
 * @param pDataSizeInBytes Return value of the size of the data buffer in bytes
 * @return DX_SUCCESS Data from the input file has been copied into the new data buffer
 * @return DX_ERROR Data from the input file was not read
 *
 * This function will allocate a memory buffer that is the same size as the
 * input file and it will then copy the data from the input file into the memory
 * buffer. The size of the buffer in bytes will be returned through the pointer
 * pDataSizeInBytes. If an error occurred then the data buffer pointer value
 * will be NULL and the data buffer size will be zero.
 *
 * NOTE: This function will allocate a memory buffer to store the data read from
 * the input file. It is the caller's responsibility to free the memory buffer
 * that has been allocated by this function. Memory is allocated with the
 * calloc() function, so use the free() function to free the memory.
 */
static int read_data_from_file( const char * const pFilename,
				uint8_t ** const pDataBuffer,
				uint32_t * const pDataSizeInBytes )
{
	int result = EXIT_FAILURE;
	int status = 0;
	off_t fileSizeInBytes;
	struct stat fileInfo;
	int inputFileHandle = 0;
	ssize_t bytesRead = 0;


	//
	// Check parameters for illegal values.
	//
	if ( NULL == pFilename ) {
		LOGERR( "Input filename does not exist.\n" );

		return ( EXIT_FAILURE );
	}

	if ( NULL == pDataBuffer ) {
		LOGERR( "Data buffer pointer is NULL.\n" );

		return ( EXIT_FAILURE );
	}

	if ( NULL == pDataSizeInBytes ) {
		LOGERR( "Data buffer size pointer is NULL.\n" );

		return ( EXIT_FAILURE );
	}


	//
	// Open the input data file for reading only.
	//
	inputFileHandle = open( pFilename, O_RDONLY );

	if ( inputFileHandle < 0 ) {
		LOGERR( "Unable to open the input file %s.\n", pFilename );

		result = EXIT_FAILURE;

		goto GET_DATA_FROM_FILE_EXIT;
	}


	//
	// Get the size of the input data file.
	//
	memset( &fileInfo, 0, sizeof( fileInfo ) );

	status = fstat( inputFileHandle, &fileInfo );

	if ( status < 0 ) {
		LOGERR( "Unable to get the size of the input file %s.\n",
		        pFilename );

		result = EXIT_FAILURE;

		goto GET_DATA_FROM_FILE_EXIT;
	}

	if ( 0 == fileInfo.st_size ) {
		LOGERR( "Input file %s is empty.\n", pFilename );

		result = EXIT_FAILURE;

		goto GET_DATA_FROM_FILE_EXIT;
	}

	*pDataSizeInBytes = (uint32_t)fileInfo.st_size;


	//
	// Allocate the memory buffer for the input file data.
	//
	*pDataBuffer = (uint8_t *)calloc( (size_t)*pDataSizeInBytes, sizeof( uint8_t ) );

	if ( NULL == *pDataBuffer ) {
		LOGERR( "Unable to allocate %u bytes of memory for the input "
		        "data.\n", *pDataSizeInBytes );

		result = EXIT_FAILURE;

		goto GET_DATA_FROM_FILE_EXIT;
	}


	//
	// Copy the data from the input file into the memory buffer.
	//
	bytesRead = read( inputFileHandle, *pDataBuffer, (size_t)*pDataSizeInBytes );

	if ( bytesRead != (ssize_t)*pDataSizeInBytes ) {
		LOGERR( "Reading data from the input file %s failed.\n",
		        pFilename );

		result = EXIT_FAILURE;

		goto GET_DATA_FROM_FILE_EXIT;
	}


	//
	// Reading data from the input file succeeded.
	//
	result = EXIT_SUCCESS;


GET_DATA_FROM_FILE_EXIT:
	if (  0 <= inputFileHandle ) {
		//
		// Close the open input file.
		//
		close( inputFileHandle );

		inputFileHandle = -1;
	}


	if ( EXIT_SUCCESS != result ) {
		//
		// Reading data from the input file was not successful so the
		// data size is zero.
		//
		*pDataSizeInBytes = 0;


		//
		// Deallocate the memory buffer if it was allocated.
		//
		if ( NULL != *pDataBuffer ) {
			free( *pDataBuffer );

			*pDataBuffer = NULL;
		}
	}


	return ( result );
} // end read_data_from_file()


/*
 * @brief Writes data from a memory data buffer to an output file
 * @param pFilename Output filename to write the data to
 * @param pDataBuffer Pointer to the memory buffer of data to write
 * @param pDataSizeInBytes Number of data bytes to write
 * @return EXIT_SUCCESS Data from the memory buffer has been written to the output file
 * @return EXIT_FAILURE Data from the memory buffer has not been written to the output file
 */
static int write_data_to_file( const char * const pFilename,
			       const uint8_t * const pDataBuffer,
			       const uint32_t dataSizeInBytes )
{
	int result = EXIT_FAILURE;
	int outputFileHandle = 0;
	ssize_t bytesWritten = 0;


	//
	// Check parameters for illegal values.
	//
	if ( NULL == pFilename ) {
		LOGERR( "Output filename does not exist.\n" );

		return ( EXIT_FAILURE );
	}

	if ( NULL == pDataBuffer ) {
		LOGERR( "Data memory buffer pointer is NULL.\n" );

		return ( EXIT_FAILURE );
	}

	if ( 0 == dataSizeInBytes ) {
		LOGERR( "Data size is zero.\n" );

		return ( EXIT_FAILURE );
	}


	//
	// Open the output data file.
	//
	outputFileHandle = open( pFilename, ( O_WRONLY | O_CREAT | O_TRUNC ) );

	if ( outputFileHandle < 0 ) {
		LOGERR( "Unable to open the output data file %s.\n", pFilename );

		result = EXIT_FAILURE;

		goto WRITE_DATA_TO_FILE_EXIT;
	}


	//
	// Write the data to the output file.
	//
	bytesWritten = write( outputFileHandle, pDataBuffer, (size_t)dataSizeInBytes );

	if ( bytesWritten != (ssize_t)dataSizeInBytes ) {
		LOGERR( "Writing data to output file %s failed.\n", pFilename );

		result = EXIT_FAILURE;

		goto WRITE_DATA_TO_FILE_EXIT;
	}


	//
	// Writing data to the output file succeeded.
	//
	result = EXIT_SUCCESS;


WRITE_DATA_TO_FILE_EXIT:
	if ( 0 <= outputFileHandle ) {
		//
		// Close the open output data file.
		//
		close( outputFileHandle );

		outputFileHandle = -1;
	}


	return ( result );
} // end write_data_to_file()

#define PMDB_READ_CMD_STR "pmdb-read"
#define PMDB_WRITE_CMD_STR "pmdb-write"
#define PMDB_LOCK_CMD_STR "pmdb-lock"
#define PMDB_WO_AREA_STR  "WO"
#define PMDB_WM_AREA_STR  "WM"

static int pmdb_read(sep_pmdb_area_t which_area, const char * const outputfile)
{
	int status = EXIT_SUCCESS;
	uint8_t data[MAX_PMDB_AREA_SIZE_IN_BYTES];


	//
	// Clear the PMDB read buffer with zeroes.
	//
	(void)memset( data, 0, sizeof( data ) );


	//
	// Read data from the PMDB in Chaabi.
	//
	do {
		pmdb_result_t result = sep_pmdb_read( which_area , data, (uint32_t)sizeof(data));
		if (PMDB_SUCCESSFUL != result) {
			LOGERR("Error reading PMDB (0x%X)\n", result);
			status = EXIT_FAILURE;
			break;
		}

		status = write_data_to_file( outputfile, data, sizeof(data) );

	} while (0);


	return status;
}
static int pmdb_write(sep_pmdb_area_t which_area, const char *const inputfile)
{
	int status = EXIT_SUCCESS;
	uint8_t *pData = NULL;
	uint32_t dataSizeInBytes = 0;


	do {
		//Read data from file
		status = read_data_from_file(inputfile, &pData, &dataSizeInBytes);

		if (status == EXIT_FAILURE) {
			LOGERR( "Unable to read data from file: %s\n", inputfile );
			break;
		}

		if ( 0 == dataSizeInBytes ) {
			LOGERR( "Data size of zero bytes is illegal.\n" );

			status = EXIT_FAILURE;

			//
			// Jump to the exit.
			//
			break;
		}

		if ( dataSizeInBytes > MAX_PMDB_AREA_SIZE_IN_BYTES ) {
			LOGERR( "Data size of %u bytes is greater than the maximum "
			        "size of %u bytes.\n", dataSizeInBytes,
			        MAX_PMDB_AREA_SIZE_IN_BYTES );

			status = EXIT_FAILURE;

			//
			// Jump to the exit.
			//
			break;
		}


		//
		// Write the data to the PMDB.
		//
		pmdb_result_t result = sep_pmdb_write( which_area, pData, dataSizeInBytes);

		if ( PMDB_SUCCESSFUL != result ) {
			LOGERR( "Error writing PMDB (0x%X)\n", result );

			status = EXIT_FAILURE;

			break;
		}

	} while (0);


	if ( NULL != pData ) {
		//
		// Deallocate the data buffer from memory.
		//
		free( pData );

		pData = NULL;
	}


	return status;
}

static int pmdb_lock()
{
	int status = EXIT_SUCCESS;


	pmdb_result_t result = sep_pmdb_lock();

	if (PMDB_SUCCESSFUL != result) {
		LOGERR( "Error locking PMDB write-once area (0x%X)\n", result );

		status = EXIT_FAILURE;
	}


	return status;
}

static int pmdb_parser(const int argc, char **argv)
{
	int status = EXIT_FAILURE;
	char *filename;
	sep_pmdb_area_t which_area = -1;

	do {
		if ( (strncmp(argv[1], PMDB_READ_CMD_STR, strlen(PMDB_READ_CMD_STR)) == 0) && (argc == 4) ) {
			// argv[2] is the area type WO or WM

			if ( strncmp(argv[2], PMDB_WO_AREA_STR, strlen(PMDB_WO_AREA_STR) ) == 0) {
				which_area = SEP_PMDB_WRITE_ONCE;
			} else if ( strncmp(argv[2], PMDB_WM_AREA_STR, strlen(PMDB_WM_AREA_STR)) == 0 ) {
				which_area = SEP_PMDB_WRITE_MANY;
			}
			else {
				LOGERR(" Invalid read area (%s != WO | WM)\n", argv[2]);

				break;
			}
			filename = argv[3];

			status = pmdb_read(which_area, filename);

		} else if ( (strncmp(argv[1], PMDB_WRITE_CMD_STR, strlen(PMDB_WRITE_CMD_STR)) == 0) && (argc == 4) ) {
			// argv[2] is the area type WO or WM
			// argv[3] is filename containing the data to be written.
			if ( strncmp(argv[2], PMDB_WO_AREA_STR, strlen(PMDB_WO_AREA_STR) ) == 0) {
				which_area = SEP_PMDB_WRITE_ONCE;
			} else if ( strncmp(argv[2], PMDB_WM_AREA_STR, strlen(PMDB_WM_AREA_STR)) == 0 ) {
				which_area = SEP_PMDB_WRITE_MANY;
			}
			else {
				LOGERR(" Invalid write area (%s != WO | WM)\n", argv[2]);
				break;
			}
			filename = argv[3];
			status = pmdb_write(which_area, filename);
		} else if ( strncmp(argv[1], PMDB_LOCK_CMD_STR, strlen(PMDB_LOCK_CMD_STR)) == 0 && (argc == 2)) {
			status = pmdb_lock();
		} else {
			LOGERR("Invalid arguments\n\n");

			usage();

			break;
		}
	} while (0);

	return status;
}
/*
 * @brief Provisions the ACD with data when a simple ACD write is insufficient
 * @param provSchema ACD provisioning schema to use
 * @param inputFilename ACD provisioning input data filename
 * @param outputFilename ACD provisioning output data filename
 * @return ACD_PROV_SUCCESS Provisioning the ACD succeeded
 * @return <0 Error occurred
 *
 * Typically ACD provision is used when a more complex provision scheme for the
 * provisioning data is required.
 */
static int provision_android_customer_data( const char * const provSchema,
                                            const char * const inputFilename,
                                            const char * const outputFilename )
{
	int status = ACD_PROV_SUCCESS;
	int provisioningSchema = -1;
	int provInputFile = 0;
	int provOutputFile = 0;
	uint32_t provInputDataSize = 0;
	uint32_t provReturnDataSizeInBytes = 0;
	void *pProvInputData = NULL;
	void *pProvOutputData = NULL;
	struct stat fileInfo;
	ssize_t bytesRead = 0;
	ssize_t bytesWritten = 0;


	/*
	 * Check the parameters for illegal values.
	 */
	if ( NULL == provSchema ) {
		LOGERR("Provisioning schema parameter does not exist.\n" );

		return ( -1 );
	}

	/*
	 * Read the ACD provisioning schema value argument and check it
	 * for legal values.
	 */
	status = sscanf( provSchema, "%d", &provisioningSchema );

	if ( 1 != status ) {
		/*
		 * An incorrect number of items were scanned.
		 */
		LOGERR("Failed to read provisioning schema.\n" );

		usage();

		return ( -1 );
	}

	if ( ( provisioningSchema < ACD_MIN_PROVISIONNG_SCHEMA ) ||
	     ( provisioningSchema > ACD_MAX_PROVISIONNG_SCHEMA ) ) {
		LOGERR("Provisioning schema value of %d is "
		                     "illegal.\n", provisioningSchema );

		usage();

		return ( -1 );
	}

	if ( NULL == inputFilename ) {
		LOGERR("APP: Provisioning input filename parameter does not exist.\n" );
		return ( -1 );
	}

#ifdef EPID_ENABLED
	/*
	 * Execute the EPID provisioning for Mediavault then return.
	 */
	if (ACD_PROVISIONING_SCHEMA_EPID == provisioningSchema) {
	    status = provision_epid( inputFilename );
	    return status;
	}
#endif

	if ( NULL == outputFilename ) {
		LOGERR("APP: Provisioning output filename parameter does not exist.\n" );
		return ( -1 );
	}

	/*
	 * Open the ACD provisioning input data file.
	 */
	provInputFile = open( inputFilename, O_RDONLY );

	if ( 0 >= provInputFile ) {
		/*
		 * The ACD provisioning input data file could not be
		 * opened.
		 */
		LOGERR("Could not open the file %s\n", inputFilename );

		status = -1;

		goto PROVISION_ANDROID_CUSTOMER_DATA_EXIT;
	}


	/*
	 * Read the size of the input data file.
	 */
	memset( &fileInfo, 0, sizeof( fileInfo ) );

	status = fstat( provInputFile, &fileInfo );

	if ( 0 != status ) {
		/*
		 * Could not read the size of the ACD provisioning input
		 * data file.
		 */
		LOGERR("Could not get the size of file %s.\n",
		                     inputFilename );

		status = -1;

		goto PROVISION_ANDROID_CUSTOMER_DATA_EXIT;
	}

	provInputDataSize = (uint32_t)fileInfo.st_size;
	if ( (provInputDataSize == 0) ||
			(provInputDataSize > ACD_MAX_DATA_SIZE_IN_BYTES) ) {
		/*
		 * The ACD provisioning input file is empty.
		 */
		LOGERR("Provisioning input file size of %u bytes "
		                     "is illegal.\n", provInputDataSize );

		status = -1;

		goto PROVISION_ANDROID_CUSTOMER_DATA_EXIT;
	}


	/*
	 * Allocate memory for the ACD provisioning input data buffer.
	 */
	pProvInputData = calloc( (size_t)provInputDataSize, sizeof( uint8_t ) );

	if ( NULL == pProvInputData ) {
		LOGERR("Could not allocate memory buffer for reading "
		                     "provisioning input data.\n" );

		status = -1;

		goto PROVISION_ANDROID_CUSTOMER_DATA_EXIT;
	}


	/*
	 * Read the data to provision from the input data file.
	 */
	bytesRead = read( provInputFile, pProvInputData, (size_t)provInputDataSize );

	if ( bytesRead != (ssize_t)provInputDataSize ) {
		/*
		 * All of the input data was not read.
		 */
		LOGERR("Reading input file failed (read %zd of %zd "
		                     "bytes).\n", (size_t)bytesRead,
		                     (size_t)provInputDataSize );

		status = -1;

		goto PROVISION_ANDROID_CUSTOMER_DATA_EXIT;
	}


	/*
	 * Provision the data into the ACD and get the return data.
	 */
	status = provision_customer_data( ptrHandle,
					  (uint32_t)provisioningSchema,
	                                  provInputDataSize,
	                                  pProvInputData,
	                                  &provReturnDataSizeInBytes,
	                                  &pProvOutputData );

	if ( ACD_PROV_SUCCESS != status ) {
		LOGERR("Provisioning ACD failed (error is %d).\n",
		                     status );

		goto PROVISION_ANDROID_CUSTOMER_DATA_EXIT;
	}


	/*
	 * Open the ACD provisioning output file for writing the return
	 * data.
	 */
	provOutputFile = open( outputFilename, ( O_WRONLY | O_CREAT | O_TRUNC ) );

	if ( 0 >= provOutputFile ) {
		/*
		 * The ACD provisioning output data file could
		 * not be opened.
		 */
		LOGERR("Could not open the file %s\n", outputFilename );

		status = -1;

		goto PROVISION_ANDROID_CUSTOMER_DATA_EXIT;
	}


	/*
	 * Write the ACD provisioning return data to the output
	 * file.
	 */
	if ( provReturnDataSizeInBytes > 0 ) {
		if ( NULL == pProvOutputData ) {
			/*
			 * The return data memory buffer does not exist.
			 */
			LOGERR("ACD provisioning return data does not "
			                     "exist.\n" );

			status = -1;

			goto PROVISION_ANDROID_CUSTOMER_DATA_EXIT;
		}

		bytesWritten = write( provOutputFile, pProvOutputData,
		                      (size_t)provReturnDataSizeInBytes );

		if ( bytesWritten != (ssize_t)provReturnDataSizeInBytes ) {
			LOGERR("Writing return data failed (wrote %zd of "
			                     "%zd bytes).\n", (size_t)bytesWritten,
			                     (size_t)provReturnDataSizeInBytes );

			status = -1;

			goto PROVISION_ANDROID_CUSTOMER_DATA_EXIT;
		}
	}


	/*
	 * ACD provisioning was successful.
	 */
	status = ACD_PROV_SUCCESS;


PROVISION_ANDROID_CUSTOMER_DATA_EXIT:
	/*
	 * Close the provisioning input and output files and free the input and
	 * output data memory buffers.
	 */
	if ( provInputFile > 0 ) {
		close( provInputFile );

		provInputFile = 0;
	}

	if ( provOutputFile > 0 ) {
		close( provOutputFile );

		provOutputFile = 0;
	}

	if ( NULL != pProvInputData ) {
		free( pProvInputData );

		pProvInputData = NULL;
	}

	if ( NULL != pProvOutputData ) {
		free( pProvOutputData );

		pProvOutputData = NULL;
	}


	return ( status );
} /* end provision_android_customer_data() */


int main(int argc, char **argv)
{
	int result = EXIT_SUCCESS;
	uint8_t cmd_idx = 0;
	int file_id = 0;
	uint16_t data_size = 0;
	uint16_t data_max_size = 0;
	uint16_t what_to_do = 0;
	void *my_buffer = NULL;
	ssize_t file_result;
	int scan_in = 0;

	/* check number of parameters */
	if ((argc < 2) || (argc > 6)) {
		usage();
		return -1;
	}

	result = acd_init(&guid, &ptrHandle);
	if ((result != EXIT_SUCCESS) || (ptrHandle == NULL))
	{
		return result;
	}

	/* check first parameter */
	if (strncmp(argv[1], "read", strlen("read")) == 0)
		what_to_do = OPCODE_IA2CHAABI_ACD_READ;
	else if (strncmp(argv[1], "write", strlen("write")) == 0)
		what_to_do = OPCODE_IA2CHAABI_ACD_WRITE;
	else if (strncmp(argv[1], "lock", strlen("lock")) == 0)
		what_to_do = OPCODE_IA2CHAABI_ACD_LOCK;
#ifdef ACD_WIPE_TEST        
	else if (strncmp(argv[1], "wipe", strlen("wipe")) == 0)
		what_to_do = OPCODE_IA2CHAABI_ACD_WIPE;
#endif        
	else if (strncmp(argv[1], "prov", strlen("prov")) == 0) {
		what_to_do = OPCODE_IA2CHAABI_ACD_PROV;
	} else if ( strncmp(argv[1], "pmdb-", strlen("pmdb-")) == 0) {
		result = pmdb_parser(argc, argv);
		goto exit;
	} else {
		/*
		 * Unknown Android Customer Data operation.
		 */
		usage();
		result = EXIT_FAILURE;
		goto exit;
	}

	/* make sure that read/write have index and file name */
	if ((what_to_do == OPCODE_IA2CHAABI_ACD_LOCK) && (argc != 2)) {
		usage();
		result = EXIT_FAILURE;
		goto exit;
    }
#ifdef ACD_WIPE_TEST    
    else if ((what_to_do == OPCODE_IA2CHAABI_ACD_WIPE) && (argc != 2)) {
		usage();
		result = EXIT_FAILURE;
		goto exit;
	} 
#endif
    else if ((what_to_do == OPCODE_IA2CHAABI_ACD_WRITE) && (argc != 6)) {
		usage();
		result = EXIT_FAILURE;
		goto exit;

	} else if ((what_to_do == OPCODE_IA2CHAABI_ACD_READ) && (argc != 4)) {
		usage();
		result = EXIT_FAILURE;
		goto exit;

	}
#ifdef EPID_ENABLED
	else if ( ( OPCODE_IA2CHAABI_ACD_PROV == what_to_do ) && ( 5 != argc && 4 != argc ) )
#else
	else if ( ( OPCODE_IA2CHAABI_ACD_PROV == what_to_do ) && ( 5 != argc ) )
#endif
	{
		/*
		 * Incorrect number of arguments for ACD provisioning.
		 */
		LOGERR("Incorrect number of arguments.\n" );

		usage();

		result = EXIT_FAILURE;
		goto exit;

	}

	if ( ( OPCODE_IA2CHAABI_ACD_READ == what_to_do ) ||
	     ( OPCODE_IA2CHAABI_ACD_WRITE == what_to_do ) ) {
		/*
		 * ACD read or write field index range check.
		 */
		result = sscanf(argv[2], "%d", &scan_in);
		cmd_idx = (uint8_t)scan_in;
		if (result != 1) {
			LOGERR("could not decipher index\n");
			usage();
			result = EXIT_FAILURE;
			goto exit;

		} else if ( ( cmd_idx < ACD_MIN_FIELD_INDEX ) ||
		            ( cmd_idx > ACD_MAX_FIELD_INDEX ) ) {
			LOGERR("index out of range\n");
			usage();
			result = EXIT_FAILURE;
			goto exit;
		}

	}

	if ( OPCODE_IA2CHAABI_ACD_PROV == what_to_do ) {
		/*
		 * Provision the ACD.
		 */
		result = provision_android_customer_data( argv[ 2 ], argv[ 3 ], argv[ 4 ] );
		goto exit;
	}

	if (what_to_do == OPCODE_IA2CHAABI_ACD_WRITE) {

		/* process size */
		result = sscanf(argv[4], "%d", &scan_in);
		data_size = (uint16_t)scan_in;
		if (result != 1) {
			LOGERR("could not decipher size\n");
			usage();
			result = EXIT_FAILURE;
			goto exit;

		} else if (data_size > ACD_MAX_DATA_SIZE_IN_BYTES) {
			LOGERR("data_size out of range\n");
			usage();
			result = EXIT_FAILURE;
			goto exit;
		}

		/* process max size */
		result = sscanf(argv[5], "%d", &scan_in);
		data_max_size = (uint16_t)scan_in;
		if (result != 1) {
			LOGERR("could not decipher max size\n");
			usage();
			result = EXIT_FAILURE;
			goto exit;

		} else if (data_max_size > ACD_MAX_DATA_SIZE_IN_BYTES) {
			LOGERR("data_max_size out of range\n");
			usage();
			result = EXIT_FAILURE;
			goto exit;
		} else if (data_size > data_max_size) {
			LOGERR("APP: data_size %d > data_max_size %d \n", data_size, data_max_size);
			usage();
			result = EXIT_FAILURE;
			goto exit;
		}
	}

	/* attempt to open the file */
	if (what_to_do == OPCODE_IA2CHAABI_ACD_WRITE) {
		file_id = open(argv[3], O_RDONLY);
		if (file_id <= 0) {
			LOGERR("could not open the file %s\n", argv[3]);
			usage();
			result = EXIT_FAILURE;
			goto exit;
		}

	} else if (what_to_do == OPCODE_IA2CHAABI_ACD_READ) {
		file_id = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC);
		if (file_id <= 0) {
			LOGERR("could not open the file %s\n", argv[3]);
			usage();
			result = EXIT_FAILURE;
			goto exit;
		}
	}

	/* allocate the buffer */
	if (what_to_do == OPCODE_IA2CHAABI_ACD_WRITE) {
		my_buffer = calloc(1, (size_t)data_size);
		if (my_buffer == NULL) {
			LOGERR("could not allocate buffer\n");
			result = EXIT_FAILURE;
			goto exit;
		}
	}

	/* Lock */
	if (what_to_do == OPCODE_IA2CHAABI_ACD_LOCK) {
		result = lock_customer_data(ptrHandle);
		goto exit;
    } 
#ifdef ACD_WIPE_TEST    
    else if (what_to_do == OPCODE_IA2CHAABI_ACD_WIPE) {
        result = wipe_customer_data(ptrHandle);
        goto exit;
	/* Read */
	} 
#endif    
    else if (what_to_do == OPCODE_IA2CHAABI_ACD_READ) {
		result = get_customer_data(ptrHandle, cmd_idx, &my_buffer);
		if (result < 0) {
			LOGERR("read operaton failed %d\n", result);
			close(file_id);
			if (my_buffer != NULL)
				free(my_buffer);
			goto exit;
		}

		file_result = write(file_id, my_buffer, result);
		if (file_result != result) {
			LOGERR("write to file failed\n");
			close(file_id);
			free(my_buffer);
			result = EXIT_FAILURE;
			goto exit;
		}

		close(file_id);
		free(my_buffer);
		result = EXIT_SUCCESS;
		goto exit;

	/* Write */
	} else if ( OPCODE_IA2CHAABI_ACD_WRITE == what_to_do ) {
		file_result = read(file_id, my_buffer, data_size);
		if (file_result != data_size) {
			LOGERR("read from file failed\n");
			close(file_id);
			free(my_buffer);
			result = EXIT_FAILURE;
			goto exit;
		}

		result = set_customer_data(ptrHandle, cmd_idx, data_size, data_max_size, my_buffer);
		if (result < 0) {
			LOGERR("write operaton failed %d\n", result);
			close(file_id);
			free(my_buffer);
			goto exit;
		}

		close(file_id);
		free(my_buffer);
		result = EXIT_SUCCESS;
		goto exit;
	} else {
		/*
		 * Unknown Android Customer Data operation.
		 */
		LOGERR("Unknown operation (%d).\n", what_to_do );

		usage();

		result = EXIT_FAILURE;
		goto exit;
	}

	exit:
	if (ptrHandle != NULL)
	{
		if (acd_deinit(ptrHandle) != 0)
		{
			LOGERR("Failed to deinit ACD");
		}
		ptrHandle = NULL;
	}
	return result;
} /* end main() */



