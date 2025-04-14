#ifndef CLIENT_H
#define CLIENT_H

#include <random>
#include <string>
#include <mutex>
#include <fstream>
#include "server.h"

std::mutex outputMutex;

void safePrint(std::ofstream& out, const std::string &message);

std::string getTypeName(const std::type_info &type);

template<typename Tserver, typename Tclient>
std::string formatMessage(size_t taskId,const std::string& operationName, Tclient arg1, Tclient *arg2 = nullptr);

template<typename Tclient, typename Tserver>
class Client {
public:
    Client(std::ofstream& out);
    virtual ~Client() = default;
    virtual size_t Client2ServerTask(Server<Tserver> &server) = 0;

protected:
    std::ofstream& out_;
    std::mt19937 gen_;
    Tclient GenerateRandom(Tclient min, Tclient max);
};

template<typename Tclient, typename Tserver>
class SinClient : public Client<Tclient, Tserver> {
public:
    SinClient(std::ofstream& out);
    size_t Client2ServerTask(Server<Tserver> &server) override;
};

template<typename Tclient, typename Tserver>
class SqrtClient : public Client<Tclient, Tserver> {
public:
    SqrtClient(std::ofstream& out);
    size_t Client2ServerTask(Server<Tserver> &server) override;
};

template<typename Tclient, typename Tserver>
class PowClient : public Client<Tclient, Tserver> {
public:
    PowClient(std::ofstream& out);
    size_t Client2ServerTask(Server<Tserver> &server) override;
};

#include "client.tpp"

#endif
