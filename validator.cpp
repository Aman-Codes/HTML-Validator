#include <iostream>
#include <vector>
#include <stack>
#include <string>
#include <algorithm>
#include <fstream>
#include <iterator>
using namespace std;

//All avaialble tags
const vector<string> TAGS = { "html", "head", "body", "p", "br", "li", "h1", "h2", "ul", "ol" };

/*
function to check iF given tag is self closing tag 
@param tag, given tag without angular brackets <>
@returns true if is a self closing tag else false
*/
bool isSelfClosingTag(string tag) {
	transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
	size_t index = tag.find('/');
	if (index == string::npos) {
		return false;
	}
	else {
		if (index+1 == tag.length()) {
			tag.erase(index, 1);
			for (auto i : TAGS) {
				if (i == tag)
					return true;
			}
		}
	}
	return false;
}


/*
function to check iF given tag is valid closing tag
@param tag, given tag without angular brackets <>
@returns true if is a valid closing tag else false
*/
bool isValidClosingTag(string tag) {
	transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
	if (tag[0] != '/') {
		return false;
	}
	tag.erase(0, 1);
	for (auto i : TAGS) {
		if (i == tag)
			return true;
	}
	return false;
}

/*
function to check iF given tag is valid opening tag
@param tag, given tag without angular brackets <>
@returns true if is a valid opening tag else false
*/
bool isValidOpenTag(string tag) {
	transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
	for (auto i : TAGS) {
		if (i == tag)
			return true;
	}
	return false;
}

/*
function to check iF given string is just content
@param str, given string to be checked
@returns true if is it content else false
*/
bool isContent(const string& str) {
	for (size_t i = 0; i < str.length(); i++) {
		if (str[i] == '<') {
			return false;
		}
	}
	return true;
}

/*
function to check if givens tag same
@param stackTag, given tag without angular brackets <>
@param tag, given tag without angular brackets <>
@returns true if both tags are same else false
*/
bool matchTags(const string& stackTag, const string& tag) {
	if (stackTag != tag) {
		return false;
	}
	return true;
}

/*
function to find closing angular bracket in given line in file
@param file, the file which is being used
@param line, line in which anglar bracket is being found
@lineNumber, current line number of file we're processing
@returns index of > bracket in line or -1 if reaches end of the file
*/
int findClosingBracket(ifstream& file, string &line, int &lineNumber) {
	if (isContent(line)) {
		if (file.eof()) {
			return -1;
		}
		getline(file, line);
		return findClosingBracket(file, line, ++lineNumber);
	}
	size_t index = line.find('>', 0);
	if (index == string::npos) {
		string nextLine = "";
		if (file.eof()) {
			return -1;
		}
		getline(file, nextLine);
		line += nextLine;
		return findClosingBracket(file, line, ++lineNumber);
	}
	else {
		return index;
	}
}

/*
function to processs stack with openning tags and given tag 
@param tags, stack of tags
@param errors, current number of errors
@param tag, tag which is being processed
@param lineNumber, lineNumber currently being processed
*/
void processStackAndTags(stack<string> &tags, int& errors, const string& tag, const int& lineNumber) {
	string *tempTags = new string[tags.size()];
	int stackSize = tags.size();
	for (int i = stackSize - 1; i >= 0; i--) {
		tempTags[i] = tags.top();
		tags.pop();
	}
	int lastClosingTag = -1;
	for (int i = stackSize - 1; i >= 0; i--) {
		if (tempTags[i] == tag) {
			lastClosingTag = i;
			break;
		}
	}
	if (lastClosingTag == -1) {
		cout << "Error on line " << lineNumber << " could not found opening tag of " << tag << endl;
		errors++;
		for (int i = 0; i < stackSize; i++) {
			tags.push(tempTags[i]);
		}
	}
	else {
		for (int i = 0; i < lastClosingTag; i++) {
			tags.push(tempTags[i]);
		}
		for (int i = stackSize - 1; i > lastClosingTag; i--) {
			errors++;
			cout << "Error on line " << lineNumber << " could not found closing tag of " << tempTags[i] << endl;
		}
	}
	delete[] tempTags;
	tempTags = nullptr;
}

