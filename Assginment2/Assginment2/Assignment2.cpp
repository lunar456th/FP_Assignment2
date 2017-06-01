#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <conio.h>

#define NODESIZE 5
#define EPS 0.0001

#define BUCKETSIZE 3
#define INIT_GLOB_DEPTH 0
#define SHOW_DUPLICATE_BUCKETS 0

using namespace std;

struct BPlusTreeNode // sizeof(*BPlusTreeNode) == 16bytes
{
	float * data;
	BPlusTreeNode **child_ptr;
	BPlusTreeNode *parent_ptr;
	bool leaf;
	int n;
}*root = NULL, *np = NULL, *x = NULL;

BPlusTreeNode * init();
void insert(float a, int b);
void insertionSort(BPlusTreeNode * x, float a, int b);
bool search(BPlusTreeNode * p, float a);
void traverse(BPlusTreeNode * p);
float split_child(BPlusTreeNode * x, int i);
BPlusTreeNode * findKthNode(BPlusTreeNode * x, int k);
int writeIndexFile(FILE * fout, BPlusTreeNode * x);

class Bucket { // sizeof(Bucket) = 20bytes
	int depth, size;
	map<int, int> values;
public:
	Bucket(int depth, int size);
	int insert(int key, int value);
	bool search(int key);
	int isFull(void);
	int isEmpty(void);
	int getDepth(void);
	int increaseDepth(void);
	int decreaseDepth(void);
	map<int, int> copy(void);
	void clear(void);
	void display(void);
	int writeHashFile(FILE * fout);
};


class Directory {
	int global_depth, bucket_size;
	vector<Bucket*> buckets;
	int hash(int n);
	int pairIndex(int bucket_no, int depth);
	void grow(void);
	void shrink(void);
	void split(int bucket_no);
	void merge(int bucket_no);
	string bucket_id(int n);
public:
	Directory(int depth, int bucket_size);
	void insert(int key, int value, bool reinserted);
	bool search(int key);
	void display(bool duplicates);
	int writeHashFile(FILE * fout);
};






int main(void)
{
	int id, n, i, block, k;
	float score;
	string choice;
	ifstream fin("input.txt"); // input.txt�� ��� ������, input2.txt�� ū ������
	FILE * fidx = fopen("Students_score.idx", "wb");
	FILE * fhash = fopen("Students.hash", "wb");

	root = init();
	Directory d(INIT_GLOB_DEPTH, BUCKETSIZE);
	//cout << endl << "Initialized directory structure" << endl;

	//cout << "enter the no of elements to be inserted\n";

	fin >> n;
	for (i = 0; i < n; i++)
	{
		//cout << "�й� ��� ��Ϲ�ȣ ���� �� ���ڵ徿 �Է��ϼ���.\n";
		fin >> id >> score >> block;
		if (!search(root, score)) // B+Ʈ�� �ߺ� ����
		{
			insert(score, block); // B+Ʈ�� ����
		}
		//if (!d.search(id)) // Ȯ���ؽ� �ߺ� ����
		//{
			d.insert(id, block, 0); // Ȯ�� �ؽ� ����
		//}
		//printf("%d\n", i);
	}

	// make .hash file
	if (d.writeHashFile(fhash) == -1)
	{
		cout << ".hash file error." << endl;
	}

	// make .idx file
	if (writeIndexFile(fidx, root) == -1)
	{
		cout << ".idx file error." << endl;
	}


	traverse(root); // B+Ʈ�� ���
	d.display(SHOW_DUPLICATE_BUCKETS); // Ȯ���ؽ� ���
	// printf("sizeof(Bucket) = %d\nsizeof(root->child_ptr[0]) = %d\n", sizeof(Bucket), sizeof(*root->child_ptr[0]));


	// print k-th leaf node
	cout << "k를 입력하시오: ";
	cin >> k;
	BPlusTreeNode * kthNode = findKthNode(root, k);
	for (int s = 0; s < kthNode->n; s++)
	{
		printf("%1.f(%0.d) ", kthNode->data[s], kthNode->child_ptr[s]);
	}
	cout << endl;
	_getch();
	return 0;
}





BPlusTreeNode * init()
{
	int i;
	np = new BPlusTreeNode;	np->data = new float[NODESIZE - 1];
	np->child_ptr = new BPlusTreeNode *[NODESIZE];
	np->parent_ptr = NULL;
	np->leaf = true;
	np->n = 0;
	for (i = 0; i < NODESIZE; i++)
	{
		np->child_ptr[i] = NULL;
	}
	return np;
}

