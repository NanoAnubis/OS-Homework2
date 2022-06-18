#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
	
    	if (argc != 2) {
		errx(1, "Scripts needs 1 argument!");
    	}

    	int p = open(argv[1], O_RDONLY);
    	if(p==-1) {
		err(2, "Couldn't open file in read only");
    	}
	
    	char magic[3];
    	read(p, &magic, sizeof(magic));
    	if(strncmp(magic, "ORC", 3) != 0) {
	    	errx(3, "File format isn't supported");
    	}

    	uint32_t ram_size;
    	read(p,&ram_size,sizeof(ram_size));
    	int64_t* ram = (int64_t*)malloc(ram_size*sizeof(int64_t));
    	if(ram == NULL) {
	   	err(4, "Can't alocate memory");
    	}	
	
    	struct stat st;
    	fstat(p,&st);
    	if((st.st_size-7)%25!=0) {
	    	errx(5, "File size is wrong");
    	}
	
    	uint8_t opcode;
    	ssize_t r;
	bool skip = 0;
    	while((r = read(p,&opcode,sizeof(opcode))) > 0) {
		uint64_t first,second,third;
		read(p,&first,sizeof(first));
	 	read(p,&second,sizeof(second));
		read(p,&third,sizeof(third));
		
		if(skip==1){
			skip=0;
			continue;
		}

	    	switch(opcode) {
			case 0x00 :
				break;

			case 0x95 :
				if(first>=ram_size) {
					errx(6, "Illegal access");
				}
				ram[first]=second;	
				break;

			case 0x5d :
				if(first>=ram_size || second>=ram_size || ram[second] >= ram_size) {
					errx(6, "Illegal access");
				}
				ram[first]=ram[ram[second]];
				break;

			case 0x63 :
				if(first>=ram_size || ram[first]>=ram_size || second>=ram_size) {
					errx(6, "Illegal access");
				}
				ram[ram[first]]=ram[second];
				break;

			case 0x91 :
				if(first>=ram_size) {
					errx(6, "Illegal access");
				}
				lseek(p,7+ram[first]*(1+3*sizeof(first)), SEEK_SET);
				break;

			case 0x25 :
				if(first>=ram_size) {
					errx(6, "Illegal access");
				}
				if(ram[first]>0){
					skip=1;
				}
				break;

			case 0xad :
				if(first>=ram_size || second>=ram_size || third>=ram_size) {
					errx(6, "Illegal access");
				}
				ram[first]=(ram[second]+ram[third]);
				break;

			case 0x33 :
				if(first>=ram_size || second>=ram_size || third>=ram_size) {
					errx(6, "Illegal access");
				}
				ram[first]=(ram[second]*ram[third]);
				break;

			case 0x04 :
				if(first>=ram_size || second>=ram_size || third>=ram_size || ram[third]==0) {
					errx(6, "Illegal access or division by zero");
				}
				ram[first]=(ram[second]/ram[third]);
				break;

			case 0xb5 :
				if(first>=ram_size || second>=ram_size || third>=ram_size || ram[third]==0) {
					errx(6, "Illegal access or division by zero");
				}
				ram[first]=(ram[second]%ram[third]);
				break;

			case 0xc1 :
				if(first>=ram_size) {
					errx(6, "Illegal access");
				}
				printf("%c", (char)ram[first]);
				break;

			case 0xbf :
				if(first>=ram_size) {
					errx(6, "Illegal access");
				}
				sleep(ram[first]/1000);
				break;

			default:
				free(ram);
				errx(6,"Unrecognised opcode");
				break;

	    	}
    	}

	free(ram);
    	exit(0);
}

