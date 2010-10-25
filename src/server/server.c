/*
 * stdlib.h: 
 *      exit()
 *      EXIT_SUCCESS, EXIT_FAILURE
 * stdio.h:
 *      fprintf()
 *      stdout
 * getopt.h:
 *      getopt()
 *      optarg
 */
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    unsigned int listen_port = 50000;
    int debug = 0;
    int opt;

    // Parse command line options
    while ((opt = getopt(argc, argv, "dp:")) != -1) {
        switch(opt) {
            case 'd':
                // Run server in debug mode
                debug = 1;
                break;
            case 'p':
                // TODO Add option for listen port
                // listen_port = strtol(opt, NULL, 10);
                break;
        }
    }
    
    exit(EXIT_SUCCESS);
}
