//1552212 端启航
#include <iostream>
#include "MMX/RsaSignature.h"
#include <sstream>
#include "TCPClient.h"
#include "Member.h"

using namespace std;


TCPClient *client;
group_sig::member *m;

string id;//id，由命令行输入
ZZ psk;

void send_req(u_int8_t type, string msg = "") {
    header_t head;
    head.proto_ori = PROTO_C2S;
    head.proto_type = type;
    if (msg == "") {
        head.len = 0;
        client->SendPacket((char *) &head, HEADLEN);
    } else {
        head.len = msg.size();
        char *buffer = new char[HEADLEN + msg.size()];
        memcpy(buffer, &head, HEADLEN);
        memcpy(buffer + HEADLEN, msg.c_str(), msg.size());
        client->SendPacket(buffer, HEADLEN + msg.size());
    }
}

void onRecv_m(ClientData *data) {
    header_t *header;
    header = (header_t *) (data->recv_playload);

    string msg;
    switch (header->proto_type) {
        case PROTO_PUB_PARA: {
//		group_sig::public_para* p=new group_sig::public_para;
            char *p = new char[header->len + 1];
            memcpy(p, data->recv_playload + HEADLEN, header->len);
            stringstream stream(p);
            group_sig::public_para *para = new group_sig::public_para;
            string temp;
            stream >> temp;
            para->a = Cryptography::stringToNumber(temp, false);
            stream >> temp;
            para->b = Cryptography::stringToNumber(temp, false);
            stream >> temp;
            para->epsilon = Cryptography::stringToNumber(temp, false);
            stream >> temp;
            para->G = Cryptography::stringToNumber(temp, false);
            stream >> temp;
            para->g = Cryptography::stringToNumber(temp, false);
            stream >> temp;
            para->n = Cryptography::stringToNumber(temp, false);
            long l;
            stream >> l;
            para->lambda = l;

            m = new group_sig::member(id, para);

            //send PROTO_JOIN_GROUP
            send_req(PROTO_JOIN_GROUP, m->JoinGroupMsg(psk));
            break;
        }
        case PROTO_JOIN_GROUP: {
            msg = get_str(data->recv_playload);
            m->onRecvV(msg);
            break;
        }
        case PROTO_KEY_EX: {
            msg = get_str(data->recv_playload);
            m->onKeyExchangeRequestRecv(msg);
            break;
        }
        case PROTO_KEY_BROADCAST: {
            msg = get_str(data->recv_playload);
            m->onGroupKeyBoardcastRecv(msg);
            break;
        }
        default:
            break;
    }
}

void onConnected(ClientData */*data*/) {
    send_req(PROTO_PUB_PARA, id);
}

int main_m(string ip, u_int16_t port) {
    client = new TCPClient(inet_addr(ip.c_str()), port);
    client->setOnConnectedCallBack(onConnected);
    client->setOnRecvCallBack(onRecv_m);
    client->ConnectServer();
    return 0;
}
