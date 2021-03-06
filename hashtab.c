/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2007, Digium, Inc.
 *
 * Steve Murphy <murf@digium.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */
/*! \file
 *
 *  \brief code to implement generic hash tables
 *
 *  \author Steve Murphy <murf@digium.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stddef.h>

#include "hashtab.h"

static void ast_hashtab_resize( struct ast_hashtab *tab);

/* some standard, default routines for general use */

int ast_hashtab_compare_strings(const void *a, const void *b)
{
	return strcmp((char*)a,(char*)b);
}

int ast_hashtab_compare_strings_nocase(const void *a, const void *b)
{
	return strcasecmp((const char*)a,(const char*)b);
}

int ast_hashtab_compare_ints(const void *a, const void *b)
{
	int ai = *((int*)a);
	int bi = *((int*)b);
	if (ai < bi)
		return -1;
	else if (ai==bi)
		return 0;
	else
		return 1;
}

int ast_hashtab_compare_shorts(const void *a, const void *b)
{
	short as = *((short*)a);
	short bs = *((short*)b);
	if (as < bs)
		return -1;
	else if (as==bs)
		return 0;
	else
		return 1;
}

int ast_hashtab_resize_java(struct ast_hashtab *tab)
{
	double loadfactor = (double)tab->hash_tab_elements / (double)tab->hash_tab_size;
	if (loadfactor > 0.75)
		return 1;
	return 0;
}

int ast_hashtab_resize_tight(struct ast_hashtab *tab)
{
	
	if (tab->hash_tab_elements > tab->hash_tab_size) /* this is quicker than division */
		return 1;
	return 0;
}

int ast_hashtab_resize_none(struct ast_hashtab *tab) /* always return 0 -- no resizing */
{
	return 0;
}


int isPrime(int num)
{
	int tnum,limit;
	    
	if ((num & 0x1) == 0) /* even number -- not prime */
		return 0;
		    
    	/* Loop through ODD numbers starting with 3 */

	tnum = 3;
	limit = num;
	while (tnum < limit)
	{
		if ((num%tnum) == 0) {
			return 0;
		}
		/* really, we only need to check sqrt(num) numbers */
		limit = num / tnum;
		/* we only check odd numbers */
		tnum = tnum+2;
	}
	/* if we made it thru the loop, the number is a prime */
	return 1;
}

int ast_hashtab_newsize_java(struct ast_hashtab *tab)
{
	int i = (tab->hash_tab_size<<1); /* multiply by two */
	while (!isPrime(i))
		i++;
	return i;
}

int ast_hashtab_newsize_tight(struct ast_hashtab *tab)
{
	int x = (tab->hash_tab_size<<1);
	int i = (tab->hash_tab_size+x);
	while (!isPrime(i))
		i++;
	return i;
}

int ast_hashtab_newsize_none(struct ast_hashtab *tab) /* always return current size -- no resizing */
{
	return tab->hash_tab_size;
}

int ast_hashtab_hash_string(const void *obj, int modulus)
{
	unsigned char *str = (unsigned char*)obj;
	unsigned int total;

	for (total=0; *str; str++)
	{
		unsigned int tmp = total;
		total <<= 1; /* multiply by 2 */
		total += tmp; /* multiply by 3 */
		total <<= 2; /* multiply by 12 */
		total += tmp; /* multiply by 13 */
		
		total += ((unsigned int)(*str));
	}
	return (total % modulus);
}

int ast_hashtab_hash_string_sax(const void *obj, int modulus) /* from Josh */
{
	unsigned char *str = (unsigned char*)obj;
	unsigned int total = 0, c = 0;

	while ((c = *str++))
		total ^= ( total << 5 ) + ( total >> 2 ) + ( total << 10) + c;

	return (total % modulus);
}

int ast_hashtab_hash_string_nocase(const void *obj, int modulus)
{
	unsigned char *str = (unsigned char*)obj;
	unsigned int total;

	for (total=0; *str; str++)
	{
		unsigned int tmp = total;
		unsigned int charval = toupper(*str);

		/* hopefully, the following is faster than multiplication by 7 */
		/* why do I go to this bother? A good compiler will do this 
		   anyway, if I say total *= 13 */
		/* BTW, tried *= 7, and it doesn't do as well in spreading things around! */
		total <<= 1; /* multiply by 2 */
		total += tmp; /* multiply by 3 */
		total <<= 2; /* multiply by 12 */
		total += tmp; /* multiply by 13 */
		
		total += (charval);
	}
	return (total % modulus);
}

