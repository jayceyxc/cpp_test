#include "radius_client.h"

namespace po = boost::program_options;

Redis* g_adsl_redis;

static const char base64Char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(char const* origSigned, unsigned origLength)
{
    unsigned char const* orig = (unsigned char const*)origSigned; // in case any input bytes have the MSB set
    if (orig == NULL) return NULL;

    unsigned const numOrig24BitValues = origLength/3;
    bool havePadding = origLength > numOrig24BitValues*3;
    bool havePadding2 = origLength == numOrig24BitValues*3 + 2;
    unsigned const numResultBytes = 4*(numOrig24BitValues + havePadding);
    char* result = new char[numResultBytes+1]; // allow for trailing '/0'

    // Map each full group of 3 input bytes into 4 output base-64 characters:
    unsigned i;
    for (i = 0; i < numOrig24BitValues; ++i) {
        result[4*i+0] = base64Char[(orig[3*i]>>2)&0x3F];
        result[4*i+1] = base64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
        result[4*i+2] = base64Char[((orig[3*i+1]<<2) | (orig[3*i+2]>>6))&0x3F];
        result[4*i+3] = base64Char[orig[3*i+2]&0x3F];
    }

    // Now, take padding into account.  (Note: i == numOrig24BitValues)
    if (havePadding) {
        result[4*i+0] = base64Char[(orig[3*i]>>2)&0x3F];
        if (havePadding2) {
            result[4*i+1] = base64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
            result[4*i+2] = base64Char[(orig[3*i+1]<<2)&0x3F];
        } else {
            result[4*i+1] = base64Char[((orig[3*i]&0x3)<<4)&0x3F];
            result[4*i+2] = '=';
        }
        result[4*i+3] = '=';
    }

    result[numResultBytes] = '\0';
    return result;
}

void parse_cmdline(int argc, char **argv, po::variables_map &vm)
{
    bool cmdline_error = false;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "display this help message")
        ("host", po::value<std::string>()->default_value("127.0.0.1"),
                    "adsl redis host, default: 127.0.0.1")
        ("port", po::value<int>()->default_value(6388),
                    "adsl redis port, default: 6388")
        ("db", po::value<int>()->default_value(10),
                    "adsl redis db, default: 10")
        ("ip", po::value<std::string>(), "the ip of the register adsl")
        ("adsl", po::value<std::string>(), "the adsl of the register adsl")
        ;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (po::error &e) {
        cmdline_error = true;
    }
    if (vm.count("help") || cmdline_error) {
        std::cout << "Usage: " << argv[0] << " [OPTIONS]\n" << std::endl;
        std::cout << desc << std::endl;
        exit(EXIT_FAILURE);
    }
}

int store_msg_redis(std::string host, int port, int db, const char* ip, std::string adsl){
    int ret = 0 ;
    in_addr addr;
//    char ip_buf[INET_ADDRSTRLEN];
//    addr.s_addr = ip;
//    inet_ntop(AF_INET, &addr, ip_buf, INET_ADDRSTRLEN);
    int on = 1;

    ret = inet_aton(ip, &addr);
    if (ret != 1) {
        cout << "invalid ip address" << endl;
        return 1;
    }
    acn_user_info_t user_info;
    memset(&user_info, 0, sizeof(acn_user_info_t));
#ifdef YINNI
    user_info.flag = USER_ONLINE;
    user_info.lac = 0;
    user_info.ci = 0;
    user_info.type = 4;
    user_info.ip = addr.s_addr;
    snprintf(user_info.name , sizeof(user_info.name) ,"%s" ,adsl.c_str()) ;
    snprintf(user_info.address, sizeof(user_info.address), "%s", "5020-7083");
    snprintf(user_info.imei, sizeof(user_info.imei), "%s", "imei123456");
    snprintf(user_info.imsi, sizeof(user_info.imsi), "%s", "imsi654321");
#elifdef WIRELESS
    user_info.flag = USER_ONLINE;
    user_info.lac = 0;
    user_info.ci = 0;
    user_info.type = 4;
    user_info.ip = addr.s_addr;
    snprintf(user_info.name , sizeof(user_info.name) ,"%s" ,adsl.c_str()) ;
    snprintf(user_info.address, sizeof(user_info.address), "%s", "5020-7083");
    snprintf(user_info.imei, sizeof(user_info.imei), "%s", "imei123456");
    snprintf(user_info.imsi, sizeof(user_info.imsi), "%s", "imsi654321");
#else
    user_info.flag = USER_ONLINE;
    user_info.lac = 0;
    user_info.ci = 0;
    user_info.type = 4;
    user_info.ip = addr.s_addr;
    snprintf(user_info.name , sizeof(user_info.name) ,"%s" ,adsl.c_str()) ;
    snprintf(user_info.imsi, sizeof(user_info.imsi), "%s", "imsi654321");

    struct timeval timeout = {1, 5};
    try {
        g_adsl_redis = new Redis(host, port, timeout, db, on);
    }catch(char const* e ) {
        std::cout << "Standard exception: " << e << std::endl;
        return -1;
    }

    if (user_info.flag == USER_ONLINE || user_info.flag == USER_UPDATE){
        printf("radius capture pkt:name:%s, ip:%s, nettype:%s, sprot:%d, eport:%d",
              user_info.name, ip, user_info.net_type, user_info.nat444_begin_port, user_info.nat444_end_port);
        char *result = base64_encode((char*)(&user_info), sizeof(acn_user_info_t));
        g_adsl_redis->set(string(user_info.name), result);//string(user_info.name));
        free(result);
        g_adsl_redis->expire(string(user_info.name), SECONDS_OF_DAY * 2);
    }
    else
    { //up line
        g_adsl_redis->del(string(user_info.name));
    }

    return ret ;
}

int main(int argc, char* argv[]) {
    po::variables_map vars_map;
    parse_cmdline(argc, argv, vars_map);
    std::string host = vars_map["host"].as<std::string>();
    int port = vars_map["port"].as<int>();
    int db = vars_map["db"].as<int>();
    std::string ip = vars_map["ip"].as<std::string>();
    std::string adsl = vars_map["adsl"].as<std::string>();
//    store_msg_redis("59.174.247.135", "test_user_tag");
    cout << "host: " << host.c_str() << ", port: " << port << ", db: " << db << endl;
    cout << "ip: " << ip.c_str() << endl;
    cout << "adsl: " << adsl.c_str() << endl;
    store_msg_redis(host, port, db, ip.c_str(), adsl);
}
