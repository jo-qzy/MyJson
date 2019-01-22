#include <iostream>
#include "json.hpp"

using namespace std;

int main() {
    string json_source = "{\
                            \"encoding\" : \"UTF-8\",\
                            \"plug-ins\" : [\
                                \"python\",\
                                \"c++\",\
                                \"ruby\"\
                            ],\
                            \"indent\" : {\
                                \"length\" : 3,\
                                \"use_space\": true\
                            }\
                          }";

    // reader test
    JSON::Reader reader;
    JSON::Value root;
    reader.parse(json_source, root);
    
    // fast writet test
    JSON::FastWriter fw;
    cout << fw.write(root) << endl << endl;
    
    // style writer test
    JSON::StyleWriter sw;
    cout << sw.write(root) << endl << endl;

    // remove test
    root.removeMember("indent");
    cout << fw.write(root) << endl << endl;

    // append test
    JSON::Value item;
    item["array_key"] = "test_key";
    root.append(item);
    cout << fw.write(root) << endl << endl;

    // get member names
    vector<string> names = root.getMemberNames();
    for (auto vt : names)
        cout << vt << " , ";
    cout << endl << endl;

    // as string test

    // resize test
    root.resize(1);
    cout << fw.write(root) << endl << endl;

    // clear test
    root.clear();
    cout << fw.write(root) << endl << endl;
    return 0;
}
