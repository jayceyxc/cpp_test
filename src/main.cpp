#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "person.h"
#include "http_client.h"
#include "zbase64.h"
#include "decimal_string_label.h"
#include "radius_client.h"

using namespace std;

typedef std::set<DecimalStringLabel> DecimalStringLabelSet;
typedef std::set<DecimalStringLabel>::iterator DecimalStringLabelSetIter;

#define YINNI_MAX_USER_TAG_LEVEL   2
#define YINNI_USER_TAG_LEN_PER_LEVEL 3

void resolver_test()
{
    try
    {
      boost::asio::io_service io_service;
      tcp::resolver resolver(io_service);
      tcp::resolver::query query("112.124.65.153", "9988");
      tcp::resolver::iterator iter = resolver.resolve(query);
      tcp::resolver::iterator end;
      while (iter != end) {
          tcp::endpoint endpoint = *iter++;
          cout << endpoint << endl;
      }
    }
    catch (std::exception& e)
    {
      std::cout << "Exception: " << e.what() << "\n";
    }

    return;
}

void http_client_test() {
    try
    {
      boost::asio::io_service io_service;
//    boostHttpclient_c c(io_service, "192.168.1.46:8187", "/handle_post_request");
      std::ifstream ifs;
      ifs.open("data/requests.txt", std::ifstream::in);
      if(ifs.is_open()) {
          while (ifs.good() && !ifs.eof()) {
              string line_content = "";
              getline(ifs, line_content);
              cout << line_content << endl;
              boostHttpclient_c c(io_service, "112.124.65.153:9988", "/inmobibid", line_content);
          }
          ifs.close();
      }
//      boostHttpclient_c c(io_service, "112.124.33.66:9988", "/inmobibid");
//    client c(io_service, "192.168.1.46:8187", "/");
      io_service.run();
    }
    catch (std::exception& e)
    {
      std::cout << "Exception: " << e.what() << "\n";
    }

    return;
}

static void insert_label(DecimalStringLabelSet &labels, string label_id,
                         int max_level, int len_per_level) {
    if (label_id.empty()) {
        return;
    }
    DecimalStringLabelSet::iterator iter = labels.find(DecimalStringLabel(label_id, max_level, len_per_level));
    if ( iter == labels.end()) {
        labels.insert(DecimalStringLabel(label_id, max_level, len_per_level));
    }
    else {
       if (iter->getLabelLevel() > DecimalStringLabel(label_id, max_level, len_per_level).getLabelLevel()) {
           labels.erase(iter);
           labels.insert(DecimalStringLabel(label_id, max_level, len_per_level));
       }
    }
}

static void label_vec_to_map(std::map<std::string, DecimalStringLabelSet> &label_map, 
                             std::set<std::string> &first_level_id_set,
                             std::vector<string> label_vec,
                             int max_level, int len_per_level) {
    std::vector<string>::iterator iter;
    for (iter = label_vec.begin(); iter != label_vec.end(); iter++) {
        DecimalStringLabel label_id = DecimalStringLabel(*iter, max_level, len_per_level);
        std::map<std::string, DecimalStringLabelSet>::iterator map_iter = label_map.find(label_id.getFirstLevelId());
        if (map_iter != label_map.end()) {
            insert_label(map_iter->second, *iter, max_level, len_per_level);
        } else {
            std::set<DecimalStringLabel> label_set;
            insert_label(label_set, *iter, max_level, len_per_level);
            first_level_id_set.insert(label_id.getFirstLevelId());
            label_map.insert(std::pair<std::string, DecimalStringLabelSet>(label_id.getFirstLevelId(), label_set));
        }
        
    }
}

void label_test() {
    map<std::string, DecimalStringLabelSet> user_tags;
    std::set<std::string> first_level_id_set;
    std::string label = "1001,2002,3001,3002,3003,4001,4002";
    std::vector<std::string> label_vec;
    boost::split(label_vec, label, boost::is_any_of(":,;"),boost::token_compress_on);
    label_vec_to_map(user_tags, first_level_id_set, label_vec, YINNI_MAX_USER_TAG_LEVEL, YINNI_USER_TAG_LEN_PER_LEVEL);
    for (map<std::string, DecimalStringLabelSet>::iterator iter = user_tags.begin(); iter != user_tags.end(); iter++) {
        cout << "first level label id: " << iter->first << endl;
        DecimalStringLabelSet label_set = iter->second;
        for (DecimalStringLabelSetIter set_iter = label_set.begin(); set_iter != label_set.end(); set_iter++) {
            cout << "label id: " << set_iter->getLabelId() << endl;
        }
    }
    cout << first_level_id_set.size() << endl;

    std::string user_tag = "1001,1002,2002,3001,4001,4003,5003";
    std::vector<std::string> user_tag_vec;
    std::set<std::string> matched_first_level_id_set;
    boost::split(user_tag_vec, user_tag, boost::is_any_of(":,;"),boost::token_compress_on);
    for (std::vector<std::string>::iterator iter = user_tag_vec.begin(); iter != user_tag_vec.end(); iter++) {
        DecimalStringLabel label_id = DecimalStringLabel(*iter, YINNI_MAX_USER_TAG_LEVEL, YINNI_USER_TAG_LEN_PER_LEVEL);
        std::map<std::string, DecimalStringLabelSet>::iterator map_iter = user_tags.find(label_id.getFirstLevelId());
        if (map_iter != user_tags.end()) {
            DecimalStringLabelSetIter set_iter = map_iter->second.find(label_id);
            if (set_iter != map_iter->second.end()) {
                cout << label_id.getLabelId() << " has match" << endl;
                matched_first_level_id_set.insert(label_id.getFirstLevelId());
            } else {
                cout << label_id.getLabelId() << " has no match" << endl;
            }
        } else {
            cout << label_id.getLabelId() << " has no first match" << endl;
        }
    }
    cout << matched_first_level_id_set.size() << endl;
    
    if (first_level_id_set.size() == matched_first_level_id_set.size()) {
        cout << "user tag matched" << endl;
    } else {
        cout << "not all user tag matched" << endl;
    }

    return;
}

int main(int argc, char* argv[]) {
    /*
    Person* person = new Person(18, "hello");
    cout << person->display();

    regex expression("^[{](.*)[}]$");
    std::string line = "{hello}";
    cmatch what;
    if(regex_search(line.c_str(),what,expression)){
        //regex_match ： 是对整个输入块的匹配，整个块如不匹配则不能成功
        cout << what.size() << endl;
        for(unsigned int i=0;i<what.size();i++)
            cout<<"str match is :"<<what[i].str()<<endl;
        printf("\n----\n");
    }
    else{
        cout<<"Error Match"<<endl;
    }
    */

//    resolver_test();

//    http_client_test();
    string url = "";
    url = "http://www.iskdpg.com/wjfzj0815/?hr=$[url]&cox=$[adsl|sea]";
    cout << url << endl;
    url = "http://123.hr33.cn/main.js?info_sp=7013";
    cout << url << endl;

    size_t len = 3;
    len = len - 18;
    cout << len << endl;
    string s("009020122000000000");
    string s2("122");
    cout << s.compare(0, len, s2, 0, len) << endl;
    cout << s2.compare(0, len, s, 0, len) << endl;

    label_test();

    return 0;
}
