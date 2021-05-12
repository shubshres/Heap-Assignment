/*
   Shubhayu Shrestha
   UTA ID: 1001724804

   Mohammed Ahmed
   UTA ID: 1001655176
*/



#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0; 
static int num_mallocs       = 0; //done
static int num_frees         = 0; //done
static int num_reuses        = 0;
static int num_grows         = 0; //done
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

//stores address of last allocation (where we stopped last)
//for next fit
struct _block *last_allocation = NULL; 

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   size_t  nitems;       //This is the number of elements to be allocated.
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;  //beginning of our list

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
   /* Best fit */

   size_t currentDifference = 0;                     //difference between size of block & our element
   size_t lowestDifference = INT_MAX;                //lowest difference between all the blocks and our element
   struct _block *differencePtr = NULL;              //stores address of block with lowest difference

   while(curr != NULL)  //loops through entire heaplist
   {
      *last = curr;     //saves current address before we move onto next block (prev pointer)

      
      
      if((curr->free) && (curr->size >= size))   //if block is free and can element can fit in block
      {
         currentDifference = (int)(curr->size - size);//find difference

         if(currentDifference < lowestDifference)  //checks if current difference is lower
         {
            lowestDifference = currentDifference;  //stores difference
            differencePtr = curr;                  //stores lowest address
         }

      }
      

      curr  = curr->next; //moves pointer to the next block
   }

   curr = differencePtr; //stores address of the spot with lowest difference
                         //so it can be returned at the end of the function

      

#endif

#if defined WORST && WORST == 0
    /* Worst fit */

   size_t currentDifference = 0;                     //difference between size of block & our element
   size_t highestDifference = 0;                      //highest difference between all the blocks and our element
   struct _block *differencePtr = NULL;              //stores address of block with highest difference

   while(curr != NULL)  //loops through entire heaplist
   {
      *last = curr;     //saves current address before we move onto next block (prev pointer)

      
      
      if((curr->free) && (curr->size >= size))   //if block is free and can element can fit in block
      {
         currentDifference = (int)(curr->size - size); //find difference

         if(currentDifference > highestDifference) //checks if current difference is greater
         {
            highestDifference = currentDifference;  //stores difference
            differencePtr = curr;                  //stores lowest address
         }

      }
      
      curr  = curr->next; //moves pointer to the next block
   }

   curr = differencePtr; //stores address of the spot with lowest difference
                         //so it can be returned at the end of the function


#endif

#if defined NEXT && NEXT == 0
   /* Next fit */

   //checks if address is being stored (new starting point)
   if(last_allocation != NULL)
   {
      curr = last_allocation;
   }   

   //first fit code
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }

   //checks beginning of the list, if we don't find it at the end
   if(curr == NULL)
   {
      while (curr != last_allocation && !(curr->free && curr->size >= size))//loops until starting point (last allocation) 
       {
          *last = curr;
           curr  = curr->next;
       }

   }

   //grows heap if we don't find anything
   if(curr == last_allocation)
   {
      curr = NULL;
   }





   
   


#endif

   last_allocation = curr; //stores address of spot we are allocating

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{

   
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   num_requested += size; //size is the number of times we request memory



   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

  

   /* TODO: Split free _block if possible */

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
      num_grows++;   //increment grows counter
      num_blocks++;  //increment blocks counter
      max_heap += sizeof(struct _block) + size; //increase size of max heap (overhead + data)

      
   }
   else
   {
      num_reuses++; //every time you don't reuse heap, you use old block
   }
   

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;

   //increment malloc counter
   num_mallocs++;

   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}


//Calloc Function
void *calloc(size_t nitems,size_t size) 
{

   void *newPtr;                    //pointer to store address
   newPtr = malloc(nitems * size);  //allocates memory (uninitialized)
   memset(newPtr,0,size);           //stores 0 inside of unititialized array
   return newPtr;                   //return address of initialized memory


   
}

//Realloc Funciton
void *realloc(void *ptr, size_t size)
{
   void *newPtr;              //pointer to store new address (with new size)    
   newPtr = malloc(size);     //initializes memory to new size
   memcpy(newPtr,ptr,size);   //copies data from old pointer to new pointer
   return newPtr;             //return new address
}




/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{

   //increment free counter
   num_frees++;
   

   if (ptr == NULL) 
   {
      return;
   }

   

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;

   /* TODO: Coalesce free _blocks if needed */
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
