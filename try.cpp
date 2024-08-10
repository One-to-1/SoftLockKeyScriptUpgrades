#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include <future>
#include <numeric>
#include <mutex>

using namespace std;

mutex coutMutex;

void rollDice(int numRolls, int &maxOnes, int &totalRolls, chrono::time_point<chrono::high_resolution_clock> start_time, int threadId) {
    vector<int> items = {1, 2, 3, 4};
    vector<int> numbers(4, 0);
    int localMaxOnes = 0;
    int localRolls = 0;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 4);

    while (numbers[0] < 177 && localRolls < numRolls) {
        fill(numbers.begin(), numbers.end(), 0);
        for (int i = 0; i < 231; ++i) {
            numbers[dis(gen) - 1]++;
        }
        localRolls++;
        localMaxOnes = max(localMaxOnes, numbers[0]);

        // Live display of elapsed time every 100000 rolls
        if (localRolls % 100000 == 0) {
            auto current_time = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed_time = current_time - start_time;
            {
                lock_guard<mutex> guard(coutMutex);
                // Move the cursor to the specific line for this thread
                cout << "\033[" << (threadId + 1) << ";0H"; // Move cursor to line (threadId + 1)
                cout << "Thread " << threadId << " - Rolls: " << localRolls << ", Elapsed Time: " << elapsed_time.count() << " seconds";
            }
        }
    }

    maxOnes = localMaxOnes;
    totalRolls = localRolls;
}

int main() {
    // Start the timer
    auto start_time = chrono::high_resolution_clock::now();

    int numThreads = thread::hardware_concurrency() - 2;
    cout << "Number of Threads: " << numThreads << endl;

    // Specify the total number of rolls you want to perform
    int totalNumRolls = 1000000000; // Change this value to your desired number of rolls

    // Calculate the number of rolls per thread
    int numRollsPerThread = totalNumRolls / numThreads;

    vector<future<void>> futures;
    vector<int> maxOnes(numThreads, 0);
    vector<int> totalRolls(numThreads, 0);

    // Clear the screen and move the cursor to the top
    cout << "\033[2J\033[H";

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(async(launch::async, rollDice, numRollsPerThread, ref(maxOnes[i]), ref(totalRolls[i]), start_time, i));
    }

    for (auto &f : futures) {
        f.get();
    }

    int finalMaxOnes = *max_element(maxOnes.begin(), maxOnes.end());
    int finalTotalRolls = accumulate(totalRolls.begin(), totalRolls.end(), 0);

    // End the timer
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_time = end_time - start_time;

    cout << "\nHighest Ones Roll: " << finalMaxOnes << endl;
    cout << "Number of Roll Sessions: " << finalTotalRolls << endl;
    cout << "Elapsed Time: " << elapsed_time.count() << " seconds" << endl;

    return 0;
}