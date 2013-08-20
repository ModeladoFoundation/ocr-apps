#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv) {
	while(1) {
		uint32_t tmp[3];
		if ( 3 == scanf("CONSOLE: XE7 >>> x=0x%x m y=0x%x m p=0x%x\n",
				&tmp[0], &tmp[1], &tmp[2]) ) {
			printf("x=%7.2fm y=%7.2fm p=%4.2f\n",
				*(float *)&tmp[0],*(float *)&tmp[1],*(float *)&tmp[2]);
		} else {
			break;
		}
	}
}
