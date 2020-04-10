/* PROGRAMA REALIZADO POR:
	- ALEJANDRO MATEOS PEDRAZA 70969732N
	- JAVIER LÓPEZ SÁNCHEZ 70921610Y
Los ángeles de Charlie - Práctica 2 - Sistemas operativos I */

#define _HPUX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

struct sigaction accionVieja;	// Única variable global (de tipo struct sigaction)

/* MANEJADORAS */

// MANEJADORA VACÍA

void handlerCharlie(int signal){

}

// MANEJADORA EN CASO DE QUE EL MALO NO SEA ALCANZADO POR NINGÚN DISPARO DE LOS ÁNGELES

void handlerSobrevive(int signal){
	char cadena[80];
	sprintf(cadena, "MALO: \"He sobrevivido a mi vigEsima reencarnaciOn. Hago mutis por el foro\"\n");
	write(1, cadena, strlen(cadena));
}

// Para imprimir se hace uso de una cadena declarada de forma estática permitiendo un máximo de 79 caracteres y el caracter de fin de cadena '\0'

// MANEJADORA EN CASO DE QUE EL MALO SEA ALCANZADO POR ALGÚN DISPARO DE LOS ÁNGELES

void handlerMalo(int sig){
	char cadena[80];
	sprintf(cadena, "MALO: \"AY, me han dado... pulvis sumus, collige, virgo, rosas\"\n");
	write(1,cadena,strlen(cadena));
}

// MANEJADORA PARA INTERRUMPIR DE FORMA ORDENADA EL PROGRAMA SI EL USUARIO PULSA CTRL C

void handler_SIGINT(int sig){
	char cadena[80];
	sprintf(cadena, "\b\bCHARLIE: \"Programa interrumpido\"\n");	// El caracter \b sirve para retroceder 2 caracteres y que no se muestre el ^C introducido por el usuario
	write(1,cadena,strlen(cadena));	
	
	if(-1==sigaction(SIGINT, &accionVieja, NULL)){	// Devuelve a SIGINT su accion original
		perror("fatal error in sigaction");
		return;
	}
	
	if(kill(0, SIGINT)==-1){		// Se envía una señal para acabar con todos los procesos de la misma familia (en este caso de Charlie)
		perror("fatal error in kill");
		return;
	}
}

/* FUNCIÓN QUE EMPLEA CHARLIE PARA CREAR EL FICHERO EN EL QUE LOS MALOS SUCESIVOS IRÁN ALMACENANDO SU PID */

int creaFicheroDePIDS(){

	int vector[20];
	int i, tam;
    
	for(i=0; i<20; i++){	// Se inicializa el vector a 0
		vector[i]=0;
	}
	    
	int fd=open("pids.bin", O_CREAT | O_RDWR, 0755);   // Creación y apertura del fichero en modo R/W con todos los permisos para el usuario y para grupo y otros permisos de lectura y ejecución
	    
	if(fd==-1){
		perror("No puedo abrir el fichero...");
		return -1;
	}
	    
	for(i=0; i<20; i++){
		if(write(fd, &vector[i], sizeof(int))==-1){	// Escribimos en el fichero el contenido del vector. Es decir 20 ceros.
			perror("No puedo escribir en el fichero...");
			return -1;
		}
	}
	    
	tam=lseek(fd, 0, SEEK_END);
	// Movemos el cursor del fichero al final del mismo y el valor que devuelve es la posición del puntero tras el desplazamiento. De esta manera sabemos cual es el tamaño del fichero
	//(80 bytes en este caso)
	    
	close(fd);	// Cerramos el fichero
	    
	return tam;	// Devolvemos el tamaño

}

// FUNCIÓN QUE EMPLEA EL MALO PARA GUARDAR SU PID EN EL FICHERO PROYECTADO EN MEMORIA

