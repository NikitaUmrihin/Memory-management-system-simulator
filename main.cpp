#include "sim_mem.cpp"
using namespace std;

extern char main_memory[MEMORY_SIZE];

int main()
{
    sim_mem mem_sm((char*)"exec_file", (char*)"ex2" , (char*)"swap_file" ,25, 50, 25,25, 25, 5, 2);
    
    printf("text: %d\t", mem_sm.text_size);
    printf("data: %d\t", mem_sm.data_size);
    printf("bss: %d \t", mem_sm.bss_size);
    printf("heap+stack: %d\n", mem_sm.heap_stack_size);
    printf("pages: %d\tsize:%d\n", mem_sm.num_of_pages, mem_sm.page_size);
    
    
    // load procces 1 txt to main memory
    
    mem_sm.load(1, 0);    mem_sm.load(1, 5);
    mem_sm.load(1, 10);    mem_sm.load(1, 15);
    mem_sm.load(1, 20);
    
    
    
    // store procces 2 data to main memory
    
    mem_sm.store(2, 50, '(');   mem_sm.store(2, 54, ')');
    mem_sm.store(2, 25, '~');   mem_sm.store(2, 29, '~');
    mem_sm.store(2, 30, '_');   mem_sm.store(2, 34, '_');
    mem_sm.store(2, 35, '|');   mem_sm.store(2, 39, '|');
    
    mem_sm.store(2, 40, ';');   mem_sm.store(2, 44, ';');
    mem_sm.store(2, 45, '=');   mem_sm.store(2, 49, '=');
    
    
    
    // store procces 1 heap to main memory (mallocs)
    
    
    mem_sm.store(1, 100, '/');   mem_sm.store(1, 104, '/');
    mem_sm.store(1, 105, 'p');   mem_sm.store(1, 109, 'q');
    mem_sm.store(1, 110, 'B');   mem_sm.store(1, 114, 'B');
    mem_sm.store(1, 115, '5');   mem_sm.store(1, 119, '5');
    mem_sm.store(1, 120, '~');   mem_sm.store(1, 124, '~');
    
    // load 1 page from p1 txt to memory
    
    mem_sm.load(1,0);
    
    // // try to load from heap
    mem_sm.load(2, 100);        // ERROR
    

    // // load from data p2
    mem_sm.load(2, 50);
    
    
    // // // load from p1 heap
    mem_sm.load(1,75);    
    
    // /////p1 try store to txt (which is in memory)
    mem_sm.store(2,0,'T');          //      should we be allowed to store ?

    // // p2 store (bring from swap)
    mem_sm.store(2, 27, 'v');

    
    // //////p1 store in data
    mem_sm.store(1, 25, 'u');   mem_sm.store(1, 29, 'u');
    mem_sm.store(1, 30, '1');   mem_sm.store(1, 34, '1');
    mem_sm.store(1, 35, '+');   mem_sm.store(1, 39, '+');
    mem_sm.store(1, 40, '2');   mem_sm.store(1, 44, '2');
    
    // // p1 store to BSS
    mem_sm.store(2, 75, 'V');   mem_sm.store(2, 79, 'V');
    mem_sm.store(2, 80, '#');   mem_sm.store(2, 84, '#');
    mem_sm.store(2, 85, '~');   mem_sm.store(2, 89, '~');
    mem_sm.store(2, 90, ',');   mem_sm.store(2, 94, ',');
    mem_sm.store(2, 95, 'S');   mem_sm.store(2, 99, 'S');
    
    // p2 store in heap (malloc)
    mem_sm.store(2, 100, '{');   mem_sm.store(2, 104, '}');
    
    ///load p1 txt
    mem_sm.load(1, 0);
    mem_sm.load(1, 5);
    mem_sm.load(1, 10);
    mem_sm.load(1, 15);
    mem_sm.load(1, 20);
    
    
    
 
    
    
    // try to fill up swap file !!!!!!!
    
    mem_sm.store(1, 80, 'Q');   mem_sm.store(1, 84, 'Q');
    mem_sm.store(1, 85, 'W');   mem_sm.store(1, 89, 'W');
    mem_sm.store(1, 90, 'j');   mem_sm.store(1, 94, 'j');
    mem_sm.store(1, 95, '{');   mem_sm.store(1, 99, '}');
    mem_sm.store(1, 101, '\'');   mem_sm.store(1, 103, '\'');
    
    
    mem_sm.store(2, 105, '.');   mem_sm.store(2, 109, '.');
    mem_sm.store(2, 110, '3');   mem_sm.store(2, 114, '3');
    mem_sm.store(2, 115, '*');   mem_sm.store(2, 119, '*');
    mem_sm.store(2, 120, 'c');   mem_sm.store(2, 124, 'c');
    
    
    mem_sm.store(2, 55, 'f');   mem_sm.store(2, 59, 'f');
    mem_sm.store(2, 60, 'h');   mem_sm.store(2, 64, 'h');
    mem_sm.store(2, 65, 'i');   mem_sm.store(2, 69, 'i');
    mem_sm.store(2, 70, 'h');   mem_sm.store(2, 74, 'h');
    
    
    mem_sm.store(1, 45, '_');   mem_sm.store(1, 49, '_');
    mem_sm.store(1, 50, '`');   mem_sm.store(1, 54, '`');
    mem_sm.store(1, 55, '!');   mem_sm.store(1, 59, '!');
    
    mem_sm.store(1, 60, 'A');   mem_sm.store(1, 64, 'A');
    mem_sm.store(1, 65, 'X');   mem_sm.store(1, 69, 'X');
    mem_sm.store(1, 70, 'Y');   mem_sm.store(1, 74, 'Y');
    mem_sm.store(1, 75, 'X');   mem_sm.store(1, 79, 'X');

    mem_sm.load(1, 0);    mem_sm.load(1, 5);
    mem_sm.load(1, 10);    mem_sm.load(1, 15);
    mem_sm.load(1, 20);
        
    mem_sm.load(2, 0);  
    mem_sm.load(2, 0);
    mem_sm.load(2, 10);
    
    
    
    
    
    
    
    
    
      // SWAP FILE SHOULD BE FULL BY NOW !!
    

    // bring pages from SWAP ( IF SWAP FULL)
    mem_sm.store(1, 76, 'o');   mem_sm.store(1, 77, 'o');   mem_sm.store(1, 78, 'o');
    mem_sm.store(1, 106, 'I');   mem_sm.store(1, 107, 'I');   mem_sm.store(1, 108, 'I');
    mem_sm.store(1, 111, ':');   mem_sm.store(1, 112, ':');   mem_sm.store(1, 113, ':');
    
    
    
    
    mem_sm.print_page_table();
    
    
    
    mem_sm.print_swap();
    mem_sm.print_memory();
    
    return 0;
}
