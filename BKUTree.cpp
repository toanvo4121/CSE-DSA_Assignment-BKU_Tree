#include <iostream>
#include <cstdlib>
#include <queue>
#include <string>
#include <vector>
#include <stdexcept>
#include <math.h>
using namespace std;


template <class K, class V>
class BKUTree
{
public:
	class AVLTree;
	class SplayTree;

	class Entry
	{
	public:
		K key;
		V value;

		Entry(K key, V value) : key(key), value(value) {}
	};

private:
	AVLTree *avl;
	SplayTree *splay;
	queue<K> keys;
	int maxNumOfKeys;

private:
	bool searchKeyQueue(K key)
	{
		if (this->keys.empty()) return false;
		bool isExist = false;
		queue<K> tempQueue;
		for (int i = 0; i < this->maxNumOfKeys; ++i) {
			K curKey = this->keys.front();
			if (curKey == key) isExist = true;
			tempQueue.push(curKey);
			this->keys.pop();
		}
		this->keys = tempQueue;
		return isExist;
	}

	void delMiddleElementOfQueue(K key)
	{
		queue<K> newQueue;
		while (!this->keys.empty())
		{
			if (this->keys.front() != key)
			{
				K oldKey = this->keys.front();
				newQueue.push(oldKey);
			}
			this->keys.pop();
		}
		while (!newQueue.empty())
		{
			K oldKey = newQueue.front();
			this->keys.push(oldKey);
			newQueue.pop();
		}
	}

	void pushKey(K key)
	{
		int size = this->keys.size();
		if (size == this->maxNumOfKeys)
			this->keys.pop();
		this->keys.push(key);
	}

public:
	BKUTree(int maxNumOfKeys = 5)
	{
		this->avl = new AVLTree();
		this->splay = new SplayTree();
		this->maxNumOfKeys = maxNumOfKeys;
	}
	~BKUTree()
	{
		this->clear();
		this->splay = NULL;
		this->avl = NULL;
		this->maxNumOfKeys = 0;
	}

	void add(K key, V value)
	{
		typename AVLTree::Node* newNode = this->avl->root;
		this->splay->add(key, value);
		Entry* newEntry = new Entry(key, value);
		this->avl->root = this->avl->addRec(this->avl->root, newEntry, newNode);
		this->splay->root->corr = newNode;
		newNode->corr = this->splay->root;

		this->pushKey(key);
		delete newEntry;
		newEntry = NULL;
	}
	void remove(K key)
	{
		this->avl->remove(key);
		this->splay->remove(key);
		if (this->searchKeyQueue(key))
		{
			this->delMiddleElementOfQueue(key);
			if (this->splay->root != NULL)
				this->keys.push(this->splay->root->entry->key);
		}
	}
	V search(K key, vector<K> &traversedList)
	{
		// Step 1:
		if (this->splay->root->entry->key == key)
		{
			return this->splay->root->entry->value;
		}

		// Step 2:
		if (this->searchKeyQueue(key))
		{
			typename SplayTree::Node *foundNode = this->splay->findBKU(key, traversedList);
			this->splay->Splay(foundNode, 1);
			this->pushKey(key);
			return foundNode->entry->value;
		}
		// Step 3:
		else
		{
			typename AVLTree::Node *avl_r = this->splay->root->corr; // a
			typename AVLTree::Node *traversedNode = avl_r;
			traversedNode = this->avl->findBKU(avl_r, key, traversedList, NULL); // b
			if (!traversedNode)
			{
				traversedNode = this->avl->findBKU(this->avl->root, key, traversedList, avl_r);
				if (!traversedNode)
					throw "Not found";
			}
			typename SplayTree::Node *splayNode = traversedNode->corr;
			this->splay->Splay(splayNode, 1);
			this->pushKey(key);
			return traversedNode->entry->value;
		}
	}

	void traverseNLROnAVL(void (*func)(K key, V value))
	{
		this->avl->traverseNLR(func);
	}
	void traverseNLROnSplay(void (*func)(K key, V value))
	{
		this->splay->traverseNLR(func);
	}
	void clear()
	{
		this->avl->clear();
		this->splay->clear();
		while (!this->keys.empty())
		{
			this->keys.pop();
		}
	}

	class SplayTree
	{
	public:
		class Node
		{
			Entry *entry;
			Node *left;
			Node *right;
			Node *parent;
			typename AVLTree::Node *corr;
			friend class BKUTree;

