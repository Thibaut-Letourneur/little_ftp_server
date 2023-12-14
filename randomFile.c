#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char** argv)
{
	int size = 1;
	
	if(argc == 2)
		size = atoi(argv[1]);
		
    FILE* output = fopen("file.txt", "w+");

    unsigned char* randomBytes = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
	unsigned char* randomBytesBig = malloc(sizeof(unsigned char)*26*1000);
	for(int i = 0; i < 26*1000; i++)
		randomBytesBig[i] = randomBytes[i % 26];
		
    for(int i = 0; i < 38461539*size/1000; i++)
        fwrite(randomBytesBig, 26*1000, 1, output);

    fclose(output);
}
