#include <stdio.h>
#include <stdlib.h>

int main( void ) {
	int score = 4560;
	char numstr[5];
	int i;

	sprintf( numstr, "%05i", score );

	printf( "Score : %i\nString: %s\n", score, numstr );

	printf( "%c:%c:%c\n", numstr[0], numstr[2], numstr[5] );

	for ( i = 0; i <= 5; i++ ) {
		char c = numstr[i];
		int n = atoi( &c );

		printf( "%d:", n );
	}
	printf( "\n" );

	return 0;
}
