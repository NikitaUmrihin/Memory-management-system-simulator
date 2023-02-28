#include "sim_mem.h"
char main_memory[MEMORY_SIZE];
int free_space[MEMORY_SIZE];
int fifo = 0;



// constructor
sim_mem::sim_mem(char exe_file_name1[], char exe_file_name2[], char swap_file_name2[], int text_size,
                        int data_size, int bss_size, int heap_stack_size,
                        int num_of_pages, int page_size, int num_of_process)
{
    if(num_of_process > 2 || num_of_process < 1)
    {
        perror("ERROR ! only 1 or 2 processes!");
        exit(1);
    }
    
    // initiatlize variables
    this->num_of_proc = num_of_process;
    this->num_of_pages = num_of_pages;
    this->page_size = page_size;
    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    
    int text_pages = text_size/page_size;
    int data_pages = data_size/page_size;
    int bss_pages = bss_size/page_size;
    int heap_stack_pages = heap_stack_size/page_size;
    
    
    // initialize main memory
    for(int i=0; i<MEMORY_SIZE; i++)
    {
        main_memory[i]='0';
        free_space[i]=1;
    }
    
    // open and initialize swap file
    swapfile_fd = open(swap_file_name2, O_RDWR|O_CREAT);
    if(swapfile_fd < 1)
    {
        perror("ERROR creating swap file...\n");
        exit(1);
    } 
    for(int i=0; i< num_of_process*page_size*(num_of_pages-text_pages); i++)
        write(swapfile_fd, "0", 1);
    
    
    // open exec files
    program_fd[0] = open(exe_file_name1, O_RDONLY);
    if(program_fd[0] < 1)
    {
        perror("ERROR reading exec file...\n");
        exit(1);
    }    

    if( num_of_process == 2 )
    {
        program_fd[1] = open(exe_file_name2, O_RDONLY);
        if( program_fd[1] < 0 )
        {
            perror("ERROR reading exec file...\n");
            exit(1);
        }
    }
    
    

    // initialize page table
    
    this->page_table = (page_descriptor**)malloc(num_of_process * sizeof(page_descriptor*));

    for(int pid=0; pid<num_of_process; pid++)
    {
        this->page_table[pid] = (page_descriptor*)malloc(num_of_pages* sizeof(page_descriptor));
        for (int i=0; i<num_of_pages; i++)
        {
            this->page_table[pid][i].V = 0;
            this->page_table[pid][i].D = 0;
            this->page_table[pid][i].frame = -1;
            this->page_table[pid][i].swap_index = -1;
            if(i<text_pages)
                this->page_table[pid][i].P = 0;
            else this->page_table[pid][i].P = 1;
        }
        
    }
}



// destructor
sim_mem::~sim_mem()
{
    for(int i=0; i<num_of_proc; i++)
    {
        free(page_table[i]);
        close(program_fd[i]);
    }
    
    close(swapfile_fd);
    free(page_table);
}




