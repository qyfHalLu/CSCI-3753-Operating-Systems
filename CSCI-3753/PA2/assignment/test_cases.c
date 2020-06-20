#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main()
{
	char key, buffer[BUFFER_SIZE];
	int file = open("/dev/pa2_char_driver", O_RDWR);
	printf("Test cases \n");

	while(1)
	{
		printf("Keys: r to read, w to write, s to seek, e to exit. Enter input: ");
		scanf("%c", &key);
		switch (key)
		{
			case 'r':
			case 'R':
				read(file, buffer, BUFFER_SIZE);
				printf("Device output: %s\n", buffer);
				while (getchar() != '\n');
				break;

			case 'w':
			case 'W':
				printf("Enter your text: ");
				scanf("%c", buffer);
				write(file , buffer, BUFFER_SIZE);
				while (getchar() != '\n');
				break;
			
			case 's':
			case 'S':
				break;

			case 'e':
			case 'E':
				printf("Exited");
				return 0;

			default:
				while (getchar() != '\n');
		}
	}
	close(file);
	return 0;
}