int ast_hashtab_hash_int(const int x, int modulus)
{
	return (x % modulus);
}

int ast_hashtab_hash_short(const short x, int modulus)
{
	/* hmmmm.... modulus is best < 65535 !! */
	return (x % modulus);
}

static inline int ast_rwlock_init(ast_rwlock_t *prwlock)
{
        pthread_rwlockattr_t attr;

        pthread_rwlockattr_init(&attr);

#ifdef HAVE_PTHREAD_RWLOCK_PREFER_WRITER_NP
        pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NP);
#endif

        return pthread_rwlock_init(prwlock, &attr);
}
static inline int ast_rwlock_destroy(ast_rwlock_t *prwlock)
{
        return pthread_rwlock_destroy(prwlock);
}
static inline int ast_rwlock_unlock(ast_rwlock_t *prwlock)
{
        return pthread_rwlock_unlock(prwlock);
}

static inline int ast_rwlock_rdlock(ast_rwlock_t *prwlock)
{
        return pthread_rwlock_rdlock(prwlock);
}

static inline int ast_rwlock_tryrdlock(ast_rwlock_t *prwlock)
{
        return pthread_rwlock_tryrdlock(prwlock);
}

static inline int ast_rwlock_wrlock(ast_rwlock_t *prwlock)
{
        return pthread_rwlock_wrlock(prwlock);
}

static inline int ast_rwlock_trywrlock(ast_rwlock_t *prwlock)
{
        return pthread_rwlock_trywrlock(prwlock);
}



struct ast_hashtab * ast_hashtab_create(int initial_buckets,
										int (*compare)(const void *a, const void *b), /* a func to compare two elements in the hash -- cannot be null  */
										int (*resize)(struct ast_hashtab *), /* a func to decide if the table needs to be resized, a NULL ptr here will cause a default to be used */
										int (*newsize)(struct ast_hashtab *tab), /* a ptr to func that returns a new size of the array. A NULL will cause a default to be used */
										int (*hash)(const void *obj, int modulus), /* a func to do the hashing */
										int do_locking ) /* use locks to guarantee safety of iterators/insertion/deletion -- real simpleminded right now */
{
	struct ast_hashtab *ht = calloc(1,sizeof(struct ast_hashtab));
	while (!isPrime(initial_buckets)) /* make sure this is prime */
		initial_buckets++;
	ht->array = calloc(initial_buckets,sizeof(struct ast_hashtab_bucket*));
	ht->hash_tab_size = initial_buckets;
	ht->compare = compare;
	ht->resize = resize;
	ht->newsize = newsize;
	ht->hash = hash;
	ht->do_locking = do_locking;
	if (do_locking)
	{
		ast_rwlock_init(&ht->lock);
	}
	if (!ht->resize)
		ht->resize = ast_hashtab_resize_java;
	if (!ht->newsize)
		ht->newsize = ast_hashtab_newsize_java;
	return ht;
}

struct ast_hashtab *ast_hashtab_dup(struct ast_hashtab *tab, void *(*obj_dup_func)(const void *obj))
{
	struct ast_hashtab *ht = calloc(1,sizeof(struct ast_hashtab));
	int i;
	
	ht->array = calloc(tab->hash_tab_size,sizeof(struct ast_hashtab_bucket*));
	ht->hash_tab_size = tab->hash_tab_size;
	ht->compare = tab->compare;
	ht->resize = tab->resize;
	ht->newsize = tab->newsize;
	ht->hash = tab->hash;
	ht->do_locking = tab->do_locking;
	if (ht->do_locking)
	{
		ast_rwlock_init(&ht->lock);
	}
	/* now, dup the objects in the buckets and get them into the table */
	/* the fast way is to use the existing array index, and not have to hash
	   the objects again */
	for (i=0;i<ht->hash_tab_size;i++)
	{
		struct ast_hashtab_bucket *b = tab->array[i];
		while( b )
		{
			void *newobj = (*obj_dup_func)(b->object);
			if (newobj) {
				ast_hashtab_insert_immediate_bucket(ht, newobj, i);
			}
			b = b->next;
		}
	}
	return ht;
}