bool search(BPlusTreeNode * p, float a)
{
	int i;
	if (p->n == 0)
	{
		return false;
	}
	while (true)
	{
		if (p->leaf)
		{
			for (i = 0; i < p->n; i++)
			{
				if (fabsf(a - p->data[i]) < EPS)
				{
					return true;
				}
			}
			return false;
		}
		else
		{
			for (i = 0; i < p->n; i++)
			{
				if (fabsf(a - p->data[i]) < EPS)
				{
					return true;
				}
				else if (a < p->data[i])
				{
					p = p->child_ptr[i];
					break;
				}
			}
			if (i == p->n && p->child_ptr[i] != NULL)
			{
				p = p->child_ptr[i];
			}
		}
	}
}

void traverse(BPlusTreeNode *p)
{
	cout << endl;
	int i;
	for (i = 0; i < p->n; i++)
	{
		if (p->leaf == false)
		{
			traverse(p->child_ptr[i]);
		}
		cout << " " << p->data[i];
		if (p->leaf && p->child_ptr[i] != NULL)
		{
			printf("(%.0d)", (int)p->child_ptr[i]);
		}
	}
	if (p->leaf == false && p->child_ptr[i] != NULL)
	{
		traverse(p->child_ptr[i]);
	}
	cout << endl;
}

float split_child(BPlusTreeNode *x, int i) // x라는 노드를 주면 걔를 두개~세개로 쪼개서 중간값 반환
{
	int j;
	float mid;
	BPlusTreeNode *np1, *np3, *y;
	np3 = init();
	np3->leaf = true;
	if (i == -1)
	{
		mid = x->data[(NODESIZE / 2 - 1)];
		np1 = init();
		np1->leaf = false;
		x->parent_ptr = np1;
		np3->parent_ptr = np1;
		//x->leaf = true;
		for (j = (NODESIZE / 2); j < NODESIZE - 1; j++)
		{
			np3->data[j - (NODESIZE / 2)] = x->data[j];
			np3->child_ptr[j - (NODESIZE / 2)] = x->child_ptr[j];
			np3->n++;
			x->data[j] = 0;
			x->child_ptr[j] = 0;
			x->n--;
		}
		np1->data[0] = mid;
		np1->child_ptr[np1->n] = x;
		np1->child_ptr[np1->n + 1] = np3;
		np1->n++;
		if (!x->leaf)
		{
			x->child_ptr[NODESIZE - 1] = np3; // ���� ��峢�� �ܹ��� ����
		}
		root = np1;
	}
	else
	{
		y = x->child_ptr[i];
		y->parent_ptr = x;
		np3->parent_ptr = x;
		mid = y->data[(NODESIZE / 2 - 1)];
		for (j = (NODESIZE / 2); j < NODESIZE - 1; j++)
		{
			np3->data[j - (NODESIZE / 2)] = y->data[j];
			np3->child_ptr[j - (NODESIZE / 2)] = y->child_ptr[j];
			np3->n++;
			y->data[j] = 0;
			y->child_ptr[j] = 0;
			y->n--;
		}
		for (int k = NODESIZE - 2; k > i; k--)
		{
			if (x->child_ptr[k] != NULL)
			{
				x->child_ptr[k + 1] = x->child_ptr[k];
			}
		}
		x->child_ptr[i + 1] = np3;
		y->child_ptr[NODESIZE - 1] = np3; // ���� ��峢�� �ܹ��� ����
	}
	return mid;
}

void insertionSort(BPlusTreeNode * x, float a, int b) // 한 노드 내에서 값 밀어내고 삽입하는거
{
	int i, j;
	for (i = 0; i < x->n; i++)
	{
		if (a < x->data[i])
		{
			for (j = x->n - 1; j >= i; j--)
			{
				x->data[j + 1] = x->data[j];
				if (x->leaf == true)
				{
					x->child_ptr[j + 1] = x->child_ptr[j];
				}
			}
			x->data[i] = a;
			if (x->leaf == true)
			{
				x->child_ptr[i] = (BPlusTreeNode *)b;
			}
			break;
		}
	}
	if (i == x->n)
	{
		x->data[i] = a;
		if (x->leaf == true)
		{
			x->child_ptr[i] = (BPlusTreeNode *)b;
		}
	}
}