			Node(Entry *entry = NULL, Node *left = NULL, Node *right = NULL, Node *parent = NULL)
			{
				this->entry = entry;
				this->left = left;
				this->right = right;
				this->parent = parent;
				this->corr = NULL;
			}
			~Node() {
				free(this->entry);
				this->entry = NULL;
				this->left = NULL;
				this->right = NULL;
				this->parent = NULL;
				this->corr = NULL;
			}
		};

	public:
		Node *root;
		friend class BKUTree;

	private:
		void zig(Node *pNode)
		{
			Node *parNode = pNode->parent;
			if (parNode->left == pNode)
			{
				Node *B = pNode->right;

				pNode->parent = NULL;
				pNode->right = parNode;

				parNode->parent = pNode;
				parNode->left = B;

				if (B != NULL)
					B->parent = parNode;
			}
			else
			{
				Node *B = pNode->left;

				pNode->parent = NULL;
				pNode->left = parNode;

				parNode->parent = pNode;
				parNode->right = B;

				if (B != NULL)
					B->parent = parNode;
			}
		}

		void zig_zig(Node *pNode)
		{
			Node *parNode = pNode->parent;
			Node *grandNode = parNode->parent;
			if (parNode->left == pNode)
			{
				Node *B = pNode->right;
				Node *C = parNode->right;

				pNode->parent = grandNode->parent;
				pNode->right = parNode;

				parNode->parent = pNode;
				parNode->left = B;
				parNode->right = grandNode;

				grandNode->parent = parNode;
				grandNode->left = C;

				if (pNode->parent != NULL)
				{
					if (pNode->parent->left == grandNode)
						pNode->parent->left = pNode;
					else
						pNode->parent->right = pNode;
				}

				if (B != NULL)
					B->parent = parNode;
				if (C != NULL)
					C->parent = grandNode;
			}
			else
			{
				Node *B = parNode->left;
				Node *C = pNode->left;

				pNode->parent = grandNode->parent;
				pNode->left = parNode;

				parNode->parent = pNode;
				parNode->left = grandNode;
				parNode->right = C;

				grandNode->parent = parNode;
				grandNode->right = B;

				if (pNode->parent != NULL)
				{
					if (pNode->parent->left == grandNode)
						pNode->parent->left = pNode;
					else
						pNode->parent->right = pNode;
				}

				if (B != NULL)
					B->parent = grandNode;
				if (C != NULL)
					C->parent = parNode;
			}
		}

		void zig_zag(Node *pNode)
		{
			Node *parNode = pNode->parent;
			Node *grandNode = parNode->parent;
			if (parNode->right == pNode)
			{
				Node *B = pNode->left;
				Node *C = pNode->right;

				pNode->parent = grandNode->parent;
				pNode->left = parNode;
				pNode->right = grandNode;

				parNode->parent = pNode;
				parNode->right = B;

				grandNode->parent = pNode;
				grandNode->left = C;

				if (pNode->parent != NULL)
				{
					if (pNode->parent->left == grandNode)
						pNode->parent->left = pNode;
					else
						pNode->parent->right = pNode;
				}

				if (B != NULL)
					B->parent = parNode;
				if (C != NULL)
					C->parent = grandNode;
			}
			else
			{
				Node *B = pNode->left;
				Node *C = pNode->right;

				pNode->parent = grandNode->parent;
				pNode->left = grandNode;
				pNode->right = parNode;

				parNode->parent = pNode;
				parNode->left = C;

				grandNode->parent = pNode;
				grandNode->right = B;

				if (pNode->parent != NULL)
				{
					if (pNode->parent->left == grandNode)
						pNode->parent->left = pNode;
					else
						pNode->parent->right = pNode;
				}

				if (B != NULL)
					B->parent = grandNode;
				if (C != NULL)
					C->parent = parNode;
			}
		}

		void Splay(Node* &node, bool opt){
			while (node->parent != NULL)
			{
				Node *parNode = node->parent;
				Node *grandNode = parNode->parent;
				if (grandNode == NULL)
					zig(node);
				else if (grandNode->left == parNode && parNode->left == node)
					zig_zig(node);
				else if (grandNode->right == parNode && parNode->right == node)
					zig_zig(node);
				else
					zig_zag(node);
				if (opt) break;
			}
			if(node->parent == NULL || opt == 0) this->root = node;
		}

