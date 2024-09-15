#include "zmq.hpp"
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Create ZMQ Context
    zmq::context_t context(1);
    // Create the Publish socket
    zmq::socket_t publisher(context, ZMQ_PUB);

    // Bind the server to a specific TCP port
    publisher.bind("tcp://*:5555");
    // Sleep for 1 sec, if this sleep is removed, you may lose some initial messages while it is
    // being binded
    usleep(1000000);

    int client_count = 0; // Number of clients that have connected
    int iteration = 0;    // Iteration counter for the server

    while (true) {
        std::string message = "Client " + std::to_string(client_count + 1) + ": Iteration " +
                              std::to_string(++iteration);

        zmq::message_t zmq_message(message.size());
        memcpy(zmq_message.data(), message.data(), message.size());
        publisher.send(zmq_message, zmq::send_flags::none);
    }

    return 0;
}