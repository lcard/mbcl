#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <modbus/modbus.h>
#include <getopt.h>
#include <unistd.h>

/* Need options:
	target ip / port
	operation read/write coils/discretes registers
	base address
	a addresses
	N times
	I interval (milliseconds)
	q quiet
*/

enum mb_operations {
	read_bits,       // aka read coils
	read_input_bits, // aka read discrete inputs
	read_registers,  // aka read holding registers
	read_input_registers, // aka read 
	write_bit, // aka write coil
	write_register, // aka write holding register
	write_bits, // aka write coils
	write_registers, // aka write holding registers 
};

int main(int argc, char **argv) {
	
	uint8_t *tab_bit;
	uint16_t *tab_reg;
	
	/* Allocate and initialize the memory to store the status */
	tab_bit = (uint8_t *)malloc(MODBUS_MAX_READ_BITS * sizeof(uint8_t));
	memset(tab_bit, 0, MODBUS_MAX_READ_BITS * sizeof(uint8_t));

	/* Allocate and initialize the memory to store the registers */
	tab_reg = (uint16_t *)malloc(MODBUS_MAX_READ_REGISTERS * sizeof(uint16_t));
	memset(tab_reg, 0, MODBUS_MAX_READ_REGISTERS * sizeof(uint16_t));

	/* Defaults */
	char host[1024] = "127.0.0.1";
	size_t port = 502;
	enum mb_operations operation = read_registers;

	size_t base_addr =  0;  // read from address 0 by deafult
	size_t n_addrs   = 10;  // read forst 10 addressxes by default.
	size_t n_times   =  1;  // one times by default. 
	size_t interval  = 0;   // no waiting by default.
	bool   quiet     = false; // Do not suppress output by default.

	size_t n_data = 0; // The number of data items from the command line.
	char **data_items; // Array of data items from the command line.

	int c;
	int rc;

	while (1)
	{
		c = getopt(argc, argv, "h:p:o:b:a:n:i:q");
		if (c == -1)
			break;

		switch (c)
		{
		case 'h':					
			printf("Connect to host: '%s'\n", optarg);
			strncpy(host, optarg, 1024); // Made host str 1024. Should define a constant. 
			break;

		case 'p':
			printf("Connect on port: '%s'\n", optarg);
			port = strtoul(optarg, (void *)0, 10);
			break;

		case 'o':
			printf("Option o with value '%s'\n", optarg);			
			if (!strcmp("read_bits", optarg))           operation = read_bits;
			if (!strcmp("read_input_bits", optarg))     operation = read_input_bits;
			if (!strcmp("read_registers", optarg))      operation = read_registers;
			if (!strcmp("read_input_registers", optarg))operation = read_input_registers;
			if (!strcmp("write_bit", optarg))           operation = write_bit;
			if (!strcmp("write_register", optarg))      operation = write_register;
			if (!strcmp("write_bits", optarg))          operation = write_bits;
			if (!strcmp("write_registers",optarg))      operation = write_registers;
			break;

		case 'b':
			printf("Base address set to: '%s'\n", optarg);
			base_addr = strtoul(optarg, (void *)0, 10);;
			break;

		case 'a':
			printf("Number of addresses to read: '%s'\n", optarg);
			n_addrs = strtoul(optarg, (void *)0, 10);
			if (n_addrs > MODBUS_MAX_READ_REGISTERS) {
				printf("Number of addresses to read adjusted to maximum: '%i'\n", MODBUS_MAX_READ_REGISTERS);
				n_addrs = MODBUS_MAX_READ_REGISTERS;
			}
			break;

		case 'n':
			printf("Repeat this many times: '%s'\n", optarg);
			n_times = strtoul(optarg, (void *)0, 10);
			break;

		case 'i':
			printf("Repeat at this interval: '%s'\n", optarg);
			interval = strtoul(optarg, (void *)0, 10);
			break;

		case 'q':
			printf("option q: supressing output.\n");
			quiet = true;
			break;

		case '?':
			printf("option ?: There is not doc only source codes.\n");
			break;

		default:
			printf("?? getopt returned character code 0%o ??\n", c);
		}
	
	}

	if (optind < argc) {
		n_data = argc - optind;

		data_items = (char **)malloc(n_data * sizeof(char *));

		printf("There are %i non-option ARGV-elements: ", (argc-optind));
		size_t n = 0;
		while (optind < argc) {
			printf("%s ", argv[optind]);
			data_items[n] = argv[optind];
			++optind;
			++n;
		}
		printf("\n");
	}

	/* Get show on the road. */
	modbus_t *ctx;
	ctx = modbus_new_tcp(host, port);

	if (modbus_connect(ctx) == -1) {
		fprintf(stderr, "Connection failed: %s\n",
			modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	for (size_t i = 0; i<n_times; i++) {
		switch (operation)
		{
		case read_bits:

			printf("READ BITS:\n");
			rc = modbus_read_bits(ctx, base_addr, n_addrs, tab_bit);
			if (rc == -1) {
				fprintf(stderr, "%s\n", modbus_strerror(errno));
				return(EXIT_FAILURE);
			}

			if (!quiet) {
				/* What did we read?*/
				for (uint8_t *reg_ptr = tab_bit; reg_ptr < tab_bit + n_addrs; reg_ptr++) {
					printf("%i ", *reg_ptr);
				}
				printf("\n");
			}
			break;

		case read_input_bits:

			printf("READ INPUT BITS:\n");
			rc = modbus_read_input_bits(ctx, base_addr, n_addrs, tab_bit);
			if (rc == -1) {
				fprintf(stderr, "%s\n", modbus_strerror(errno));
				return(EXIT_FAILURE);
			}

			if (!quiet) {
				/* What did we read?*/
				for (uint8_t *reg_ptr = tab_bit; reg_ptr < tab_bit + n_addrs; reg_ptr++) {
					printf("%i ", *reg_ptr);
				}
				printf("\n");
			}
			break;

		case read_registers:

			printf("READ REGISTERS:\n");
			rc = modbus_read_registers(ctx, base_addr, n_addrs, tab_reg);
			if (rc == -1) {
				fprintf(stderr, "%s\n", modbus_strerror(errno));
				return(EXIT_FAILURE);
			}

			if (!quiet) {
				/* What did we read?*/
				for (uint16_t *reg_ptr = tab_reg; reg_ptr < tab_reg + n_addrs; reg_ptr++) {
					printf("%i ", *reg_ptr);
				}
				printf("\n");
			}
			break;

		case read_input_registers:

			printf("READ INPUT REGISTERS\n\n");
			rc = modbus_read_input_registers(ctx, base_addr, n_addrs, tab_reg);
			
			if (rc == -1) {
				fprintf(stderr, "%s\n", modbus_strerror(errno));
				return(EXIT_FAILURE);
			}

			if (!quiet) {
				/* What did we read?*/
				for (uint16_t *reg_ptr = tab_reg; reg_ptr < tab_reg + n_addrs; reg_ptr++) {
					printf("%i ", *reg_ptr);
				}
				printf("\n");
			}
			break;

		case write_bit:

			printf("WRITE BIT\n");

			n_addrs = 1;

			if (n_addrs > n_data) {
				fprintf(stderr, "Not enough data items on command line to write to requested address.\n");
				return(EXIT_FAILURE);
			}

			for (uint8_t *reg_ptr = tab_bit; reg_ptr < tab_bit + n_addrs; reg_ptr++) {
				*reg_ptr = strtoul(*data_items++, (void *)0, 10);
			}

			rc = modbus_write_bit(ctx, base_addr, tab_bit);
			if (rc == -1) {
				fprintf(stderr, "%s\n", modbus_strerror(errno));
				return(EXIT_FAILURE);
			}
			break;

		case write_register:

			printf("WRITE REGISTER\n");

			// There is only one register. Almost everything else stays the same as write_registers.
			n_addrs = 1;

			if (n_addrs > n_data) {
				fprintf(stderr, "Not enough data items on command line to write to requested address.\n");
				return(EXIT_FAILURE);
			}

			for (uint16_t *reg_ptr = tab_reg; reg_ptr < tab_reg + n_addrs; reg_ptr++) {
				*reg_ptr = strtoul(*data_items++, (void *)0, 10);
			}

			rc = modbus_write_register(ctx, base_addr, tab_reg);

			if (rc == -1) {
				fprintf(stderr, "%s\n", modbus_strerror(errno));
				return(EXIT_FAILURE);
			}

			break;

		case write_bits:

			printf("WRITE BITS\n");

			if (n_addrs > n_data) {
				fprintf(stderr, "Not enough data items on command line to write to requested addresses.\n");
				return(EXIT_FAILURE);
			}

			for (uint8_t *reg_ptr = tab_bit; reg_ptr < tab_bit + n_addrs; reg_ptr++) {
				*reg_ptr = strtoul(*data_items++, (void *)0, 10);
			}

			rc = modbus_write_bits(ctx, base_addr, n_addrs, tab_bit);
			if (rc == -1) {
				fprintf(stderr, "%s\n", modbus_strerror(errno));
				return(EXIT_FAILURE);
			}

			break;

		case write_registers:

			printf("WRITE REGISTERS\n");

			if (n_addrs > n_data) {
				fprintf(stderr, "Not enough data items on command line to write to requested addresses.\n");
				return(EXIT_FAILURE);
			}

			for (uint16_t *reg_ptr = tab_reg; reg_ptr < tab_reg + n_addrs; reg_ptr++) {
				*reg_ptr = strtoul(*data_items++, (void *)0, 10);
			}
	
			rc = modbus_write_registers(ctx, base_addr, n_addrs, tab_reg);
			if (rc == -1) {
				fprintf(stderr, "%s\n", modbus_strerror(errno));
				return(EXIT_FAILURE);
			}

			break;

		default:
			break;
		}

		sleep(interval);
	}
	
	/* Free the memory */
	free(tab_bit);
	free(tab_reg);
	//free(data_items);
	
	/* Close the connection */
	modbus_close(ctx);
	modbus_free(ctx);

	exit(EXIT_SUCCESS);
}