int proyectaPIDMalo(int PID){

	int fd, tam;
	int *buffer;
	int a=0, b=19, pos;
	
	fd=open("pids.bin", O_RDWR);	// Se abre el fichero en modo lectura y escritura

	if(fd==-1){
		perror("No puedo abrir el archivo");
		return -1;
	}
	
	tam=lseek(fd, 0, SEEK_END);	// De esta manera sabemos cual es el tamaño del fichero que usaremos en mmap
	buffer=(int*)mmap(0, tam, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);	// Se lleva a cabo la proyección en memoria con un casting a int ya que en encina el buffer es de tipo char*
	close(fd);	// Tras la proyección en memoria no hace falta tener el fichero abierto y se cierra
	
	if(buffer==MAP_FAILED){
		perror("Error de memoria");
		return -1;
	}

	do{
		pos = a+(int)(rand()/(1.0+RAND_MAX)*(b-a+1));	// Generamos una posición al azar en la que cada malo va a introducir su PID SIEMPRE QUE LA POSICIÓN NO SEA 0. SI NO, SE SIGUE BUSCANDO
	}while(buffer[pos]!=0);

	buffer[pos]=PID;	// Se asigna a buffer en la posición aleatorio elegida el PID
	
	munmap((char*)buffer, tam);	// Se deshace la proyección en memoria
	return 0;

}

// FUNCIÓN QUE EMPLEAN LOS ÁNGELES PARA SELECCIONAR UN PID AL QUE DISPARAR

int proyectaPIDAngeles(){

	int fd, tam;
	int *buffer;
	int a=0, b=19, pos;
	int pid;
	
	fd=open("pids.bin", O_RDWR);	// Se abre el fichero en modo lectura y escritura

	if(fd==-1){
		perror("No puedo abrir el archivo");
		return -1;
	}
	
	tam=lseek(fd, 0, SEEK_END);	// De esta manera sabemos cual es el tamaño del fichero que usaremos en mmap
	buffer=(int*)mmap(0, tam, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);	// Se lleva a cabo la proyección en memoria con un casting a int ya que en encina el buffer es de tipo char*
	close(fd);	// Tras la proyección en memoria no hace falta tener el fichero abierto y se cierra
	
	if(buffer==MAP_FAILED){
		perror("Error de memoria");
		return -1;
	}
	
	pos = a+(int)(rand()/(1.0+RAND_MAX)*(b-a+1));	// Generamos una posición al azar para que el ángel correspondiente recoja el valor que ahí se encuentra
	pid=buffer[pos];	// La variable pid tiene el valor del buffer en esa posición para no perderlo
	
	munmap((char*)buffer, tam);	// Se deshace la proyección en memoria
	return pid;	// Se devuelve el pid

}

int soyMalo(char *velocidad){
	int i, pid, pidCharlie;
	char cadena[100];

	srand(getpid()+time(NULL));	// Semilla para generar valores aleatorios en la que interviene el valor del PID correspondiente para asegurar que la semilla es única para cada proceso
	proyectaPIDMalo(getpid());
	
	pidCharlie=getppid();	// Recogemos el pid de Charlie accesible únicamente desde el primer malo

	for(i=1; i<20; i++){	// Bucle para los 20 malos

		if(strcmp(velocidad,"normal")==0) sleep(rand()%2+1);	// Posible sleep si estamos en velocidad normal para los malos del primero al penúltimo
		
		if(pid=fork()!=0){	// Cada malo se reproduce pero solo el hijo continua la ejecución finalizando el malo que está su código
			break;
		}

		srand(getpid()+time(NULL));	// Semilla para generar valores aleatorios en la que interviene el valor del PID correspondiente para asegurar que la semilla es única para cada proceso

		sprintf(cadena, "MALO: \"JA, JA, JA, me acabo de reencarnar y mi nuevo PID es: %d. QuE malo que soy...\"\n", getpid());
		write(1,cadena,strlen(cadena));

		proyectaPIDMalo(getpid());	// Cada malo proyecta su PID en una posición al azar de un fichero atendiendo a la función mostrada previamente
		
		if(i==19){	// En caso de que estemos en el último malo:
			if(strcmp(velocidad,"normal")==0) sleep(rand()%2+1);	// Su sleep ya que no llega al otro
	  		if(kill(pidCharlie,SIGUSR1)==-1){			// Manda a Charlie una señal que solo servirá si no han dado a ningún malo para asegurar que se reencarnen todos
				perror("fatal error in kill");
				return -1;
			}
	  	}
	}
}

