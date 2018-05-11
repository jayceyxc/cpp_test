#include <cmath>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "wumanber.hpp"

/**
 * @brief   String hash function.
 * @param str   string to be hashed
 * @param len   length of the string
 * @return hash code
 */
static unsigned int HashCode(const char* str, int len)
{
    unsigned int hash = 0;
    while (*str != '\0' && len > 0) {
        hash = (*str++) + (hash << 6) + (hash << 16) - hash;
        --len;
    }
    return (hash & 0x7FFFFFFF);
}

/**
 * @brief constructor
 */
WuManber::WuManber():mMin(0), mTableSize(0), mBlock(3)
{
    //VOID
}

bool WuManber::Init(const std::vector<std::string>& patterns)
{
    int patternSize = patterns.size();

    //check if no pattern specified
    if (patternSize == 0) {
        cout << "Error: wumanber init failed because no pattern specified." << endl;
        return false;
    }

    //calculate the minimal pattern length
    mMin = patterns[0].length();
    int32_t lenPattern = 0;
    for (int i = 0; i < patternSize; ++i) {
        lenPattern = patterns[i].length();
        if (lenPattern < mMin) {
            mMin = lenPattern;
            cout << format("mBlock=%d mMin=%d pat=%s i=%d" % mBlock % mMin % patterns[i].c_str() % i);    
        }
    }
    WARN("mBlock=%d mMin=%d num patterns=%d", mBlock, mMin, patternSize);
    //check if mBlock larger than mMin
    if (mBlock > mMin) {
        WARN("mBlock=%d mMin=%d: mBlock is larger than mMin, reset mBlock to mMin, but it will seriously affect the effiency.", mBlock, mMin);
        mBlock = mMin;
    }

    //choose a suitable mTableSize for SHIFT, HASH table
    int32_t primes[6] = {1003, 10007, 100003, 1000003, 10000019, 100000007};
    std::vector<int32_t> primeList(&primes[0], &primes[6]);

    int32_t threshold = 10 * mMin;
    for (size_t i = 0; i < primeList.size(); ++i) {
        if (primeList[i] > patternSize && primeList[i] / patternSize > threshold) {
            mTableSize = primeList[i];
            break;
        }
    }

    //if size of patternList is huge.
    if (0 == mTableSize) {
        std::cerr << "Warning: amount of pattern is very large, will cost a great amount of memory." << std::endl;
        mTableSize = primeList[5];
    }

    //construct ShiftTable and HashTable, and set default value for SHIFT table
    mPatterns = patterns;
    mHashTable.resize(mTableSize);
    // default value is m-mBlock+1 for shift
    int32_t defaultValue = mMin - mBlock + 1;
    mShiftTable.resize(mTableSize, defaultValue);
    //loop through patterns
    for (int id = 0; id < patternSize; ++id) {
        // loop through each pattern from right to left
        for (int index = mMin; index >= mBlock; --index) {
            unsigned int hash = HashCode(patterns[id].c_str() + index - mBlock, mBlock) % mTableSize;
            if (mShiftTable[hash] > (mMin - index)) {
                mShiftTable[hash]  = mMin - index;
            }
            if (index == mMin) {
                unsigned int prefixHash = HashCode(patterns[id].c_str(), mBlock);
                mHashTable[hash].push_back(std::make_pair(prefixHash, id));
            }
        }
    }

    return true;
}

