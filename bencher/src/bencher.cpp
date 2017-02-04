#include "popenRWE.h"

#include <array>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using Timer = std::chrono::high_resolution_clock;

enum class State { Commanding, Verifying };

void writeAll(int fd, std::string str)
{
   auto data = str.c_str();
   size_t offset = 0;
   while (offset != str.size()) {
      auto res = write(fd, data + offset, str.size() - offset);
      if (res < 0) {
         std::cerr << "Error writing: " << res << std::endl;
         exit(-1);
      }
      else {
         offset += res;
      }
   }
}

void readBytes(int fd, size_t totalBytes, std::vector<char>& data)
{
   data.resize(totalBytes);
   size_t readBytes = 0;
   while (readBytes != totalBytes) {
      auto res = read(fd, data.data() + readBytes, totalBytes - readBytes);
      if (res < 0) {
         std::cerr << "Error reading: " << res << std::endl;
         exit(-1);
      }
      else {
         readBytes += res;
      }
   }
}

int main(int argc, char** argv)
{
   if (argc != 3) {
      std::cerr << "Error:   Invalid number of arguments: " << argc << std::endl;
      std::cerr << std::endl;
      std::cerr << "Usage:   bencher <workloadFile> <program>" << std::endl;
      std::cerr << std::endl;
      std::cerr << "Example: bencher sample.load ./run.sh" << std::endl;
      return -1;
   }

   std::string workloadFile = std::string(argv[1]);
   std::string program = std::string(argv[2]);

   std::array<int, 3> pipes;
   auto pid = popenRWE(reinterpret_cast<int*>(&pipes), program.c_str());

   // Open workload
   std::ifstream file(workloadFile);
   if (!file) {
      std::cerr << "Error opening workload file: " << workloadFile << std::endl;
      return -1;
   }

   // Output initial ngrams
   std::stringstream outputBuffer;
   for (std::string line; std::getline(file, line);) {
      outputBuffer << line << std::endl;
      if (line == "S") {
         writeAll(pipes[0], outputBuffer.str());
         outputBuffer.clear();
         outputBuffer.str(std::string());
         break;
      }
   }

   if (outputBuffer.gcount() > 0) {
      std::cerr << "Invalid load file, no S encountered" << std::endl;
      return -1;
   }

   // Read ready sign
   std::vector<char> readBuffer;
   readBytes(pipes[1], 2, readBuffer);
   if (readBuffer[0] != 'R' || readBuffer[1] != '\n') {
      std::cerr << "No proper ready sign by program" << std::endl;
      return -1;
   }
   else {
      std::cout << "Program finished loading" << std::endl;
   }

   // Output commands
   size_t totalDuration = 0;
   auto startTime = Timer::now();
   std::vector<char> verifyBuffer;
   auto state = State::Commanding;
   for (std::string line; std::getline(file, line);) {
      switch (state) {
         case State::Commanding: {
            outputBuffer << line << std::endl;
            if (line == "F") {
               startTime = Timer::now(); // Start timer when sending data to client
               writeAll(pipes[0], outputBuffer.str());
               outputBuffer.clear();
               outputBuffer.str(std::string());
               state = State::Verifying;
            }
            break;
         }
         case State::Verifying: {
            if (line[0] != 'R') {
               std::cerr << "Invalid load file, no S encountered" << std::endl;
               return -1;
            }
            auto count = std::strtoll(line.substr(2).c_str(), nullptr, 0);
            readBytes(pipes[1], count, readBuffer);
            // Measure time when receiving complete answer from client
            auto duration = Timer::now() - startTime;
            totalDuration += std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
            // Verify answer
            verifyBuffer.resize(count);
            file.read(verifyBuffer.data(), count);
            if (!file) {
               std::cerr << "Invalid load file, no proper verify data" << std::endl;
               return -1;
            }
            if (strncmp(readBuffer.data(), verifyBuffer.data(), count) != 0) {
               std::cerr << "Invalid results!" << std::endl;
               return -1;
            }
            state = State::Commanding;
            break;
         }
      }
   }

   pcloseRWE(pid, reinterpret_cast<int*>(&pipes));

   std::cout << totalDuration << " microseconds" << std::endl;

   return 0;
}