static void tlist_del_item(struct ast_hashtab_bucket **head, struct ast_hashtab_bucket *item)
{
	/* item had better be in the list! or suffer the weirdness that occurs, later! */
	if (*head == item) { /* first item in the list */
		*head = item->tnext;
		if (item->tnext)
			item->tnext->tprev = NULL;
	} else {
		/* short circuit stuff */
		item->tprev->tnext = item->tnext;
		if (item->tnext)
			item->tnext->tprev = item->tprev;
	}
}

static void tlist_add_head(struct ast_hashtab_bucket **head, struct ast_hashtab_bucket *item)
{
	if (*head) {
		item->tnext = *head;
		item->tprev = NULL;
		(*head)->tprev = item;
		*head = item;
	} else {
		/* the list is empty */
		*head = item;
		item->tprev = NULL;
		item->tnext = NULL;
	}
}

/* user-controlled hashtab locking. Create a hashtab without locking, then call the
   following locking routines yourself to lock the table between threads. */

void ast_hashtab_wrlock(struct ast_hashtab *tab)
{
	ast_rwlock_wrlock(&tab->lock);
}

void ast_hashtab_rdlock(struct ast_hashtab *tab)
{
	ast_rwlock_rdlock(&tab->lock);
}

void ast_hashtab_initlock(struct ast_hashtab *tab)
{
	ast_rwlock_init(&tab->lock);
}

void ast_hashtab_destroylock(struct ast_hashtab *tab)
{
	ast_rwlock_destroy(&tab->lock);
}

void ast_hashtab_unlock(struct ast_hashtab *tab)
{
	ast_rwlock_unlock(&tab->lock);
}


void ast_hashtab_destroy( struct ast_hashtab *tab, void (*objdestroyfunc)(void *obj))
{
	/* this func will free the hash table and all its memory. It
	   doesn't touch the objects stored in it */
	if (tab) {
		
		if (tab->do_locking)
			ast_rwlock_wrlock(&tab->lock);

		if (tab->array) {
			/* go thru and destroy the buckets */
			struct ast_hashtab_bucket *t;
			int i;
			
			while (tab->tlist) {
				t = tab->tlist;
				if (t->object && objdestroyfunc) {
					(*objdestroyfunc)((void*)t->object); /* I cast this because I'm not going to MOD it, I'm going to DESTROY it */
				}
				
				tlist_del_item(&(tab->tlist), tab->tlist);
				free(t);
			}
			
			for (i=0;i<tab->hash_tab_size;i++) { 
				tab->array[i] = NULL; /* not totally necc., but best to destroy old ptrs */
			}
			
			free(tab->array);
		}
		if (tab->do_locking) {
			ast_rwlock_unlock(&tab->lock);
			ast_rwlock_destroy(&tab->lock);
		}
		free(tab);
	}
}

int ast_hashtab_insert_immediate(struct ast_hashtab *tab, const void *obj)
{
	/* normally, you'd insert "safely" by checking to see if the element is
	   already there; in this case, you must already have checked. If an element
	   is already in the hashtable, that matches this one, most likely this one
	   will be found first, but.... */

	/* will force a resize if the resize func returns 1 */
	/* returns 1 on success, 0 if there's a problem */
	int h;
	int c;
	struct ast_hashtab_bucket *b;
	
	if (!tab) {
		printf("NULL table pointer given to INSERT.\n");
		return 0;
	}
	if (!obj) {
		printf("NULL object pointer given to INSERT.\n");
		return 0;
	}
	if (tab->do_locking)
		ast_rwlock_wrlock(&tab->lock);
	h = (*tab->hash)(obj, tab->hash_tab_size);
	for (c=0,b=tab->array[h];b;b=b->next) {
		c++;
	}
	if (c+1 > tab->largest_bucket_size)
		tab->largest_bucket_size = c+1;
	b = malloc(sizeof(struct ast_hashtab_bucket));
	b->object = obj;
	b->next = tab->array[h];
	b->prev = NULL;
	if (b->next)
		b->next->prev = b;
	tlist_add_head(&(tab->tlist),b);
	
	tab->array[h] = b;
	tab->hash_tab_elements++;
	if ((*tab->resize)(tab))
		ast_hashtab_resize(tab);
	if (tab->do_locking)
		ast_rwlock_unlock(&tab->lock);
	return 1;
}