int WuManber::Search(const std::string& str, std::set<std::string>& res)
{
    if(mTableSize==0) {return 0;}
    //hit count: value to be returned
    int hits = 0;
    uint32_t index = mMin - 1; // start off by matching end of largest common pattern

    int32_t blockMaxIndex = mBlock - 1;
    int32_t windowMaxIndex = mMin - 1;
    const char* text = str.c_str();

    while (index < str.size())
    {
        unsigned int blockHash = HashCode(text + index - blockMaxIndex, mBlock);
        blockHash = blockHash % mTableSize;
        int shift = mShiftTable[blockHash];
        if (shift > 0)
        {
            index += shift;
        }
        else
        {
            // we have a potential match when shift is 0
            unsigned int prefixHash = HashCode(text + index - windowMaxIndex, mBlock);
            PrefixTableType &element = mHashTable[blockHash];
            PrefixTableType::iterator iter = element.begin();

            while (element.end() != iter)
            {
                if (prefixHash == iter->first)
                {
                    // since prefindex matches, compare target substring with pattern
                    // we know first two characters already match
                    const char* indexTarget = text + index - windowMaxIndex;    //+mBlock
                    const char* indexPattern = mPatterns[iter->second].c_str(); //+mBlock
                    while (('\0' != *indexTarget) && ('\0' != *indexPattern))
                    {
                        // match until we reach end of either string
                        if (*indexTarget == *indexPattern)
                        {
                            // match against chosen case sensitivity
                            ++indexTarget;
                            ++indexPattern;
                        }
                        else
                            break;
                    }
                    // match succeed since we reach the end of the pattern.
                    if ('\0' == *indexPattern)
                    {
                        res.insert(std::string(mPatterns[iter->second]));
                        // std::cout << "Found at index: " << index << std::endl;
                        ++hits;
                    }
                }//end if
                ++iter;
            }//end while
            ++index;
        }//end else
    }//end while

    return hits;
}

/**
 * @brief _init
 */
bool WuManber::_init(const std::vector<std::string>& patterns)
{    
    int patternSize = patterns.size();

    //check if no pattern specified
    if (patternSize == 0) {
        WARN("Error: wumanber init failed because no pattern specified.");
        return false;
    }
#if 0
    static uint32_t count=0;
    count++;
    std::string filename="test.txt";
    string str_count = static_cast<ostringstream*>( &(ostringstream() << count) )->str();
    filename += str_count;
    std::ofstream ofs (filename, std::ofstream::out);
    for (int i = 0; i < patternSize; ++i) {
        ofs << patterns[i] << endl;
    }
    ofs.close();
#endif
    //caculate the minmum pattern length
    mMin = patterns[0].length();
    int32_t lenPattern = 0;
    for (int i = 0; i < patternSize; ++i) {
        lenPattern = patterns[i].length();
        if (lenPattern < mMin) {
            mMin = lenPattern;
            WARN("mBlock=%d mMin=%d pat=%s i=%d", mBlock, mMin, patterns[i].c_str(), i);    
        }
    }

    WARN("WuManber::_init() mBlock=%d mMin=%d num patterns=%d", mBlock, mMin, patternSize);
    //check if mBlock larger than mMin
    if (mBlock > mMin) {
        WARN("mBlock=%d mMin=%d: mBlock is larger than mMin, reset mBlock to mMin, but it will seriously affect the effiency.", mBlock, mMin);
        mBlock = mMin;
    }

    //choose a suitable mTableSize for SHIFT, HASH table
    int32_t primes[6] = {1003, 10007, 100003, 1000003, 10000019, 100000007};
    std::vector<int32_t> primeList(&primes[0], &primes[6]);

    int32_t threshold = 10 * mMin;
    for (size_t i = 0; i < primeList.size(); ++i) {
        if (primeList[i] > patternSize && primeList[i] / patternSize > threshold) {
            mTableSize = primeList[i];
            break;
        }
    }

    //if size of patternList is huge.
    if (0 == mTableSize) {
        std::cerr << "Warning: amount of pattern is very large, will cost a great amount of memory." << std::endl;
        mTableSize = primeList[5];
    }

    WARN("WuManber::_init() mTableSize=%d", mTableSize);

    //construct ShiftTable and HashTable, and set default value for SHIFT table
    mPatternStrs = patterns;
    mHashTable.resize(mTableSize);
    // default value is m-mBlock+1 for shift
    int32_t defaultValue = mMin - mBlock + 1;
    mShiftTable.resize(mTableSize, defaultValue);

    //loop through patterns
    for (int id = 0; id < patternSize; ++id) {
        // loop through each pattern from right to left
        for (int index = mMin; index >= mBlock; --index) {
            unsigned int hash = HashCode(patterns[id].c_str() + index - mBlock, mBlock) % mTableSize;
            if (mShiftTable[hash] > (mMin - index)) {
                mShiftTable[hash]  = mMin - index;
            }
            if (index == mMin) {
                unsigned int prefixHash = HashCode(patterns[id].c_str(), mBlock);
                mHashTable[hash].push_back(std::make_pair(prefixHash, id));
            }
        } /*End index loop */
    }   /* End id loop */

    int c[5]={0,0,0,0,0};
    for (int id = 0; id < mTableSize; ++id) {
        int s=mHashTable[id].size();
        if(s==0) {++c[0]; continue;}
        if(s>c[1]) {c[1]=s; c[2]=id;}
        c[3] += s;
    }
    WARN("mHashTable size=%d empty slots=%d max len=%d max slot index=%d total patterns=%d",
         mTableSize, c[0], c[1], c[2], c[3]);
    buildPatternIDPamp(patterns, mPtnIdMap);
    DEBUG("WuManber::_init() Init done");
    return true;
}