/*
function to process that process file and tags and validates html
@param file, the file which is being used
@param line, line being processed
@param tags, stack of tags
@param lineNumber, lineNumber currently being processed
@param errors, current number of errors
*/
void processTags(ifstream &file, string &line, stack<string>& tags, int& lineNumber, int &errors, size_t start=0) {
	if (start >= line.length()) {
		return;
	}
	if (line[start] == '<') {
		size_t index = line.find('>', start+1);
		if (index != string::npos) {
			string aTag = line.substr(start + 1, index-start-1);
			
			if (isValidOpenTag(aTag)) {
				//cout << "Opening Tag: " << aTag << endl;
				tags.push(aTag);
				processTags(file, line, tags, lineNumber, errors, index + 1);
			}
			else if (isValidClosingTag(aTag)) {
				aTag.erase(0, 1);
				if (!tags.empty() && matchTags(tags.top(), aTag)) {
					tags.pop();
				}
				else {
					processStackAndTags(tags, errors, aTag, lineNumber);
				}
				//cout << "Closing Tag: " << aTag << endl;
				processTags(file, line, tags, lineNumber, errors, index + 1);
			}
			else if (isSelfClosingTag(aTag)) {
				processTags(file, line, tags, lineNumber, errors, index + 1);
			}
			else {
				cout << "Undefined tag: " << aTag << " in line-> " << lineNumber << endl;
				errors++;
				return;
			}
		}
		else {
			if (file.eof()) {
				return;
			}
			string nextLine = "";
			getline(file, nextLine);
			nextLine = line + nextLine;
			nextLine = nextLine.substr(start);
			int i = findClosingBracket(file, nextLine, ++lineNumber);
			size_t openingBracket = nextLine.rfind('<', i);
			string tag = nextLine.substr(openingBracket + 1, i - openingBracket - 1);
			tag.erase(remove_if(tag.begin(), tag.end(), ::isspace), tag.end());
			if (i == -1) {
				if (!tags.empty()) {
					cout << "Error could find closing tag: " << lineNumber << "tag: " << tags.top();
					tags.pop();
					errors++;
				}
				cout << "Error could find opening tag on " << lineNumber << "tag: " << tag;
				errors++;

			}
			else {
				if (isValidOpenTag(tag)) {
					//cout << "Opening Tag: " << tag << endl;
					tags.push(tag);
				}
				else if (isValidClosingTag(tag)) {
					tag.erase(0, 1);
					if (!tags.empty() && matchTags(tags.top(), tag)) {
						tags.pop();
					}
					else {
						processStackAndTags(tags, errors, tag, lineNumber);
					}
				}
				else {
					if (!isSelfClosingTag(tag)) {
						cout << "Undefined tag: " << tag << " in line-> " << lineNumber << endl;
						errors++;
					}
				}
				//cout << "lalalCol:" << tag << endl;
			}
			line = nextLine;
			processTags(file, line, tags, lineNumber, errors, i + 1);
		}
	}
	else {
		processTags(file, line, tags, lineNumber, errors, start + 1);
	}
}


/*
function that validates html file
@param file, the file which is being used
@param tags, stack of tags
*/
void validator(ifstream& file, stack<string>& tag) {
	cout << "enter file name to be validated: ";
	string fileName = "test.html";
	cin >> fileName;
	file.open(fileName.c_str());
	string line = "";
	int lineNumber = 0;
	int errors = 0;
	if (file.is_open()) {
		while (!file.eof()) {
			lineNumber++;
			getline(file, line);
			processTags(file, line, tag, lineNumber, errors);
			
		}
		if (errors == 0 && tag.empty()) {
			cout << "HTML validated sucessfully without any errors" << endl;
		}
		else {
			while (!tag.empty()) {
				cout << "Missing closing tag of " << tag.top() << endl;
				tag.pop();
			}
			cout << "HTML validated with " << errors << " errors" << endl;
		}
	}
	else {
		throw runtime_error("Cannot open file");
	}
	file.close();
}

//main method
int main() {
	ifstream file;
	stack<string> tag;
	validator(file, tag);
	system("pause");
	return 0;

}
