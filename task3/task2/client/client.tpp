#ifndef CLIENT_TPP
#define CLIENT_TPP

#include "client.h"
#include "functions.h"

void safePrint(std::ofstream& out, const std::string &message) {
    std::lock_guard<std::mutex> lock(outputMutex);
    out << message << std::endl;
}

std::string getTypeName(const std::type_info &type) {
    std::string name = type.name();
    if (name == "d") return "double";
    if (name == "i") return "int";
    if (name == "f") return "float";
    if (name == "c") return "char";
    if (name == "l") return "long";
    if (name == "x") return "long long";
    if (name == "s") return "short";
    if (name == "m") return "unsigned long";
    if (name == "j") return "unsigned int";
    if (name == "h") return "unsigned short";
    if (name == "y") return "unsigned long long";
    if (name == "b") return "bool";
    return name;
}

template<typename Tserver, typename Tclient>
std::string formatMessage(size_t taskId, const std::string& operationName, Tclient arg1, Tclient *arg2) {
    std::string typeNameServer = getTypeName(typeid(Tserver));
    std::string typeNameClient = getTypeName(typeid(Tclient));

    if (arg2 != nullptr) {
        return std::to_string(taskId) + "," + typeNameServer + "," + typeNameClient + operationName +
               std::to_string(static_cast<double>(arg1)) + "," + std::to_string(static_cast<double>(*arg2));
    } else {
        return std::to_string(taskId) + "," + typeNameServer + "," + typeNameClient + operationName +
               std::to_string(static_cast<double>(arg1));
    }
}

template<typename Tclient, typename Tserver>
Client<Tclient, Tserver>::Client(std::ofstream& out) : out_(out), gen_(std::random_device{}()) {}

template<typename Tclient, typename Tserver>
Tclient Client<Tclient, Tserver>::GenerateRandom(Tclient min, Tclient max) {
    if constexpr (std::is_integral_v<Tclient>) {
        std::uniform_int_distribution<Tclient> dist(min, max);
        return dist(gen_);
    } else {
        std::uniform_real_distribution<Tclient> dist(min, max);
        return dist(gen_);
    }
}

template<typename Tclient, typename Tserver>
SinClient<Tclient, Tserver>::SinClient(std::ofstream& out) : Client<Tclient, Tserver>(out) {}

template<typename Tclient, typename Tserver>
size_t SinClient<Tclient, Tserver>::Client2ServerTask(Server<Tserver> &server) {
    Tclient arg = this->GenerateRandom(Tclient(-10), Tclient(10));
    int delay_ms = this->GenerateRandom(1000, 4000);
    size_t task_id = server.AddTask([arg, delay_ms] { return MathFunctions::FunSin(arg, delay_ms); });

    safePrint(this->out_, formatMessage<Tserver, Tclient>(task_id,  ",sin,", arg));

    return task_id;
}

template<typename Tclient, typename Tserver>
SqrtClient<Tclient, Tserver>::SqrtClient(std::ofstream& out) : Client<Tclient, Tserver>(out) {}

template<typename Tclient, typename Tserver>
size_t SqrtClient<Tclient, Tserver>::Client2ServerTask(Server<Tserver> &server) {
    Tclient arg = this->GenerateRandom(Tclient(0), Tclient(100));
    int delay_ms = this->GenerateRandom(1000, 4000);
    size_t task_id = server.AddTask([arg, delay_ms] { return MathFunctions::FunSqrt(arg, delay_ms); });

    safePrint(this->out_, formatMessage<Tserver, Tclient>(task_id, ",sqrt,", arg));

    return task_id;
}

template<typename Tclient, typename Tserver>
PowClient<Tclient, Tserver>::PowClient(std::ofstream& out) : Client<Tclient, Tserver>(out) {}

template<typename Tclient, typename Tserver>
size_t PowClient<Tclient, Tserver>::Client2ServerTask(Server<Tserver> &server) {
    Tclient x = this->GenerateRandom(Tclient(0), Tclient(10));

    Tclient y = x == 0 ? this->GenerateRandom(Tclient(0), Tclient(5))
                       : this->GenerateRandom(Tclient(-5), Tclient(5));
    int delay_ms = this->GenerateRandom(1000, 4000);
    size_t task_id = server.AddTask([x, y, delay_ms] { return MathFunctions::FunPow(x, y, delay_ms); });

    safePrint(this->out_, formatMessage<Tserver, Tclient>(task_id, ",pow,", x, &y));

    return task_id;
}

#endif