/**
 * @brief destructor
 */
WuManber::~WuManber()
{
    //VOID
}

bool WuManber::Init(const std::vector<std::string>& whitelist,
                    const std::vector<std::string>& blacklist)
{
    for (auto it = whitelist.begin(); it != whitelist.end(); it++) {
        pattadsetmap[Pattern(*it)].insert(white_virt_ad);
    }
    for (auto it = blacklist.begin(); it != blacklist.end(); it++) {
        pattadsetmap[Pattern(*it)].insert(black_virt_ad);
    }
    std::set<std::string> patts;
    for (auto it = pattadsetmap.begin(); it != pattadsetmap.end(); it++) {
        patts.insert(it->first.pattern_str);
    }
    // Pattern strings with no duplicates, asterisk stripped
    std::vector<std::string> patts_vec(patts.begin(), patts.end());
    // Assign the core initialization work to _init
    return _init(patts_vec);
}

/**
 * @brief search multiple pattern in text at one time
 */
int WuManber::_search(const char* text, const int textLength, ResultPattPosMap& res)
{
    if(mTableSize==0) {return 0;}
    //hit count: value to be returned
    int hits = 0;
    int32_t index = mMin - 1; // start off by matching end of largest common pattern

    int32_t blockMaxIndex = mBlock - 1;
    int32_t windowMaxIndex = mMin - 1;

    std::string src_text(text, textLength);

    //int count[4]={0, 0, 0, 0};
    //DEBUG("WuManber::_search() text=%.*s length=%d mBlock=%d mMin=%d index=%d size=%ld", 
    //      textLength, text, textLength, mBlock, mMin, index, mPatternStrs.size());

    while (index < textLength) {
        //++count[0];
        unsigned int blockHash = HashCode(text + index - blockMaxIndex, mBlock);
        blockHash = blockHash % mTableSize;
        int shift = mShiftTable[blockHash];
        //DEBUG("index=%d shift=%d blockHash=%d prefix=%.*s", index, shift, blockHash, textLength-(index - blockMaxIndex), text + index - blockMaxIndex);
        if (shift > 0) {
            index += shift;
        } else {
            // we have a potential match when shift is 0
            unsigned int prefixHash = HashCode(text + index - windowMaxIndex, mBlock);
            PrefixTableType &element = mHashTable[blockHash];
            PrefixTableType::iterator iter = element.begin();
            while (element.end() != iter) {
                //++count[1];
                if (prefixHash == iter->first)
                {
                    //++count[2];
                    // since prefindex matches, compare target substring with pattern
                    // we know first two characters already match
                    const char* indexTarget = text + index - windowMaxIndex;    //+mBlock
                    const char* indexPattern = mPatternStrs[iter->second].c_str(); //+mBlock
                    //DEBUG("index=%d size=%ld target=%.*s pattern=%s", index, element.size(), textLength-(index - windowMaxIndex), indexTarget, indexPattern);
                    while (('\0' != *indexTarget) && ('\0' != *indexPattern))
                    {
                        //++count[3];
                        // match until we reach end of either string
                        if (*indexTarget == *indexPattern)
                        {
                            // match against chosen case sensitivity
                            ++indexTarget;
                            ++indexPattern;
                        }
                        else
                            break;
                    }
                    // match succeed since we reach the end of the pattern.
                    if ('\0' == *indexPattern) {
                        /*
                        if (hits == 0) {
                            if (src_text.find(mPatternStrs[iter->sencod]) > 0) {
                                continue
                            }
                        }
                        */
                        DEBUG("Found at index=%d pattern=%s hits=%d blockHash=%d lastindex:%d", \
                                index, mPatternStrs[iter->second].c_str(), hits, blockHash, index - mMin +1);
                        //res.insert(std::make_pair(std::string(mPatternStrs[iter->second]), index - mMin +1));
                        res[std::string(mPatternStrs[iter->second])] = index - mMin+1;
                        ++hits;
                    }
                }//end if
                ++iter;
            }//end while
            ++index;
        }//end else
    }//end while
    /*
      if(count[3]>100) {
      WARN("search mMin=%d mBlock=%d count=%d %d %d %d", mMin, mBlock, count[0], count[1], count[2], count[3]);
      }*/
    return hits;
}

