#include <cstdio>
#include <cstdlib>
#include <utility>
#include <thread>
#include <mutex>

#define MAXTHREADNUM 100

using namespace std;
struct node
{
	std::recursive_mutex m;
	node *left, *right;
	int key, priority;
	node () : key (0), priority (0), left (nullptr), right (nullptr) { }
	node (int key, int priority) : key (key), priority (priority), left (nullptr), right (nullptr) { }
	
};
typedef node* treap;

void dumpTreap (treap out, int spacingCounter = 0)
{
	if (out)
	{
		dumpTreap (out->right, spacingCounter + 1);
		for (int i = 0; i < spacingCounter; i++) printf ("_________");
		printf ("(%d.%d)\n", out->key, out->priority);
		dumpTreap (out->left, spacingCounter + 1);
	}
}

pair<treap, treap> split (treap root, int key, treap* dupl) //операция split разделяет дерево на два по ключу, если есть совпадение со значением ключа
// - кидает эту вершину в dupl
{
	if (root == nullptr) return make_pair (nullptr, nullptr);
	
	if (root->key < key)
	{
		(*dupl) = nullptr;
		root->m.lock();
		
		pair<treap, treap> splitted = split (root->right, key, dupl);
		root->right = splitted.first;
		
		root->m.unlock();
		return make_pair (root, splitted.second);
	}
	else if (root->key > key)
	{
		(*dupl) = nullptr;
		root->m.lock();
		
		pair<treap, treap> splitted = split (root->left, key, dupl);
		root->left = splitted.second;
		
		root->m.unlock();
		return make_pair (splitted.first, root);
	}
	else
	{
		root->m.lock();
		
		(*dupl) = root;
		
		root->m.unlock();
		return make_pair (root->left, root->right);
		
	}
}

treap merge (treap left, treap right) //операция merge - сливает два дерева
{
	if (left == nullptr || right == nullptr) return right == nullptr ? left : right;
	
	left->m.lock ();
	right->m.lock ();
	
	if (left->key > right->key)
	{
		treap tmp = right;
		right = left;
		left = tmp;
		
	}
	
	if (left->priority > right->priority)
	{
		left->right = merge (left->right, right);
		//dump (left);
		
		left->m.unlock ();
		right->m.unlock ();
		return left;
	}
	else
	{
		right->left = merge (left, right->left);
		//dump (right);
		
		left->m.unlock ();
		right->m.unlock ();
		return right;
	}
}

void erase (treap& t, int key)
{
	if (t->key == key)
	{
		t = merge (t->left, t->right);
	}
	else
	{
		if (key < t->key)
		{
			erase (t->left, key);
		}
		else
		{
			erase (t->right, key);
		}
	}
}

void insert (treap& t, treap toInsert)
{
	if (t == nullptr) t = toInsert;
	else if (toInsert->priority > t->priority)
	{
		treap dupl;
		auto tmp = split (t, toInsert->key, &dupl);
		toInsert->left = tmp.first;
		toInsert->right = tmp.second;
		t = toInsert;
	}
	else
	{
		if (toInsert->key < t->key)
		{
			insert (t->left, toInsert);
		}
		else
		{
			insert (t->right, toInsert);
		}
	}
}

void testMerge (treap result, treap toMerge)
{
	result = merge (result, toMerge);
}

int main ()
{
	srand (time (NULL));
	
	auto t1 = new node (13, 3);
	auto t2 = new node (9, 7);
	auto t3 = new node (14, 4);
	auto t4 = new node (13, 8);
	
	t2 = merge (t1, t2);
	t4 = merge (t3, t4);
	
	dumpTreap (t4);
	printf ("*****************************\n");
	
	dumpTreap (t2);
	printf ("*****************************\n");
	
	auto t5 = merge (t2, t4);
	
	dumpTreap (t5);
	printf ("*****************************\n");
	
	
	treap t [MAXTHREADNUM];
	for (int i = 0; i < MAXTHREADNUM; i++)
	{
		t[i] = new node (i, rand()%100);
	}
	
	for (int i = 0; i < MAXTHREADNUM; i++)
	{
		dumpTreap (t[i]);
		printf ("@@@@@@@@@@@@@@@@@@@@@@\n");
	}
	std::thread thr [MAXTHREADNUM];
	auto result = new node (101, 101);
	for (int i = 0; i < MAXTHREADNUM; i++)
	{
		thr[i] = std::thread (testMerge, result, t[i]);
	}
	
	for (int i = 0; i < MAXTHREADNUM; i++)
	{
		thr[i].join ();
	}
	
	dumpTreap (result);
	
	printf ("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	
	treap a0 = new node();
	auto a1 = new node (13, 3);
	auto a2 = new node (9, 7);
	auto a3 = new node (14, 4);
	auto a4 = new node (11, 8);
	insert (a0, a1);
	insert (a0, a2);
	insert (a0, a3);
	insert (a0, a4);
	dumpTreap (a0);
	
	printf ("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
	
	erase (a0, 11);
	dumpTreap (a0);
	
	return 0;
}