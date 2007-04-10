#include <string.h>
#include <stdlib.h>
#include "etlib/generic.h"
//#include "etlib/xmalloc.h"
#include "etlib/hash.h"
#include "stdlib.h"
/*
    Mixture of hash table and binary tree.

    First level is a standard hash with the hashpjw function
    from the dragon book. But instead of a linked list in each
    slot I use a binary tree.
    To balance the tree, I take the low-byte of the full hash value
    (before the modulo) as the first char of each key.
    Storing increasing keys does not generate a perfectly balanced
    tree but one that is as good as one generated by random keys.

    usage:
	to define a hash table:
	    hashtable(data-type, table-size) identifier;
	    hashtable(data-type, table-size) id1, id2, id3;

	    data-type should be a simple type, a struct, or
	    a union. the special type hash_no_data is provided
	    when no data is needed.
	    table-size should be a prime.

	to look for an entry:
	    hash_find(table-identifier, string)

	to look for an entry and create a new one if not present:
	    hash_get(table-identifier, string)

	to add an entry
	    hash_add(table-identifier, string)

	    (this is an alias for hash_get(...))

	to get the string associated with an entry:
	    hash_name(table-identifier, hash_get/find(...))

	to get statistics about a hashtable;
	    struct hash_st st;
	    hash_stat(table-identifier, &st);
*/

struct symbol
{
    struct symbol *left;
    struct symbol *right;
    u8 misc[2];		/* contains user struct and name */
};



static inline u32
hash(u8 *str)
{
    u32 h = 0;

    while (*str)
	h = (h << 4) ^ (h >> 28) ^ *str++;

    return h ? h : 1;
}


/*
    Find a symbol. Return 0 if not found.
*/

void *
_hash_find(u8 *id, void *tab, int size, int usize)
{
    struct symbol *s;
    u32 h;
    int i;

    h = hash(id);
    s = ((struct symbol **)tab)[h % size];

    while (s)
    {
	i = (u8)h - s->misc[usize];
	if (i == 0)
	{
	    i = strcmp(id, s->misc + usize + 1);
	    if (i == 0)
		return s->misc;
	}
	s = i < 0 ? s->left : s->right;
    }
    return 0;
}



/*
    Get a symbol. Create if not found.
*/

void *
_hash_get(u8 *id, void *tab, int size, int usize)
{
    struct symbol *s, **ss;
    u32 h;
    int i;
    
    h = hash(id);
    ss = &((struct symbol **)tab)[h % size];

    while ( (s = *ss) )
    {
	i = (u8)h - s->misc[usize];
	if (i == 0)
	{
	    i = strcmp(id, s->misc + usize + 1);
	    if (i == 0)
		return s->misc;
	}
	ss = i < 0 ? &s->left : &s->right;
    }

//    *ss = s = xmalloc(sizeof(*s) + usize + strlen(id));
    *ss = s = malloc(sizeof(*s) + usize + strlen(id));

    s->left = 0;
    s->right = 0;
    memset(s->misc, 0, usize);
    s->misc[usize] = h;
    strcpy(s->misc + usize + 1, id);

    return s->misc;
}



static void
_stat(int depth, struct symbol *s, struct hash_st *st)
{
    while (s)
    {
	if (st->maxdepth < depth)
	    st->maxdepth = depth;
	st->nelem++;
	st->middepth += depth;
	depth++;
	_stat(depth, s->left, st);
	s = s->right;
    }
}



void
_hash_stat(void *tab, int size, struct hash_st *st)
{
    struct symbol **s;

    s = (struct symbol **)tab;

    st->nelem = 0;
    st->maxdepth = 0;
    st->middepth = 0;
    st->hashsize = size;

    while (size--)
	_stat(1, *s++, st);

    if (st->nelem)
	st->middepth = (st->middepth * 1000) / st->nelem;
}




#if 0

/* some primes:
    11 23 31 41 53 61 71 83 97
    101 211 307 401 503 601 701 809 907
    1009 2003 3001 4001 5003 6007 7001 8009 9001
    10007 20011 30011 40009 50021 60013 70001 80021 90001
    100003
*/

hashtable(int, 97) pseudoop;

main()
{
    struct hash_st st;
    u8 buf[256];

    while (gets(buf))
	if (buf[0])
	    hash_get(pseudoop, buf);
    hash_stat(testtable, &st);
    printf("nelem   : %d\n", st.nelem);
    printf("hashsize: %d\n", st.hashsize);
    printf("maxdepth: %d\n", st.maxdepth);
    printf("middepth: %d.%03d\n", st.middepth / 1000, st.middepth % 1000);
}
#endif