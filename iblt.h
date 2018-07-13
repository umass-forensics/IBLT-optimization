#ifndef IBLT_H
#define IBLT_H

#include <inttypes.h>
#include <set>
#include <vector>
#include <fstream>
#include <unordered_map>
//
// Invertible Bloom Lookup Table implementation
// References:
//
// "What's the Difference? Efficient Set Reconciliation
// without Prior Context" by Eppstein, Goodrich, Uyeda and
// Varghese
//
// "Invertible Bloom Lookup Tables" by Goodrich and
// Mitzenmacher
//


class IBLT
{
public:

    IBLT(size_t _expectedNumEntries, size_t _ValueSize, float hedge, size_t _numHashes);
    IBLT(size_t _expectedNumEntries, size_t _ValueSize);
    IBLT(const IBLT& other);
    virtual ~IBLT();

    std::pair<int,double> OptimalParameters(size_t entries);

    void insert(uint64_t k, const std::vector<uint8_t> v);
    void erase(uint64_t k, const std::vector<uint8_t> v);

    // Returns true if a result is definitely found or not
    // found. If not found, result will be empty.
    // Returns false if overloaded and we don't know whether or
    // not k is in the table.
    bool get(uint64_t k, std::vector<uint8_t>& result) const;

    // Adds entries to the given sets:
    //  positive is all entries that were inserted
    //  negative is all entries that were erased but never added (or
    //   if the IBLT = A-B, all entries in B that are not in A)
    // Returns true if all entries could be decoded, false otherwise.
    bool listEntries(std::set<std::pair<uint64_t,std::vector<uint8_t> > >& positive,
        std::set<std::pair<uint64_t,std::vector<uint8_t> > >& negative) const;

    void peelEntries(std::set<std::pair<uint64_t,std::vector<uint8_t> > >& positive,
        std::set<std::pair<uint64_t,std::vector<uint8_t> > >& negative);

    // Subtract two IBLTs
    IBLT operator-(const IBLT& other) const;

    // For debugging:
    std::string DumpTable() const;

    std::string DumpEntry(size_t i) const;

    int bytesize();

    int hashTableSize();

    void static set_parameter_file(char * filename);

    // these need to be public for python integration 
    size_t valueSize;
    size_t numHashes;  

private:
    void _insert(int plusOrMinus, uint64_t k, const std::vector<uint8_t> v);
    
    static std::string parameter_file; 

    class HashTableEntry
    {
    public:
        int32_t count;
        uint64_t keySum;
        uint32_t keyCheck;
        std::vector<uint8_t> valueSum;

        bool isPure() const;
        bool empty() const;
        void addValue(const std::vector<uint8_t> v);
    };

    std::vector<HashTableEntry> hashTable;
};

#endif /* IBLT_H */
