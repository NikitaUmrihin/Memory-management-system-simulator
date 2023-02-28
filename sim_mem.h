#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#ifndef SIM_MEM
#define SIM_MEM

#define MEMORY_SIZE 200

extern char main_memory[MEMORY_SIZE];
class sim_mem 
{
    public:
    typedef struct page_descriptor
    {
        int V; // valid
        int D; // dirty
        int P; // permission
        int frame; //the number of a frame if in case it is page-mapped
        int swap_index; // where the page is located in the swap file.
    } page_descriptor;
    
    

    int swapfile_fd; //swap file fd
    int program_fd[2]; //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    int num_of_proc;
    page_descriptor **page_table; //pointer to page table

    sim_mem(char exe_file_name1[],char exe_file_name2[], char swap_file_name[], 
            int text_size, int data_size, int bss_size, int heap_stack_size,
            int num_of_pages, int page_size, int num_of_process);
    
    
    ~sim_mem();
    char load(int process_id, int address);
    void store(int process_id, int address, char value);
    
    
    char whatever(int process_id, int address, char value, char* status);
    
    
    void swap_out(int process_id, int swapfile_fd, int address);
    void swap_in(int process_id, int swapfile_fd, int address);
    
    void print_memory();
    void print_swap ();
    void print_page_table();
    
    
    
};
#endif