int ast_hashtab_insert_immediate_bucket(struct ast_hashtab *tab, const void *obj, int h)
{
	/* normally, you'd insert "safely" by checking to see if the element is
	   already there; in this case, you must already have checked. If an element
	   is already in the hashtable, that matches this one, most likely this one
	   will be found first, but.... */

	/* will force a resize if the resize func returns 1 */
	/* returns 1 on success, 0 if there's a problem */
	int c;
	struct ast_hashtab_bucket *b;
	
	if (!tab || !obj)
		return 0;
	
	for (c=0,b=tab->array[h];b;b=b->next) {
		c++;
	}
	if (c+1 > tab->largest_bucket_size)
		tab->largest_bucket_size = c+1;
	b = malloc(sizeof(struct ast_hashtab_bucket));
	b->object = obj;
	b->next = tab->array[h];
	b->prev = NULL;
	tab->array[h] = b;
	if (b->next)
		b->next->prev = b;
	tlist_add_head(&(tab->tlist), b);
	tab->hash_tab_elements++;
	if ((*tab->resize)(tab))
		ast_hashtab_resize(tab);
	return 1;
}

int ast_hashtab_insert_safe(struct ast_hashtab *tab, const void *obj)
{
	/* check to see if the element is already there; insert only if
	   it is not there. */
	/* will force a resize if the resize func returns 1 */
	/* returns 1 on success, 0 if there's a problem, or it's already there. */
	int bucket = 0;
	if (tab->do_locking)
		ast_rwlock_wrlock(&tab->lock);

	if (ast_hashtab_lookup_bucket(tab,obj,&bucket) == 0)
	{
		return ast_hashtab_insert_immediate_bucket(tab,obj,bucket);
	}
	if (tab->do_locking)
		ast_rwlock_unlock(&tab->lock);
	return 0;
}

void * ast_hashtab_lookup(struct ast_hashtab *tab, const void *obj)
{
	/* lookup this object in the hash table. return a ptr if found, or NULL if not */
	int h;
	const void *ret;
	struct ast_hashtab_bucket *b;
	if (!tab || !obj)
		return 0;
	
	if (tab->do_locking)
		ast_rwlock_rdlock(&tab->lock);
	h = (*tab->hash)(obj, tab->hash_tab_size);
	for (b=tab->array[h]; b; b=b->next) {
		if ((*tab->compare)(obj,b->object) == 0) {
			ret = b->object;
			if (tab->do_locking)
				ast_rwlock_unlock(&tab->lock);
			return (void*)ret; /* I can't touch obj in this func, but the outside world is welcome to */
		}
	}
	if (tab->do_locking)
		ast_rwlock_unlock(&tab->lock);

	return 0;
}

void * ast_hashtab_lookup_bucket(struct ast_hashtab *tab, const void *obj, int *bucket)
{
	/* lookup this object in the hash table. return a ptr if found, or NULL if not */
	int h;
	struct ast_hashtab_bucket *b;
	if (!tab || !obj)
		return 0;
	
	h = (*tab->hash)(obj, tab->hash_tab_size);
	for (b=tab->array[h]; b; b=b->next) {
		if ((*tab->compare)(obj,b->object) == 0) {
			if (tab->do_locking)
				ast_rwlock_unlock(&tab->lock);
			return (void*)b->object; /* I can't touch obj in this func, but the outside world is welcome to */
		}
	}
	*bucket = h;
	return 0;
}

void ast_hashtab_get_stats( struct ast_hashtab *tab, int *biggest_bucket_size, int *resize_count, int *num_objects, int *num_buckets)
{
	/* returns key stats for the table */
	if (tab->do_locking)
		ast_rwlock_rdlock(&tab->lock);
	*biggest_bucket_size = tab->largest_bucket_size;
	*resize_count = tab->resize_count;
	*num_objects = tab->hash_tab_elements;
	*num_buckets = tab->hash_tab_size;
	if (tab->do_locking)
		ast_rwlock_unlock(&tab->lock);
}

	/* this function returns the number of elements stored in the hashtab */
int  ast_hashtab_size( struct ast_hashtab *tab)
{
	return tab->hash_tab_elements;
}

	/* this function returns the size of the bucket array in the hashtab */
int  ast_hashtab_capacity( struct ast_hashtab *tab)
{
	return tab->hash_tab_size;
}



