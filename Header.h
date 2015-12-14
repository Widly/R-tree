#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stddef.h>
#include <string>
#include <list>
#include <map>

#define LEAF 20
#define NODE 10
#define OBJECT 30
#define XAXIS 100
#define YAXIS 200
#define MAX 80
#define MIN 90

using namespace std;

struct MBR {
	int x1, y1; // Координаты верхней левой точки
	int x2, y2; // Координаты нижней правой точки
};

struct Node {
	MBR covering; // покрытие вершины
	Node** childs; // массив потомков
	Node* parent; // родитель
	int type; // тип вершины - лист, узел или объект - LEAF NODE OBJECT 
	int objCount; // число потомков
	int data; // данные (для объекта)
};

class OperationsWithObjects {
public:
	Node* ChooseNodeWithMinMBR(Node* curNode, Node* newNode); // нахождение потомка с минимальным увеличением MBR при добавлении новой вершины
	long Area(MBR mbr); // нахождение площади
	int FindMaxBottomBorder(Node** nodes, int nodesCount, int axis); // поиск максимально удаленного элемента по оси
	void CalculateMBR(Node* node); // обновление MBR вершины
	bool IsIntersect(MBR mbr1, MBR mbr2); // проверка на пересечение двух MBR
	bool IsFullyContains(MBR container, MBR content); // проверка на полное вхождение одной MBR в другую
	bool IsEqual(MBR mbr1, MBR mbr2); // проверка на равенство двух MBR
};

class Rtree:OperationsWithObjects {
public:
	Rtree();
	~Rtree();
	void InsertObject(Node* neObject); // вставка объекта в дерево (интерфейс)
	bool DeleteObject(MBR covering, int data); // удаление объекта
	void SearchByArea(MBR area, list<Node*>* res); // поиск по площади (интерфейс)
	Node* SearchObject(MBR covering, int data); // поиск объекта (интерфейс)
	Node* FindExtremumByArea(int borderType); // поиск максимального/минимального по площади объекта (интерфейс)
	void PrintRtree(ofstream &outputStream); // вывод структуры дерева (интерфейс)

	int m; // минимальное число потомков у вершины
	int M; // максимальное число потомков у вершины

private:
	Node* root;
	Node* curExtremum;

	void MemFree(Node* nodeToDelete); // освобождение памяти

	void InsertNode(Node* neObject, int level);
	void _PrintRtree(Node* curNode, int level, ofstream &outputStream);
	Node* _FindExtremumByArea(Node* curNode, int borderType);
	Node* _SearchObject(Node* curNode, MBR covering, int data);
	void _SearchByArea(Node* curNode, MBR area, list<Node*>* res);

	Node* ChooseNode(Node* newObject, int level); // поиск узла для вставки
	void CorrectTree(Node* leaf, Node* splitLeaf); // процедура корректировки дерева
	Node* SplitNode(Node* curNode, Node* newNode); // процедура деления узла
	int TreeLevel(); // определение текущей высоты дерева
};

#endif HEADER_H