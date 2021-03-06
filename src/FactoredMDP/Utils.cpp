#include <AIToolbox/FactoredMDP/Utils.hpp>

#include <AIToolbox/Utils/Core.hpp>

namespace AIToolbox::FactoredMDP {
    PartialFactors removeFactor(const PartialFactors & pf, const size_t f) {
        size_t i = 0;
        while (i < pf.first.size() && pf.first[i] < f) ++i;
        if (i == pf.first.size() || pf.first[i] != f) return pf;

        PartialFactors retval;
        retval.first.reserve(pf.first.size() - 1);
        retval.second.reserve(pf.first.size() - 1);

        for (size_t j = 0; j < pf.first.size(); ++j) {
            if (i == j) continue;
            retval.first.push_back(pf.first[j]);
            retval.second.push_back(pf.second[j]);
        }
        return retval;
    }

    bool match(const PartialFactors & lhs, const PartialFactors & rhs) {
        const PartialFactors * smaller = &rhs, * bigger = &lhs;
        if (lhs.first.size() < rhs.first.size()) std::swap(smaller, bigger);

        size_t i = 0, j = 0;
        while (j < smaller->second.size()) {
            if (bigger->first[i] < smaller->first[j]) ++i;
            else if (bigger->first[i] > smaller->first[j]) return false;
            else {
                if (bigger->second[i] != smaller->second[j]) return false;
                ++i;
                ++j;
            }
        }
        return true;
    }

    PartialFactors join(const size_t S, const PartialFactors & lhs, const PartialFactors & rhs) {
        PartialFactors retval;
        retval.first.reserve(lhs.first.size() + rhs.first.size());
        retval.second.reserve(lhs.first.size() + rhs.first.size());
        // lhs part is the same.
        retval = lhs;
        // rhs part is shifted by S elements (values are the same)
        std::transform(std::begin(rhs.first), std::end(rhs.first), std::back_inserter(retval.first), [S](const size_t a){ return a + S; });
        retval.second.insert(std::end(retval.second), std::begin(rhs.second), std::end(rhs.second));

        return retval;
    }

    Factors join(const Factors & lhs, const Factors & rhs) {
        Factors retval;
        retval.reserve(lhs.size() + rhs.size());
        retval.insert(std::end(retval), std::begin(lhs), std::end(lhs));
        retval.insert(std::end(retval), std::begin(rhs), std::end(rhs));
        return retval;
    }

    PartialFactors merge(const PartialFactors & lhs, const PartialFactors & rhs) {
        PartialFactors retval;
        retval.first.reserve(lhs.first.size() + rhs.first.size());
        retval.second.reserve(lhs.first.size() + rhs.first.size());
        retval = lhs;

        inplace_merge(&retval, rhs);

        return retval;
    }

    void inplace_merge(PartialFactors * plhs, const PartialFactors & rhs) {
        if (!plhs) return;
        auto & lhs = *plhs;

        lhs.first.reserve(lhs.first.size() + rhs.first.size());
        lhs.second.reserve(lhs.first.size() + rhs.first.size());

        size_t i = 0, j = 0;
        while (i < lhs.first.size() && j < rhs.first.size()) {
            if (lhs.first[i] < rhs.first[j]) { ++i; continue; }
            lhs.first.insert(std::begin(lhs.first) + i, rhs.first[j]);
            lhs.second.insert(std::begin(lhs.second) + i, rhs.second[j]);
            ++i;
            ++j;
        }
        lhs.first.insert(std::end(lhs.first), std::begin(rhs.first) + j, std::end(rhs.first));
        lhs.second.insert(std::end(lhs.second), std::begin(rhs.second) + j, std::end(rhs.second));
    }

    size_t factorSpace(const Factors & space) {
        size_t retval = 1;
        for (const auto f : space) {
            // Detect wraparound
            if (std::numeric_limits<size_t>::max() / f < retval)
                return std::numeric_limits<size_t>::max();
            retval *= f;
        }
        return retval;
    }