// store function
void sim_mem::store(int process_id, int address, char value)
{
    // initialize variables
    
    int text_pages = text_size/page_size;
    int data_pages = data_size/page_size;
    int bss_pages = bss_size/page_size;
    int heap_stack_pages = heap_stack_size/page_size;
    
    if(address>text_size+data_size+bss_size+heap_stack_size)
    {
        perror("ERROR ! no such address....\n");
        return;
    }
    
    if(process_id>2 || process_id<1)
    {
        perror("ERROR ! no such process !\n");
        return;
    }
    
    // get page and offset
    int page = address / page_size;
    int offset = address % page_size;
    

    // V=1 <-> page in main memory
    if(page_table[process_id-1][page].V == 1)
    {
        // go to physical address
        main_memory[page_table[process_id-1][page].frame*page_size+offset] = value;
        page_table[process_id-1][page].D = 1;
    }
    // V=0 <-> page not in main memory
    else
    {
        // P=1 <-> bss/data/heap/stack page
        if(page_table[process_id-1][page].P == 1)
        {
            
            // D=1 <-> dirty page, page is in swap
            if(page_table[process_id-1][page].D == 1)
            {
                swap_in(process_id, swapfile_fd, address);
                main_memory[page_table[process_id-1][page].frame*page_size+offset] = value;
            }
            else
            {
                // in DATA
                if(page>=text_pages && page<=text_pages+data_pages)
                {
                    lseek(program_fd[process_id-1], page*page_size, SEEK_SET);
                    int t;
                    // find free space in memory
                    for(t=0; t<MEMORY_SIZE; t++)
                    {
                        if(free_space[t]==1)
                        {
                            // copy from EXE file
                            for(int i = 0; i < page_size; i++)
                            {   
                                read(program_fd[process_id-1], &main_memory[t+i], 1);
                                free_space[t+i] = 0;
                            }
                            main_memory[t+offset] = value;
                            page_table[process_id-1][page].frame = t/page_size;
                            page_table[process_id-1][page].V = 1;
                            page_table[process_id-1][page].D = 1;
                            break;
                        }
                        
                        // if memory is full , swap out a page
                        if(t==MEMORY_SIZE-1)
                        {
                            swap_out(process_id, swapfile_fd, address);
                            free_space[page_table[process_id-1][page].frame*page_size]=1;
                            t=-1;
                        }
                    }
                }
                
                // in bss or heap/stack
                else 
                {
                    // find free space in main memory
                    int t;
                    for(t=0; t<MEMORY_SIZE; t++)
                    {
                        // free space found !
                        if(free_space[t]==1)
                        {
                            for(int k=0; k<page_size; k++)
                            {
                                main_memory[t+k] = '0';
                                free_space[t+k] = 0;
                            }
                            main_memory[t+offset] = value;
                            page_table[process_id-1][page].frame = t/page_size;
                            page_table[process_id-1][page].V = 1;
                            page_table[process_id-1][page].D = 1;
                            break;
                        }
                    }
                        // memory is full !! swap out a page
                        if(t == MEMORY_SIZE)
                        {
                            swap_out(process_id, swapfile_fd, address);
                            main_memory[page_table[process_id-1][page].frame*page_size+offset] = value;
                            page_table[process_id-1][page].D = 1;
                        }
                }
            }
        }

        else // page is in text
            perror("ERROR ! can't store in text...\n");
            
        
    }
    
}





//##############################################################################
//______________________________________________________________________________



// load function
char sim_mem::load(int process_id, int address)
{
    // initialize variables
    
    int text_pages = text_size/page_size;
    int data_pages = data_size/page_size;
    int bss_pages = bss_size/page_size;
    int heap_stack_pages = heap_stack_size/page_size;
    
    if(address>text_size+data_size+bss_size+heap_stack_size)
    {
        perror("ERROR ! no such address....");
        return '\0';
    }
    
    if(process_id>2)
    {
        perror("ERROR ! maximum 2 processes !\n");
        return '\0';
    }
    
    // get page and offset
    int page = address / page_size;
    int offset = address % page_size;
    

    // V=1 <-> page in main memory
    if(page_table[process_id-1][page].V == 1)
        return main_memory[page_table[process_id-1][page].frame*page_size+offset];
    
    // V=0 <-> page not in main memory
    else
    {
        // P=1 <-> bss/data/heap/stack page
        if(page_table[process_id-1][page].P == 1)
        {
            
            // D=1 <-> dirty page, page is in swap
            if(page_table[process_id-1][page].D == 1)
            {
                swap_in(process_id, swapfile_fd, address);
                return main_memory[page_table[process_id-1][page].frame*page_size+offset];
            }
            
            // in bss / heap / stack
            else
            {
                // in DATA
                if(page>=text_pages && page<text_pages+data_pages)
                {
                    
                    lseek(program_fd[process_id-1], page*page_size, SEEK_SET);
                    // copy from EXE file
                    int t;
                    for(t=0; t<MEMORY_SIZE; t++)
                    {
                        if(free_space[t]==1)
                        {
                            for(int i = 0; i < page_size; i++)
                            {   
                                read(program_fd[process_id-1], &main_memory[t+i], 1);
                                free_space[t+i] = 0;
                            }
                            page_table[process_id-1][page].frame = t/page_size;
                            page_table[process_id-1][page].V = 1;
                            break;
                        }
                        // if memory is full, swap out a page
                        if(t==MEMORY_SIZE-1)
                        {
                            swap_out(process_id, swapfile_fd, address);
                            t=-1;
                            
                            if(fifo!=0)
                                free_space[(fifo-1)*page_size] = 1;
                            else
                                free_space[MEMORY_SIZE-page_size] = 1;
                        }
                    }
                    return main_memory[page_table[process_id-1][page].frame*page_size+offset];
                }
                
                // in bss
                else if(page>=text_pages+data_pages && page<text_pages+data_pages+bss_pages)
                {
                    // find free space in memory
                    int t;
                    for(t=0; t<MEMORY_SIZE; t++)
                    {
                        // free space found !
                        if(free_space[t]==1)
                        {
                            for(int k=0; k<page_size; k++)
                            {
                                main_memory[t+k] = '0';
                                free_space[t+k] = 0;
                            }
                            page_table[process_id-1][page].frame = t/page_size;
                            page_table[process_id-1][page].V = 1;
                            return 0;
                        }
                    }
                        // memory is full !! swap out a pge
                        if(t == MEMORY_SIZE)
                        {
                            swap_out(process_id, swapfile_fd, address);
                        }

                }
                // in heap+stack
                else if(page>=text_pages+data_pages+bss_pages)
                {
                    perror("ERROR! cant read from address that wasn't allocated !\n");
                    return '\0';   
                }   
                
            }
        }

        else // page is in text
        {
            lseek(program_fd[process_id-1], page*page_size, SEEK_SET); 

            // copy from EXE file
            int t;
            for(t=0; t<MEMORY_SIZE; t++)
            {
                if(free_space[t]==1)
                {for(int i = 0; i < page_size; i++)
                    {   
                        read(program_fd[process_id-1], &main_memory[t+i], 1);
                        free_space[t+i] = 0;
                    }
                    page_table[process_id-1][page].frame = t/page_size;
                    page_table[process_id-1][page].V = 1;
                    break;
                }
                // if memory is full swap out a page    
                if(t==MEMORY_SIZE-1)
                {
                    swap_out(process_id, swapfile_fd, address);
                    t=-1;
                    if(fifo!=0)
                        free_space[(fifo-1)*page_size] = 1;
                    else if(fifo==1)
                        free_space[(fifo)*page_size] = 1;
                    else
                        free_space[MEMORY_SIZE-page_size] = 1;
                }
            }
            page_table[process_id-1][page].swap_index = -1;
            return main_memory[t+offset];
        }
        
    }
    return '\0';
}




