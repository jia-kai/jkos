/*
 * $File: rbtree.h
 * $Date: Thu Dec 02 11:12:04 2010 +0800
 *
 * red-black tree template
 *
 * defining RBT_SIZE will enable size manipulating, which allows operation such as find_kth
 * defining RBT_DEBUG will add method check(), which checks the property of red-black tree and throw
 *	const char * on error
 */
/*
This file is part of JKOS

Copyright (C) <2010>  Jiakai <jia.kai66@gmail.com>

JKOS is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

JKOS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with JKOS.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HEADER_RBTREE
#define HEADER_RBTREE

#include <common.h>

template <typename Key_t>
class Rbt
{
public:
	class Node;
	// Node only has one public method:
	//	const Key_t& Node::get_key()
	typedef void* (*Nalloc_func_t)();
	typedef void (*Nfree_func_t)(void *);

	// the allocator must return addreses aligned to even byte boundaries
	Rbt(Nalloc_func_t alloc, Nfree_func_t free);

	typedef void (*Walk_callback_t)(const Key_t &key);

	void walk(Walk_callback_t callback) const;

	void insert(const Key_t &val);

	// find the minimal key in the tree not less than @val
	// return NULL if not found
	Node* find_ge(const Key_t &val);

	Node* find_le(const Key_t &val);

	void erase(Node *ptr);

#ifdef RBT_SIZE
	// find the kth large value (k starts counting at 0)
	// return NULL if k >= the size of tree
	Node* find_kth(int k);
#endif

	void clear();

#ifdef RBT_DEBUG
	void check() throw (const char *);
#endif

private:

	Nalloc_func_t node_alloc;
	Nfree_func_t node_free;


	static Node NIL_INSTANCE, *NIL;
	Node *tree_root;

	// dir = 0: left rotate
	// dir = 1: right rotate
	static void rotate(Node *&root, int dir);

#ifdef RBT_DEBUG
	// return the number of black nodes on a path
	static int do_check(Node *root);
#endif

	static void do_walk(const Node *root, Walk_callback_t callback);

	static Node *do_find_ge(Node *root, Key_t val);
	static Node *do_find_le(Node *root, Key_t val);

	// n must have at most one non-leaf child
	void remove_bottom(Node *&n);

	void do_clear(Node *root);

	inline Node *& get_ref(Node *n)
	{ return n == tree_root ? tree_root : n->get_par()->ch[n->get_par()->ch[1] == n];}

};

template <typename Key_t>
class Rbt<Key_t>::Node
{
	uint32_t par_and_color;
#ifdef RBT_SIZE
	int size;
#endif
	Node *ch[2];
	Key_t key;
	friend class Rbt;

	inline bool is_red()
	{ return !(par_and_color & 1); }

	inline bool is_black() const
	{ return par_and_color & 1; }

	inline void set_red()
	{ par_and_color &= ~1; }

	inline void set_black()
	{ par_and_color |= 1; }

	inline void copy_color(const Node *from)
	{ par_and_color = (par_and_color & (~1)) | (from->par_and_color & 1); }

	inline Node *get_par() const
	{ return (Node*)(par_and_color & (~1)); }

	inline void copy_par(const Node *from)
	{ par_and_color = (par_and_color & 1) | (from->par_and_color & (~1)); }

	inline void set_par(Node *par)
	{ par_and_color = (par_and_color & 1) | (uint32_t)par; }

public:
	inline const Key_t& get_key()
	{ return key; }
};

template <typename T>
typename Rbt<T>::Node Rbt<T>::NIL_INSTANCE;
template <typename T>
typename Rbt<T>::Node *Rbt<T>::NIL = &Rbt<T>::NIL_INSTANCE;


template <typename Key_t>
Rbt<Key_t>::Rbt(Nalloc_func_t alloc, Nfree_func_t free) :
	node_alloc(alloc), node_free(free), tree_root(NIL)
{
	NIL->set_black();
	NIL->set_par(NIL);
	NIL->ch[0] = NIL->ch[1] = NIL;
#ifdef RBT_SIZE
	NIL->size = 0;
#endif
}

template <typename Key_t>
void Rbt<Key_t>::rotate(Node *&root, int dir)
{
	Node *t = root->ch[!dir];
	t->copy_par(root);
	(root->ch[!dir] = t->ch[dir])->set_par(root);
	(t->ch[dir] = root)->set_par(t);

#ifdef RBT_SIZE
	t->size = root->size;
	root->size = root->ch[0]->size + root->ch[1]->size + 1;
#endif

	root = t;
}

template <typename Key_t>
void Rbt<Key_t>::insert(const Key_t &val)
{
	if (tree_root == NIL)
	{
		tree_root = static_cast<Node*>(node_alloc());
		tree_root->set_black();
		tree_root->set_par(NIL);
		tree_root->ch[0] = tree_root->ch[1] = NIL;
		tree_root->key = val;
#ifdef RBT_SIZE
		tree_root->size = 1;
#endif
		return;
	}

	Node *par = NIL, *cur = tree_root;
	while (cur != NIL)
	{
#ifdef RBT_SIZE
		cur->size ++;
#endif
		par = cur;
		cur = cur->ch[cur->key < val];
	}
	cur = static_cast<Node*>(node_alloc());

	par->ch[par->key < val] = cur;

	cur->set_red();

	cur->ch[0] = cur->ch[1] = NIL;
	cur->set_par(par);
	cur->key = val;
#ifdef RBT_SIZE
	cur->size = 1;
#endif

	while ((par = cur->get_par()) != NIL && par->is_red())
	{
		int par_dir;
		Node *gpar = par->get_par(),
			 *uncle = gpar->ch[par_dir = (par == gpar->ch[0])];
		if (uncle->is_red())
		{
			par->set_black();
			uncle->set_black();
			gpar->set_red();
			cur = gpar;
			continue;
		}

		if ((cur == par->ch[0]) != par_dir)
		{
			rotate(gpar->ch[!par_dir], !par_dir);
			cur = par;
			par = cur->get_par();
		}

		par->set_black();
		gpar->set_red();

		rotate(get_ref(gpar), par_dir);

		return;
	}

	if (par == NIL)
		cur->set_black();
}

template <typename Key_t>
void Rbt<Key_t>::remove_bottom(Node *&n_ref)
{
#ifdef RBT_SIZE
	for (Node *i = n_ref; i != NIL; i = i->get_par())
		i->size --;
#endif
	Node *n = n_ref,
		 *c = n->ch[n->ch[0] == NIL];
	c->copy_par(n);
	n_ref = c;
	if (n->is_red())
	{
		node_free(n);
		return;
	}

	node_free(n);

	if (c->is_red())
	{
		c->set_black();
		return;
	}
	
	n = c;

	while (n != tree_root && n->is_black())
	{
		Node *p = n->get_par(), *s = p->ch[n == p->ch[0]];
		// p: parent
		// s: sibling
		if (s->is_red())
		{
			p->set_red();
			s->set_black();
			int d = (s == p->ch[0]);
			rotate(get_ref(p), d);
			s = p->ch[!d];
		}
		// from now on, s is black
		if (s->ch[0]->is_black() && s->ch[1]->is_black())
		{
			s->set_red();
			n = p;
			continue;
		}

		int dir = (n == p->ch[1]);
		if (s->ch[!dir]->is_black())
		{
			s->ch[dir]->set_black();
			s->set_red();
			rotate(p->ch[s == p->ch[1]], !dir);
			s = p->ch[!dir];
		}

		// now, s->ch[!dir] is red

		s->copy_color(p);
		p->set_black();
		s->ch[!dir]->set_black();
		rotate(get_ref(p), dir);
		return;

#undef INIT_PS
	}

	n->set_black();
}

template <typename Key_t>
void Rbt<Key_t>::erase(Node *ptr)
{
	if (ptr == NIL || ptr == NULL)
		return;
	Node *prev = NIL,
		 *cur = ptr->ch[1];
	while (cur != NIL)
	{
		prev = cur;
		cur = cur->ch[0];
	}

	if (prev != NIL)
		ptr->key = prev->key;
	else prev = ptr;

	remove_bottom(get_ref(prev));
}

template <typename Key_t>
typename Rbt<Key_t>::Node* Rbt<Key_t>::find_ge(const Key_t &val)
{
	Node *pos = do_find_ge(tree_root, val);
	if (pos == NIL)
		return NULL;
	return pos;
}

template <typename Key_t>
typename Rbt<Key_t>::Node* Rbt<Key_t>::find_le(const Key_t &val)
{
	Node *pos = do_find_le(tree_root, val);
	if (pos == NIL)
		return NULL;
	return pos;
}

template <typename Key_t>
typename Rbt<Key_t>::Node* Rbt<Key_t>::do_find_ge(Node *root, Key_t val)
{
	Node *cur = root, *pos = NIL;

	while (cur != NIL)
	{
		if (cur->key < val)
			cur = cur->ch[1];
		else
		{
			pos = cur;
			cur = cur->ch[0];
		}
	}

	return pos;
}

template <typename Key_t>
typename Rbt<Key_t>::Node* Rbt<Key_t>::do_find_le(Node *root, Key_t val)
{
	Node *cur = root, *pos = NIL;

	while (cur != NIL)
	{
		if (cur->key <= val)
		{
			pos = cur;
			cur = cur->ch[1];
		}
		else
			cur = cur->ch[0];
	}

	return pos;
}

#ifdef RBT_DEBUG
template <typename Key_t>
int Rbt<Key_t>::do_check(Node *root)
{
	if (root == NIL)
		return 0;
	int nb[2] = {0, 0};
#ifdef RBT_SIZE
	int s = 1;
#endif
	for (int i = 0; i < 2; i ++)
		if (root->ch[i] != NIL)
		{
			Node *ch = root->ch[i];
			nb[i] = do_check(ch);
#ifdef RBT_SIZE
			s += ch->size;
#endif
			if ((i == 0 && root->key < ch->key) ||
					(i == 1 && ch->key < root->key))
				throw "partial order check error";
			if (ch->get_par() != root)
				throw "parent pointer check error";
		}
#ifdef RBT_SIZE
	if (s != root->size)
		throw "size check error";
#endif

	if (nb[0] != nb[1])
		throw "black node number check error";

	return nb[0] + root->is_black();
}

template <typename Key_t>
void Rbt<Key_t>::check() throw (const char *)
{
	do_check(tree_root);
	if (!tree_root->is_black())
		printf("root is not black\n");
	if (!NIL->is_black() || 
#ifdef RBT_SIZE
			NIL->size ||
#endif
			NIL->ch[0] != NIL || NIL->ch[1] != NIL)
		printf("NIL corrupted\n");
}

#endif // RBT_DEBUG

#ifdef RBT_SIZE
template <typename Key_t>
typename Rbt<Key_t>::Node* Rbt<Key_t>::find_kth(int k)
{
	if (k >= tree_root->size)
		return NULL;
	Node *t = tree_root;
	while (1)
	{
		if (k >= t->ch[0]->size)
		{
			if (k == t->ch[0]->size)
				return t;
			k -= t->ch[0]->size + 1;
			t = t->ch[1];
		}
		else t = t->ch[0];
	}
}
#endif // RBT_SIZE


template <typename Key_t>
void Rbt<Key_t>::do_walk(const Node *root, Walk_callback_t callback)
{
	if (root != NIL)
	{
		do_walk(root->ch[0], callback);
		callback(root->key);
		do_walk(root->ch[1], callback);
	}
}

template <typename Key_t>
void Rbt<Key_t>::walk(Walk_callback_t callback) const
{
	do_walk(tree_root, callback);
}

template <typename Key_t>
void Rbt<Key_t>::clear()
{
	do_clear(tree_root);
	tree_root = NIL;
}

template <typename Key_t>
void Rbt<Key_t>::do_clear(Node *root)
{
	if (root == NIL)
		return;
	do_clear(root->ch[0]);
	do_clear(root->ch[1]);
	node_free(root);
}

#endif

