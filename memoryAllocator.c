#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

typedef char ALIGN[16];
// we are using align to make page align to 16 bytes
union header {

	struct{
		size_t size;
		unsigned is_free;
		union header *next;
	}s;
	ALIGN stud;
};

typedef union header header_t;

// we will need head and tail to take care of our memory linked list
header_t *head, *tail;

// two thread can access mormory concurrently, to avoid this we are using malloc lock
pthread_mutex_t global_malloc_lock;

header_t *get_free_block(size_t size)
{
	header_t *curr = head;
	while(curr) {
		/* see if there's a free block that can accomodate requested size */
		if (curr->s.is_free && curr->s.size >= size)
			return curr;
		curr = curr->s.next;
	}
	return NULL;
}

void *malloc(size_t size)
{
	void *block;
	size_t required_size;
	header_t *header;
	if(!size)
		return NULL;

	// lock the process
	pthread_mutex_lock(&global_malloc_lock);

	header = get_free_block(size);
	//printf("using malloc");
	if(header)
	{
		header->s.is_free = 0;
		pthread_mutex_unlock(&global_malloc_lock);
		return (void *)(header + 1);
	}
	required_size = sizeof(header_t) + size;
	block = sbrk(required_size); // will increment data segment and give us a block
	if(block == (void *)-1) // sbrk return -1 on failure
	{
		pthread_mutex_unlock(&global_malloc_lock);
		return NULL;
	}
	header = block;
	header->s.size = size;
	header->s.is_free = 0;
	header->s.next = NULL;
	if(!head)
		head = header;
	if(tail)
		tail->s.next = header;
	tail = header;
	pthread_mutex_unlock(&global_malloc_lock);
	return (void *)(header + 1);
}

void free(void *block)
{
	header_t *header, *tmp;
	void *programbreak;

	if (!block)
		return;
	pthread_mutex_lock(&global_malloc_lock);
	header = (header_t*)block - 1;

	programbreak = sbrk(0);
	if ((char*)block + header->s.size == programbreak) {
		if (head == tail) {
			head = tail = NULL;
		} else {
			tmp = head;
			while (tmp) {
				if(tmp->s.next == tail) {
					tmp->s.next = NULL;
					tail = tmp;
				}
				tmp = tmp->s.next;
			}
		}
		sbrk(0 - sizeof(header_t) - header->s.size);
		pthread_mutex_unlock(&global_malloc_lock);
		return;
	}
	header->s.is_free = 1;
	pthread_mutex_unlock(&global_malloc_lock);
}

void *calloc(size_t num, size_t nsize)
{
	size_t size;
	void *block;
	if (!num || !nsize)
		return NULL;
	size = num * nsize;
	/* check mul overflow */
	if (nsize != size / num)
		return NULL;
	block = malloc(size);
	if (!block)
		return NULL;
	memset(block, 0, size);
	return block;
}

void *realloc(void *block, size_t size)
{
	header_t *header;
	void *ret;
	if (!block || !size)
		return malloc(size);
	header = (header_t*)block - 1;
	if (header->s.size >= size)
		return block;
	ret = malloc(size);
	if (ret) {
		/* Relocate contents to the new bigger block */
		memcpy(ret, block, header->s.size);
		/* Free the old memory block */
		free(block);
	}
	return ret;
}

