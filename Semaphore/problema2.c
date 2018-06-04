#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_H 2 //numero maximo de hidrogenos que vamos a generar por tanda
#define MAX_O 1 //numero maximo de oxigenos que vamos a generar por tanda

//semaforos genericos
sem_t hSemaphore;   //semaforos para que el hidrogeno y el oxigeno esperen a la creacion del agua para mostrar el mensaje de que han desaparecido
sem_t oSemaphore;

//semaforos binarios
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  //controlara la zona critica del agua
pthread_mutex_t principal = PTHREAD_MUTEX_INITIALIZER;  //controlara el main

//arrays de threads que guardaran su ejecucion
pthread_t makeH [MAX_H];
pthread_t makeO [MAX_O];

//numero de atomos y de oxigenos que hay actualmente
int numHAtoms=0;
int numOAtoms=0;

void producirAgua() //funcion de produccion del agua
{
    printf("The chemical reaction for water has occurred correctly.\n");
    numHAtoms-=MAX_H;   //restamos los elementos que vamos a usar
    numOAtoms-=MAX_O;
    sem_post(&hSemaphore);  //levantamos los semaforos de los dos hidrogenos y el oxigeno que estan esperando para terminar su ejecucion
    sem_post(&hSemaphore);
    sem_post(&oSemaphore);
}

void oReady()   //cuando preparamos un oxigeno comprobamos si la reaccion esta ready
{
    printf("I have made an O atom\n");
    isReady();
}

void isReady()  //la mezclara estara ready cuando tengamos 1 oxigeno y 2 hidrogenos
{
    if(numOAtoms>=1 && numHAtoms>=2)    //comprobamos que haya el numero necesario de atomos
    {
        producirAgua();
    }
}

void hReady()   //comprobamos cuando preparamos el hidrogeno si la reaccion esta ready
{
    printf("I have made an H atom\n");
    isReady();
}

void *hFuncion() //funcion de creacion del hilo de hidrogeno   
{
    sleep(3);   //sleep para que se vea mejor
    pthread_mutex_lock(&mutex); //bloqueamos el semaforo binario para que ningun otro hilo vaya a modificar el dato mientras nosotros estemos modificandolo
    numHAtoms++;
    hReady();
    pthread_mutex_unlock(&mutex);   //lo levantamos aqui (el semaforo) porque el hReady podria meter en la produccion del agua 
    sem_wait(&hSemaphore);  //esperamos a que se produzca la reaccion de agua para hacerlo desaparecer al hilo  
    printf("Este atomo de hidrogeno ha desaparecido\n");
    pthread_exit(NULL);
}


void *oFuncion()
{
//Hacemos un atomo O
    sleep(3);
    pthread_mutex_lock(&mutex); //bloqueamos la zona critica, es critica porque estamos cambiando variables globales y podria llegar a suceder dos veces la creacion del agua por ejemplo si no lo controlamos
    numOAtoms++;    //aumentamos el numero de atomos
    oReady();
    pthread_mutex_unlock(&mutex);   //desbloqueamos zona critica
    sem_wait(&oSemaphore);  //esperamos hasta que hayamos hecho el agua para hacer desaparecer el hilo
    printf("Este atomo de oxigeno ha desaparecido\n");
    pthread_mutex_unlock(&principal);   //desbloqueamos el semaforo en el main, no lo ponemos en el hidrogeno porque lo desbloquearia dos veces. 
    pthread_exit(NULL); //puede ser que se cree otro proceso en el array donde tenemos la referencia antes de que finalice, no pasa nada porqueel hilo sigue ejecutandose y terminaria igual.
}

void *mainThread() //funcion principal de creacion de hilos
{
    int i;
    for(i=0;i<MAX_H;i++)    //creamos los hidrogenos necesarios
    {
        pthread_create(&makeH[i],NULL,hFuncion,NULL);
    }

    for(i=0;i<MAX_O;i++)    //creamos los oxigenos necesarios
    {
        pthread_create(&makeO[i],NULL,oFuncion,NULL);
    }
    pthread_exit(NULL);

}



void main()
{
    sem_init(&hSemaphore,0,0); //ponemos los semaforos a 0 para que se esperen y solo desaparezcan cuando se haya creado el agua
    sem_init(&oSemaphore,0,0);

    pthread_t maint;    //declaramos el hilo principal maint
    while(1){   //bucle de creacion del agua
        pthread_mutex_lock(&principal); //con este semaforo creamos un margen de tiempo para poder crear el agua sin que se pisen los hilos y los mensajes
        pthread_create(&maint,NULL,mainThread,NULL);    //creamos el hilo de ejecucion maint
    }
}
