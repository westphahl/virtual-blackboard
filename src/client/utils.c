#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../net_message.h"

void *alloc(size_t size) {
	/* allocate disk space */
	void *tmp = malloc(size);
	if(tmp == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	
	/* set all to NULL */
	memset(tmp, 0, size);
	
	return tmp;
}