/**
 * the core search member func for internal use
 */
int WuManber::_search(const std::string& str, ResultPattPosMap& res)
{
    return _search(str.c_str(), str.length(), res);
}

void WuManber::_convert(const int textLength, const ResultPattPosMap& pattposmap,
                        std::vector<Pattern>& candidates, int endPos)
{
    std::string patt_str;
    unsigned pos;
    int patt_len;
    for (auto it = pattposmap.begin(); it != pattposmap.end(); it++) {
        patt_str = it->first;
        pos = it->second;
        patt_len = patt_str.length();
        DEBUG("convert: patt_str=%s, pos=%d patt_len=%d textlen=%d endPos=%d", patt_str.c_str(), pos, patt_len, textLength, endPos);
        if(endPos>0 && (int)pos > endPos) {continue;}
        candidates.push_back(Pattern(patt_str, KeywordMode, pos));

        if (pos == 0) {
            candidates.push_back(Pattern(patt_str, SuffixAsterisk, pos));
            if (textLength == patt_len) {
                candidates.push_back(Pattern(patt_str, ExactAsSeen, pos));
                candidates.push_back(Pattern(patt_str, PrefixAsterisk, pos));
            }
        } else if (textLength - pos == patt_str.length()) {
            candidates.push_back(Pattern(patt_str, PrefixAsterisk, pos));
        } /* should be a case for both prefix asterisk and suffix asterisk? */
    }
}

/**
 * the core search member func for internal use
 */
int WuManber::search(const char* text, const int textLength)
{
    if(mTableSize==0) {return 0;}
    //hit count: value to be returned
    int hits = 0;
    int index = mMin - 1; // start off by matching end of largest common pattern

    uint32_t blockMaxIndex = mBlock - 1;
    uint32_t windowMaxIndex = mMin - 1;

    while (index < textLength)
    {
        unsigned int blockHash = HashCode(text + index - blockMaxIndex, mBlock);
        blockHash = blockHash % mTableSize;
        int shift = mShiftTable[blockHash];
        if (shift > 0)
        {
            index += shift;
        }
        else
        {
            // we have a potential match when shift is 0
            unsigned int prefixHash = HashCode(text + index - windowMaxIndex, mBlock);
            //prefixHash = prefixHash % mTableSize;
            PrefixTableType &element = mHashTable[blockHash];
            PrefixTableType::iterator iter = element.begin();

            while (element.end() != iter)
            {
                if (prefixHash == iter->first)
                {
                    // since prefindex matches, compare target substring with pattern
                    // we know first two characters already match
                    const char* indexTarget = text + index - windowMaxIndex;    //+mBlock
                    const char* indexPattern = mPatternStrs[iter->second].c_str();  //+mBlock

                    while (('\0' != *indexTarget) && ('\0' != *indexPattern))
                    {
                        // match until we reach end of either string
                        if (*indexTarget == *indexPattern)
                        {
                            // match against chosen case sensitivity
                            ++indexTarget;
                            ++indexPattern;
                        }
                        else
                            break;
                    }
                    // match succeed since we reach the end of the pattern.
                    if ('\0' == *indexPattern)
                    {
                        ++hits;
                    }
                }//end if
                ++iter;
            }//end while
            ++index;
        }//end else
    }//end while

    return hits;
}

int WuManber::search(const std::string& str)
{
    return search(str.c_str(), str.length());
}

