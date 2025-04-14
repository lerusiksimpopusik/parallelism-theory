#include <iostream>
#include <fstream>
#include <numeric>
#include "client.h"
#include "functions.h"
#include "server.h"


int main() {

    std::ofstream file1("information_tasks.csv");
    file1 << "task ID"<< "," << "TServer" << "," << "TClient" << "," <<"Operation" << "," << "arg1" << "," <<"arg2" << std::endl;
    std::ofstream file2("tasks_results.csv");
    file2 << "task ID"<< "," << "result" << std::endl;

    Server<double> server(4);
    server.Start();
    SinClient<double, double> sin_client(file1);
    SqrtClient<double, double> sqrt_client(file1);
    PowClient<double, double> pow_client(file1);


    auto client2ServerTask = [&](Client<double, double> &client, Server<double> &server) {
        client.Client2ServerTask(server);
    };

    std::vector<std::thread> client_threads;
    for (int i = 0; i < 10; ++i) {
        client_threads.emplace_back(client2ServerTask, std::ref(sin_client), std::ref(server));
        client_threads.emplace_back(client2ServerTask, std::ref(sqrt_client), std::ref(server));
        client_threads.emplace_back(client2ServerTask, std::ref(pow_client), std::ref(server));
    }

    for (auto &thread: client_threads) {
        thread.join();
    }

    int remainingTasks = server.GetTaskNumber();
    std::vector<size_t> ids_tasks(remainingTasks);
    std::iota(ids_tasks.begin(), ids_tasks.end(), 1);

    while (remainingTasks>0) {
        for (int i = 0; i < remainingTasks; ++i) {
            try {
                auto result = server.RequestResult(ids_tasks[i]);
                if (result.has_value()) {
                    file2 << ids_tasks[i] << "," << result.value() << std::endl;
                    remainingTasks--;
                    ids_tasks.erase(std::remove(ids_tasks.begin(), ids_tasks.end(), ids_tasks[i]), ids_tasks.end());

                }
            } catch (const std::exception &e) {
                std::cerr << "Error for task " << ids_tasks[i] << ": " << e.what() << std::endl;
                i++;
            }
        }
    }

    server.Stop();
    return 0;
}
