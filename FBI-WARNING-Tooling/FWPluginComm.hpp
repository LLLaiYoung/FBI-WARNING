#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

static bool isUppercase(const char c)
{
    return (c >= 'A' && c <= 'Z')?true:false;
}

static bool isLowercase(const char c)
{
    return (c >= 'a' && c <= 'z')?true:false;
}

static void remove_char_from_string(string &str,char ch)
{
    str.erase(remove(str.begin(), str.end(), ch), str.end());
}

static vector<string> split(const string &s, char delim)
{
    vector<string> elems;
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

static bool isVenderSourceCode(const string filename)
{
    vector<string> venders = {"AFNetworking","stdatomic"/*...*/};
    for (size_t idx = 0; idx < venders.size(); idx++ ) {
        const string vender = venders[idx];
        if (filename.find(vender) != string::npos) {
            return true;
        }
    }
    return false;
}

static bool isUserSourceCode(const string filename)
{
    if (filename.empty() ||
        filename.find("/Applications/Xcode.app/") == 0 ||// 非Xcode中的源码都认为是用户源码
        isVenderSourceCode(filename)) {
        return false;
    }
    return true;
}

static bool isMutableContainer(const string str)
{
    vector<string> mutableContainers = {"NSMutableArray","NSMutableDictionary"};
    for (size_t idx = 0; idx < mutableContainers.size(); idx++ ) {
        const string item = mutableContainers[idx];
        if (str.find(item) != string::npos) {
            return true;
        }
    }
    return false;
}

// 是否使用了泛型
static bool isGenericType(const string typeStr)
{
    if (typeStr.find("<") != string::npos &&
        typeStr.find(">") != string::npos) {
        return true;
    }
    return false;
}

static bool isUppercaseName(const string name)
{
    return isUppercase(name[0])?true:false;
}

static bool isMethodUppercaseName(string name)
{
    vector<string> v = split(name, ':');
    size_t size = v.size();
    if (size > 1) {
        for (size_t idx = 0; idx < size; idx++) {
            if (isUppercase(v[idx][0])) {
                return true;
            }
        }
        return false;
    } else {
        return isUppercase(name[0])?true:false;
    }
}

static bool isLowercaseName(const string name)
{
    return isLowercase(name[0])?true:false;
}

static char fw_toLowercase(const char c)
{
    return !isUppercase(c)?c:c+32;
}

static char fw_toUppercase(const char c)
{
    return isUppercase(c)?c:c-32;
}

static bool isUsedMemoryKeyword(const string str)
{
    vector<string> memoryKeywords = {"alloc","new","copy","mutableCopy"};
    for (size_t idx = 0; idx < memoryKeywords.size(); idx++ ) {
        string item = memoryKeywords[idx];
        char itemNextC = str[item.length()];
        if (str.find(item) != string::npos &&
            (isUppercase(itemNextC) || itemNextC == '\0')) {
            return true;
        }
    }
    return false;
}

static void remove_blank(string &str) {
    remove_char_from_string(str,' ');
    remove_char_from_string(str,'\t');
    remove_char_from_string(str,'\r');
    remove_char_from_string(str,'\n');
}

static bool has_Prefix(const string &str, const string &prefix)
{
    return str.size() >= prefix.size() &&
    str.compare(0, prefix.size(), prefix) == 0;
}