/* the insert operation calls this, and is wrlock'd when it does. */
/* if you want to call it, you should set the wrlock yourself */


static void ast_hashtab_resize( struct ast_hashtab *tab)
{
	/* this function is called either internally, when the resize func returns 1, or
	   externally by the user to force a resize of the hash table */
	int newsize = (*tab->newsize)(tab), i, h,c;
	struct ast_hashtab_bucket *b,*bn;
	
	/* Since we keep a DLL of all the buckets in tlist,
	   all we have to do is free the array, malloc a new one,
	   and then go thru the tlist array and reassign them into 
	   the bucket arrayj.
	*/
	for (i=0;i<tab->hash_tab_size;i++) { /* don't absolutely have to do this, but
											why leave ptrs laying around */
		tab->array[i] = 0; /* erase old ptrs */
	}
	free(tab->array);
	tab->array = calloc(newsize,sizeof(struct ast_hashtab_bucket *));
	/* now sort the buckets into their rightful new slots */
	tab->resize_count++;
	tab->hash_tab_size = newsize;
	tab->largest_bucket_size = 0;

	for (b=tab->tlist;b;b=bn)
	{
		b->prev = 0;
		bn = b->tnext;
		h = (*tab->hash)(b->object, tab->hash_tab_size);
		b->next = tab->array[h];
		if (b->next)
			b->next->prev = b;
		tab->array[h] = b;
	}
	/* recalc the largest bucket size */
	for (i=0;i<tab->hash_tab_size;i++) {
		c=0;
		for (b=tab->array[i]; b; b=b->next) {
			c++;
		}
		if (c > tab->largest_bucket_size)
			tab->largest_bucket_size = c;
	}
}

struct ast_hashtab_iter *ast_hashtab_start_traversal(struct ast_hashtab *tab)
{
	/* returns an iterator */
	struct ast_hashtab_iter *it = malloc(sizeof(struct ast_hashtab_iter));
	it->next = tab->tlist;
	it->tab = tab;
	if (tab->do_locking)
		ast_rwlock_rdlock(&tab->lock);
	return it;
}

/* use this function to get a write lock */
struct ast_hashtab_iter *ast_hashtab_start_write_traversal(struct ast_hashtab *tab)
{
	/* returns an iterator */
	struct ast_hashtab_iter *it = malloc(sizeof(struct ast_hashtab_iter));
	it->next = tab->tlist;
	it->tab = tab;
	if (tab->do_locking)
		ast_rwlock_wrlock(&tab->lock);
	return it;
}

void ast_hashtab_end_traversal(struct ast_hashtab_iter *it)
{
	if (it->tab->do_locking)
		ast_rwlock_unlock(&it->tab->lock);
	free(it);
}

void *ast_hashtab_next(struct ast_hashtab_iter *it)
{
	/* returns the next object in the list, advances iter one step */
	struct ast_hashtab_bucket *retval;
	
	if (it && it->next) { /* there's a next in the bucket list */
		retval = it->next;
		it->next = retval->tnext;
		return (void*)retval->object;
	}
	return NULL;
}

static void *ast_hashtab_remove_object_internal(struct ast_hashtab *tab, struct ast_hashtab_bucket *b, int h)
{
	const void *obj2;
	
	if (b->prev) {
		b->prev->next = b->next;
	} else {
		tab->array[h] = b->next;
	}
	
	if (b->next) {
		b->next->prev = b->prev;
	}
	
	tlist_del_item(&(tab->tlist), b);
	
	obj2 = b->object;
	b->object = b->next = (void*)2;
	free(b); /* free up the hashbucket */
	tab->hash_tab_elements--;
#ifdef DEBUG
	{
		int c2;
		struct ast_hashtab_bucket *b2;
		/* do a little checking */
		for (c2 = 0, b2 = tab->tlist;b2;b2=b2->tnext) {
			c2++;
		}
		if (c2 != tab->hash_tab_elements) {
			printf("Hey! we didn't delete right! there are %d elements in the list, and we expected %d\n",
				   c2, tab->hash_tab_elements);
		}
		for (c2 = 0, b2 = tab->tlist;b2;b2=b2->tnext) {
			unsigned int obj3 = (unsigned long)obj2;
			unsigned int b3 = (unsigned long)b;
			if (b2->object == obj2)
				printf("Hey-- you've still got a bucket pointing at ht_element %x\n", obj3);
			if (b2->next == b)
				printf("Hey-- you've still got a bucket with next ptr pointing to deleted bucket %x\n", b3);
			if (b2->prev == b)
				printf("Hey-- you've still got a bucket with prev ptr pointing to deleted bucket %x\n", b3);
			if (b2->tprev == b)
				printf("Hey-- you've still got a bucket with tprev ptr pointing to deleted bucket %x\n", b3);
			if (b2->tnext == b)
				printf("Hey-- you've still got a bucket with tnext ptr pointing to deleted bucket %x\n", b3);
		}
	}
#endif
	return (void*)obj2; /* inside this code, the obj's are untouchable, but outside, they aren't */
}

