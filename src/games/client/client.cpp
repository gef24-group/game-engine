#include "zmq.hpp"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    // Create ZMQ Context
    zmq::context_t context(1);
    // Create the Subscribe socket
    zmq::socket_t subscriber(context, ZMQ_SUB);

    subscriber.connect("tcp://localhost:5555");

    // Subscribe to all incoming messages
    subscriber.set(zmq::sockopt::subscribe, "");

    while (true) {
        zmq::message_t zmq_message;

        // Wait for the next message from the server
        subscriber.recv(zmq_message, zmq::recv_flags::none);

        // Convert the message to a string and print it
        std::string message(static_cast<char *>(zmq_message.data()), zmq_message.size());
        std::cout << "Client received: " << message << std::endl;
    }

    return 0;
}