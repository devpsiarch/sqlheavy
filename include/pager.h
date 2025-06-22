#ifndef PAGER_H
#define PAGER_H

#include <stdio.h>
#include <fcntl.h>      // For O_* constants
#include <sys/stat.h>   // For mode constants (S_*)
#include <unistd.h>     // For close()

#define PAGE_SIZE 4096
#define TABLE_MAX_PAGES 100

typedef struct {
    int file_descriptor;         // fd for file we read from
    unsigned int file_length;    // duh 
    void *pages[TABLE_MAX_PAGES];// this acts as a cache.
}Pager;

Pager* init_pager(const char*filename);
void kill_pager(Pager*p);
void* get_page(Pager*p,unsigned int page_num);
// writes the data from the cache to the file
void pager_flush(Pager*p,unsigned int page_num,unsigned int size);

#endif
#ifndef PAGER_IMPLI 
#define PAGER_IMPLI

Pager* init_pager(const char*filename){
    int fd = open(filename,
            O_RDWR |      // Read/Write mode
            O_CREAT,  // Create file if it does not exist
            S_IWUSR |     // User write permission
            S_IRUSR   // User read permission
            );
    if(fd == -1){
        printf("Could not open file while creating pager.\n"); 
        goto defer_fail;
    }
    off_t file_length = lseek(fd,0,SEEK_END);
    Pager* pager = malloc(sizeof(Pager));
    pager->file_descriptor = fd;
    pager->file_length = (unsigned int)file_length;
    for(size_t i = 0 ; i < TABLE_MAX_PAGES ; i++){
        pager->pages[i] = NULL;
    }
    return pager;
defer_fail:
    exit(EXIT_FAILURE); 
}
void kill_pager(Pager*p){
    if(close(p->file_descriptor) == -1){
        printf("Error closing file.\n");
    }
    for(size_t i = 0 ; i < TABLE_MAX_PAGES; i++){
        free(p->pages[i]);
    }
    free(p);
    p = NULL;
}


void* get_page(Pager*p,unsigned int page_num){
    ASSERT(page_num < TABLE_MAX_PAGES,"EXCEEDED MAX PAGE COUNT");
    if(p->pages[page_num] == NULL){
        printf("It is null help\n");
        // basicly cache miss. therefore load from file.
        void *page = malloc(PAGE_SIZE);
        unsigned int num_pages = p->file_length/PAGE_SIZE;
        if(p->file_length % PAGE_SIZE != 0){
            // partial page goes brrr.
            num_pages += 1;
        }
        if(page_num <= num_pages){
            lseek(p->file_descriptor,page_num*PAGE_SIZE,SEEK_SET);
            ssize_t bytes_read = read(p->file_descriptor,page,PAGE_SIZE);
            if(bytes_read == -1){
                printf("Error reading the file with fd:%d\n",p->file_descriptor);
                goto defer_fail;
            }
        }
        p->pages[page_num] = page;
    }
    return p->pages[page_num];
defer_fail:
    kill_pager(p);
    exit(EXIT_FAILURE);
}


void pager_flush(Pager*p,unsigned int page_num,unsigned int size){
    if(p->pages[page_num] == NULL){
        printf("Tried to flush NULL page... exiting\n");
        exit(EXIT_FAILURE);
    }
    // we consider the lengh of the file is the number of rows
    // this is for a specific table.
    // well worry about the other later
    off_t offset = lseek(p->file_descriptor,page_num*PAGE_SIZE,SEEK_SET);
    if(offset == -1){
        printf("Error while seeking while flushing pager.\n");
        exit(EXIT_FAILURE);
    }
    // write to file
    ssize_t bytes_written = write(p->file_descriptor,p->pages[page_num],size);
    if(bytes_written == -1){
        printf("Error while writting to file.\n");
        exit(EXIT_FAILURE);
    }
}

#endif