void insert(float a, int b)
{
	int i;
	float temp;
	x = root;
	if (x->leaf)
	{
		if (x->n != NODESIZE - 1)
		{
			insertionSort(x, a, b);
			x->n++;
			return;
		}
		else
		{
			temp = split_child(x, -1);
			x = root;
			if (a >= x->data[0])
			{
				for (i = 0; i < (x->n); i++)
				{
					if ((a > x->data[i]) && (a < x->data[i + 1]))
					{
						i++;
						break;
					}
				}
			}
			x = x->child_ptr[i];
		}
	}
	else
	{
		while (!x->leaf)
		{
			if (a >= x->data[0])
			{
				for (i = 0; i < (x->n); i++)
				{
					if ((a > x->data[i]) && (a < x->data[i + 1]))
					{
						i++;
						break;
					}
				}
			}
			if (x->child_ptr[i] != NULL && (x->child_ptr[i])->n == NODESIZE - 1)
			{
				temp = split_child(x, i);
				if (x->n == NODESIZE - 1)
				{
					float temp2 = split_child(x, -1);
					// insertionSort(root, temp2, b);
				}
				insertionSort(x, temp, b);
				x->n++;
				//continue;
				break;
			}
			else if (x->child_ptr[i] != NULL)
			{
				x = x->child_ptr[i];
			}
		}
	}
	insertionSort(x, a, b);
	x->n++;

}

BPlusTreeNode * findKthNode(BPlusTreeNode * x, int k)
{
	BPlusTreeNode * temp = x;
	while (temp->child_ptr[0] != NULL && temp->leaf == false)
	{
		temp = temp->child_ptr[0];
	}
	for (int i = 0; i < k; i++)
	{
		temp = temp->child_ptr[NODESIZE - 1];
	}
	return temp;
}

int writeIndexFile(FILE * fout, BPlusTreeNode * x)
{
	for (BPlusTreeNode * firstNode = findKthNode(root, 0); firstNode->child_ptr[NODESIZE - 1] != NULL; firstNode = firstNode->child_ptr[NODESIZE - 1])
	{
		if (fwrite(firstNode, 4096, 1, fout) == -1)
		{
			return -1;
		}
	}
	return 0;
}





//

Directory::Directory(int depth, int bucket_size)
{
	this->global_depth = depth;
	this->bucket_size = bucket_size;
	for (int i = 0; i < 1 << depth; i++)
	{
		buckets.push_back(new Bucket(depth, bucket_size));
	}
}

int Directory::hash(int n)
{
	return n&((1 << global_depth) - 1);
}

int Directory::pairIndex(int bucket_no, int depth)
{
	return bucket_no ^ (1 << (depth - 1));
}

void Directory::grow(void)
{
	for (int i = 0; i < 1 << global_depth; i++)
		buckets.push_back(buckets[i]);
	global_depth++;
}

void Directory::shrink(void)
{
	int flag = 1, i;
	for (i = 0; i < buckets.size(); i++)
	{
		if (buckets[i]->getDepth() == global_depth)
		{
			flag = 0;
			return;
		}
	}
	global_depth--;
	for (i = 0; i < 1 << global_depth; i++)
		buckets.pop_back();
}

void Directory::split(int bucket_no)
{
	int local_depth, pair_index, index_diff, dir_size, i;
	map<int, int> temp;
	map<int, int>::iterator it;

	local_depth = buckets[bucket_no]->increaseDepth();
	if (local_depth>global_depth)
		grow();
	pair_index = pairIndex(bucket_no, local_depth);
	buckets[pair_index] = new Bucket(local_depth, bucket_size);
	temp = buckets[bucket_no]->copy();
	buckets[bucket_no]->clear();
	index_diff = 1 << local_depth;
	dir_size = 1 << global_depth;
	for (i = pair_index - index_diff; i >= 0; i -= index_diff)
		buckets[i] = buckets[pair_index];
	for (i = pair_index + index_diff; i<dir_size; i += index_diff)
		buckets[i] = buckets[pair_index];
	for (it = temp.begin(); it != temp.end(); it++)
		insert((*it).first, (*it).second, 1);
}

void Directory::merge(int bucket_no)
{
	int local_depth, pair_index, index_diff, dir_size, i;

	local_depth = buckets[bucket_no]->getDepth();
	pair_index = pairIndex(bucket_no, local_depth);
	index_diff = 1 << local_depth;
	dir_size = 1 << global_depth;

	if (buckets[pair_index]->getDepth() == local_depth)
	{
		buckets[pair_index]->decreaseDepth();
		delete(buckets[bucket_no]);
		buckets[bucket_no] = buckets[pair_index];
		for (i = bucket_no - index_diff; i >= 0; i -= index_diff)
			buckets[i] = buckets[pair_index];
		for (i = bucket_no + index_diff; i<dir_size; i += index_diff)
			buckets[i] = buckets[pair_index];
	}
}

