#include <iostream>
#include "FWPluginComm.hpp"

using namespace std;

static vector<string> protocolDecls;
static bool isShouldUseCopy(const string typeStr)
{
    vector<string> shouldUseCopys = {"NSString","(^)"/*block*/};
    for (size_t idx = 0; idx < shouldUseCopys.size(); idx++ ) {
        const string item = shouldUseCopys[idx];
        if (typeStr.find(item) != string::npos) {
            return isGenericType(typeStr)?false:true;
        }
    }
    return false;
}

static bool isShouldUseWeak(const string typeStr)
{
    if (isGenericType(typeStr)) {
        vector<string> v = split(typeStr, '<');
        size_t size = v.size();
        string fs;// first object
        if (size >= 1) {
            fs = v[0];
        }
        
        if (fs.find("id") == string::npos) return false;

        string ls;// last object
        if (size >= 2) {
            ls = v[1];
        }
        
        if (!ls.length()) return false;
        
        v = split(ls, '>');
        size = v.size();
        fs = "";
        if (size >= 1) {
            fs = v[0];
        }
        if (fs == "") return false;
        size_t length = protocolDecls.size();
        for (size_t idx = 0; idx < length; idx++) {
            const string item = protocolDecls[idx];
            if (fs.find(item) != string::npos) {
                return true;
            }
        }
        return false;
    }
    return false;
}

static void protocolDecl_puch_back(const string protocolStr)
{
    if (protocolStr.length()) {
        protocolDecls.push_back(protocolStr);
    }
}
