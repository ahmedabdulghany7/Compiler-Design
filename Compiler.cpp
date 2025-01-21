#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
using namespace std;

struct Token {
    string type;
    string value;
    size_t lineNumber = 1;
};

const unordered_set<string> reservedKeywords = {
    "int", "float", "string", "read", "write", "repeat", "until", "if",
    "elseif", "else", "then", "return", "end"
};

bool isReservedKeyword(const string& word) {
    return reservedKeywords.find(word) != reservedKeywords.end();
}

vector<Token> tokenize(const string& text) {
    vector<Token> tokens;
    string word;
    size_t lineNumber = 1;

    for (size_t i = 0; i < text.size(); ++i) {
        char currentChar = text[i];

        if (currentChar == '\n') {
            ++lineNumber;
            continue;
        }

        if (isspace(currentChar)) {
            continue;
        }

        if (currentChar == '"') {
            word = '"';
            while (text[++i] != '"' && i < text.size() - 1) {
                word += text[i];
            }
            word += '"';
            tokens.push_back({ "STRING", word, lineNumber });
        }

        else if (isdigit(currentChar)) {
            word = currentChar;
            while (isdigit(text[i + 1]) || text[i + 1] == '.') {
                word += text[++i];
            }
            tokens.push_back({ "NUMBER", word, lineNumber });
        }

        else if (text.substr(i, 2) == "/*") {
            word = "/*";
            while (text.substr(i, 2) != "*/" && i < text.size() - 1) {
                word += text[++i];
            }
            word += "*/";
            tokens.push_back({ "COMMENT", word, lineNumber });
            ++i;
        }

        else if (isalpha(currentChar) || currentChar == '_') {
            word = currentChar;
            while (isalnum(text[i + 1]) || text[i + 1] == '_') {
                word += text[++i];
            }
            if (isReservedKeyword(word)) {
                string upperWord = word;
                transform(upperWord.begin(), upperWord.end(), upperWord.begin(), ::toupper);
                tokens.push_back({ upperWord, word, lineNumber });
            }
            else {
                tokens.push_back({ "IDENTIFIER", word, lineNumber });
            }
        }

        else {
            switch (currentChar) {
            case ':':
                if (text[i + 1] == '=') {
                    tokens.push_back({ "COLON_EQUALS", ":=", lineNumber });
                    ++i;
                }
                else {
                    tokens.push_back({ "COLON", ":", lineNumber });
                }
                break;
            case '<':
                if (text[i + 1] == '=') {
                    tokens.push_back({ "LESS_THAN_OR_EQUAL", string(1, currentChar) + "=" , lineNumber });
                    ++i;
                }
                else {
                    tokens.push_back({ "LESS_THAN", string(1, currentChar) , lineNumber });
                }
                break;
            case '>':
                if (text[i + 1] == '=') {
                    tokens.push_back({ "MORE_THAN_OR_EQUAL", string(1, currentChar) + "=" , lineNumber });
                    ++i;
                }
                else {
                    tokens.push_back({ "MORE_THAN", string(1, currentChar) , lineNumber });
                }
                break;
            case '=':
                tokens.push_back({ "EQUALS", "=" , lineNumber });
                break;
            case '!':
                if (text[i + 1] == '=') {
                    tokens.push_back({ "CONDITION_OPERATOR", "!=" , lineNumber });
                    ++i;
                }
                else {
                    cerr << "ERROR: Unexpected character '!' at position " << i << endl;
                }
                break;
            case '&':
                if (text[i + 1] == '&') {
                    tokens.push_back({ "BOOLEAN_OPERATOR", "&&" , lineNumber });
                    ++i;
                }
                else {
                    cerr << "ERROR: Unexpected character '&' at position " << i << endl;
                }
                break;
            case '|':
                if (text[i + 1] == '|') {
                    tokens.push_back({ "BOOLEAN_OPERATOR", "||" , lineNumber });
                    ++i;
                }
                else {
                    cerr << "ERROR: Unexpected character '|' at position " << i << endl;
                }
                break;
            case ';':
                tokens.push_back({ "SEMICOLON", ";" , lineNumber });
                break;
            case '+':
                tokens.push_back({ "PLUS", "+" , lineNumber });
                break;
            case '-':
                tokens.push_back({ "MINUS", "-" , lineNumber });
                break;
            case '*':
                tokens.push_back({ "STAR", "*" , lineNumber });
                break;
            case '/':
                tokens.push_back({ "SLASH", "/" , lineNumber });
                break;
            case '(':
                tokens.push_back({ "LEFT_PAREN", "(" , lineNumber });
                break;
            case ')':
                tokens.push_back({ "RIGHT_PAREN", ")" , lineNumber });
                break;
            case '{':
                tokens.push_back({ "LEFT_BRACE", "{" , lineNumber });
                break;
            case '}':
                tokens.push_back({ "RIGHT_BRACE", "}" , lineNumber });
                break;
            case ',':
                tokens.push_back({ "COMMA", "," , lineNumber });
                break;
            default:
                cerr << "ERROR: Unexpected character '" << currentChar << "' at position " << i << endl;
                break;
            }
        }
    }
    if (!text.empty() && text.back() != '\n') {
        ++lineNumber;
    }
    return tokens;
}

