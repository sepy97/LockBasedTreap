#include <cstdio>
#include <cstdlib>
#include <utility>
#include <thread>
#include <mutex>
#include <queue>
#include <algorithm>
#include <x86intrin.h>

#define INIT_PUSH 100
#define MAXTHREADNUM 100
#define MAX_VOLUME 100000

using namespace std;

class FastRandom {
private:
	unsigned long long rnd;
public:
	FastRandom(unsigned long long seed) { //time + threadnum
		rnd = seed;
	}
	unsigned long long rand() {
		rnd ^= rnd << 21;
		rnd ^= rnd >> 35;
		rnd ^= rnd << 4;
		return rnd;
	}
};

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
		
		right->m.unlock ();
		left->m.unlock ();
		return right;
	}
}

void erase (treap& t, int key)
{
	//проверка на NULL
	if (t != NULL)// return;
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
}

void insert (treap& t, treap toInsert)
{
	//проверить вставку по имеющимся ключу
	
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

treap toTest;

void testMerge (const int volume, int threadNum)
{
	//queue <int> testedValues;
	
	FastRandom* ran = new FastRandom (time(NULL) + threadNum);
	for (int i = 0; i < volume; i++)
	{
		int insOrDel = ran->rand()%2;
		if (insOrDel)
		{
			/*int toInsert = ran->rand()%volume; // testedValues.front ();
			testedValues.pop ();*/
			auto toAdd = new node (ran->rand()%volume, ran->rand ()%volume);
			insert (toTest, toAdd);
		}
		else
		{
			int data = ran->rand ()%volume;
			
			erase (toTest, data);
			//testedValues.push (data);
		}
	}
	
}

int main (int argc, char** argv)
{
	int maxThreads = 2;
	/*if (argc > 1)
	{
		maxThreads = atoi(argv[1]);
	}
	else
	{
		printf ("no arguments :( \n");
		return 0;
	}*/
	
	toTest = new node ();
	FastRandom* ran = new FastRandom (time(NULL));
	
	/*
	while (testedValues.size () < MAX_VOLUME + INIT_PUSH )
	{
		int toInsert = ran->rand () % ((MAX_VOLUME+ INIT_PUSH)*10);
		testedValues.push (toInsert);
	}
	*/
	
	for (int i = 0; i < INIT_PUSH; i++)
	{
		/*auto toAdd = new node (testedValues.front (), ran->rand()%(INIT_PUSH));
		testedValues.pop ();
		insert (toTest, toAdd);
		*/
		auto toAdd = new node (ran->rand()%INIT_PUSH, ran->rand ()%INIT_PUSH);
		insert (toTest, toAdd);
		
	}
	
	
	//printf ("\n %d \n", testedValues.size ());
	
	std::thread thr[maxThreads];
	
	uint64_t tick = __rdtsc ()/100000;
	
	for (int i = 0; i < maxThreads; i++)
	{
		//toTest.push (i);
		thr[i] = std::thread (testMerge, MAX_VOLUME/maxThreads, i); //testPush, &toTest, i);
	}
	
	for (int i = 0; i < maxThreads; i++)
	{
		thr[i].join ();
	}
	
	uint64_t tick2 = __rdtsc ()/100000;
	printf ("%llu\n", tick2 - tick);
	
	
	return 0;
}