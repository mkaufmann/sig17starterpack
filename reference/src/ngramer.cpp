#include "baseline/ngramer.hpp"
#include <algorithm>
#include <iterator>
#include <sstream>

std::vector<std::pair<std::string, size_t>> Ngramer::query(const std::string& document)
{
   std::vector<std::pair<std::string, size_t>> results;
   auto text = " " + document + " ";

   for (auto& ngram : ngrams) {
      auto needle = " " + ngram + " ";
      auto pos = text.find(needle);
      if (pos != std::string::npos) {
         results.push_back({ngram, pos});
      }
   }
   return results;
}

void Ngramer::add(std::string&& ngram) { ngrams.emplace_back(ngram); }

void Ngramer::remove(const std::string& ngram)
{
   ngrams.erase(std::remove(ngrams.begin(), ngrams.end(), ngram), ngrams.end());
}

std::vector<std::string> orderResult(std::vector<std::pair<std::string, size_t>>&& ngrams)
{
   auto cmp = [](const std::pair<std::string, size_t>& a, const std::pair<std::string, size_t>& b) {
      return (a.second < b.second) || ((a.second == b.second) && a.first.size() < b.first.size());
   };
   std::sort(ngrams.begin(), ngrams.end(), cmp);

   std::vector<std::string> result;
   for (auto ngram : ngrams) {
      result.emplace_back(std::move(ngram.first));
   }
   return result;
}

std::string stringify(const std::vector<std::string>& strings)
{
   std::ostringstream buffer;
   bool first = true;
   for (auto& string : strings) {
      if (first) {
         first = false;
      }
      else {
         buffer << '|';
      }
      buffer << string;
   }
   return buffer.str();
}
