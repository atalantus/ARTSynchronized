//
// Created by florian on 18.11.15.
//

#ifndef ART_OPTIMISTICLOCK_COUPLING_N_H
#define ART_OPTIMISTICLOCK_COUPLING_N_H
#include "N.h"

using namespace ART;

namespace ART_OLC {

    class Tree {
    public:
        using LoadKeyFunction = void (*)(TID tid, Key &key);

    private:
        N *const root;

        TID checkKey(const TID tid, const Key &k) const;

        LoadKeyFunction loadKey;

        Epoche epoche{256};

    public:
        enum class CheckPrefixResult : uint8_t {
            Match,
            NoMatch,
            OptimisticMatch
        };

        enum class CheckPrefixPessimisticResult : uint8_t {
            Match,
            NoMatch,
        };

        enum class PCCompareResults : uint8_t {
            Smaller,
            Equal,
            Bigger,
        };
        enum class PCEqualsResults : uint8_t {
            BothMatch,
            Contained,
            NoMatch
        };
        static CheckPrefixResult checkPrefix(N* n, const Key &k, uint32_t &level);

        static CheckPrefixPessimisticResult checkPrefixPessimistic(N *n, const Key &k, uint32_t &level,
                                                                   uint8_t &nonMatchingKey,
                                                                   Prefix &nonMatchingPrefix,
                                                                   LoadKeyFunction loadKey, bool &needRestart);

        static PCCompareResults checkPrefixCompare(const N* n, const Key &k, uint8_t fillKey, uint32_t &level, LoadKeyFunction loadKey, bool &needRestart);

        static PCEqualsResults checkPrefixEquals(const N* n, uint32_t &level, const Key &start, const Key &end, LoadKeyFunction loadKey, bool &needRestart);

    public:

        Tree(LoadKeyFunction loadKey);

        Tree(const Tree &) = delete;

        Tree(Tree &&t) : root(t.root), loadKey(t.loadKey) { }

        ~Tree();

        ThreadInfo getThreadInfo();

        TID lookup(const Key &k, ThreadInfo &threadEpocheInfo) const;

        bool lookupRange(const Key &start, const Key &end, Key &continueKey, TID result[], std::size_t resultLen,
                         std::size_t &resultCount, ThreadInfo &threadEpocheInfo) const;

        bool lookupRange(const Key &start, TID result[], std::size_t resultLen, std::size_t &resultCount, ThreadInfo &threadEpocheInfo) const;

        void insert(const Key &k, TID tid, ThreadInfo &epocheInfo);

        void remove(const Key &k, TID tid, ThreadInfo &epocheInfo);

        /**
         * Read-only root, for the space-analysis census in src/art/space_census.hpp.
         *
         * The tiny variants expose their dereference tables as a public member and
         * so can be measured without touching them; this reference has no such
         * handle, and a footprint comparison needs both sides. Nothing on a measured
         * path calls it -- the census runs after the tree is built and quiescent,
         * from art_space, a binary the throughput and counter sweeps never invoke.
         * An unused inline accessor emits no code and adds no field, so Tree.cpp's
         * object file is unchanged by it; that is checked rather than assumed.
         */
        const N *getRoot() const { return root; }
    };
}
#endif //ART_OPTIMISTICLOCK_COUPLING_N_H
