#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv) {
	while(!feof(stdin)) {
		uint32_t tmp[3],xe_num;
		uint32_t junk1, junk2;
		if ( (4+2) == fscanf(stdin, "CONSOLE: XE%d >>> @ (%d; %d) (x=0x%x m, y=0x%x m, p=0x%x)\n",
				&junk1, &junk2,
				&xe_num,&tmp[0], &tmp[1], &tmp[2]) ) {
			printf("(x=%7.2f m, y=%7.2f m, p=%4.2f)\n",
				*(float *)&tmp[0],*(float *)&tmp[1],*(float *)&tmp[2]);
		} else {
			perror("fscanf");
			exit(1);
		}
	}
	exit(0);
}
