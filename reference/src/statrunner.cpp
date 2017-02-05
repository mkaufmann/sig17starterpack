#include "baseline/ngramer.hpp"
#include "baseline/statngramer.hpp"
#include <fstream>
#include <iostream>

int main(int, char**)
{
   StatNgramer<Ngramer> ngramer;

   // Read initial ngrams
   for (std::string line; std::getline(std::cin, line) && line != "S";) {
      ngramer.add(std::move(line));
   }
   uint32_t initialAdds = ngramer.countAdd;
   ngramer.countAdd = 0;
   std::cout << "R" << std::endl;

   // Parse input
   uint32_t countFlush = 0;
   std::stringstream buffer;
   for (std::string line; std::getline(std::cin, line);) {
      if (line == "F") {
         countFlush++;
         std::cout << buffer.str() << std::flush;
         buffer.str(std::string());
         buffer.clear();
         continue;
      }

      switch (line[0]) {
         case 'Q': {
            auto result = ngramer.query(line.substr(2));
            buffer << stringify(result) << std::endl;
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

   std::cerr << "Initial: " << initialAdds << std::endl;
   std::cerr << "NumQueries: " << ngramer.countDocs << std::endl;
   std::cerr << "NumAdditions: " << ngramer.countAdd << std::endl;
   std::cerr << "NumDeletes: " << ngramer.countRemove << std::endl;
   std::cerr << "NumFlushes: " << countFlush << std::endl;
   std::cerr << "WordsPerDoc: " << ngramer.wordInDocs / ngramer.countDocs << std::endl;
   std::cerr << "MatchProbability: " << ngramer.matchesDocs / (double)ngramer.wordInDocs << std::endl;

   std::ofstream searchNgrams("search_ngrams.txt");
   std::ofstream docNgrams("doc_ngrams.txt");
   if (!searchNgrams || !docNgrams) {
      std::cerr << "Error: Couldn't open files to write ngram information." << std::endl;
      return -1;
   }

   for (auto& ngram : ngramer.searchNgrams) {
      searchNgrams << ngram << std::endl;
   }

   for (auto& ngram : ngramer.docWords) {
      docNgrams << ngram.second << "|" << ngram.first << std::endl;
   }

   searchNgrams.close();
   docNgrams.close();

   return 0;
}