int main(int argc, char *argv[]){

	// EMPIEZA LA EJECUCIÓN

	/*---------------------------VARIABLES-------------------------------*/

	char cadena[100];
	int pidCharlie, pidBosley, pidSabrina, pidJill, pidKelly, pidMalo;
	int pidMaloSabrina, pidMaloJill, pidMaloKelly;
	int i, w, status;
	int retSabrina, retJill, retKelly;
	int statusS, statusJ, statusK, totalStatus;

	/*-------------------------PRECONDICIONES----------------------------*/

	if(argc>2){	// Se controla que el número de argumentos sea menor o igual que 2 siendo el 0 el propio programa
		sprintf(cadena, "Forma de uso: %s {velocidad}", argv[0]);
		perror(cadena);
		return -1;
	}

	if(argc==2){
		if((strcmp(argv[1],"normal")!=0) && (strcmp(argv[1],"veloz")!=0)){	// Se comprueba que haya introducido como segundo argumento si lo hay normal o veloz únicamente
			sprintf(cadena, "Forma de uso: %s {velocidad}", argv[0]);
			perror(cadena);
			return -1;
		}
	}

	//*---------------------MÁSCARAS Y SIGACTIONS-----------------------*/

	struct sigaction accion, accion1, accionMalo, accionSobrevive;	// Declaraciones de struct sigaction
	sigset_t mascara, mascara_sin_SIGUSR1;	// Declaración de máscaras
	sigfillset(&mascara);	// Se llena máscara
	sigdelset(&mascara, SIGTERM);	// Se quita SIGTERM de máscara
	sigdelset(&mascara, SIGINT);	// Se quita SIGINT de máscara
	sigdelset(&mascara, SIGUSR2);	// Se quita SIGUSR2 de máscara
	sigfillset(&mascara_sin_SIGUSR1); // Se llena una máscara_sin_SIGUSR1
	sigdelset(&mascara_sin_SIGUSR1, SIGUSR1);	// Se quita SIGUSR1 de mascara_sin_SIGUSR1

	// Se da valor a todos los elementos del sigaction

	accion1.sa_handler=handlerCharlie;
	accion1.sa_mask=mascara_sin_SIGUSR1;
	accion1.sa_flags=0;

	// Se bloquean todas las señales menos SIGTERM, SIGINT y SIGUSR2

	sigprocmask(SIG_BLOCK, &mascara, NULL);

	if(-1==sigaction(SIGUSR1, &accion1, NULL)){	// Se realiza el sigaction para la señal SIGUSR1 sin devolver a ninguna variable su acción original
		perror("fatal error in sigaction");
		return -1;
	}

	// Se da valor a todos los elementos del sigaction

	accionSobrevive.sa_handler=handlerSobrevive;
	accionSobrevive.sa_mask=mascara;
	accionSobrevive.sa_flags=0;

	if(-1==sigaction(SIGUSR2, &accionSobrevive, NULL)){	// Se realiza el sigaction para la señal SIGUSR2 sin devolver a ninguna variable su acción original
		perror("fatal error in sigaction");
		return -1;
	}

	// Se da valor a todos los elementos del sigaction

	accionMalo.sa_handler=handlerMalo;
	accionMalo.sa_mask=mascara;
	accionMalo.sa_flags=0;

	if(-1==sigaction(SIGTERM, &accionMalo, NULL)){
		perror("fatal error in sigaction");	// Se realiza el sigaction para la señal SIGTERM sin devolver a ninguna variable su acción original
		return -1;
	}

	/*-------------------------PRECONDICIONES----------------------------*/
	
	// Se compara el nombre del programa con ./charlie, si este es el nombre se cambia a charlie y además si no se ha pasado argumento se deja pasado como velocidad normal	
	if(strcmp(argv[0],"./charlie")==0){	
		if(argc<2){
			if(execl("./charlie", "charlie", "normal", (char*)NULL)<0){
				perror("fatal error in execl");
				return -1;
			}
		}
		else if(argc==2){
			if(execl("./charlie", "charlie", argv[1], (char*)NULL)<0){
				perror("fatal error in execl");
				return -1;
			}
		}
	}

	/*---------------------------PROCESOS--------------------------------*/
	// Se compara el nombre con charlie, malo... Si es charlie se van a generar sus procesos hijos. En este primer momento Bosley. Si es el malo va a entrar por el default a ver si se ha generado.
	// Si es Bosley o alguno de los ángeles entra por 0 para ejecutar el código del proceso hijo

	if(strcmp(argv[0],"charlie")==0) pidBosley=fork();
	else if(strcmp(argv[0],"malo")==0) pidBosley=1;
	else pidBosley=0;

	switch(pidBosley){
		case -1: perror("fatal error in fork"); return -1;
		case 0:
			// Se compara el nombre del proceso con charlie. Si es este nombre se llama al programa con el nombre de bosley y con lo establecido previamente entrará por donde corresponda
			if(strcmp(argv[0], "charlie")==0){
				if(execl("charlie", "bosley", argv[1], (char*)NULL)<0){
					perror("fatal error in execl");
					return -1;
				}
			}

			// Semilla para generar valores aleatorios en la que interviene el valor del PID correspondiente para asegurar que la semilla es única para cada proceso
			srand(getpid()+time(NULL));

		// Se compara el nombre con bosley. Si este es el nombre se hace el fork. Si no, si es sabrina se entra por el case 0. Los otros dos ángeles entran en el default para generarse después

			if(strcmp(argv[0],"bosley")==0) pidSabrina=fork();
			else if(strcmp(argv[0],"sabrina")==0) pidSabrina=0;
			else pidSabrina=1;

			switch(pidSabrina){
				case -1: perror("fatal error in fork"); return -1;
				case 0:
			// Se compara el nombre del proceso con bosley. Si es este nombre se llama al programa con el nombre de sabrina y con lo establecido previamente entrará por donde corresponda
					if(strcmp("bosley", argv[0])==0){
						if(execl("charlie", "sabrina", argv[1], (char*)NULL)<0){
							perror("fatal error in execl");
							return -1;
						}
					}

					// Semilla para generar valores aleatorios en la que interviene el valor del PID correspondiente para asegurar que la semilla es única para cada proceso
					srand(getpid()+time(NULL));

					sigsuspend(&mascara_sin_SIGUSR1);	// Espera a que Bosley le avise de que puede presentarse

					sprintf(cadena, "SABRINA: \"Hola, he nacido y mi PID es %d\"\n", getpid());
				  	write(1, cadena, strlen(cadena));
					
					if(kill(getppid(),SIGUSR1)==-1){	// Avisa a Bosley de que se ha presentado
						perror("fatal error in kill");
						return -1;
					}

					sigsuspend(&mascara_sin_SIGUSR1);	// Espera que Bosley les indique disparar

					for(i=0; i<3; i++){	// Bucle que indica los 3 disparos posibles de cada ángel

						if(strcmp(argv[1],"normal")==0) sleep(rand()%6+6);	// En el modo lento cada angel espera entre 3 y 6 segundos

						pidMaloSabrina=proyectaPIDAngeles();	// Sabrina coge al azar un pid al que disparar
						
						sprintf(cadena, "SABRINA: \"Voy a disparar al PID %d\"\n", pidMaloSabrina);
					  	write(1, cadena, strlen(cadena));

						// En función de si acierta, da a un pid erroneo o coge una posicion sin PID imprimirá 3 posibles mensajes
						// Si falla de cualquiera de las dos maneras devuelve un 0. Si acierta un 1 para calcular después el resultado general

						if(pidMaloSabrina==0){
							sprintf(cadena, "SABRINA: \"Pardiez! La pistola se ha encasquillado\"\n");
							write(1,cadena,strlen(cadena));
							retSabrina=1;
						}

						else if(kill(pidMaloSabrina,SIGTERM)==-1){
							sprintf(cadena, "SABRINA: \"He fallado. Vuelvo a intentarlo\"\n");
							write(1,cadena,strlen(cadena));
							retSabrina=1;
						}

						else{
							sprintf(cadena, "SABRINA: \"BINGO! He hecho diana! Un malo menos\"\n");
							write(1,cadena,strlen(cadena));
							retSabrina=0;
							break;	// Si acierta el disparo al pid del malo vivo salimos del bule directamente
						}
						
						if(i==2){	// Si falla 3 veces imprime el mensaje final
							sprintf(cadena, "SABRINA: \"He fallado ya tres veces y no me quedan mAs balas. Muero\"\n");
							write(1,cadena,strlen(cadena));
						}

					}
					
					return retSabrina;	// Sabrina devuelve su resultado de mision

				default:

			// Se compara el nombre con bosley. Si este es el nombre se hace el fork. Si no, si es jill se entra por el case 0. Si es kelly entra en el default para generarse después

					if(strcmp(argv[0],"bosley")==0) pidJill=fork();
					else if(strcmp(argv[0],"jill")==0) pidJill=0;
					else pidJill=1;

					switch(pidJill){
						case -1: perror("fatal error in fork"); return -1;
						case 0:
			// Se compara el nombre del proceso con bosley. Si es este nombre se llama al programa con el nombre de jill y con lo establecido previamente entrará por donde corresponda
							if(strcmp("bosley", argv[0])==0){
								if(execl("charlie", "jill", argv[1], (char*)NULL)<0){
									perror("fatal error in execl");
									return -1;
								}
							}
						// Semilla para generar valores aleatorios en la que interviene el valor del PID correspondiente para asegurar que la semilla es única para cada proceso
							srand(getpid()+time(NULL));

							sigsuspend(&mascara_sin_SIGUSR1);	// Espera a que Bosley le avise de que Sabrina se ha presentado

							sprintf(cadena, "JILL: \"Hola, he nacido y mi PID es %d\"\n", getpid());
						  	write(1, cadena, strlen(cadena));

							if(kill(getppid(),SIGUSR1)==-1){	// Avisa a Bosley de que se ha presentado
								perror("fatal error in kill");
								return -1;
							}

							sigsuspend(&mascara_sin_SIGUSR1);	// Espera que Bosley les indique disparar

							for(i=0; i<3; i++){	// Bucle que indica los 3 disparos posibles de cada ángel

								if(strcmp(argv[1],"normal")==0) sleep(rand()%6+6);	// En el modo lento cada angel espera entre 3 y 6 segundos

								pidMaloJill=proyectaPIDAngeles();	// Jill coge al azar un pid al que disparar
								
								sprintf(cadena, "JILL: \"Voy a disparar al PID %d\"\n", pidMaloJill);
							  	write(1, cadena, strlen(cadena));

								// En función de si acierta, da a un pid erroneo o coge una posicion sin PID imprimirá 3 posibles mensajes
								// Si falla de cualquiera de las dos maneras devuelve un 0. Si acierta un 1 para caluclar después el resultado general

								if(pidMaloJill==0){
									sprintf(cadena, "JILL: \"Pardiez! La pistola se ha encasquillado\"\n");
									write(1,cadena,strlen(cadena));
									retJill=1;
								}

								else if(kill(pidMaloJill,SIGTERM)==-1){
									sprintf(cadena, "JILL: \"He fallado. Vuelvo a intentarlo\"\n");
									write(1,cadena,strlen(cadena));
									retJill=1;
								}

								else{
									sprintf(cadena, "JILL: \"BINGO! He hecho diana! Un malo menos\"\n");
									write(1,cadena,strlen(cadena));
									retJill=0;
									break;	// Si acierta el disparo al pid del malo vivo salimos del bule directamente
								}
								
								if(i==2){	// Si falla 3 veces imprime el mensaje final
									sprintf(cadena, "JILL: \"He fallado ya tres veces y no me quedan mAs balas. Muero\"\n");
									write(1,cadena,strlen(cadena));
								}
								

							}
							
							return retJill;	// Jill devuelve su resultado de mision

						default:

							// Se compara el nombre con bosley. Si este es el nombre se hace el fork. Sino si es kelly se entra por el case 0

							if(strcmp(argv[0],"bosley")==0) pidKelly=fork();
							else pidKelly=0;

							switch(pidKelly){
								case -1: perror("fatal error in fork"); return -1;
								case 0:
			// Se compara el nombre del proceso con bosley. Si es este nombre se llama al programa con el nombre de kelly y con lo establecido previamente entrará por donde corresponda
									if(strcmp("bosley", argv[0])==0){
										if(execl("charlie", "kelly", argv[1], (char*)NULL)<0){
											perror("fatal error in execl");
											return -1;
										}
									}
						// Semilla para generar valores aleatorios en la que interviene el valor del PID correspondiente para asegurar que la semilla es única para cada proceso
									srand(getpid()+time(NULL));

									sigsuspend(&mascara_sin_SIGUSR1);	// Espera a que Bosley le avise de que se han presentado Sabrina y Jill

									sprintf(cadena, "KELLY: \"Hola, he nacido y mi PID es %d\"\n", getpid());
								  	write(1, cadena, strlen(cadena));

									if(kill(getppid(),SIGUSR1)==-1){	// Avisa a Bosley de que se ha presentado
										perror("fatal error in kill");
										return -1;
									}

									sigsuspend(&mascara_sin_SIGUSR1);	// Espera que Bosley les indique disparar

									for(i=0; i<3; i++){	// Bucle que indica los 3 disparos posibles de cada ángel

										if(strcmp(argv[1],"normal")==0) sleep(rand()%6+6);	// En el modo lento cada angel espera entre 3 y 6 segundos

										pidMaloKelly=proyectaPIDAngeles();	// Kelly coge al azar un pid al que disparar
										
										sprintf(cadena, "KELLY: \"Voy a disparar al PID %d\"\n", pidMaloKelly);
									  	write(1, cadena, strlen(cadena));

										// En función de si acierta, da a un pid erroneo o coge una posicion sin PID imprimirá 3 posibles mensajes
										// Si falla de cualquiera de las dos maneras devuelve un 0. Si acierta un 1 para caluclar después el resultado general

										if(pidMaloKelly==0){
											sprintf(cadena, "KELLY: \"Pardiez! La pistola se ha encasquillado\"\n");
											write(1,cadena,strlen(cadena));
											retKelly=1;
										}

										else if(kill(pidMaloKelly,SIGTERM)==-1){
											sprintf(cadena, "KELLY: \"He fallado. Vuelvo a intentarlo\"\n");
											write(1,cadena,strlen(cadena));
											retKelly=1;
										}

										else{
											sprintf(cadena, "KELLY: \"BINGO! He hecho diana! Un malo menos\"\n");
											write(1,cadena,strlen(cadena));
											retKelly=0;
											break;	// Si acierta el disparo al pid del malo vivo salimos del bule directamente
										}
										
										if(i==2){	// Si falla 3 veces imprime el mensaje final
											sprintf(cadena, "KELLY: \"He fallado ya tres veces y no me quedan mAs balas. Muero\"\n");
											write(1,cadena,strlen(cadena));
										}

									}

										return retKelly;	// Kelly devuelve su resultado de mision
								default:

									sigsuspend(&mascara_sin_SIGUSR1);	// Espera a que Charlie se haya presentado. Se presenta y presenta a los ángeles.
									
									sprintf(cadena, "BOSLEY: \"Hola, papA, dOnde estA mamA? Mi PID es %d y voy a crear a los Angeles...\"\n", getpid());
									write(1, cadena, strlen(cadena));

									if(kill(pidSabrina,SIGUSR1)==-1){	// Indica a Sabrina que puede presentarse
										perror("fatal error in kill");
										return -1;
									}

									sigsuspend(&mascara_sin_SIGUSR1);	// Espera a que Sabrina se presente

									if(kill(pidJill,SIGUSR1)==-1){		// Indica a Jill que puede presentarse
										perror("fatal error in kill");
										return -1;
									}

									sigsuspend(&mascara_sin_SIGUSR1);	// Espera a que Jill se presente

									if(kill(pidKelly,SIGUSR1)==-1){		// Indica a Kelly que puede presentarse
										perror("fatal error in kill");
										return -1;
									}

									
									sigsuspend(&mascara_sin_SIGUSR1);	// Espera a que Kelly se presente

									if(kill(getppid(),SIGUSR1)==-1){	// Avisa a Charlie de que ya están los ángeles para que cree al malo
										perror("fatal error in kill");
										return -1;
									}

									sigsuspend(&mascara_sin_SIGUSR1);	// Espera a que Charlie haya creado al malo

									if(kill(pidSabrina,SIGUSR1)==-1){	// Indica a Sabrina que empiece a disparar
										perror("fatal error in kill");
										return -1;
									}

									if(kill(pidJill,SIGUSR1)==-1){		// Indica a Jill que empiece a disparar
										perror("fatal error in kill");
										return -1;
									}


									if(kill(pidKelly,SIGUSR1)==-1){		// Indica a Kelly que empiece a disparar
										perror("fatal error in kill");
										return -1;
									}

									//Espera a que los 3 angeles hayan acabado

									waitpid(pidSabrina, &statusS, 0);
									waitpid(pidJill, &statusJ, 0);
									waitpid(pidKelly, &statusK, 0);
									
									sprintf(cadena, "BOSLEY: \"Los tres Angeles han acabado su misiOn. Informo del resultado a Charlie y muero\"\n");
									write(1, cadena, strlen(cadena));

								// Usa el valor de retorno de cada angel para calcular un valor que le devolverá a Charlie para indicarle el resultado de la mision
									
									if(WIFEXITED(statusS) && WIFEXITED(statusJ) && WIFEXITED(statusK)){
										totalStatus=100*WEXITSTATUS(statusS)+10*WEXITSTATUS(statusJ)+WEXITSTATUS(statusK);
									}
							
							// Devuelve los resultados como un numero de 3 cifras donde la primera es el resultado de Sabrina, la segunda el de Jill y la tercera el de Kelly
									return totalStatus;
							}
					}
			}
		break;

		default:

		// Se compara el nombre con charlie. Si este es el nombre se ejecuta la primera parte del código de charlie y el fork. Si no, es malo y se entra por el case 0.

			if(strcmp(argv[0],"charlie")==0){
				sprintf(cadena, "CHARLIE: \"Bosley, hijo de mis entretelas, tu PID es %d. Espero a que me avises...\"\n", pidBosley);
				write(1, cadena, strlen(cadena));
				
				if(kill(pidBosley,SIGUSR1)==-1){	// Le indica a Bosley que ya se ha presentado
					perror("fatal error in kill");
					return -1;
				}

				sigsuspend(&mascara_sin_SIGUSR1);	// Espera a que Bosley haya creado a los ángeles

				sprintf(cadena, "CHARLIE: \"Veo que los Angeles ya han nacido. Creo al malo...\"\n");
				write(1, cadena, strlen(cadena));
				
				creaFicheroDePIDS();			// Se crea el fichero de PIDs

				pidMalo=fork();				// Se lleva a cabo el fork y se crea el primer malo
			}
			else pidMalo=0;

			switch(pidMalo){
			case -1: perror("fatal error in fork"); return -1;
			case 0:
			// Se compara el nombre del proceso con charlie. Si es este nombre se llama al programa con el nombre de malo y con lo establecido previamente entrará por donde corresponda
				if(strcmp(argv[0], "charlie")==0){
					execl("charlie", "malo", argv[1], (char*)NULL);
				}
				soyMalo(argv[1]);	// Llamada a la función en la que se van a crear el resto de malos
				break;
			default:
				accion.sa_handler=handler_SIGINT;
				accion.sa_mask=mascara;
				accion.sa_flags=0;

				if(-1==sigaction(SIGINT, &accion, &accionVieja)){	// Manejadora para matar a todos los procesos en caso de interrupción de programa
					perror("fatal error in sigaction");
					return -1;
				}

				// Semilla para generar valores aleatorios en la que interviene el valor del PID correspondiente para asegurar que la semilla es única para cada proceso
				srand(getpid()+time(NULL));	

				sprintf(cadena, "CHARLIE: \"El malo ha nacido y su PID es %d. Aviso a Bosley\"\n", pidMalo);
				write(1, cadena, strlen(cadena));

				if(kill(pidBosley,SIGUSR1)==-1){	// Envia a Bosley la seña de que ha creado al malo
					perror("fatal error in kill");
					return -1;
				}
				
				waitpid(pidMalo, &status, 0);	// Espera por el primer malo para que no se quede zombie
				waitpid(pidBosley, &status, 0);	// Espera a que Bosley le informe de los resultados
				
				if(WIFEXITED(status)){ // Si el programa termina con éxito
					// Examina el valor devuelto por la función de Bosley para enviar el mensaje correspondiente en función de los hechos de los ángeles
					switch(WEXITSTATUS(status)){
						case 111: 
							sigsuspend(&mascara_sin_SIGUSR1);	// Espera todas las reencarnaciones de los malos si los ángeles no han alcanzado ninguna

							// Si no se ha alcanzado a ningún malo, este gana e imprime que ha logrado su objetivo por medio de la manejadora especificada arriba
							if(kill(getpid(), SIGUSR2)==-1){
								perror("fatal error in kill");
								return -1;
							}

							sprintf(cadena, "CHARLIE: \"El pAjaro volO. Ahora se pone tibio a daiquiris en el Caribe\"\n");
							write(1, cadena, strlen(cadena));
						break;
						case 110: sprintf(cadena, "CHARLIE: \"Bravo por Kelly\"\n");
							write(1, cadena, strlen(cadena));
						break;
						case 101: sprintf(cadena, "CHARLIE: \"Jill, donde pones el ojo, pones la bala\"\n");
							 write(1, cadena, strlen(cadena));
						break;
						case 100: sprintf(cadena, "CHARLIE: \"Sabrina, otra vez serA, te apuntarE a una academia de tiro\"\n");
							 write(1, cadena, strlen(cadena));
						break;
						case 11: sprintf(cadena, "CHARLIE: \"Bien hecho, Sabrina, siempre fuiste mi favorita\"\n");
							  write(1, cadena, strlen(cadena)); 
						break;
						case 10: sprintf(cadena, "CHARLIE: \"Jill, no te preocupes, seguro que la pistola estA mal\"\n");
							  write(1, cadena, strlen(cadena));
						break;
						case 1: sprintf(cadena, "CHARLIE: \"Kelly, mala suerte, tus compaNeras acertaron y tU, no\"\n");
							  write(1, cadena, strlen(cadena));
						break;
						case 0: sprintf(cadena, "CHARLIE: \"Pobre malo. Le habEis dejado como un colador... Sois unos Angeles letales\"\n");
							  write(1, cadena, strlen(cadena)); 
						break;
					}
					
					if(kill(0, SIGKILL) == -1){	// Acaba con todos los procesos que puedan quedar vivos del programa
						perror("fatal error in kill");
						return -1;
					}
				}
			}
	}
	
	return 0;
}
