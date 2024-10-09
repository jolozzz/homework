#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
//#pragma warning(disable : 4996)

int s = 0, e = 0, b = 0;
char sourcefile[100];
int verbose_flag = 0;
int misses = 0, hits = 0, evictions = 0;

typedef struct block {
    int tag;

    //used为1表示已使用，为0表示未使用
    int used;

    struct block* next;
    struct block* prev;
}block;
typedef struct set {
    //队首为最近访问，发生eviction时队尾出队
    //squeue_head指向一个空block，squeue_end指向一个实际的block
    block* squeue_head;
    block* squeue_end;
}set;
typedef struct cache {
    set* s;
}cache;
cache mycache;

block* initBlock(block* prev) {
    block* temp = (block*)malloc(sizeof(block));
    temp->used = 0;
    temp->next = NULL;
    temp->prev = prev;
    return temp;
}

void init() {
    mycache.s = (set*)malloc(sizeof(set) * (int)pow(2,s));
    for (int i = 0; i < (int)pow(2, s); i++) {
        block* temp = initBlock(NULL);
        mycache.s[i].squeue_head = temp;
        for (int j = 0; j < e; j++) {
            temp->next = initBlock(temp);
            temp = temp->next;
        }
        mycache.s[i].squeue_end = temp;
    }
}

void push(block* temp, block* head) {
    block* next = temp->next;
    block* prev = temp->prev;
    if (temp->next != NULL) {
        temp->next->prev = prev;
    }
    if (temp->prev != NULL) {
        temp->prev->next = next;
    }
    block* tt = head->next;
    head->next = temp;
    temp->prev = head;
    temp->next = tt;
    if (tt != NULL) {
        tt->prev = temp;
    }
}

void caching(int myset, int mytag) {
    //从首遍历used block判断是否hit
        //若hit,将该block从队中取出，并移动至队首（hit）
        //若不hit
            //若cache未占满，使用未使用的block，并将该block移动至队首（miss）
            //若cache已满，则使用队首的block,并将该block移动至队首 (miss eviction)
    block* temp = mycache.s[myset].squeue_head->next;
    if (temp == NULL) {
        printf("error in caching\n");
        return;
    }

    while (temp && temp->used) {
        if (temp->tag == mytag) {
            //hit
            if (verbose_flag)printf("hit ");
            hits++;
            if (temp == mycache.s[myset].squeue_end && e != 1) {
                mycache.s[myset].squeue_end = mycache.s[myset].squeue_end->prev;
            }
            push(temp, mycache.s[myset].squeue_head);
            return;
        }
        temp = temp->next;
    }
    if (temp) {
        //未满
        if (verbose_flag)printf("miss ");
        misses++;
        temp->used = 1;
        temp->tag = mytag;
        if (temp == mycache.s[myset].squeue_end && e != 1) {
            mycache.s[myset].squeue_end = mycache.s[myset].squeue_end->prev;
        }
        push(temp, mycache.s[myset].squeue_head);
    }
    else {
        //已满
        if (verbose_flag)printf("miss eviction ");
        misses++, evictions++;
        block* tt;
        if (e != 1) tt = mycache.s[myset].squeue_end->prev;
        else tt = mycache.s[myset].squeue_end;
        mycache.s[myset].squeue_end->tag = mytag;
        push(mycache.s[myset].squeue_end, mycache.s[myset].squeue_head);
        mycache.s[myset].squeue_end = tt;
    }
}

void finalize() {
    for (int i = 0; i < s; i++) {
        block* temp = mycache.s[i].squeue_head;
        while (temp != NULL) {
            block* tt = temp->next;
            free(temp);
            temp = tt;
        }
    }
    free(mycache.s);
}

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            char op = argv[i][1];
            int j = 0;
            switch (op) {
            case 'v':
                verbose_flag = 1;
                break;
            case 's':
                i++;
                j = 0;
                while (argv[i][j] != '\0') {
                    s = s * 10 + argv[i][j] - '0';
                    j++;
                }
                break;
            case 'E':
                i++;
                j = 0;
                while (argv[i][j] != '\0') {
                    e = e * 10 + argv[i][j] - '0';
                    j++;
                }
                break;
            case 'b':
                i++;
                j = 0;
                while (argv[i][j] != '\0') {
                    b = b * 10 + argv[i][j] - '0';
                    j++;
                }
                break;
            case 't':
                i++;
                strcpy(sourcefile, argv[i]);
                break;
            default:
                printf("wrong flag\n");
                return 0;
            }
        }
    }

    init();

    FILE* file;
    file = fopen(sourcefile, "r");
    if (!file) {
        printf("file error\n");
        return 0;
    }
    char op;
    unsigned long long int address;
    int size;
    while (fscanf(file, " %c %llx,%d", &op, &address, &size) > 0)
    {
        int mytag = address >> (s + b);
        int myset = (address >> b) & ((unsigned)(-1) >> (8 * sizeof(unsigned) - s));
        switch (op)
        {
            //store采用写回的方式，所以从单纯的cache访问角度来看与load一致
            //I直接跳过，M修改直接看成一次存取
        case 'M':
            //printf("                        myset=%d mytag=%d\n",myset,mytag);
            if (verbose_flag)printf("%c %llx,%d ", op, address, size);
            caching(myset, mytag);
            caching(myset, mytag);
            if (verbose_flag)printf("\n");
            break;
        case 'L':
            //printf("                        myset=%d mytag=%d\n",myset,mytag);
            if (verbose_flag)printf("%c %llx,%d ", op, address, size);
            caching(myset, mytag);
            if (verbose_flag)printf("\n");
            break;
        case 'S':
            //printf("                        myset=%d mytag=%d\n",myset,mytag);
            if (verbose_flag)printf("%c %llx,%d ", op, address, size);
            caching(myset, mytag);
            if (verbose_flag)printf("\n");
            break;
        case 'I':
            break;
        }
    }

    finalize();

    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);

    return 0;
}
