#include "baseline/ngramer.hpp"
#include <iostream>

int main(int, char**)
{
   Ngramer ngramer;
   // Read initial ngrams
   for (std::string line; std::getline(std::cin, line) && line != "S";) {
      ngramer.add(std::move(line));
   }
   std::cout << "R" << std::endl;

   // Parse input
   for (std::string line; std::getline(std::cin, line);) {
      if (line == "F") {
         std::cout << std::flush;
         continue;
      }

      switch (line[0]) {
         case 'Q': {
            auto result = ngramer.query(line.substr(2));
            auto ordered = orderResult(std::move(result));
            std::cout << stringify(ordered) << std::endl;
            break;
         }
         case 'A': {
            ngramer.add(line.substr(2));
            break;
         }
         case 'D': {
            ngramer.remove(line.substr(2));
            break;
         }
         default: {
            std::cerr << "Error unrecognized line: \"" << line << "\"" << std::endl;
            return 1;
         }
      }
   }
   return 0;
}