		void split(Node* &x, Node* &s, Node* &t){
			Splay(x, 0);
			if (x->right) {
				t = x->right;
				t->parent = NULL;
			}
			else {
				t = NULL;
			}
			s = x;
			s->right = NULL;
			x = NULL;
		}

		Node* join(Node* s, Node* t){
			if (!s) {
				return t;
			}
			if (!t) {
				return s;
			}
			Node* x = subtree_max(s);
			Splay(x, 0);
			x->right = t;
			t->parent = x;
			return x;
		}

		Node *subtree_max(Node *subTree)
		{
			Node *curr = subTree;
			while (curr->right != NULL)
				curr = curr->right;
			return curr;
		}

		Node *find(K key)
		{
			Node *ret = NULL;
			Node *curr = this->root;
			Node *prev = NULL;
			while (curr != NULL)
			{
				prev = curr;
				if (key < curr->entry->key) {
					curr = curr->left;
				}
				else if (key > curr->entry->key) {
					curr = curr->right;
				}
				else
				{
					ret = curr;
					break;
				}
			}
			return ret;
		}

		Node *findBKU(K key, vector<K>& traversedList)
		{
			Node *ret = NULL;
			Node *curr = this->root;
			while (curr != NULL)
			{
				if (key < curr->entry->key) {
					traversedList.push_back(curr->entry->key);
					curr = curr->left;
				}
				else if (key > curr->entry->key) {
					traversedList.push_back(curr->entry->key);
					curr = curr->right;
				}
				else
				{
					ret = curr;
					break;
				}
			}
			return ret;
		}

		void deleteNodeHelper(Node* node, K key){
			Node* x = NULL;
			Node* t, *s;
			while(node != NULL){
				if(node->entry->key == key){
					x = node;
				}
				if(node->entry->key < key){
					node = node->right;
				}
				else{
					node = node->left;
				}
			}
			if(x == NULL){
				throw "Not found" ;
			}
			split(x, s, t); // split the tree
			if (s->left){ // remove x
				s->left->parent = NULL;
			}
			root = join(s->left, t);
			if (s!=NULL) {
				delete s;
				s = NULL;
			}
		}

		void traverseSplayTreeRec(Node *pNode, void (*func)(K key, V value))
		{
			if (pNode == NULL)
				return;
			func(pNode->entry->key, pNode->entry->value);
			this->traverseSplayTreeRec(pNode->left, func);
			this->traverseSplayTreeRec(pNode->right, func);
		}

	public:
		SplayTree() : root(NULL){};
		~SplayTree() { this->clear(); };

		void add(K key, V value)
		{
			Entry *entry = new Entry(key, value);
			this->add(entry);
			delete entry;
			entry = NULL;
		}
		void add(Entry *entry)
		{
			if (this->root == NULL)
			{
				root = new Node();
				root->entry = new Entry(entry->key, entry->value);
				return;
			}
			Node *curr = this->root;
			while (curr != NULL)
			{
				if (entry->key == curr->entry->key) {
					throw "Duplicate key";
				}
				else if (entry->key < curr->entry->key)
				{
					if (curr->left == NULL)
					{
						Node *newNode = new Node();
						newNode->entry = new Entry(entry->key, entry->value);
						curr->left = newNode;
						newNode->parent = curr;
						Splay(newNode, 0);
						return;
					}
					else
						curr = curr->left;
				}
				else if (entry->key > curr->entry->key)
				{
					if (curr->right == NULL)
					{
						Node *newNode = new Node();
						newNode->entry = new Entry(entry->key, entry->value);
						curr->right = newNode;
						newNode->parent = curr;
						Splay(newNode, 0);
						return;
					}
					else
						curr = curr->right;
				}
				else
				{
					Splay(curr, 0);
					return;
				}
			}
		}
		void remove(K key)
		{
			Node* node= this->root;
			while (node!=NULL){
				if (node->entry->key == key) break;
				else {
					if (node->entry->key > key)	node = node->left;
					else node = node->right;
				}
			}
			if (!node) throw "Not found";
			else this->deleteNodeHelper(node, key);
		}
		V search(K key)
		{
			Node *ret = NULL;
			Node *curr = this->root;
			Node *prev = NULL;
			while (curr != NULL)
			{
				prev = curr;
				if (key < curr->entry->key)
					curr = curr->left;
				else if (key > curr->entry->key)
					curr = curr->right;
				else
				{
					ret = curr;
					break;
				}
			}
			if (ret != NULL)
				Splay(ret, 0);
			else if (prev != NULL)
				Splay(prev, 0);
			if (ret != NULL)
				return ret->entry->value;
			else
			{
				throw "Not found";
			}
		}
		void traverseNLR(void (*func)(K key, V value))
		{
			this->traverseSplayTreeRec(this->root, func);
		}
		void clear()
		{
			while (this->root != NULL)
				remove(this->root->entry->key);
		}
	};

