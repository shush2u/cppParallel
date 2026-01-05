#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std::chrono_literals;

class Monitor {
    std::string line_;
    std::mutex mutex_;
    std::condition_variable cv_;
    int vowelCounter_;
    
public:
    Monitor() {
        line_ = "*";
        vowelCounter_ = 0;
    }

    void addSymbol(char symbol) {
        if (symbol != 'A' && symbol != 'B' && symbol != 'C') {
            throw new std::invalid_argument("addSymbol() can only the the symbols 'A', 'B' and 'C' (case-sensitive).");
        }

        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this, symbol] { return isVowel_(symbol) || canAddConsonant_() || isFinished_(); });

        if (isFinished_()) {
            cv_.notify_all();
            return;
        };

        if(isVowel_(symbol)) {
            vowelCounter_++;
            cv_.notify_one();
        }
        else {
            vowelCounter_ = 0;
        }

        line_ += symbol;
    }

    void printLine() {
        std::unique_lock lock(mutex_);
        std::cout << line_.length() - 1 << " " << line_ << std::endl;
    }

    bool isFinished() {
        std::unique_lock lock(mutex_);
        return isFinished_();
    }

private:
    bool isFinished_() {
        return line_.length() == 16; // Because we start with "*"
    }

    bool canAddConsonant_() {
        return vowelCounter_ >= 3;
    }

    bool isVowel_(char symbol) {
        return symbol == 'A';
    }
};

void addSymbolsToMonitor(Monitor *monitor, char symbol) {
    while(!monitor->isFinished()) {
        monitor->addSymbol(symbol);
        std::this_thread::sleep_for(5ms); // Prevent 'A' (or any other) thread from just spamming addSymbol()
    }
}

int main(int argc, char** argv) {

    Monitor monitor;
    std::vector<std::thread*> threads;
    threads.reserve(3);

    threads[0] = new std::thread( [&monitor] { addSymbolsToMonitor(&monitor, 'A'); } );
    threads[1] = new std::thread( [&monitor] { addSymbolsToMonitor(&monitor, 'B'); } );
    threads[2] = new std::thread( [&monitor] { addSymbolsToMonitor(&monitor, 'C'); } );

    while (!monitor.isFinished()) {
        monitor.printLine();
    }

    threads[0]->join();
    threads[1]->join();
    threads[2]->join();

    monitor.printLine();

    return 0;
}