#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void trimStr(char * str)
{
    int index, i;

    /* Set default index */
    index = -1;

    /* Find last index of non-white space character */
    i = 0;
    while(str[i] != '\0')
    {
        if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
        {
            index= i;
        }

        i++;
    }

    /* Mark next character to last non-white space character as NULL */
    str[index + 1] = '\0';
}

int main( int argc, char *argv[] )
{

  FILE *fp;
  char path[1035];

  /* Open the command for reading. */
  fp = popen("dhcp-lease-list", "r");
  if (fp == NULL) {
    printf("Failed to run command\n");
    exit(1);
  }

  /* Read the output a line at a time - output it. */
  int i = 0;
  int ip_id = 19;
  int name_id = 35;
  int ip_id_end = name_id - 2;
  int name_id_end = name_id + 15;
  while (fgets(path, sizeof(path), fp) != NULL) {
  	int ind2 = 0;
  	int ind1 = 0;
	char ip_addr[20] = "\0";
	char name[20] = "\0";	
	if(i > 2){
		for (int k = 0; k < strlen(path); k++){
			if(k >= ip_id && k < ip_id_end){
				ip_addr[ind1++] = path[k];
			}else if(k >= name_id && k < name_id_end){
				name[ind2++] = path[k];
			}
		}
	
	trimStr(ip_addr);
	trimStr(name);
	printf("%s\n", ip_addr);
	printf("%ld\n", strlen(ip_addr));
	printf("%s\n", name);
	printf("%ld\n", strlen(name));
	}
	i++;
  }
  printf("\n");
  /* close */
  pclose(fp);

  return 0;
}