//______________________________________________________________________________

// function brings page from swap to main memory
void sim_mem::swap_in(int process_id, int swapfile_fd, int address)
{
    // initialize variables
    int swap_size = data_size+bss_size+heap_stack_size;
    int page = address / page_size;
    int offset = address % page_size;
    char sw[page_size];
    int j=0;
    
    // copy page into temporary string
    lseek(swapfile_fd, page_table[process_id-1][page].swap_index, SEEK_SET);
    for(int i=0 ; i<page_size; i++)
    {
        read(swapfile_fd, &sw[j], 1);
        j++;
    }
    
    j=0;
    int tmp = page_table[process_id-1][page].swap_index;
    
    // delete page from swap
    lseek(swapfile_fd, tmp, SEEK_SET);
    for(int p=tmp; p<tmp+page_size; p++)
    {
        write(swapfile_fd, "0", 1);
    }
    
    // swap out a page from main memory
    lseek(swapfile_fd, tmp, SEEK_SET);
    for(int p=0; p<num_of_proc; p++)
    {
        for(int u=0; u < num_of_pages; u++)
        {
            if( (page_table[p][u].frame == fifo))
            {
                
                for(int i=fifo*page_size; i<fifo*page_size+page_size; i++)
                {
                    if(u>=text_size/page_size)                      // make sure not to write txt pages to swap
                        write(swapfile_fd, &main_memory[i], 1);
                    
                    main_memory[i] = sw[j];
                    j++;
                }
            }
        }
    }

    
    fifo++;
    if(fifo == MEMORY_SIZE/page_size)
        fifo = 0;

    // update page table
    page_table[process_id-1][page].swap_index = -1;
    page_table[process_id-1][page].V = 1;
    
    for(int p=0; p<num_of_proc; p++)
    {
        for(int i=0; i<num_of_pages; i++)
        {
            if( (fifo!=0  && page_table[p][i].frame == fifo-1) ||
                (fifo==0 && page_table[p][i].frame ==  MEMORY_SIZE/page_size-1) )
            {
                page_table[p][i].frame = -1;
                page_table[p][i].V = 0;
                if(i<text_size/page_size)
                    page_table[p][i].swap_index = -1;
                else
                    page_table[p][i].swap_index =  tmp;
            
            }
        }
    }
    
    // update frame
    if (fifo!=0)
        page_table[process_id-1][page].frame = fifo-1;
    else
    {
        page_table[process_id-1][page].frame =  MEMORY_SIZE/page_size-1;
    } 
}


