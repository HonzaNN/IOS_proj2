/* 

Projek na paralerni procesy. Vystup do souboru "proj2.out"

$ ./proj2 NO NH TI TB
* NO: Pocet kysliku
* NH: Pocet vodiku
* TI: Maximalni cas milisekundach, po ktery atom kysliku/vodiku po svem vytvoreni ceka, nez se zaradi do fronty na vytvareni molekul. 0<=TI<=1000
* TB: Maximalni cas v milisekundach nutny pro vytvoreni jedne molekuly. 0<=TB<=1000 

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#define semOPEN(sem, adresa, hodnota) { (sem) = sem_open((adresa), O_CREAT | O_EXCL, 0666, (hodnota)); }
#define alloc_int(x){ x = mmap(NULL, sizeof(*(x)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}


#define x() {printf("%d\n", __LINE__);}

#define ADRESA "/semmm"
#define AD2 "/sem2"
#define AD3 "/sem333"
#define POC "/poc"
#define M2 "/m2"
#define T "/t"
#define T2 "/t2"
#define HYD "/hydr"
#define OX "/oxy"
#define OSEM "/osem"
#define HSEM "/hsem"
#define POCITADLO "/pocitadlo"
#define SEM_POC "/sem_poc"
#define H "/h"
#define O "/o"
#define CEK "/cek"
#define OPOC "/opoc"
#define SEM_HYD "/sem_hyd"

sem_t *mutex = NULL;
sem_t *oxyd = NULL;
sem_t *hydro = NULL;
sem_t *mutex2 = NULL;
sem_t *t = NULL;
sem_t *t2 = NULL;
sem_t *osem = NULL;
sem_t *hsem = NULL;
sem_t *sem_poc = NULL;
sem_t *sem_hyd = NULL;

int *poc = NULL;
int *hyd = NULL;
int *ox = NULL;
int *pocitadlo = NULL;
int *h = NULL;
int *o = NULL;
int *cek = NULL;
int *Opoc = NULL;




void alloc_sem(){ //######## FUNKCE PRO ALOKACI SEMAFORU ########//
    semOPEN(mutex, ADRESA, 0);
    semOPEN(oxyd, AD2, 0);
    semOPEN(hydro, AD3, 0);
    semOPEN(mutex2, M2, 1);
    semOPEN(t, T, 0);
    semOPEN(t2, T2, 1);
    semOPEN(osem, OSEM, 1);
    semOPEN(hsem, HSEM, 1);
    semOPEN(sem_poc, SEM_POC, 1);
    semOPEN(sem_hyd, SEM_HYD, 0);
}

void dealoc_vse(){   //######## FUNKCE PRO DEALOKACI PROMENYCH ########//
    munmap(poc, sizeof(int));
    munmap(cek, sizeof(int));
    sem_close(mutex);
    sem_close(hydro);
    sem_close(mutex2);
    sem_close(oxyd);
    sem_close(t);
    sem_close(t2);
    sem_close(hsem);
    sem_close(osem);
    sem_close(sem_poc);
    sem_close(sem_hyd);
    shm_unlink(POC);
    shm_unlink(CEK);
    sem_unlink(ADRESA);
    sem_unlink(SEM_POC);
    sem_unlink(SEM_HYD);
    sem_unlink(AD2);
    sem_unlink(AD3);
    sem_unlink(T);
    sem_unlink(T2);
    sem_unlink(M2);
    sem_unlink(HSEM);
    sem_unlink(OSEM);
    munmap(pocitadlo, sizeof(int));
    munmap(ox, sizeof(int));
    munmap(hyd, sizeof(int));
    munmap(o, sizeof(int));
    munmap(h, sizeof(int));
    munmap(Opoc, sizeof(int));
    shm_unlink(POCITADLO);
    shm_unlink(HYD);
    shm_unlink(OX);
    shm_unlink(H);
    shm_unlink(O);
    shm_unlink(OPOC);
}



void cekacka(){ //zajisti aby 3 atomy skoncili ve stejnou dobu
    

    sem_wait(mutex2);
    
    cek[0]++;
    if(cek[0] == 3){
        sem_wait(t2);
        sem_post(t);
    }
    sem_post(mutex2);
   
    sem_wait(t);
    sem_post(t);

    sem_wait(mutex2);
    cek[0]--;
    
    if(cek[0] == 0){
        sem_wait(t);
        sem_post(t2);
    }
    sem_post(mutex2);
    
    sem_wait(t2);
    sem_post(t2);
    

}


//#########  FUNKCE PRO PROCESY VODIK  #########//
void hydrogen(int id, FILE *s, int TI, int NO, int NH)
{
    
    int no;
    
    sem_wait(sem_poc);
    fprintf(s,"%d: H %d: started\n", ++pocitadlo[0], id);
    
    sem_post(sem_poc);

    sem_wait(hsem);
    hyd[0]++;    //Pocitadlo molekul H se incrementuje
    if(ox[0]==NO && hyd[0]==NH) sem_post(mutex);
    sem_post(hsem);

    usleep(1000*(rand()%(TI +1)));

    sem_wait(sem_poc);
    fprintf(s,"%d: H %d: going to queue\n",  ++pocitadlo[0],id); ///Nainicializuje se atom H
     
    sem_post(sem_poc);
   
    
    sem_wait(hydro); //ceka na otevreni semaforu (takto by to melo fungovat ze?)
    
    sem_wait(hsem);
    if(h[0] >=2 && o[0]>=1) //kontrola dostatku atomu
    {
        no = poc[0]; //ulozeni cisla molekulu a snizeni poctu vodiku
        hyd[0]--;
        sem_post(hsem);
        
        sem_post(oxyd);//uvolni semafor pro pokracovani procesu kysliku po tom co si vodik vsechny stavy ulozil nebo zpracoval

       
        
    }
    else{ 
        sem_post(hsem);

        sem_wait(sem_poc);
        fprintf(s,"%d: H %d: not enough O or H\n", ++pocitadlo[0],id); //ukonceni procesu
        
        sem_post(sem_poc);
        sem_post(hydro);  //uvolni sememafor pro pripad ze v rade jsou jeste nejake vodiky

        exit(0);
    }
    
    sem_wait(sem_poc);
    fprintf(s,"%d: H %d: creating molecule %d\n", ++pocitadlo[0], id, no);
     
    sem_post(sem_poc);

    

    cekacka();  //toto tam ma byt, princip fungovani jsem opsal z internetu a discordu
    sem_wait(sem_hyd);
    sem_wait(sem_poc);
    fprintf(s,"%d: H %d: molecule %d created\n", ++pocitadlo[0], id, no);  // tiskneme ze delame molekulu
    
    sem_post(sem_poc);
    exit(1);
}


//#########  FUNKCE PRO PROCESY KYSLIK  #########//
void oxygen(int id, FILE *s, int NO, int NH, int TI, int TB)
{
    
     
    int no;
    
    
    
    sem_wait(sem_poc);
    fprintf(s,"%d: O %d: started\n", ++pocitadlo[0], id); //Vypis hlasky started
    sem_post(sem_poc);

    sem_wait(hsem);
    ox[0]++;     //Pocitadlo molekul O se incrementuje
    if(ox[0]==NO && hyd[0]==NH) sem_post(mutex); // ve chvili kdy uz jsou vechny atomy alespon za fazi started se spusti otevre semafor mutex
    sem_post(hsem);

    usleep(1000*(rand()%(TI +1))); //Cekani na nainicializovani atomu O

    sem_wait(sem_poc);
    fprintf(s,"%d: O %d: going to queue\n", ++pocitadlo[0], id); ///Nainicializuje se atom kysliku
    sem_post(sem_poc);
    
    

    

    sem_wait(mutex); //ceka se dokus se neotevre semafor mutex a pak spusti vytvareni molekuly

    sem_wait(hsem);
    Opoc[0]++;      //pocitadlo zpracovanych atomu kysliku
    sem_post(hsem);
    
    sem_wait(hsem);
    o[0] = ox[0];
    h[0] = hyd[0];      //ulozeni aktualniho stavu pocitadel atomu O a H
    sem_post(hsem);
    
    
    sem_wait(hsem);
    if(h[0] >= 2) //kontrola dostatku vodiku
    {
        poc[0]++;  //zvedne se pocitadlo molekul h2o
       
        no = poc[0]; //ulozeni aktualniho cisla molekuly
        sem_post(hsem);

        
           
        
        sem_wait(hsem);
        ox[0]--;
                            //snizeni cisla pro pocitani volnych atomu kysliku
        sem_post(hsem);
           
    }
    else //kdyz neni dostatek vodiku
    {
        
        sem_post(hsem);
        sem_wait(hsem);
        ox[0]--;        //snizi pocitadlo kysliku o 1
        sem_post(hsem); 

        sem_post(hydro); //pro pripad ze je v rade jeste nejaky vodik tak mu otevre semafor
        
        sem_wait(sem_poc);
        fprintf(s,"%d: O %d: not enough H\n", ++pocitadlo[0], id );  //kdyz uz neni dostatek vodiku
        
        
        sem_post(sem_poc);

        sem_post(mutex); //uvolneni semaforu pro nasledujici atom kysliku

        exit(0);
    }
   
    
    sem_wait(sem_poc);
    fprintf(s,"%d: O %d: creating molecule %d\n", ++pocitadlo[0], id, no);  // tiskneme ze delame molekulu
    sem_post(hydro);
    sem_post(hydro); //uvolni semafor vodiky na vypsani hlasky creating
     
    sem_post(sem_poc);
    
    cekacka(); //zaridi aby procesy jedne molekyly skoncili ve stejnou dobu

    sem_wait(hsem);

        if(Opoc[0] == NO) //kdy se jsou spotrebovany vsechny kysliky tak se ulozi aktualni stav 
        {                   
            o[0] = ox[0];
            h[0] = hyd[0];
            sem_post(hydro); // a uvolni se semafor pro vodik pro pripad ze ve fronte jsou jeste nejake volne vodiky
        }
    sem_post(hsem);
    
    usleep(1000*(rand()%(TB +1)));
    
    sem_wait(sem_poc);
    fprintf(s,"%d: O %d: molecule %d created\n", ++pocitadlo[0], id, no);  // tiskneme ze delame molekulu
     
    sem_post(sem_poc);
    sem_post(sem_hyd);  //uvolni semafor pro 2 vodiky na vypsani hlasky created
    sem_post(sem_hyd);
    sem_post(mutex);
 
    exit(1);
}

int arg(long *NO, long *NH, long *TI,long *TB, int agrc, char **args){
    char *zb;
    
    *NO = *TI = *TB = *NH;
    //Kontrola vsech moznych chybovych stavu na vstupech
    if(agrc!=5){
        fprintf(stderr, "Spatny pocet vstupu\n");
        return -1;
    }

    *NO = strtol(args[1], &zb, 10);
    if(strlen(zb)!=0) 
    {
        fprintf(stderr, "Spatny vstup\n");
        return -1;
    }

    if(!(*NO > 0))
    {
        fprintf(stderr, "Spatny vstup\n");
        return -1;
    }

    *NH = strtol(args[2], &zb, 10);
    if(strlen(zb)!=0) 
    {
        fprintf(stderr, "Spatny vstup\n");
        return -1;
    }

    if(!(*NH > 0))
    {
        fprintf(stderr, "Spatny vstup\n");
        return -1;
    }

    *TI = strtol(args[3], &zb, 10);
    if(strlen(zb)!=0) 
    {
        fprintf(stderr, "Spatny vstup\n");
        return -1;
    }

    if(!(0<=*TI && *TI <=1000))
    {
        fprintf(stderr, "Spatna veliost TI\n");
        return -1;
    }

    *TB = strtol(args[4], &zb, 10);
    if(strlen(zb)!=0) 
    {
        fprintf(stderr, "Spatny vstup\n");
        return -1;
    }

    if(!(0<=*TB && *TB <=1000))
    {
        fprintf(stderr, "Spatna veliost TB\n");
        return -1;
    }
    return 1;
}


int main (int argc, char *argv[]){

    FILE *s = fopen("proj2.out", "w"); //otevreni souboru

    setbuf(stderr ,NULL );
    setbuf(s, NULL);

    long NO, NH, TI, TB; //promene na ulozeni parametru a fce na jejich ziskani
    if((arg(&NO, &NH, &TI, &TB, argc, argv)) == -1){
        return 1;
    }

    alloc_sem();
    
    alloc_int(pocitadlo);
    alloc_int(poc);
    alloc_int(ox);      
    alloc_int(hyd);
    alloc_int(o);
    alloc_int(h);
    alloc_int(cek);
    alloc_int(Opoc);

    
    pocitadlo[0] = 0;
    ox[0] = 0;          ///da se pridat do fce alloc_int
    hyd[0] = 0;
    poc[0] = 0;
    cek[0] = 0;
    Opoc[0] = 0;

    

    
    

    for(int id = 0; id < NH; id++)
    {

        pid_t pid = fork();
        if(pid == 0) 
        {
            
            hydrogen(id,s, TI, NO, NH); //spusteni procesu vodik
            
            
        }
    }
    
    for(int id = 0; id < NO; id++)
    {
        pid_t pid = fork();
        if(pid == 0) 
        {
            
            oxygen(id,s, NO, NH, TI, TB); //spusteni procesu kyslik
            
           
        }
    }
    
    
    
    
    while(wait(NULL)>0);
    dealoc_vse();
    return 0;
}