    size_t factorSpacePartial(const std::vector<size_t> & ids, const Factors & space) {
        size_t retval = 1;
        for (const auto id : ids) {
            // Detect wraparound
            if (std::numeric_limits<size_t>::max() / space[id] < retval)
                return std::numeric_limits<size_t>::max();
            retval *= space[id];
        }
        return retval;
    }

    PartialFactors toPartialFactors(const Factors & f) {
        PartialFactors retval;

        retval.first.resize(f.size());
        for (size_t i = 0; i < f.size(); ++i)
            retval.first[i] = i;
        retval.second = f;

        return retval;
    }

    Factors toFactors(const size_t F, const PartialFactors & pf) {
        Factors f(F);
        for (size_t i = 0; i < pf.first.size(); ++i)
            f[pf.first[i]] = pf.second[i];

        return f;
    }

    Factors toFactors(const Factors & space, size_t id) {
        Factors f(space.size());
        for (size_t i = 0; i < space.size(); ++i) {
            f[i] = id % space[i];
            id /= space[i];
        }
        return f;
    }

    size_t toIndex(const Factors & space, const Factors & f) {
        size_t result = 0; size_t multiplier = 1;
        for (size_t i = 0; i < f.size(); ++i) {
            result += multiplier * f[i];
            multiplier *= space[i];
        }
        return result;
    }

    size_t toIndex(const Factors & space, const PartialFactors & f) {
        size_t result = 0; size_t multiplier = 1;
        for (size_t i = 0, j = 0; i < space.size(); ++i) {
            if (i == f.first[j]) {
                result += multiplier * f.second[j++];
                if (j == f.first.size()) break;
            }
            multiplier *= space[i];
        }
        return result;
    }

    size_t toIndexPartial(const std::vector<size_t> & ids, const Factors & space, const Factors & f) {
        size_t result = 0; size_t multiplier = 1;
        for (size_t i = 0; i < ids.size(); ++i) {
            result += multiplier * f[ids[i]];
            multiplier *= space[ids[i]];
        }
        return result;
    }

    size_t toIndexPartial(const Factors & space, const PartialFactors & f) {
        size_t result = 0; size_t multiplier = 1;
        for (size_t i = 0; i < f.first.size(); ++i) {
            result += multiplier * f.second[i];
            multiplier *= space[f.first[i]];
        }
        return result;
    }

    // PartialFactorsEnumerator below.

    PartialFactorsEnumerator::PartialFactorsEnumerator(Factors f, std::vector<size_t> factors) :
        F(std::move(f)), factorToSkipId_(factors.size())
    {
        factors_.first = std::move(factors);
        factors_.second.resize(factors_.first.size());
    }

    PartialFactorsEnumerator::PartialFactorsEnumerator(Factors f, std::vector<size_t> factors, const size_t factorToSkip) :
        PartialFactorsEnumerator(std::move(f), std::move(factors))
    {
        // Set all used agents and find the skip id.
        for (size_t i = 0; i < factors_.first.size(); ++i) {
            if (factorToSkip == factors_.first[i]) {
                factorToSkipId_ = i;
                break;
            }
        }
    }

    void PartialFactorsEnumerator::advance() {
        // Start from 0 if skip is not zero, from 1 otherwise.
        size_t id = !factorToSkipId_;
        while (id < factors_.second.size()) {
            ++factors_.second[id];
            if (factors_.second[id] == F[factors_.first[id]]) {
                factors_.second[id] = 0;
                if (++id == factorToSkipId_) ++id;
            } else
                return;
        }
        factors_.second.clear();
    }

    bool PartialFactorsEnumerator::isValid() const {
        return factors_.second.size() > 0;
    }

    size_t PartialFactorsEnumerator::getFactorToSkipId() const { return factorToSkipId_; }

    PartialFactors& PartialFactorsEnumerator::operator*() { return factors_; }
}