string generateParseTreeXML(vector<Token>& tokens) {
    string xml = "<Program>\n";

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type == "READ") {
            xml += "    <Read>" + tokens[++i].value + "</Read>\n";
        }
        else if (tokens[i].type == "IDENTIFIER") {
            xml += "    <Assign>\n    <Identifier>" + tokens[i].value + "</Identifier>\n";
            if (tokens[++i].type == "COLON_EQUALS") {
                xml += "    <Number>" + tokens[++i].value + "</Number>\n";
            }
            xml += "    </Assign>\n";
        }
    }

    xml += "</Program>";
    return xml;
}

void parseStatement(vector<Token>& tokens, size_t& index) {
    if (tokens[index].type == "READ") {
        if (tokens[index + 1].type == "IDENTIFIER") {
            if (tokens[index + 2].type != "SEMICOLON") {
                cerr << "Syntax error: Missing ';' after identifier on line " << tokens[index].lineNumber << endl;
            }
        }
        else {
            cerr << "Syntax error: Expected identifier after 'read' on line " << tokens[index].lineNumber << endl;
        }
    }
    else if (tokens[index].type == "IDENTIFIER") {
        if (tokens[index + 1].type == "COLON_EQUALS") {
            if (tokens[index + 2].type != "NUMBER") {
                cerr << "Syntax error: Missing number after ':=' on line " << tokens[index].lineNumber << endl;
            }
            if (tokens[index + 2].type != "SEMICOLON") {
                cerr << "Syntax error: Missing ';' after expression on line " << tokens[index].lineNumber << endl;
            }
        }
        else {
            // cerr << "Syntax error: Expected ':=' after identifier on line " << tokens[index].lineNumber << endl;
        }
    }
    else if (tokens[index].type == "IF") {
        if (tokens[index + 1].type != "LEFT_PAREN") {
            cerr << "Syntax error: Missing '(' after 'if' on line " << tokens[index].lineNumber << endl;
        }
        if (tokens[index + 1].type != "RIGHT_PAREN") {
            cerr << "Syntax error: Missing ')' after condition on line " << tokens[index].lineNumber << endl;
        }
        if (tokens[index + 1].type != "THEN") {
            cerr << "Syntax error: Missing 'then' after condition on line " << tokens[index].lineNumber << endl;
        }
    }
    else if (tokens[index].type == "ELSEIF") {
        if (tokens[index + 1].type != "LEFT_PAREN") {
            cerr << "Syntax error: Missing '(' after 'elseif' on line " << tokens[index].lineNumber << endl;
        }
        if (tokens[index + 1].type != "RIGHT_PAREN") {
            cerr << "Syntax error: Missing ')' after condition on line " << tokens[index].lineNumber << endl;
        }
        if (tokens[index + 1].type != "THEN") {
            cerr << "Syntax error: Missing 'then' after condition on line " << tokens[index].lineNumber << endl;
        }
    }
    else if (tokens[index].type == "ELSE") {
        //++index; 
    }
    else if (tokens[index].type == "REPEAT") {
        //++index; 
    }
    else if (tokens[index].type == "UNTIL") {
        //++index; 
        if (tokens[index + 1].type != "LEFT_PAREN") {
            cerr << "Syntax error: Missing '(' after 'until' on line " << tokens[index].lineNumber << endl;
        }
        //++index; 
        if (tokens[index + 1].type != "RIGHT_PAREN") {
            cerr << "Syntax error: Missing ')' after condition on line " << tokens[index].lineNumber << endl;
        }
    }
    else if (tokens[index].type == "RETURN") {
        //++index; 
    }
    else if (tokens[index].type == "END") {
        return;
    }
}

void parse(vector<Token>& tokens) {
    size_t index = 0;
    while (index < tokens.size()) {
        parseStatement(tokens, index);
        ++index;
    }
}

int main() {
    ifstream file("1.txt");
    string line;
    string input_text;

    while (getline(file, line)) {
        input_text += line + "\n";
    }

    vector<Token> tokens = tokenize(input_text);

    for (const auto& token : tokens) {
        cout << token.type << " : " << token.value << endl;
    }

    cout << '\n';

    parse(tokens);

    cout << '\n';

    string parseTreeXML = generateParseTreeXML(tokens);
    cout << "Parse Tree XML:\n" << parseTreeXML << endl;

    return 0;
}
