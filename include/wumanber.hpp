#ifndef WUMANBER_HPP_
#define WUMANBER_HPP_

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <set>
#include <map>
#include "ad_pattern.hpp"

typedef std::map<std::string, unsigned> ResultPattPosMap;
typedef std::pair<unsigned int, int> PrefixIdPairType;
typedef std::vector<PrefixIdPairType> PrefixTableType;

/*
 * handle the case for general pattern: with asterisk in the middle, 
 * current wumanber based pattern matching only supports prefix and suffix of asterisk (*)
 * added support for pattern with * in the middle. 
 * 1. split pattern into multiple base patterns: an ordered set of base patterns
 *    base pattern is string without *
 *    each ad maintains a table of {base pattern set}
 * 2. all base pattersn are added into wumanber as individual pattern 
 * 3. search with incoming url returns a set of maching patterns with position info
 * 4. filter candidates ads by checking each base pattern set
 * 
 * multimap key: the largest pattern id in the set
 */
typedef unsigned int PatternID;
typedef boost::unordered_multimap<unsigned, std::vector<PatternID>> PatternSet;
typedef boost::unordered_map<unsigned, PatternSet> AdPatternSet;
typedef boost::unordered_map<std::string, unsigned> PatternIDMap;

class WuManber
{
    private:
        /**
         * @param patterns      pattern list to be matched (duplicate patterns
         *                      should have better be avoided)
         */
        bool _init(const std::vector<std::string>& patterns);

        /**
         * @param text           raw text
         * @param textLength     length of text
         * @param res            string map that maps matched patterns with
         *                       its position within the text
         *
         * @return value 0: no pattern matches, n: n patterns match (n>0)
         */
        int _search(const char* text, const int textLength, ResultPattPosMap& res);

        /**
         * @param  str           raw text
         * @param  res            string map that maps matched patterns with
         *                       its position within the text
         *
         * @return value 0: no pattern matchs, n: n patterns matched(n>0)
         */
         int _search(const std::string& str, ResultPattPosMap& res);
         void _convert(const int textLength, const ResultPattPosMap& pattposmap,
                       std::vector<Pattern>& candidates, int endPos=0);

    public:
         bool Init(const std::vector<std::string>& patterns);
         int Search(const std::string& str, std::set<std::string>& res);

    public:
        WuManber();
        ~WuManber();

        bool Init(const std::vector<std::string>& whitelist, const std::vector<std::string>& blacklist);

        bool Init(const std::vector<std::string>& whitelist, const std::vector<std::string>& blacklist,
                  RawPattAdSetMap &adp_redir_map,    
                  RawPattAdSetMap &se_redir_map,
                  RawPattAdSetMap &tbad_redir_map);

        bool Init(RawPattAdSetMap &adp_redir_map, const AdCard &adcard);

        /**
         * @param res           Pattern-to-AdCardSet map that maps patterns
         *                      (with match_type) to a set of AdCards.
         * @return value 0: no pattern matches, n: n patterns match(n>0)
         */
        int search(const char* text, const int textLength, PatternAdSetMap& res, int endPos=0);

        int search(const std::string& str, PatternAdSetMap&res, int endPos=0);

        /**
         * @param res           All the uniq AdCards that the incoming text
         *                      matches.
         * @param endPos: if set, check matching position must be less than endPos (match host)
         * @return value 0: no pattern matches, n: n patterns match(n>0)
         */
        int search(const char* text, const int textLength, AdCardSet& res, int endPos=0);

        int search(const std::string& str, AdCardSet& res, int endPos=0);

        /**
         * @brief search text
         *
         * @return value 0: no pattern matchs, n: n patterns matched(n>0)
         */
        int search(const char* text, const int textLength);

        /**
         * @brief search text
         *
         * @return value 0: no pattern matchs, n: n patterns matched(n>0)
         */
        int search(const std::string& str);

    private:
        // minmum length of patterns
        int32_t mMin;
        // SHIFT table
        std::vector<int32_t> mShiftTable;
        // a combination of HASH and PREFIX table
        std::vector<PrefixTableType> mHashTable;
        // pattern strings
        std::vector<std::string> mPatternStrs;
        std::vector<std::string> mPatterns;
        // size of SHIFT and HASH table
        int32_t mTableSize;
        // size of block
        int32_t mBlock;
        PatternAdSetMap pattadsetmap;

private:
        AdPatternSet mAdPtnSet;
        PatternIDMap mPtnIdMap;

private:
    int dupGeneralPattern(RawPattAdSetMap &redir_map);
    int matchPatternSet(const std::vector<PatternID>& cid, unsigned int key, const PatternSet& ptns, int& match_index);
    int buildAdPatternSet(const RawPattAdSetMap &redir_map, AdPatternSet& aps, const PatternIDMap& ptnid);
    int buildPatternIDPamp(const std::vector<std::string>& patterns, PatternIDMap& ptnid);
    int filterAdByPatternset(std::vector<Pattern>& candidates_ptn, AdCardSet& candidates_ad, int& did_filter);

};

struct WuManberPair 
{
    WuManberPair():mMinLength(7) {}

    bool Init(const std::vector<std::string>& whitelist, const std::vector<std::string>& blacklist);    
    bool Init(const std::vector<std::string>& whitelist, const std::vector<std::string>& blacklist,
              RawPattAdSetMap &adp_redir_map,    
              RawPattAdSetMap &se_redir_map,
              RawPattAdSetMap &tbad_redir_map);    
    bool Init(RawPattAdSetMap &adp_redir_map, const AdCard &adcard);

    int search(const char* text, const int textLength, AdCardSet& res, int endPos=0);
    int search(const std::string& str, AdCardSet& res, int endPos=0);

    int Split(const std::vector<std::string>& inlist, 
              std::vector<std::string>& outlist_s, 
              std::vector<std::string>& outlist_t);

    int Split(RawPattAdSetMap &adp_inmap,  
              RawPattAdSetMap &adp_outmap_s,  
              RawPattAdSetMap &adp_outmap_t);

    int mMinLength;
    std::pair<WuManber, WuManber> wmbp;
}; 

#endif  /* WUMANBER_HPP_*/