	class AVLTree
	{
	public:
		class Node
		{
			Entry *entry;
			Node *left;
			Node *right;
			int balance;
			typename SplayTree::Node *corr;
			friend class BKUTree;

			Node(Entry *entry = NULL, Node *left = NULL, Node *right = NULL)
			{
				this->entry = entry;
				this->left = left;
				this->right = right;
				this->corr = NULL;
			}
			~Node() {
				delete this->entry;
				this->entry = NULL;
				this->left = NULL;
				this->right = NULL;
				this->corr = NULL;
			}
		};

	private: // methods
		int getHeightRec(Node *pNode)
		{
			if (pNode == NULL)
				return 0;
			int left_h = this->getHeightRec(pNode->left);
			int right_h = this->getHeightRec(pNode->right);
			return (left_h > right_h ? left_h : right_h) + 1;
		}

		Node *rightRotate(Node *pNode)
		{
			Node *tempPtr = pNode->left;
			pNode->left = tempPtr->right;
			tempPtr->right = pNode;
			return tempPtr;
		}

		Node *leftRotate(Node *pNode)
		{
			Node *tempPtr = pNode->right;
			pNode->right = tempPtr->left;
			tempPtr->left = pNode;
			return tempPtr;
		}

		int getBalance(Node *pNode)
		{
			if (pNode == NULL)
				return 0;
			return getHeightRec(pNode->right) - getHeightRec(pNode->left);
		}

		Node *addRec(Node *pNode, Entry *entry, Node *&newNode)
		{
			if (pNode == NULL)
			{
				Node *addNode = new Node();
				addNode->entry = new Entry(entry->key, entry->value);
				newNode = addNode;
				return addNode;
			}

			if (entry->key < pNode->entry->key) pNode->left = this->addRec(pNode->left, entry, newNode);
			else {
				if (entry->key == pNode->entry->key) throw "Duplicate key";
				else pNode->right = this->addRec(pNode->right, entry, newNode);
			}

			int balance= getBalance(pNode);

			if(balance==-1){
				pNode->balance=-1;
				return pNode;
			}
			if(balance==1){
				pNode->balance=1;
				return pNode;
			}

			// Left Left Case
			if (balance < -1 && entry->key < pNode->left->entry->key){
				pNode->balance=0;
				pNode->left->balance=0;
				return rightRotate(pNode);
			}

			// Right Right Case
			if (balance > 1 && entry->key > pNode->right->entry->key){
				pNode->balance=0;
				pNode->right->balance=0;
				return leftRotate(pNode);
			}

			// Left Right Case
			if (balance < -1 && entry->key > pNode->left->entry->key){
				pNode->left->balance=0;
				pNode->left = leftRotate(pNode->left);
				pNode->balance=0;
				pNode->left->balance=0;
				return rightRotate(pNode);
			}

			// Right Left Case
			if (balance > 1 && entry->key < pNode->right->entry->key)
			{
				pNode->right->balance=0;
				pNode->right = rightRotate(pNode->right);
				pNode->balance=0;
				pNode->right->balance=0;
				return leftRotate(pNode);
			}
			return pNode;
		}

		bool find(Node *pNode, K key)
		{
			if (pNode == NULL)
				return false;
			else if (pNode->entry->key == key)
				return true;
			else if (pNode->entry->key > key)
				return this->find(pNode->left, key);
			else
				return this->find(pNode->right, key);
		}

		Node* findBKU(Node *pNode, K key, vector<K> &traversedList, Node *avl_r)
		{
			if (pNode == NULL)
				return NULL;
			else if (pNode == avl_r)
				return NULL;
			else if (pNode->entry->key == key)
			{
				return pNode;
			}
			else if (pNode->entry->key > key)
			{
				traversedList.push_back(pNode->entry->key);
				return this->findBKU(pNode->left, key, traversedList, avl_r);
			}
			else
			{
				traversedList.push_back(pNode->entry->key);
				return this->findBKU(pNode->right, key, traversedList, avl_r);
			}
		}

