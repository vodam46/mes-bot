#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int cond = 0;
pthread_mutex_t m;
pthread_cond_t c;

void* thread(void* arg) {
	void* a = arg;

	pthread_mutex_lock(&m);
	while (!cond)
		pthread_cond_wait(&c, &m);
	pthread_mutex_unlock(&m);

	printf("thread %llu\n", (unsigned long long)a);

	return NULL;
}

pthread_t threads[10];

int main(void) {
	for (unsigned long long i = 0; i < 10; i++) {
		int error = (pthread_create(&threads[i], NULL, &thread, (void*)i));
		if (error) {
			printf("error creating thread %s\n", strerror(error));
		}
	}

	for (int i = 0; i < 3; i++) {
		printf("%d\n", i);
		sleep(1);
	}

	pthread_mutex_lock(&m);
	cond = 1;
	for (int i = 0; i < 10; i++) {
		pthread_cond_signal(&c);
	}
	pthread_mutex_unlock(&m);

	for (int i = 0; i < 10; i++)
		pthread_join(threads[i], NULL);

	return 0;
}
