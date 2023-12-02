#include <iostream>
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <fstream>
#include <string>

using namespace std;


struct Message {
    string header;
    string payload;
    string additionalContent;


    // Serialize the struct to a string
    string serialize() const {
        return header + ',' + payload + ',' + additionalContent;
    }

    static Message deserialize(const string& data) {
        Message message;
        size_t pos = data.find(',');
        if (pos != string::npos) {
            message.header = data.substr(0, pos);

            size_t nextPos = data.find(',', pos + 1);
            if (nextPos != string::npos) {
                message.payload = data.substr(pos + 1, nextPos - pos - 1);

                // Corrected the starting position for additionalContent
                pos = nextPos + 1;
                message.additionalContent = data.substr(pos);
            }
        }
        return message;
    }
};




void handle_client(asio::ip::tcp::socket& socket) {
    asio::streambuf buffer;
    std::error_code error;

    // Read data from the client (file name)
    asio::read_until(socket, buffer, '\n', error);
    std::cout << asio::buffer_cast<const char*>(buffer.data());
    if (error) {
        std::cerr << "Error reading file name: " << error.message() << std::endl;
        return;
    }

    Message m = Message::deserialize(asio::buffer_cast<const char*>(buffer.data()));
    std::string fileName = m.payload;
    std::ifstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return;
    }
    if (m.header == "d") {
        std::ifstream file(fileName, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << fileName << std::endl;
            return;
        }

        buffer.consume(buffer.size());  
        file.seekg(0, std::ios::end);
        std::size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);


        buffer.prepare(fileSize);

        file.read(asio::buffer_cast<char*>(buffer.prepare(fileSize)), fileSize);

        buffer.commit(fileSize);



        std::cout << "Sent: " << asio::buffer_cast<const char*>(buffer.data());


        if (error) {
            std::cerr << "Error reading file content: " << error.message() << std::endl;
            return;
        }

        asio::write(socket, buffer, error);
        if (error) {
            std::cerr << "Error writing file content: " << error.message() << std::endl;
            return;
        }

        std::cout << "File sent successfully." << std::endl;
    }
    else if(m.header == "u"){
        ofstream outputFile(fileName, ios::app);
        if (!outputFile.is_open()) {
            cerr << "Error opening output file." << endl;
            return;
        }
        cout << m.additionalContent;
        outputFile << m.additionalContent;

        outputFile.close();


        
    }

    
}


int main() {
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor(io_context,
        asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8888));

    std::cout << "Listening on port 8888." << std::endl;

    while (true) {
        asio::ip::tcp::socket socket(io_context);
        acceptor.accept(socket);

        std::cout << "New connection from: " << socket.remote_endpoint() << std::endl;
        handle_client(socket);

    }

    return 0;
}