void *ast_hashtab_remove_object_via_lookup(struct ast_hashtab *tab, void *obj)
{
	/* looks up the object; removes the corresponding bucket */
	int h;
	struct ast_hashtab_bucket *b;

	if (!tab || !obj)
		return 0;
	if (tab->do_locking)
		ast_rwlock_wrlock(&tab->lock);
	h = (*tab->hash)(obj, tab->hash_tab_size);
	for (b=tab->array[h]; b; b=b->next)
	{
		void *obj2;
		
		if ((*tab->compare)(obj,b->object) == 0) {

			obj2 = ast_hashtab_remove_object_internal(tab,b,h);
			
			if (tab->do_locking)
				ast_rwlock_unlock(&tab->lock);
			
			return (void*)obj2; /* inside this code, the obj's are untouchable, but outside, they aren't */
		}
	}
	if (tab->do_locking)
		ast_rwlock_unlock(&tab->lock);
	return 0;
}

void *ast_hashtab_remove_object_via_lookup_nolock(struct ast_hashtab *tab, void *obj)
{
	/* looks up the object; removes the corresponding bucket */
	int h;
	struct ast_hashtab_bucket *b;

	if (!tab || !obj)
		return 0;
	h = (*tab->hash)(obj, tab->hash_tab_size);
	for (b=tab->array[h]; b; b=b->next)
	{
		void *obj2;
		
		if ((*tab->compare)(obj,b->object) == 0) {

			obj2 = ast_hashtab_remove_object_internal(tab,b,h);
			
			if (tab->do_locking)
				ast_rwlock_unlock(&tab->lock);
			
			return (void*)obj2; /* inside this code, the obj's are untouchable, but outside, they aren't */
		}
	}
	return 0;
}

void *ast_hashtab_remove_this_object(struct ast_hashtab *tab, void *obj)
{
	/* looks up the object by hash and then comparing pts in bucket list instead of
	   calling the compare routine; removes the bucket -- a slightly cheaper operation */
	/* looks up the object; removes the corresponding bucket */
	int h;
	struct ast_hashtab_bucket *b;

	if (!tab || !obj)
		return 0;
 
	if (tab->do_locking)
		ast_rwlock_wrlock(&tab->lock);

	h = (*tab->hash)(obj, tab->hash_tab_size);
	for (b=tab->array[h]; b; b=b->next)
	{
		const void *obj2;
		
		if (obj == b->object) {

			obj2 = ast_hashtab_remove_object_internal(tab,b,h);
			
			if (tab->do_locking)
				ast_rwlock_unlock(&tab->lock);

			return (void*)obj2; /* inside this code, the obj's are untouchable, but outside, they aren't */
		}
	}
	
	if (tab->do_locking)
		ast_rwlock_unlock(&tab->lock);
	return 0;
}

void *ast_hashtab_remove_this_object_nolock(struct ast_hashtab *tab, void *obj)
{
	/* looks up the object by hash and then comparing pts in bucket list instead of
	   calling the compare routine; removes the bucket -- a slightly cheaper operation */
	/* looks up the object; removes the corresponding bucket */
	int h;
	struct ast_hashtab_bucket *b;

	if (!tab || !obj)
		return 0;
 
	h = (*tab->hash)(obj, tab->hash_tab_size);
	for (b=tab->array[h]; b; b=b->next)
	{
		const void *obj2;
		
		if (obj == b->object) {

			obj2 = ast_hashtab_remove_object_internal(tab,b,h);
			
			if (tab->do_locking)
				ast_rwlock_unlock(&tab->lock);

			return (void*)obj2; /* inside this code, the obj's are untouchable, but outside, they aren't */
		}
	}

	return 0;
}
