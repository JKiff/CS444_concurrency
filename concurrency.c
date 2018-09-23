#include <stdio.h>
#include <math.h>
#include <stdlib.h>
int main(){
	init_genrand(1);
	for (int i = 0; i < 100; i++){
		printf("%d\n", produceRandom(2, 9));
	}
}
int produceRandom(int lo, int hi){
	return (abs(genrand_int32()) % (hi + 1 - lo)) + lo;
}
struct bufferNode{
	int value;
	int hangTime;
}
