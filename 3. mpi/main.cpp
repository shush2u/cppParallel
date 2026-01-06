#include <iostream>
#include <mpi.h>

namespace RankID {
    const int SENDER_1 = 0;
    const int SENDER_2 = 1;
    const int RECEIVER = 2;
    const int PRINTER_EVEN = 3;
    const int PRINTER_ODD = 4;
}

namespace Tag {
    const int DATA = 100;
    const int CONTROL = 101;
}

void Sender(int start) {
    while (true) {
        bool finished = MPI::COMM_WORLD.Iprobe(MPI::ANY_SOURCE, Tag::CONTROL);
        if (finished) {
            std::cout << "Sender finished." << std::endl;
            return;
        };
        
        MPI::COMM_WORLD.Send(&start, 1, MPI::INT, RankID::RECEIVER, Tag::DATA);
        start++;
    }
}

void Receiver(int messageLimit = 20) {
    int receivedMessages = 0;
    MPI_Status status;

    while (receivedMessages < messageLimit) {
        MPI_Probe(MPI_ANY_SOURCE, Tag::DATA, MPI_COMM_WORLD, &status);
        receivedMessages++;

        int number;
        MPI_Recv(&number, 1, MPI_INT, MPI_ANY_SOURCE, Tag::DATA, MPI_COMM_WORLD, &status);
        
        if (number % 2 == 0) {
            MPI_Send(&number, 1, MPI_INT, RankID::PRINTER_EVEN, Tag::DATA, MPI_COMM_WORLD);
        } 
        else {
            MPI_Send(&number, 1, MPI_INT, RankID::PRINTER_ODD, Tag::DATA, MPI_COMM_WORLD);
        }
    }

    MPI_Send(nullptr, 0, MPI_INT, RankID::SENDER_1, Tag::CONTROL, MPI_COMM_WORLD);
    MPI_Send(nullptr, 0, MPI_INT, RankID::SENDER_2, Tag::CONTROL, MPI_COMM_WORLD);
    MPI_Send(nullptr, 0, MPI_INT, RankID::PRINTER_EVEN, Tag::CONTROL, MPI_COMM_WORLD);
    MPI_Send(nullptr, 0, MPI_INT, RankID::PRINTER_ODD, Tag::CONTROL, MPI_COMM_WORLD);

    std::cout << "Receiver finished." << std::endl;
}

void Printer(bool isEven, int arraySize = 20) {
    int* array = new int[arraySize];
    int count = 0;
    MPI_Status status;
    std::string output = "";

    while (true) {
        MPI_Probe(RankID::RECEIVER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        switch (status.MPI_TAG)
        {
        case Tag::CONTROL:
            MPI_Recv(nullptr, 0, MPI_INT, RankID::RECEIVER, Tag::CONTROL, MPI_COMM_WORLD, &status);
            
            output += "[";
            for (int i = 0; i < count; i++) {
                output += std::to_string(array[i]);
                if (i + 1 < count) {
                    output += " ";
                }
            }
            output += "]";
            
            delete[] array;
            std::cout << "Printer finished. Array: " << output << std::endl;
            return;

        case Tag::DATA:
            int number;
            MPI_Recv(&number, 1, MPI_INT, RankID::RECEIVER, Tag::DATA, MPI_COMM_WORLD, &status);
            array[count] = number;
            count++;
            break;

        default:
            std::cout << "Error: Unknown Tag " << status.MPI_TAG << std::endl;
            delete[] array;
            return;
        }
    }
}

int main(int argc, char** argv) {

    MPI::Init();

    int processCount = MPI::COMM_WORLD.Get_size();
    int rank = MPI::COMM_WORLD.Get_rank();

    if (processCount != 5) {
        if (rank == RankID::SENDER_1) {
            std::cout << "Error: Program should be started with exactly 5 processes! Command: mpirun -np 5 <path-to-executable>" << std::endl;
        }
        return 1;
    }

    switch (rank)
    {
    case RankID::SENDER_1:
        Sender(0);
        break;
    case RankID::SENDER_2:
        Sender(11);
        break;
    case RankID::RECEIVER:
        Receiver();
        break;
    case RankID::PRINTER_EVEN:
        Printer(true);
        break;
    case RankID::PRINTER_ODD:
        Printer(false);
        break;
    default:
        break;
    }

    MPI::Finalize();

    return 0;
}