string Directory::bucket_id(int n)
{
	int d;
	string s;
	d = buckets[n]->getDepth();
	s = "";
	while (n>0 && d>0)
	{
		s = (n % 2 == 0 ? "0" : "1") + s;
		n /= 2;
		d--;
	}
	while (d>0)
	{
		s = "0" + s;
		d--;
	}
	return s;
}

void Directory::insert(int key, int value, bool reinserted)
{
	int bucket_no = hash(key);
	int status = buckets[bucket_no]->insert(key, value);
	if (status == 1)
	{
		if (!reinserted)
		{
			//cout << "Inserted key " << key << " in bucket " << bucket_id(bucket_no) << endl;
		}
		else
		{
			//cout << "Moved key " << key << " to bucket " << bucket_id(bucket_no) << endl;
		}
	}
	else if (status == 0)
	{
		split(bucket_no);
		insert(key, value, reinserted);
	}
	else
	{
		//cout << "Key " << key << " already exists in bucket " << bucket_id(bucket_no) << endl;
	}
}

bool Directory::search(int key)
{
	int bucket_no = hash(key);
	//cout << "Searching key " << key << " in bucket " << bucket_id(bucket_no) << endl;
	return buckets[bucket_no]->search(key);
}

void Directory::display(bool duplicates)
{
	int i, j, d;
	string s;
	set<string> shown;
	//cout << "Global depth : " << global_depth << endl;
	for (i = 0; i<buckets.size(); i++)
	{
		d = buckets[i]->getDepth();
		s = bucket_id(i);
		if (duplicates || shown.find(s) == shown.end())
		{
			shown.insert(s);
			for (j = d; j <= global_depth; j++)
				cout << " ";
			cout << s << " ~~~> ";
			buckets[i]->display();
		}
	}
}

int Directory::writeHashFile(FILE * fout) {
	int i, j, d;
	string s;
	set<string> shown;
	//cout << "Global depth : " << global_depth << endl;
	for (i = 0; i<buckets.size(); i++)
	{
		d = buckets[i]->getDepth();
		s = bucket_id(i);
		if (shown.find(s) == shown.end())
		{
			shown.insert(s);
			for (j = d; j <= global_depth; j++)
				cout << " ";
//			cout << s << " ~~~> ";
			if (fwrite(&s, 4096, 1, fout) == -1)
			{
				return -1;
			}
			if (buckets[i]->writeHashFile(fout) == -1)
			{
				return -1;
			}
		}
	}
	return 0;

}





Bucket::Bucket(int depth, int size)
{
	this->depth = depth;
	this->size = size;
}

int Bucket::insert(int key, int value)
{
	map<int, int>::iterator it;
	it = values.find(key);
	if (it != values.end())
		return -1;
	if (isFull())
		return 0;
	values[key] = value;
	return 1;
}

bool Bucket::search(int key)
{
	map<int, int>::iterator it;
	it = values.find(key);
	if (it != values.end())
	{
		//cout << "Value = " << it->second << endl;
		return true;
	}
	else
	{
		//cout << "This key does not exists" << endl;
		return false;
	}
}

int Bucket::isFull(void)
{
	if (values.size() == size)
		return 1;
	else
		return 0;
}

int Bucket::isEmpty(void)
{
	if (values.size() == 0)
		return 1;
	else
		return 0;
}

int Bucket::getDepth(void)
{
	return depth;
}

int Bucket::increaseDepth(void)
{
	depth++;
	return depth;
}

int Bucket::decreaseDepth(void)
{
	depth--;
	return depth;
}

map<int, int> Bucket::copy(void)
{
	map<int, int> temp(values.begin(), values.end());
	return temp;
}

void Bucket::clear(void)
{
	values.clear();
}

void Bucket::display()
{
	map<int, int>::iterator it;
	for (it = values.begin(); it != values.end(); it++)
		cout << it->first << "(" << it->second << ") ";
	cout << endl;
}

int Bucket::writeHashFile(FILE * fout)
{
	if (fwrite(this, 4096, 1, fout) == -1)
	{
		return -1;
	}
	return 0;
}