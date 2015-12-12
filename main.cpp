#include "Header.h"

int main(int argc, char* argv[]) {
	char buf[50]; // буфер для строки
	string bufStr; // строка для парсинга
	string operation; // текущая операция
	string outFileName = "output.txt"; // выходной файл
	string inFileName; // входной файл
	Rtree myRtree; // R-tree
	list<Node*> searchRes; // результаты поиска
	MBR covering; // параметр покрытия для запроса к дереву  
	Node* bufNode; // буфер для ответа от дерева
	int inputNum; // праметр парсинга
	int separPos = 0; // текущая позиция разделителя (пробела)
	int obj[5]; // отпарсенные значения


	if (argc == 1) {
		cout << "wrong input, use -help for info" << endl;
		return 0;
	}

	if (strcmp(argv[1], "-help") == 0) {
		cout << "First arg - input file .txt" << endl << "Second arg - output file .txt (optional, default - output.txt)" << endl;
		return 0;
	}

	if (_access(argv[1], 0) != 0) {
		cout << "input file is not existing" << endl;
		return 0;
	} else {
		inFileName = argv[1];
		if (inFileName.find(".txt") != inFileName.length() - 4) {
			cout << "input file is not .txt" << endl;
			return 0;
		}
	}

	if (argc == 3) {
		outFileName = argv[2];

		if (outFileName.find(".txt") != outFileName.length() - 4) {
			cout << "output file is not .txt" << endl;
			return 0;
		}
	}

	ifstream inputFile(inFileName); // входной поток
	ofstream outFile; // выходной поток
	outFile.open(outFileName);
	
	// читаем посторочно
	while (!inputFile.eof()) {
		inputFile.getline(buf, 50);
		bufStr = string(buf);
		bufNode = NULL;

		// парсим строку
		separPos = bufStr.find(' ');
		operation = bufStr.substr(0, separPos); 
		bufStr = bufStr.substr(separPos + 1, bufStr.length() - 1);

		if (operation == "add" || operation == "searchByArea" || operation == "searchObject" || operation == "delete") {
			Node* newN;
			inputNum = 5;

			if (operation == "searchByArea")
				inputNum = 4;

			try {
				for (int i = 0; i < inputNum; i++) {
					separPos = bufStr.find(' ');
					obj[i] = stoi(bufStr.substr(0, separPos));
					if (obj[i] < 0)
						throw invalid_argument("error");
					bufStr = bufStr.substr(separPos + 1, bufStr.length() - 1);
				}	

				if (obj[0] >= obj[1] || obj[2] >= obj[3])
					throw invalid_argument("error");
			}
			catch (exception const &e) {
				outFile << "wrong input" << endl;
				inputFile.close();
				outFile.close();
				return 0;
			}

			if (operation == "add") {
				newN = new Node;
				newN->type = OBJECT;
				newN->covering.x1 = obj[0];
				newN->covering.x2 = obj[1];
				newN->covering.y1 = obj[2];
				newN->covering.y2 = obj[3];
				newN->data = obj[4];
				newN->parent = NULL;
				newN->childs = NULL;
				myRtree.InsertObject(newN);
				continue;
			}

			if (operation == "delete") {
				covering.x1 = obj[0];
				covering.x2 = obj[1];
				covering.y1 = obj[2];
				covering.y2 = obj[3];

				outFile << "object" << " x1=" << covering.x1
								    << " x2=" << covering.x2
									<< " y1=" << covering.y1
									<< " y2=" << covering.y2 << " data=" << obj[4];

				if (myRtree.DeleteObject(covering, obj[4]) == true) {
					outFile << " was successfully deleted" << endl << endl;
				}
				else {
					outFile << " don't exist" << endl << endl;
				}
				continue;
			}

			if (operation == "searchByArea") {
				covering.x1 = obj[0];
				covering.x2 = obj[1];
				covering.y1 = obj[2];
				covering.y2 = obj[3];

				myRtree.SearchByArea(covering, &searchRes);

				outFile << "search in" << " x1=" << covering.x1
									   << " x2=" << covering.x2
									   << " y1=" << covering.y1
									   << " y2=" << covering.y2 << " area results:" << endl;

				if (searchRes.empty() == true) {
					outFile << "\tno objects in this area" << endl << endl;
					continue;
				}
					
				while (searchRes.empty() != true) {
					bufNode = searchRes.back();
					searchRes.pop_back();
					outFile << '\t' << " x1=" << bufNode->covering.x1
						            << " x2=" << bufNode->covering.x2
						            << " y1=" << bufNode->covering.y1
						            << " y2=" << bufNode->covering.y2 << " data=" << bufNode->data << endl;
				}
				outFile << endl;
				continue;
			}

			if (operation == "searchObject") {
				covering.x1 = obj[0];
				covering.x2 = obj[1];
				covering.y1 = obj[2];
				covering.y2 = obj[3];

				outFile << "search object" << " x1=" << covering.x1
										   << " x2=" << covering.x2
										   << " y1=" << covering.y1
										   << " y2=" << covering.y2 << " data=" << obj[4] << " result:" << endl;

				bufNode = myRtree.SearchObject(covering, obj[4]);
				if (bufNode != NULL)
					outFile << "\tobject is exists" << endl << endl;
				else
					outFile << "\tobject not found" << endl << endl;

				continue;
			}

		}

		if (operation == "print") {
			myRtree.PrintRtree(outFile);
			continue;
		}

		if (operation == "max") {
			bufNode = myRtree.FindExtremumByArea(MAX);
			outFile << "max:" << " x1=" << bufNode->covering.x1
					<< " x2=" << bufNode->covering.x2
					<< " y1=" << bufNode->covering.y1
					<< " y2=" << bufNode->covering.y2 << " data=" << bufNode->data << endl;
			continue;
		}

		if (operation == "min") {
			bufNode = myRtree.FindExtremumByArea(MIN);
			outFile << "min:" << " x1=" << bufNode->covering.x1
					<< " x2=" << bufNode->covering.x2
					<< " y1=" << bufNode->covering.y1
					<< " y2=" << bufNode->covering.y2 << " data=" << bufNode->data << endl;
			continue;
		}
	}

	inputFile.close();
	outFile.close();
	cout << "Success!" << endl;
	return 0;
}