// takes a frame from memory, and puts it in swap file
void sim_mem::swap_out(int process_id, int swapfile_fd, int address)
{
    // initialize variables
    int swap_size = num_of_proc*(data_size+bss_size+heap_stack_size);
    int page = address / page_size;
    int offset = address % page_size;
    char sw[page_size];    strcpy(sw, "11111");
    int j=0;    int k = 0;
    
    
    // find free space in swap file
    for (k ; k <= swap_size; k+=page_size)
    {
        lseek(swapfile_fd, k, SEEK_SET);
        read(swapfile_fd, &sw[j], 1);
        if(sw[j]=='0')
        {
            int q;
            for (q=0; q<page_size-1; q++)
            {
                j++;
                read(swapfile_fd, &sw[j], 1);
                if(sw[j]!='0')
                {
                    j=0;
                    break;
                }
            }
            if(strcmp(sw, "00000")==0 )
                break;
        }
    }
    

    char check[page_size];

    for(int p=0; p<num_of_proc; p++)
    {
        for(int u=0; u < num_of_pages; u++)
        {
            if( (page_table[p][u].frame == fifo))
            {
                if( strcmp(sw, "00000")==0)
                    lseek(swapfile_fd, k, SEEK_SET);
                
                else
                    lseek(swapfile_fd, page_table[p][u].swap_index, SEEK_SET);
                    
                for(int i=fifo*page_size; i<fifo*page_size+page_size; i++)
                    check[i-fifo*page_size] = main_memory[i];
               
                for(int i=fifo*page_size; i<fifo*page_size+page_size; i++)
                {
                    // make sure you dont wrie txt file to swap
                    if(u>=text_size/page_size && strcmp(check, "00000")!=0)                      
                        write(swapfile_fd, &main_memory[i], 1);
                    
                    main_memory[i]='0';
                }
            }
        }
    }
    fifo++;
    

    if(fifo==MEMORY_SIZE/page_size)
        fifo=0;

    // update page table
    for(int p=0; p<num_of_proc; p++)
    {
        for(int u=0; u < num_of_pages; u++)
        {
            if( (fifo!=0 && page_table[p][u].frame == fifo-1) ||
                (fifo==0 && page_table[p][u].frame == MEMORY_SIZE/page_size-1) )
                {
                    page_table[p][u].V = 0;
                    page_table[p][u].frame = -1;
                    if(u<text_size/page_size || strcmp(check, "00000")==0)
                        page_table[p][u].swap_index = -1;
                    else
                        page_table[p][u].swap_index =  k;
                    break;
                }
        }
    }
        
    page_table[process_id-1][page].V = 1;

    // update fifo
    if (fifo!=0)
        page_table[process_id-1][page].frame = fifo-1;
    else
    {
        page_table[process_id-1][page].frame =  MEMORY_SIZE/page_size-1;
    }
    
    // update swap index, clean page from swap file
    if(page_table[process_id-1][page].swap_index != -1)
    {
        lseek(swapfile_fd, page_table[process_id-1][page].swap_index, SEEK_SET);
        for(int f=0; f<5; f++)
            write(swapfile_fd, "0", 1);
        page_table[process_id-1][page].swap_index = -1;
    }
}

/************************************************************************************/
void sim_mem::print_memory() 
{
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++)
    {
        if(i%page_size==0)
            printf("____FRAME  %d____\n", i/page_size);
        printf("[%c]\t\t%d\n", main_memory[i], free_space[i]);
    }
}
/************************************************************************************/
void sim_mem::print_swap() 
{
    char* str = (char*)malloc(this->page_size *sizeof(char));
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(swapfile_fd, str, this->page_size) == this->page_size) 
    {
        for(i = 0; i < page_size; i++)
            printf("%d - [%c]\t", i, str[i]);
        printf("\n");
    }
    free(str);
}
/***************************************************************************************/
void sim_mem::print_page_table() 
{
    int i;
    for (int j = 0; j < num_of_proc; j++) 
    {
        printf("\n page table of process: %d \n\n", j);
        printf("Page\tAddress\t\tValid\t Dirty\t Permission \t Frame\t Swap index\n");
        for(i = 0; i < num_of_pages; i++) 
        {
            if(i==text_size/page_size || i==(text_size+data_size)/page_size || i==(text_size+data_size+bss_size)/page_size)
                printf("_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _\n");
            printf("%d\t%d-%d\t\t[%d]\t[%d]\t\t[%d]\t[%d]\t\t[%d]\n", i, i*page_size, (i+1)*page_size-1,
            page_table[j][i].V,
            page_table[j][i].D,
            page_table[j][i].P,
            page_table[j][i].frame ,
            page_table[j][i].swap_index);
        }
    }
}

