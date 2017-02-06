#include "baseline/ngramer.hpp"
#include <cassert>
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
      std::cerr << "Error opening search ngrams file: " << fileName << std::endl;
      throw;
   }

   std::vector<std::string> ngrams;
   for (std::string line; std::getline(file, line);) {
      if (line.size() == 0) {
         continue;
      }
      ngrams.emplace_back(std::move(line));
   }
   return ngrams;
}

std::pair<std::vector<std::string>, std::vector<uint32_t>> readWeightedNgrams(const std::string& fileName)
{
   std::ifstream file(fileName);
   if (!file) {
      std::cerr << "Error opening doc ngrams file: " << fileName << std::endl;
      throw;
   }

   std::vector<std::string> docNgrams;
   std::vector<uint32_t> docNgramWeights;
   for (std::string line; std::getline(file, line);) {
      if (line.size() == 0) {
         continue;
      }
      auto seperatorPos = line.find("|");
      if (seperatorPos == std::string::npos) {
         std::cerr << "Invalid doc ngram line: " << line << std::endl;
         throw;
      }

      auto weight = std::strtoll(line.substr(0, seperatorPos).c_str(), nullptr, 0);
      auto ngram = line.substr(seperatorPos + 1);
      docNgrams.emplace_back(std::move(ngram));
      docNgramWeights.emplace_back(weight);
   }
   return std::make_pair(std::move(docNgrams), std::move(docNgramWeights));
}

struct Generator {
   Rng rng;
   std::uniform_int_distribution<size_t> searchNgramDist;
   std::discrete_distribution<size_t> docNgramsDist;
   std::normal_distribution<> docNgramsCount;
   std::bernoulli_distribution ngramMatch;

   Generator(uint64_t seed, size_t searchNgramCount, std::vector<uint32_t>& docNgramWeights, size_t avgNgramsPerDoc, double matchProb)
      : rng(seed), searchNgramDist(0, searchNgramCount - 1), docNgramsDist(docNgramWeights.begin(), docNgramWeights.end()),
        docNgramsCount(avgNgramsPerDoc, avgNgramsPerDoc / 4), ngramMatch(matchProb)
   {
   }

   std::unordered_set<std::string> generatePrologue(const std::vector<std::string>& ngrams, const uint32_t ngramCount)
   {
      std::unordered_set<std::string> result;
      result.reserve(ngramCount);
      for (uint32_t i = 0; i < ngramCount;) {
         auto ix = searchNgramDist(rng);
         assert(ix < ngrams.size());
         auto ngram = ngrams[ix];
         if (result.find(ngram) == result.end()) {
            result.insert(ngram);
            i++;
         }
      }

      return result;
   }

   std::string generateAddNgram(const std::vector<std::string>& ngrams, std::set<std::string>& active)
   {
      auto ngram = getRandomPassive(ngrams, active, searchNgramDist);
      active.insert(ngram);
      return ngram;
   }

   std::string generateDeleteNgram(const std::vector<std::string>& ngrams, std::set<std::string>& active)
   {
      auto ngram = getRandomActive(ngrams, active, searchNgramDist);
      active.erase(ngram);
      return ngram;
   }

   std::string generateQuery(const std::vector<std::string>& docNgrams, const std::set<std::string>& active)
   {
      auto numNgrams = docNgramsCount(rng);
      while (numNgrams < 1) {
         numNgrams = docNgramsCount(rng);
      }


      std::unordered_set<std::string> matched;
      std::stringstream query;
      for (uint32_t i = 0; i < numNgrams; i++) {
         bool matches = ngramMatch(rng);
         query << " ";
         if (matches) {
            while (true) {
               auto ngram = getRandomActive(docNgrams, active, docNgramsDist);
               query << ngram;
               if (matched.find(ngram) == matched.end()) {
                  matched.insert(ngram);
                  break;
               }
               else {
                  if (matched.size() == active.size()) {
                     std::cerr << "Not enough ngrams!" << std::endl;
                     throw;
                  }
                  query << " ";
               }
            }
         }
         else {
            query << getRandomPassive(docNgrams, active, docNgramsDist);
         }
      }

      return query.str();
   }

   template <typename Dist>
   std::string getRandomActive(const std::vector<std::string>& ngrams, const std::set<std::string>& active, Dist& dist)
   {
      while (true) {
         if (active.size() == 0) {
            std::cerr << "Not enough ngrams!" << std::endl;
            throw;
         }

         // Should find random element to delete in constant time
         auto pick = dist(rng);
         auto pos = active.lower_bound(ngrams[pick]);
         if (pos != active.end()) {
            auto ngram = *pos;
            return ngram;
         }
      }
   }

   template <typename Dist>
   std::string getRandomPassive(const std::vector<std::string>& ngrams, const std::set<std::string>& active, Dist& dist)
   {
      while (true) {
         if (active.size() == ngrams.size()) {
            std::cerr << "Not enough ngrams!" << std::endl;
            throw;
         }

         auto pick = dist(rng);
         if (active.find(ngrams[pick]) == active.end()) {
            auto ngram = ngrams[pick];
            return ngram;
         }
      }
   }
};

int main(int argc, char** argv)
{
   if (argc != 11 && argc != 12) {
      std::cerr << "Error:   Invalid number of arguments: " << argc << std::endl;
      std::cerr << std::endl;
      std::cerr << "Usage:   generator <searchNgramFile> <docNgramFile> <initialNgramCount> "
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

   const std::string searchNgramFile = std::string(argv[1]);
   const std::string docNgramFile = std::string(argv[2]);
   const uint32_t initialNgramCount = std::strtoul(argv[3], nullptr, 0);
   const uint32_t weightQuery = std::strtoul(argv[4], nullptr, 0);
   const uint32_t weightAdd = std::strtoul(argv[5], nullptr, 0);
   const uint32_t weightRemove = std::strtoul(argv[6], nullptr, 0);
   const uint32_t weightFlush = std::strtoul(argv[7], nullptr, 0);
   const uint32_t numOperations = std::strtoul(argv[8], nullptr, 0);
   const uint32_t averageNgramsPerQuery = std::strtoul(argv[9], nullptr, 0);
   const double matchProbability = std::strtod(argv[10], nullptr);
   // High probability
   // Document length
   uint64_t seed = time(NULL);
   if (argc == 12) {
      seed = std::strtoll(argv[11], nullptr, 0);
   }

   auto searchNgrams = readNgrams(searchNgramFile);
   if (searchNgrams.size() < initialNgramCount) {
      std::cerr << "Too few ngrams for parameters" << std::endl;
      return -1;
   }

   auto docNgrams = readWeightedNgrams(docNgramFile);

   Generator gen(seed, searchNgrams.size(), docNgrams.second, averageNgramsPerQuery, matchProbability);

   auto initialNgrams = gen.generatePrologue(searchNgrams, initialNgramCount);

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
            auto query = gen.generateQuery(docNgrams.first, activeNgrams);
            {
               auto result = oracle.query(query);
               if (result[0].size() == 0) {
                  throw;
               }
               batchResults.emplace_back(stringify(result));
            }
            std::cout << "Q" << query << std::endl;
            break;
         }
         case 1: {
            auto ngram = gen.generateAddNgram(searchNgrams, activeNgrams);
            oracle.add(std::move(ngram));
            std::cout << "A " << ngram << std::endl;
            break;
         }
         case 2: {
            auto ngram = gen.generateDeleteNgram(searchNgrams, activeNgrams);
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