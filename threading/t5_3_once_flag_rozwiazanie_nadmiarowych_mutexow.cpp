/*****************************************************
  [Rozwiązanie]	nadmiarowosć mutexów - once_flag
******************************************************
*/

#include "pch.h"
#include <iostream>
#include <thread>
#include <string>
#include <fstream>
#include <mutex>
using namespace std;

class LogFile {
private:
	mutex _mu; // mutex do zsynchronizowania zapisu do pliku - ale takich operacji moze być bardzo dużo
	//mutex _mu_open; //drugi mutex, do otwierania pliku który ma byc otwarty tylko raz - nadmiarowy mutex, lepsza flaga
	once_flag _flag; //rozwiązanie nadmaowości mutexu
	ofstream _f;

public:
	LogFile() {
		//_f.open("log.txt"); - wywalenie otwarcia pliku z konstruktora 
			//- bez sensu otwierac plik, skoro np. nigdy nie wywowalamy funkcji korzystajacej z plik
	}
	void shared_print(string id, int value) {
		
		/*{ //nadmiarowe użycie mutexów
			//teraz kazde wywolanie funkcji shared_print sprawdza i  blokuje albo odlokowuje dostęp do otwarcia pliku - marnotrastwo cykli procka
			//dodatkowo program przestaje być uruchamiany współbieżnie, cojest bardzo brzydkie :<
			//rozwiazanie to flaga
			//
			unique_lock<mutex> locker2(_mu_open); //teraz zanim plik zostanie otwarty mutext dla otwarcia pliku jest zablokowany
			if (!_f.is_open()) { //sprawdzenie czy plik jest otwarty też musi być chronione mutexem, bo wątki sprawdzajac warotść, mogą nie zauważyć, jeszcze tej zmienionej- że plik jest już otwarty, przez inny wątek
				// the file will be open ony once in shared_print function
				// this is known as LAZY INITILIZATION albo INITIALIZATION UPON FIRST USE IDIOM
				_f.open("log.txt");
			}
		}
		*/

		//rozwiazanie nadmiarowego użycia mutexów, za pomoca rozwiazania z biblioteki stndardowej
		call_once(_flag, [&]() { _f.open("log.txt");  }); //file will be opened only once by one thread with lambda function which opens the file
		//dzięki temu życie staje się prostsze a cały program porządny <3

		unique_lock<mutex> locker(_mu, defer_lock); //now the locker is the owner of the mutex, but mutex is not locked yet
		locker.lock();
		_f << "From " << id << ": " << value << endl; //tylko to printowanie ma być synchornizowae za pomocą mutexa
		locker.unlock();


};


void function_1(LogFile& log) {
	for (int i = 0; i > -100; i--)
		log.shared_print("From t1: ", i);
}


int main() {
	LogFile log;
	thread t1(function_1, ref(log)); //przekazywanie przez referencje obiektu klasy

	for (int i = 0; i < 100; i++)
		log.shared_print2("From main: ", i);


	t1.join(); //wait for t1 to finish
	return 0;
}