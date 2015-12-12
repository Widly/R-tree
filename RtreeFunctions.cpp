#include "Header.h"

Rtree::Rtree() {
	root = new Node;
	root->type = LEAF;
	root->childs = NULL;
	root->objCount = 0;
	root->parent = NULL;
	curExtremum = NULL;

	m = 2; M = 4;
}

Rtree::~Rtree() {
	MemFree(root);
}

void Rtree::MemFree(Node* nodeToDelete) {
	if (nodeToDelete->type == NODE) {
		for (int i = 0; i < nodeToDelete->objCount; i++) {
			Rtree::MemFree(nodeToDelete->childs[i]);
		}
	}
	else if (nodeToDelete->type == LEAF){
		for (int i = 0; i < nodeToDelete->objCount; i++) {
			delete nodeToDelete->childs[i];
		}
	}
	delete[] nodeToDelete->childs;
	delete nodeToDelete;
}


void Rtree::InsertObject(Node* newObject) {
	InsertNode(newObject, 0);
}

bool Rtree::DeleteObject(MBR covering, int data) {
	Node* curNode = root;
	Node* objParent = NULL;
	Node* curNodeParent = NULL;
	list<Node*> objectsForInsertion;
	map<Node*, int> nodesForInsertion;
	Node* bufNode = NULL;
	int levelShift = 0; // сдвиг уровня, если был удален старый, и сделан новый корень


	// ищем лист, в котором лежит объект. Если такого нет, то выходим, иначе запускаем процедуру удаления
	objParent = _SearchObject(curNode, covering, data);
	if (objParent == NULL)
		return false;

	// ищем запись в листе об объекте и удаляем ее
	for (int i = 0; objParent->objCount; i++) {
		if (IsEqual(objParent->childs[i]->covering, covering) && objParent->childs[i]->data == data) {
			swap(objParent->childs[i], objParent->childs[objParent->objCount - 1]);
			delete objParent->childs[objParent->objCount - 1];
			objParent->objCount--;
			break;
		}
	}

	// корректировка дерева
	curNode = objParent;
	if (root->objCount == 0) {
		return true;
	}
	int curLevel = TreeLevel();

	while (curNode != root) {
		curNodeParent = curNode->parent;

		// если число потомков в текущей вершине меньше n, то необходимо изъять их, а затем снова вставить
		if (curNode->objCount < m) {

			// удаляем запись о текущей вершине в родителе
			for (int i = 0; i < curNodeParent->objCount; i++) {
				if (IsEqual(curNodeParent->childs[i]->covering, curNode->covering)) {
					swap(curNodeParent->childs[i], curNodeParent->childs[curNodeParent->objCount - 1]);
					curNodeParent->objCount--;
				}
			}

			// переносим записи текущей вершины. Если это объекты, то просто записываем их в список.
			// если это вершины, то их будет необходимо вставить на тот же уровень, на котором они находились
			for (int i = 0; i < curNode->objCount; i++) {
				switch (curNode->type) {
				case LEAF:
					objectsForInsertion.push_back(curNode->childs[i]);
					break;
				case NODE:
					nodesForInsertion.insert(pair<Node*, int>(curNode->childs[i], curLevel));
					break;
				}
			}

			delete[] curNode->childs;
			delete curNode;
			curLevel--;
		}
		else
			CalculateMBR(curNode); // иначе просто обновляем покрытие

		curNode = curNodeParent;
	}

	// мы в корне. Если у корня один потомок, то делаем его корнем, сдвигаем уровень вставки
	if (curNode->objCount == 1 && curNode->type == NODE) {
		bufNode = root;
		root = root->childs[0];
		root->parent = NULL;
		delete[] bufNode->childs;
		delete bufNode;
		levelShift++;
	}

	// вставить все обратно
	while (objectsForInsertion.empty() != true) {
		bufNode = objectsForInsertion.back();
		objectsForInsertion.pop_back();
		InsertObject(bufNode);
	}

	for (auto it = nodesForInsertion.begin(); it != nodesForInsertion.end(); ++it) {
		InsertNode((*it).first, (*it).second - levelShift);
	}
	nodesForInsertion.clear();

	CalculateMBR(root);
	return true;
}

void Rtree::SearchByArea(MBR area, list<Node*>* res) {
	if (root->childs != NULL)
		_SearchByArea(root, area, res);
}

Node* Rtree::SearchObject(MBR covering, int data) {
	if (root->childs != NULL)
		return _SearchObject(root, covering, data);

	return NULL;
}

