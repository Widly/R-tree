#include "Header.h"

long OperationsWithObjects::Area(MBR mbr) {
	return (mbr.x2 - mbr.x1) * (mbr.y2 - mbr.y1);
}

void OperationsWithObjects::CalculateMBR(Node* node) {
	int maxX, minX, maxY, minY;

	maxX = node->childs[0]->covering.x2;
	minX = node->childs[0]->covering.x1;
	maxY = node->childs[0]->covering.y2;
	minY = node->childs[0]->covering.y1;

	for (int i = 1; i < node->objCount; i++) {
		if (node->childs[i]->covering.x2 > maxX)
			maxX = node->childs[i]->covering.x2;

		if (node->childs[i]->covering.x1 < minX)
			minX = node->childs[i]->covering.x1;

		if (node->childs[i]->covering.y2 > maxY)
			maxY = node->childs[i]->covering.y2;

		if (node->childs[i]->covering.y1 < minY)
			minY = node->childs[i]->covering.y1;
	}

	node->covering.x1 = minX;
	node->covering.x2 = maxX;
	node->covering.y1 = minY;
	node->covering.y2 = maxY;
}

int OperationsWithObjects::FindMaxBottomBorder(Node** nodes, int nodesCount, int axis) {
	int maxBottomBorder = 0;
	int maxBottomBorderNum;

	if (axis == XAXIS) {

		maxBottomBorder = nodes[0]->covering.x1;
		maxBottomBorderNum = 0;

		for (int i = 1; i < nodesCount; i++) {
			if (nodes[i]->covering.x1 > maxBottomBorder) {
				maxBottomBorder = nodes[i]->covering.x1;
				maxBottomBorderNum = i;
			}
		}
	}
	else {

		maxBottomBorder = nodes[0]->covering.y1;
		maxBottomBorderNum = 0;

		for (int i = 1; i < nodesCount; i++) {
			if (nodes[i]->covering.y1 > maxBottomBorder) {
				maxBottomBorder = nodes[i]->covering.y1;
				maxBottomBorderNum = i;
			}
		}
	}

	return maxBottomBorderNum;
}

Node* OperationsWithObjects::ChooseNodeWithMinMBR(Node* curNode, Node* newNode) {
	MBR curDeltaMBR;
	long curArea = 0;
	long curDeltaArea = 0;
	long minDeltaArea = 0;
	int childWithMinDeltaArea;

	for (int i = 0; i < curNode->objCount; i++) {
		//X1
		if (newNode->covering.x1 < curNode->childs[i]->covering.x1)
			curDeltaMBR.x1 = newNode->covering.x1;
		else
			curDeltaMBR.x1 = curNode->childs[i]->covering.x1;

		//X2
		if (newNode->covering.x2 > curNode->childs[i]->covering.x2)
			curDeltaMBR.x2 = newNode->covering.x2;
		else
			curDeltaMBR.x2 = curNode->childs[i]->covering.x2;

		//Y1
		if (newNode->covering.y1 < curNode->childs[i]->covering.y1)
			curDeltaMBR.y1 = newNode->covering.y1;
		else
			curDeltaMBR.y1 = curNode->childs[i]->covering.y1;

		//Y2
		if (newNode->covering.y2 > curNode->childs[i]->covering.y2)
			curDeltaMBR.y2 = newNode->covering.y2;
		else
			curDeltaMBR.y2 = curNode->childs[i]->covering.y2;

		curArea = Area(curNode->childs[i]->covering);
		curDeltaArea = Area(curDeltaMBR) - curArea;

		if (minDeltaArea == 0) {
			minDeltaArea = curDeltaArea;
			childWithMinDeltaArea = i;
		}


		if (curDeltaArea == 0)
			return curNode->childs[i];
		else
			if (curDeltaArea < minDeltaArea) {
			minDeltaArea = curDeltaArea;
			childWithMinDeltaArea = i;
			}
	}
	return curNode->childs[childWithMinDeltaArea];
}

bool OperationsWithObjects::IsIntersect(MBR mbr1, MBR mbr2) {
	if (Area(mbr2) < Area(mbr1))
		swap(mbr1, mbr2);

	if (mbr1.x1 >= mbr2.x1 && mbr1.x1 <= mbr2.x2 && mbr1.y1 >= mbr2.y1 && mbr1.y1 <= mbr2.y2) //x1 y1
		return true;

	if (mbr1.x2 >= mbr2.x1 && mbr1.x2 <= mbr2.x2 && mbr1.y1 >= mbr2.y1 && mbr1.y1 <= mbr2.y2) //x2 y1
		return true;

	if (mbr1.x1 >= mbr2.x1 && mbr1.x1 <= mbr2.x2 && mbr1.y2 >= mbr2.y1 && mbr1.y2 <= mbr2.y2) //x1 y2
		return true;

	if (mbr1.x2 >= mbr2.x1 && mbr1.x2 <= mbr2.x2 && mbr1.y2 >= mbr2.y1 && mbr1.y2 <= mbr2.y2) //x2 y2
		return true;

	return false;
}

bool OperationsWithObjects::IsFullyContains(MBR container, MBR content) {
	if (container.x1 <= content.x1 &&
		container.x2 >= content.x2 &&
		container.y1 <= content.y1 &&
		container.y2 >= content.y2)
		return true;

	return false;
}

bool OperationsWithObjects::IsEqual(MBR mbr1, MBR mbr2) {
	if (mbr1.x1 == mbr2.x1 &&
		mbr1.x2 == mbr2.x2 &&
		mbr1.y1 == mbr2.y1 &&
		mbr1.y2 == mbr2.y2)
		return true;

	return false;
}