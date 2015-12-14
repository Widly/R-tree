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
	int x1, y1; // ���������� ������� ����� �����
	int x2, y2; // ���������� ������ ������ �����
};

struct Node {
	MBR covering; // �������� �������
	Node** childs; // ������ ��������
	Node* parent; // ��������
	int type; // ��� ������� - ����, ���� ��� ������ - LEAF NODE OBJECT 
	int objCount; // ����� ��������
	int data; // ������ (��� �������)
};

class OperationsWithObjects {
public:
	Node* ChooseNodeWithMinMBR(Node* curNode, Node* newNode); // ���������� ������� � ����������� ����������� MBR ��� ���������� ����� �������
	long Area(MBR mbr); // ���������� �������
	int FindMaxBottomBorder(Node** nodes, int nodesCount, int axis); // ����� ����������� ���������� �������� �� ���
	void CalculateMBR(Node* node); // ���������� MBR �������
	bool IsIntersect(MBR mbr1, MBR mbr2); // �������� �� ����������� ���� MBR
	bool IsFullyContains(MBR container, MBR content); // �������� �� ������ ��������� ����� MBR � ������
	bool IsEqual(MBR mbr1, MBR mbr2); // �������� �� ��������� ���� MBR
};

class Rtree:OperationsWithObjects {
public:
	Rtree();
	~Rtree();
	void InsertObject(Node* neObject); // ������� ������� � ������ (���������)
	bool DeleteObject(MBR covering, int data); // �������� �������
	void SearchByArea(MBR area, list<Node*>* res); // ����� �� ������� (���������)
	Node* SearchObject(MBR covering, int data); // ����� ������� (���������)
	Node* FindExtremumByArea(int borderType); // ����� �������������/������������ �� ������� ������� (���������)
	void PrintRtree(ofstream &outputStream); // ����� ��������� ������ (���������)

	int m; // ����������� ����� �������� � �������
	int M; // ������������ ����� �������� � �������

private:
	Node* root;
	Node* curExtremum;

	void MemFree(Node* nodeToDelete); // ������������ ������

	void InsertNode(Node* neObject, int level);
	void _PrintRtree(Node* curNode, int level, ofstream &outputStream);
	Node* _FindExtremumByArea(Node* curNode, int borderType);
	Node* _SearchObject(Node* curNode, MBR covering, int data);
	void _SearchByArea(Node* curNode, MBR area, list<Node*>* res);

	Node* ChooseNode(Node* newObject, int level); // ����� ���� ��� �������
	void CorrectTree(Node* leaf, Node* splitLeaf); // ��������� ������������� ������
	Node* SplitNode(Node* curNode, Node* newNode); // ��������� ������� ����
	int TreeLevel(); // ����������� ������� ������ ������
};

#endif HEADER_H