Node* Rtree::FindExtremumByArea(int borderType) {
	curExtremum = NULL;
	return _FindExtremumByArea(root, borderType);
}

void Rtree::PrintRtree(ofstream &outputStream) {
	if (outputStream.is_open() == true) {
		if (root->childs == NULL) {
			outputStream << "tree is empty" << endl;
			return;
		}
		_PrintRtree(root, 0, outputStream);
		outputStream << endl;
	}
	else
		cout << "output stream error" << endl;
}



void Rtree::InsertNode(Node* newNode, int level) {
	Node* node;
	Node* secondNode = NULL;

	// ищем место для вставки с минимальным увеличением MBR
	node = ChooseNode(newNode, level);
	
	// если свободное место есть, то просто вставляем, иначе делим вершину на две
	if (node->objCount < M) {
		if (node->objCount == 0 && node->childs == NULL)
			node->childs = new Node*[M];

		node->childs[node->objCount] = newNode;
		newNode->parent = node;
		node->objCount++;
		CalculateMBR(node);
	}
	else
		secondNode = SplitNode(node, newNode);

	// корректируем дерево
	CorrectTree(node, secondNode);
}

void Rtree::_SearchByArea(Node* curNode, MBR area, list<Node*>* res) {
	for (int i = 0; i < curNode->objCount; i++) {
		if (IsIntersect(curNode->childs[i]->covering, area)) {
			switch (curNode->type)
			{
			case NODE:
				_SearchByArea(curNode->childs[i], area, res);
				break;
			case LEAF:
				res->push_back(curNode->childs[i]);
				break;
			}
		}
	}
}

Node* Rtree::_SearchObject(Node* curNode, MBR covering, int data) {
	Node* response = NULL;

	for (int i = 0; i < curNode->objCount; i++) {
		if (IsFullyContains(curNode->childs[i]->covering, covering)) {
			switch (curNode->type)
			{
			case NODE:
				response = _SearchObject(curNode->childs[i], covering, data);
				if (response != NULL)
					return response;
				break;
			case LEAF:
				if (IsEqual(curNode->childs[i]->covering, covering) && curNode->childs[i]->data == data)
					return curNode;
				break;
			}
		}
	}

	return NULL;
}

Node* Rtree::_FindExtremumByArea(Node* curNode, int borderType) {
	int i = 0;

	if (curNode->type == LEAF && curExtremum == NULL && curNode->objCount != 0) {
		curExtremum = curNode->childs[0];
		i++;
	}
		

	for (i; i < curNode->objCount; i++) {
		switch (curNode->type) {
		case NODE:
			_FindExtremumByArea(curNode->childs[i], borderType);
			break;
		case LEAF:
			if (Area(curNode->childs[i]->covering) < Area(curExtremum->covering)){
				if (borderType == MIN)
					curExtremum = curNode->childs[i];
			} else
				if (borderType == MAX)
					curExtremum = curNode->childs[i];
			break;
		}
	}

	return curExtremum;
}

void Rtree::_PrintRtree(Node* curNode, int level, ofstream &outputStream) {
	for (int i = 0; i < level; i++) {
		outputStream << "\t";
	}

	switch (curNode->type) {
	case NODE:
		outputStream << "NODE";
		break;
	case LEAF:
		outputStream << "LEAF";
		break;
	case OBJECT:
		outputStream << "OBJECT";
		break;
	}
	outputStream << " x1=" << curNode->covering.x1
		         << " x2=" << curNode->covering.x2
				 << " y1=" << curNode->covering.y1
				 << " y2=" << curNode->covering.y2;
	
	if (curNode->type == OBJECT)
		outputStream << " data=" << curNode->data << endl;
	else {
		outputStream << endl;
		for (int i = 0; i < curNode->objCount; i++) {
			_PrintRtree(curNode->childs[i], level + 1, outputStream);
		}
	}
}



Node* Rtree::ChooseNode(Node* newNode, int level) {
	Node* N = root;
	int curLevel = 2;

	if (level == 0) {
		while (N->type != LEAF) {
			N = ChooseNodeWithMinMBR(N, newNode);
		}
	}
	else {
		while (curLevel != level) {
			N = ChooseNodeWithMinMBR(N, newNode);
			curLevel++;
		}
	}

	return N;
}