		V searchRec(Node *pNode, K key)
		{
			if (pNode == NULL)
			{
				throw "Not found";
			}
			else if (pNode->entry->key == key)
				return pNode->entry->value;
			else if (pNode->entry->key > key)
				return searchRec(pNode->left, key);
			else
				return searchRec(pNode->right, key);
		}

		Node *maxValueNode(Node* root) {
			Node* curr = root;
			while (curr->right != NULL) {
				if (curr->right->right==NULL) {
					if (curr->left!=NULL) curr->balance = -1;
					else curr->balance = 0;
				}
				curr = curr->right;
			}
			return curr;
		}

		Node* deleteNode(Node* root, K key) {
			if (root == NULL)
				return root;
			if (key < root->entry->key)
				root->left = deleteNode(root->left, key);
			else {
				if (key > root->entry->key)	root->right = deleteNode(root->right, key);
				else
				{
					Node* temp= NULL;
					if ((root->left == NULL) || (root->right == NULL))
					{
						temp= root->left ?root->left :root->right;
						if (temp == NULL){
							temp = root;
							root = NULL;
						}
						else{	// TH2: co 1 la ( trai hoac phai)
							*root = *temp;
							if (root->corr!=NULL) root->corr->corr = root;
						}
					}
					else // Node la 1 cay
					{
						temp = maxValueNode(root->left);
						root->entry = temp->entry;
						root->corr = temp->corr;
						if (root->corr!=NULL) root->corr->corr = root;
						root->left = deleteNode(root->left, temp->entry->key);
					}
				}
			}

			if (root == NULL)
				return root;


			// kiem tra balance
			int balance = getBalance(root);

			if (balance==-1){
				root->balance = -1;
				return root;
			}
			if (balance==1) {
				root->balance = 1;
				return root;
			}

			// Left Left Case
			if (balance < -1 && getBalance(root->left) <= 0) {
				root->balance=0;
				root->left->balance=0;
				return rightRotate(root);
			}

			// Right Right Case
			if (balance > 1 && getBalance(root->right)>=0){
				root->balance=0;
				root->right->balance=0;
				return leftRotate(root);
			}

			// Left Right Case
			if (balance < -1 && getBalance(root->left)>0){
				root->left->balance=-1;
				root->left = leftRotate(root->left);
				root->balance=0;
				root->left->balance=0;
				return rightRotate(root);
			}

			// Right Left Case
			if (balance > 1 && key < getBalance(root->right)<0)
			{
				root->right->balance=1;
				root->right = rightRotate(root->right);
				root->balance=0;
				root->right->balance=0;
				return leftRotate(root);
			}

			return root;
		}

		void traverseAVLTreeRec(Node *pNode, void (*func)(K key, V value))
		{
			if (pNode == NULL)
				return;
			func(pNode->entry->key, pNode->entry->value);
			this->traverseAVLTreeRec(pNode->left, func);
			this->traverseAVLTreeRec(pNode->right, func);
		}

	public:
		Node *root;
		friend class BKUTree;

	public:
		AVLTree() : root(NULL){};
		~AVLTree() { this->clear(); };

		void add(K key, V value)
		{
			Entry *newEntry = new Entry(key, value);
			Node* newNode = NULL;
			this->root = this->addRec(this->root, newEntry, newNode);
		}
		void add(Entry *entry)
		{
			Node* newNode = NULL;
			if (this->find(this->root, entry->key))
			{
				throw "Duplicate key";
			}
			else
				this->root = this->addRec(this->root, entry, newNode);
		}
		void remove(K key) {
			Node* t_root= this->root;
			while (t_root != NULL) {
				if (t_root->entry->key > key) t_root=t_root->left;
				else{
					if (t_root->entry->key == key) break;
					else t_root= t_root->right;
				}
			}
			if (t_root == NULL)	throw "Not found";
			else this->root = deleteNode(root, key);
		}
		V search(K key)
		{
			return this->searchRec(this->root, key);
		}
		void traverseNLR(void (*func)(K key, V value))
		{
			this->traverseAVLTreeRec(this->root, func);
		}
		void clear()
		{
			while (this->root != NULL)
				remove(this->root->entry->key);
		}
	};
};