#include <stdio.h>
#include <stdlib.h>
#include "../ccl/containers.h"

static void ABORT(char *file,int line)
{
        fprintf(stderr,"*****\n\nABORT\nFile %s Line %d\n**********\n\n",file,line);
        abort();
}
#define Abort() ABORT(__FILE__,__LINE__)

// from test.c
static void PrintstrCollection(strCollection *SC){
    size_t i;
    printf("Count %d, Capacity %d\n",(int)istrCollection.Size(SC),(int)istrCollection.GetCapacity(SC));
    for (i=0; i<istrCollection.Size(SC);i++) {
        printf("%s\n",istrCollection.GetElement(SC,i));
    }
    printf("\n");
}
static void teststrCollection(void)
{
    strCollection *SC = istrCollection.Create(10);
    const char *p; char buf[40];
        size_t idx;
    istrCollection.Add(SC,"Martin");
    istrCollection.Insert(SC,(char *)"Jakob");
    if (!istrCollection.Contains(SC,(char *)"Martin")) {
        Abort();
    }
    if (2 != istrCollection.Size(SC))
                Abort();
    istrCollection.InsertAt(SC,1,(char *)"Position 1");
    istrCollection.InsertAt(SC,2,(char *)"Position 2");
        if (0 == istrCollection.Contains(SC,(char *)"Position 1"))
                Abort();
        istrCollection.IndexOf(SC,(char *)"Position 2",&idx);
        if (idx != 2)
                Abort();
        if (4 != istrCollection.Size(SC))
                Abort();
    istrCollection.Erase(SC,(char *)"Jakob");
    if (istrCollection.Contains(SC,(char *)"Jakob"))
                Abort();
        if (3 != istrCollection.Size(SC))
                Abort();
    istrCollection.PushFront(SC,(char *)"pushed");
        if (4 != istrCollection.Size(SC))
                Abort();
        istrCollection.IndexOf(SC,(char *)"pushed",&idx);
        if (0 != idx)
                Abort();
    istrCollection.PopFront(SC,(char *)buf,sizeof(buf));
        if (3 != istrCollection.Size(SC))
                Abort();
        if (strcmp((char *)buf,"pushed"))
                Abort();
    PrintstrCollection(SC);
    p = istrCollection.GetElement(SC,1);
    printf("Item position 1:%s\n",p);
    PrintstrCollection(SC);
        istrCollection.Finalize(SC);
#if 0
        /* Here you should add a file path (text file)
           that can be used to test the string collection */
        SC = istrCollection.CreateFromFile((unsigned char *)"../../test.c");
        PrintstrCollection(SC);
        istrCollection.Finalize(SC);
#endif
}

///

static void PrintVector(Vector *AL)
{
        size_t i;
        printf("Count %ld, Capacity %ld\n",(long)iVector.Size(AL),(long)iVector.GetCapacity(AL));
        for (i=0; i<iVector.Size(AL);i++) {
                printf("%s\n",*(char **)iVector.GetElement(AL,i));
        }
        printf("\n");
}

static int compareStrings(const void *s1,const void *s2,CompareInfo *ExtraArgs)
{
        char **str1=(char **)s1,**str2=(char **)s2;
        return strcmp(*str1,*str2);
}

char *Table[] = {
        "Martin",
        "Jakob",
        "Position 1",
        "Position 2",
        "pushed",
};
static int testVector(void)
{
        int errors=0;
        Vector *AL = iVector.Create(sizeof(void *),10);
        char **p;
        iVector.SetCompareFunction(AL,compareStrings);
        iVector.Add(AL,&Table[0]);
        iVector.Insert(AL,&Table[1]);
        if (!iVector.Contains(AL,&Table[0],NULL)) {
                Abort();
        }
        if (2 != iVector.Size(AL))
                Abort();
        iVector.InsertAt(AL,1,&Table[2]);
        iVector.InsertAt(AL,2,&Table[3]);
        if (4 != iVector.Size(AL))
                Abort();
        iVector.Erase(AL,&Table[1]);
        /*PrintVector(AL);*/
        iVector.PushBack(AL,&Table[4]);
        /*PrintVector(AL);*/
        iVector.PopBack(AL,NULL);
        /*PrintVector(AL);*/
        p = iVector.GetElement(AL,1);
        printf("Item position 1:%s\n",*p);
        PrintVector(AL);
        iVector.Finalize(AL);
        return errors;
}

#if 1
static void testVectorPerformance(void)
{
#define MAX_IT 50000000

        Vector *l = iVector.Create(sizeof(int),500000);
        size_t i;
        long long sum=0;
        for (i=0; i<MAX_IT; i++) {
                iVector.Add(l,&i);
        }
        for (i=0; i<iVector.Size(l); i++) {
                sum += *(int *)iVector.GetElement(l,i);
        }
        printf("sum is: %lld\n",sum);
/*getchar();*/
}
#endif


///
static int TestDictionary(void)
{
        Dictionary *d = iDictionary.Create(sizeof(int *),30);
        int data[12];
        size_t count;
        int *pi,sum,r;
        Iterator *it;

        data[1] = 1;
        data[2] = 2;
        iDictionary.Add(d,"One",&data[1]);
        iDictionary.Add(d,"Two",&data[2]);
        pi = (int *)iDictionary.GetElement(d,"Two");
        if (*pi != 2)
                Abort();
        pi = (int *)iDictionary.GetElement(d,"One");
        if (*pi != 1)
                Abort();
        count = iDictionary.Size(d);
        if (count != 2)
                Abort();
        it = iDictionary.NewIterator(d);
        sum = 0;
        for (pi = it->GetFirst(it);
                pi != NULL; pi = it->GetNext(it)) {
                sum += *pi;
        }
        iDictionary.DeleteIterator(it);
        if (sum != 3)
                Abort();
        r=iDictionary.Erase(d,"long data");
        if (r != CONTAINER_ERROR_NOTFOUND)
                Abort();
        count = iDictionary.Size(d);
        if (count != 2)
                Abort();
        iDictionary.Erase(d,"One");
        count = iDictionary.Size(d);
        if (count != 1)
                Abort();
        iDictionary.Finalize(d);
        return 0;
}

int main()
{
    printf("start...\n");

    teststrCollection();
    testVector();
    //testList();
    TestDictionary();

    testVectorPerformance();

    return 0;
}