int WuManber::buildPatternIDPamp(const std::vector<std::string>& patterns, PatternIDMap& ptnid)
{
    PatternIDMap tmp;
    ptnid.swap(tmp);
    unsigned id=0;
    DEBUG("buildPatternIDPamp, pattern size=%ld", patterns.size());
    for (unsigned int i = 0; i < patterns.size(); ++i) {
        vector<string> strs;
        boost::split(strs, patterns[i], boost::is_any_of("*"));
        if(strs.size()<=1) {
            auto it=ptnid.find(patterns[i]);
            if(it != ptnid.end()) {continue;}
            ptnid[patterns[i]] = id;  id++;
        } else {
            for (unsigned int j = 0; j < strs.size(); j++) {
                if(strs[j].size()==0) {continue;}
                auto it=ptnid.find(strs[j]);
                if(it != ptnid.end()) {continue;}
                DEBUG("buildPatternIDPamp, * found:  ptn=%s split ptn=%s id=%d", patterns[i].c_str(), strs[j].c_str(), id);
                ptnid[strs[j]] = id;  id++;
            }
        }
    }
    DEBUG("buildPatternIDPamp, end: pattern size=%ld ptn id size=%ld id=%d", patterns.size(), ptnid.size(), id);
    return 0;
}

/* check if any set is in a list of candidate ads (cid) */
int WuManber::matchPatternSet(const std::vector<PatternID>& cid, unsigned int key, const PatternSet& ptns, int& match_index)
{
    unsigned int i, j, k, ks=0;
    int rc=0;
    match_index=-1; i=0; 
    for(k=ks; k<cid.size(); k++) {
        DEBUG("matchPatternSet() input: k=%d cid=%d", k, cid[k]);
    }
    auto range = ptns.equal_range(key);
    auto it_s = range.first;
    auto it_e = range.second;
    if(ptns.size()<5) {        
        it_s = ptns.begin();
        it_e = ptns.end();
    } 
    for (auto it = it_s; it != it_e; ++it, ++i) {
        const std::vector<PatternID>& ptn=it->second;
        int found_set=1;
        for(j=0; j<ptn.size(); j++) {
            DEBUG("matchPatternSet() pattern: i=%d j=%d ptn=%d", i, j, ptn[j]);
        }
        for(j=0; j<ptn.size(); j++) { /* check for one pattern set */
            ks=0;
            for(k=ks; k<cid.size(); k++) {
                if(cid[k]==ptn[j]) {break;}
            }
            if(k==cid.size()) { /* one pattern not found */
                DEBUG("matchPatternSet() match: i=%d j=%d ptn=%d not found", i, j, ptn[j]);
                found_set=0;
                break;
            }
            ks=k;
        }
        if(found_set) {
            match_index=i;
            rc=1; break;
        }
        DEBUG("matchPatternSet() match: i=%d failed", i);
    }
    return rc;
}

/* sort by position */
struct PatternPredicate
{
    bool operator()(Pattern const & a, Pattern const & b) const
        {
            return a.mPosition < b.mPosition;
        }
};

int WuManberPair::Split(const std::vector<std::string>& inlist, 
                        std::vector<std::string>& outlist_s, 
                        std::vector<std::string>& outlist_t)
{
    int count=0;
    for (auto it = inlist.begin(); it != inlist.end(); it++) {
        Pattern p(*it);
        int len = p.pattern_str.length();
        //WARN("vSplit: in=%s len=%d pattern_str=%s", it->c_str(), len, p.pattern_str.c_str());
        vector<string> strs;
        boost::split(strs, p.pattern_str, boost::is_any_of("*"));
        if(strs.size()>1) {
            outlist_s.push_back(*it); count++;
            continue;
        } 
        if(len < mMinLength) {
            outlist_s.push_back(*it); count++;
        } else {
            outlist_t.push_back(*it);
        }
    }
    WARN("vSplit: in=%ld out=%ld:%ld", inlist.size(), outlist_s.size(), outlist_t.size());
    return count;
}

bool WuManberPair::Init(const std::vector<std::string>& whitelist,
                        const std::vector<std::string>& blacklist)
{
    std::vector<std::string> whitelist_s, whitelist_t;
    std::vector<std::string> blacklist_s, blacklist_t;
    whitelist_s.reserve(whitelist.size());
    whitelist_t.reserve(whitelist.size());
    Split(whitelist, whitelist_s, whitelist_t);
    blacklist_s.reserve(blacklist.size());
    blacklist_t.reserve(blacklist.size());
    Split(blacklist, blacklist_s, blacklist_t);
    bool rcs, rct;
    rcs = wmbp.first.Init(whitelist_s, blacklist_s);
    rct = wmbp.second.Init(whitelist_t, blacklist_t);
    return (rcs||rct);
}
