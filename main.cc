
#include <crypt.h>

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <sys/time.h>
// #include <mutex>          // std::mutex

using namespace std;

void checkPass(int len, char s[]);

// #define MAX_THREADS 4
int maxLen;
//const char letters[94]=
//{
//    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
//    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
//    '0','1','2','3','4','5','6','7','8','9',
//    '~','`','!','@','#','$','%','^','&','*','(',')','-','_','+','=','[','{',']','}','\\','|',';',':','\'','\"',',','<','>','.','?','/'
//};

//const char letters[62] =
//{
//    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
//    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
//    '0','1','2','3','4','5','6','7','8','9'
//};

 const char letters[26]=
 {
  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'
 };


//HOW TO CREATE HASHED PASSWORD
//openssl passwd -1 -salt xyz  yourpass
// perl -e "print crypt('password','sa');"

char salt[100] = "";
char hash[100] = "";

struct thread_data{
    int  thread_id;
    int length;
    int start;
    int stop;
};


int generateCombinations(int size, const char map[100],int start, int stop)
{
    int carry = 0;
    int indicies[size];
    fill_n(indicies, size, start);
    char guess[size+1];
    guess[size]='\0';
    int testcnt = 0;
    while(carry != 1)
    {
        bool same=true;
        for(int i = 0; i < size;++i)
        {
            guess[i] = map[indicies[i]];
            if(indicies[i] != stop)
                same = false;

        }
        ++testcnt;
        char *pwd=  crypt(guess,salt);
        int test = strcmp(hash,pwd);
        //cout << s << endl;
        if( test == 0)
        {
            //  cout << "hash=" << hash << "\nours=" << pwd << endl;
            test = strcmp(hash,pwd);
            if(test == 0)
            {
                cout << "found string: " << guess << endl;
                exit(0);
            }
        }
        if(same){
            //cerr << "cnt = "<<testcnt <<endl; 
            return testcnt;
        }
        carry = 1;
        for(int i = size -1; i >= 0; --i)
        {
            if(carry == 0)
                break;
            indicies[i]+=carry;
            carry = 0;

            if(indicies[i] == strlen(map))
            {
                carry = 1;
                indicies[i]=0;
            }
        }
    }
    return -1;
}
void *MyThread(void *threadarg)
{
    char s[100] = "";
    struct thread_data *data;
    data = (struct thread_data *) threadarg;
    // cout << "Thread(" << data->thread_id << ") Searching length " << data->length << " start=" << data->start << " stop=" << data->stop << endl;
    struct timeval t0, t1, dt;

    gettimeofday(&t0,NULL);
    int total_comb = generateCombinations(data->length,letters,data->start,data->stop);
    gettimeofday(&t1,NULL);
    timersub(&t1,&t0,&dt);
    //cerr << "Thread[" << data->thread_id << "] checked " << total_comb << " took " << dt.tv_sec << "." << dt.tv_usec << " sec " << endl;
}


int checkInteger(char value[], const char *message)
{
    char *p;
    long conv = strtol(value,&p,10);

    if(errno != 0 || *p != '\0' || conv > INT_MAX)
    {
        cerr << message << endl;
        exit(1);
    }
    return conv;
}

int main(int argc, char *argv[])
{

    if(argc != 5)
    {
        cout << "./a.out <salt> <hash> <length> <THREADS>" << endl;
        return 1;
    }

    maxLen=checkInteger(argv[3],"Length of string was not an integer");
    int MAX_THREADS = checkInteger(argv[4],"Number of threads was not an integer");
    if(MAX_THREADS <= 0)
        MAX_THREADS = 1;

    strcat(salt,"$1$");
    strcat(salt,argv[1]);

    strcat(hash,salt);
    strcat(hash,"$");
    strcat(hash,argv[2]);

    char s[100] = "";
    int rc;
    cout << "Searching for password..." << endl;
    int midpoint = strlen(letters) / MAX_THREADS;

    //  for(int i =1; i <= maxLen;++i)
    //  {
    //      generateCombinations(i,letters);
    //     // checkPass(i,s);
    //  }

    struct timeval t0, t1, dt;


    pthread_t threads[MAX_THREADS];
    struct thread_data td[MAX_THREADS];
    for (int length = 1; length <= maxLen; ++length){

        gettimeofday(&t0,NULL);
        for(int i = 0; i < MAX_THREADS; ++i)
        {
            td[i].thread_id=i;
            td[i].length=length;
            td[i].start=i * midpoint;

            if(i+1 >= maxLen)
                td[i].stop=strlen(letters);
            else
                td[i].stop=(i+1) * midpoint ;


            // td[i-1].str=s;
            rc=pthread_create(&threads[i],NULL,MyThread,(void *)&td[i]);
            if (rc){
                cout << "Error: unable to create thread, " << rc << endl;
                exit(-1);
            }
            // checkPass(i,s);
        }

        for (int i = 0; i < MAX_THREADS; ++i)
        {
            // status *st;

            pthread_join(threads[i], NULL);
        }
        gettimeofday(&t1,NULL);
        timersub(&t1,&t0,&dt);
        int days = 0,
            hours = 0,
            minutes = 0;

        if(dt.tv_sec > 86400){
            days = dt.tv_sec / 86400;
        }
        if(dt.tv_sec > 3600){
            hours = dt.tv_sec / 3600;
            dt.tv_sec = dt.tv_sec % 86400;
        }
        if(dt.tv_sec > 60){
            minutes = dt.tv_sec / 60;
            dt.tv_sec = dt.tv_sec % 60;
        }
//<< dt.tv_sec << "." << dt.tv_usec << " sec "
        cerr << "FINISHED CHECKING STRING LENGTH " << length  << " in " << days << "." << hours << "." << minutes << "." << dt.tv_sec  << "." << dt.tv_usec << endl;

    }

    //  pthread_exit(NULL);
    cout << endl << "COULDNT FIND ANYTHING :(" << endl;
    return 0;
}

//old recursive solution

//void checkPass(int len, char s[100])
//{
//    if(len ==0 )
//    {
//        char *pwd=  crypt(s,salt);
//
//        //      cout << s << endl;
//        if(strcmp(hash,pwd) == 0)
//        {
//            cout << "found string: " << s << endl;
//            exit(0);
//        }
//        return;
//    }
//
//    for(int i=0;i < strlen(letters);++i)
//    {
//        s[100 + (len - 100) - 1] = letters[i];
//        checkPass(len-1,s);
//    }
//
//    //  for(int i = 0; i < 26; ++i)
//    //  {
//    //      s[100 + (len - 100) - 1] = 'a' +i;
//    //      checkPass(len-1,s);
//    //  }
//
//}