void Rtree::CorrectTree(Node* leaf, Node* splitLeaf) {
	Node* node1 = leaf;
	Node* node2 = splitLeaf;

	while (node1 != root) {
		Node* nodeParent = node1->parent;
		CalculateMBR(node1);
		CalculateMBR(nodeParent);

		Node* newNodeParent = NULL;

		// если для вершин для новой вершины находится место, то просто вставляем, иначе делим на две
		if (node2 != NULL) {
			if (nodeParent->objCount < M) {
				nodeParent->childs[nodeParent->objCount] = node2;
				node2->parent = nodeParent;
				nodeParent->objCount++;
			}
			else
				newNodeParent = SplitNode(nodeParent, splitLeaf);
		}

		node1 = nodeParent;
		node2 = newNodeParent;
	}

	// изменения дошли до корня
	if (node1 == root) {
		if (node2 != NULL) {
			//создаем новый корень
			root = new Node;
			root->childs = new Node*[M];
			root->childs[0] = node1;
			root->childs[1] = node2;
			root->objCount = 2;
			CalculateMBR(root);
			root->type = NODE;
			root->parent = NULL;

			node1->parent = root;
			node2->parent = root;
		}
		return;
	}
}

Node* Rtree::SplitNode(Node* curNode, Node* newNode) {

	//новый узел
	Node* secondNode = new Node;
	secondNode->objCount = 0;
	secondNode->type = curNode->type;
	secondNode->childs = new Node*[M];
	secondNode->parent = curNode->parent;

	//буфер для всех вершин, которые нужно разбить на две группы
	Node** bufChilds = new Node*[M + 1];
	for (int i = 0; i < curNode->objCount; i++)
		bufChilds[i] = curNode->childs[i];
	bufChilds[M] = newNode;
	int bufChildsNum = curNode->objCount + 1;

	//очищаем старый узел
	delete[] curNode->childs;
	curNode->childs = new Node*[M];
	curNode->objCount = 0;

	//два узла для вызова функции ChooseNodeWithMinMBR
	Node* curAndSecNodes = new Node;
	curAndSecNodes->childs = new Node*[2];
	curAndSecNodes->childs[0] = curNode;
	curAndSecNodes->childs[1] = secondNode;
	curAndSecNodes->objCount = 2;

	//Ищем максимальный по X, кладем в старый узел
	int xMax = FindMaxBottomBorder(bufChilds, bufChildsNum, XAXIS);
	curNode->childs[curNode->objCount] = bufChilds[xMax];
	bufChilds[xMax]->parent = curNode;
	curNode->covering = bufChilds[xMax]->covering;
	swap(bufChilds[bufChildsNum - 1], bufChilds[xMax]);
	bufChildsNum--;
	curNode->objCount++;

	//Ищем максимальный по Y, кладем в новый узел
	int yMax = FindMaxBottomBorder(bufChilds, bufChildsNum, YAXIS);
	secondNode->childs[secondNode->objCount] = bufChilds[yMax];
	bufChilds[yMax]->parent = secondNode;
	secondNode->covering = bufChilds[yMax]->covering;
	swap(bufChilds[bufChildsNum - 1], bufChilds[yMax]);
	bufChildsNum--;
	secondNode->objCount++;

	//присваиваем узлам оставшиеся
	int num = bufChildsNum;
	for (int i = 0; i < num; i++) {
		if (secondNode->objCount + bufChildsNum == m || curNode->objCount == M) {
			//копируем все в second, если в second есть место, чтобы достичь m, или если current заполнен
			for (int j = i; j < num; j++) {
				secondNode->childs[secondNode->objCount] = bufChilds[i];
				bufChilds[i]->parent = secondNode;
				secondNode->objCount++;
			}
			CalculateMBR(secondNode);
			break;
		}

		if (curNode->objCount + bufChildsNum == m || secondNode->objCount == M) {
			//копируем все в current, если в current есть место, чтобы достичь m, или если second заполнен
			for (int j = i; j < num; j++) {
				curNode->childs[curNode->objCount] = bufChilds[i];
				bufChilds[i]->parent = curNode;
				curNode->objCount++;
			}
			CalculateMBR(curNode);
			break;
		}

		// вставляем в один из с наименьшим изменением площади
		Node* n;
		n = ChooseNodeWithMinMBR(curAndSecNodes, bufChilds[i]);
		n->childs[n->objCount] = bufChilds[i];
		bufChilds[i]->parent = n;
		n->objCount++;
		CalculateMBR(n);

		bufChildsNum--;
	}

	delete[] bufChilds;
	delete[] curAndSecNodes->childs;
	delete curAndSecNodes;

	return secondNode;
}

int Rtree::TreeLevel() {
	int level = 1;
	Node* curNode = root;
	do {
		curNode = curNode->childs[0];
		level++;
	} while (curNode->type != OBJECT);

	return level;
}