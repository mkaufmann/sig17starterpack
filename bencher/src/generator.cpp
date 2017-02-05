#include "baseline/ngramer.hpp"
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <unordered_set>

using Rng = std::mt19937_64;

std::vector<std::string> readNgrams(const std::string& fileName)
{
   std::ifstream file(fileName);
   if (!file) {
      std::cerr << "Error opening ngrams file: " << fileName << std::endl;
      throw;
   }

   std::vector<std::string> ngrams;
   for (std::string line; std::getline(file, line);) {
      ngrams.emplace_back(std::move(line));
   }
   return ngrams;
}

struct Generator {
   Rng rng;
   std::uniform_int_distribution<size_t> ngramDist;
   std::normal_distribution<> docNgrams;
   std::bernoulli_distribution ngramMatch;

   Generator(uint64_t seed, size_t ngramCount, size_t avgNgramsPerDoc, double matchProb)
      : rng(seed), ngramDist(0, ngramCount), docNgrams(avgNgramsPerDoc, avgNgramsPerDoc / 4), ngramMatch(matchProb)
   {
   }

   std::unordered_set<std::string> generatePrologue(const std::vector<std::string>& ngrams, const uint32_t ngramCount)
   {
      std::unordered_set<std::string> result;
      result.reserve(ngramCount);
      for (uint32_t i = 0; i < ngramCount;) {
         auto ngram = ngrams[ngramDist(rng)];
         if (result.find(ngram) == result.end()) {
            result.insert(ngram);
            i++;
         }
      }

      return result;
   }

   std::string generateAddNgram(const std::vector<std::string>& ngrams, std::set<std::string>& active)
   {
      auto ngram = getRandomPassive(ngrams, active);
      active.insert(ngram);
      return ngram;
   }

   std::string generateDeleteNgram(const std::vector<std::string>& ngrams, std::set<std::string>& active)
   {
      auto ngram = getRandomActive(ngrams, active);
      active.erase(ngram);
      return ngram;
   }

   std::string generateQuery(const std::vector<std::string>& ngrams, const std::set<std::string>& active)
   {
      auto numNgrams = docNgrams(rng);
      while (numNgrams < 1) {
         numNgrams = docNgrams(rng);
      }

      std::stringstream query;
      for (uint32_t i = 0; i < numNgrams; i++) {
         bool matches = ngramMatch(rng);
         query << " ";
         if (matches) {
            query << getRandomActive(ngrams, active);
         }
         else {
            query << getRandomPassive(ngrams, active);
         }
      }

      return query.str();
   }

   std::string getRandomActive(const std::vector<std::string>& ngrams, const std::set<std::string>& active)
   {
      while (true) {
         if (active.size() == 0) {
            std::cerr << "Not enough ngrams!" << std::endl;
            throw;
         }

         // Should find random element to delete in constant time
         auto pick = ngramDist(rng);
         auto pos = active.lower_bound(ngrams[pick]);
         if (pos != active.end()) {
            auto ngram = *pos;
            return ngram;
         }
      }
   }

   std::string getRandomPassive(const std::vector<std::string>& ngrams, const std::set<std::string>& active)
   {
      while (true) {
         if (active.size() == ngrams.size()) {
            std::cerr << "Not enough ngrams!" << std::endl;
            throw;
         }

         auto pick = ngramDist(rng);
         if (active.find(ngrams[pick]) == active.end()) {
            auto ngram = ngrams[pick];
            return ngram;
         }
      }
   }
};

int main(int argc, char** argv)
{
   if (argc != 10 && argc != 11) {
      std::cerr << "Error:   Invalid number of arguments: " << argc << std::endl;
      std::cerr << std::endl;
      std::cerr << "Usage:   generator <ngramFile> <initialNgramCount> "
                   "<weightQuery> <weightAdd> <weightRemove> <weightFlush> "
                   "<numOperations> <avgNgramsPerDoc> <ngramMatchProb> (seed)"
                << std::endl;
      std::cerr << std::endl;
      std::cerr << "Example: generator /usr/share/myspell/dicts/en_US.dic 100 "
                   "100 20 20 5 200 12 0.2 1988"
                << std::endl;
      std::cerr << " (Note: As the example uses a dictionary it won't create "
                   "ngrams using several word.)"
                << std::endl;
      return -1;
   }

   const std::string ngramFile = std::string(argv[1]);
   const uint32_t initialNgramCount = std::strtoul(argv[2], nullptr, 0);
   const uint32_t weightQuery = std::strtoul(argv[3], nullptr, 0);
   const uint32_t weightAdd = std::strtoul(argv[4], nullptr, 0);
   const uint32_t weightRemove = std::strtoul(argv[5], nullptr, 0);
   const uint32_t weightFlush = std::strtoul(argv[6], nullptr, 0);
   const uint32_t numOperations = std::strtoul(argv[7], nullptr, 0);
   const uint32_t averageNgramsPerQuery = std::strtoul(argv[8], nullptr, 0);
   const double matchProbability = std::strtod(argv[9], nullptr);
   // High probability
   // Document length
   uint64_t seed = time(NULL);
   if (argc == 11) {
      seed = std::strtoll(argv[10], nullptr, 0);
   }

   auto ngrams = readNgrams(ngramFile);
   if (ngrams.size() < initialNgramCount) {
      std::cerr << "Too few ngrams for parameters" << std::endl;
      return -1;
   }

   Generator gen(seed, ngrams.size(), averageNgramsPerQuery, matchProbability);

   auto initialNgrams = gen.generatePrologue(ngrams, initialNgramCount);

   std::set<std::string> activeNgrams;
   Ngramer oracle;
   for (auto& ngram : initialNgrams) {
      std::string ngramCopy = ngram;
      oracle.add(std::move(ngramCopy));
      activeNgrams.insert(ngram);
      std::cout << ngram << std::endl;
   }
   initialNgrams.clear();

   std::cout << "S" << std::endl;

   std::discrete_distribution<uint8_t> opDist({(double)weightQuery, (double)weightAdd, (double)weightRemove, (double)weightFlush});
   std::vector<std::string> batchResults;
   for (uint32_t i = 0; i < numOperations; i++) {
      auto op = opDist(gen.rng);
      switch (op) {
         case 0: {
            auto query = gen.generateQuery(ngrams, activeNgrams);
            {
               auto result = oracle.query(query);
               batchResults.emplace_back(stringify(result));
            }
            std::cout << "Q" << query << std::endl;
            break;
         }
         case 1: {
            auto ngram = gen.generateAddNgram(ngrams, activeNgrams);
            oracle.add(std::move(ngram));
            std::cout << "A " << ngram << std::endl;
            break;
         }
         case 2: {
            auto ngram = gen.generateDeleteNgram(ngrams, activeNgrams);
            oracle.remove(std::move(ngram));
            std::cout << "D " << ngram << std::endl;
            break;
         }
         case 3: {
            std::cout << "F" << std::endl;
            std::stringstream results;
            for (auto& line : batchResults) {
               results << line << std::endl;
            }
            batchResults.clear();
            auto resultStr = results.str();
            std::cout << "R " << resultStr.size() << std::endl;
            std::cout << resultStr << std::flush;
            break;
         }
         default: {
            std::cerr << "Impossible" << std::endl;
            return 1;
         }
      }
   }
   return 0;
}