#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "Tokenizer.h"

using namespace std;

typedef struct _students {

	char name[20];
	unsigned int studentID;
	float score;
	unsigned int advisorID;


}Students;


//Funtion which gets inputData from file
int getInputData(Students*& students, string input_str) {
	
	//Get input data from .csv
	ifstream input_data(input_str);
	string buf;
	Tokenizer tokenizer; //include "Tokenize.h"
	int count;

	tokenizer.setDelimiter(","); //parsing Delimiter = ","
	getline(input_data, buf);
	tokenizer.setString(buf);
	count = atoi(tokenizer.next().c_str()); //the num of Students
	students = new Students[count];

	for (int i = 0; i < count; i++) {
		getline(input_data, buf);
		tokenizer.setString(buf);

		//set Students values
		strcpy(students[i].name, tokenizer.next().c_str());
		students[i].studentID = atoi(tokenizer.next().c_str());
		students[i].score = atof(tokenizer.next().c_str());
		students[i].advisorID = atoi(tokenizer.next().c_str());
	}

	return count;
}

void main() {
	
	Students *students;
	int count = getInputData(students, "sampleData.csv");

	for (int i = 0; i < count; i++) {
		//print out Students values
		cout << students[i].name << " " << students[i].studentID << " "
			<< students[i].score << " " << students[i].advisorID << endl;
		
		}
	

}