#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define NumberCons 17

bool ready = 0; // Формальный критерий проверки наступления события
int data = 0;	// Разделяемые данные, передаваемые от 
				// потока-постовщика к потоку-потребителю

// Инструмент блокировки участков кода, имеющих доступ к разделяемым данным
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main() {
	pthread_t _provider;
	pthread_t _consumer[NumberCons];
	int _arg[NumberCons];
	if (pthread_create(&_provider, NULL, provider, NULL) != 0) {
		return 1;
	}
	for (int i = 0; i < NumberCons; i++) {
		_arg[i] = i;
		if (pthread_create(&_consumer[i], NULL, consumer, &_arg[i]) != 0) {
			return 1;
		}
	}
	pthread_join(_provider, NULL);
	for (int i = 0; i < NumberCons; i++) {
		pthread_join(_consumer[i], NULL);
	}
}

void* provider(void* arg) {
	for (int i = 0; i < NumberCons; i++) {
		while (true) {
			pthread_mutex_lock(&lock); // Блокировка мьтекса
			if (ready == 1) {
				pthread_mutex_unlock(&lock);
				continue;
			}
			break;
		}
		ready = 1;
		data = i;
		pthread_cond_signal(&cond);		// Уведомление потока-потребителя о наступившем событии
		pthread_mutex_unlock(&lock);	// Освобождение мьютекса
	}
	return NULL;
}

void* consumer(void* arg) {
	pthread_mutex_lock(&lock);	// Блокировка мьютекса
	while (ready == 0) {		// Проверка наступления события				
		// Ожидание наступления события с временным освобождением мьютекса
		pthread_cond_wait(&cond, &lock);
		// Мьютекс разблокируется до специально сигнала
		// Мьютекс заблокируется только тогда, когда поток-поставщик разблокирует мьютекс			
	}
	ready = 0;
	data = 0;
	pthread_mutex_unlock(&lock); // Окончательное освобождение мютекса
	return NULL;
}
