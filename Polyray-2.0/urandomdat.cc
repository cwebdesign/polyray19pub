/** @file urandomdat.cc
 *  @brief Uniform random-number generator backed by Mersenne Twister 64-bit.
 *
 *  Implements the openpolyray::Urandomdat class, which wraps @c std::mt19937_64
 *  and a @c std::uniform_real_distribution<double> to produce reproducible or
 *  time-seeded sequences of doubles in [0, 1].  Two seed strategies are
 *  provided: a time-based constructor for production rendering (non-repeatable)
 *  and a fixed-seed constructor for unit testing and regression reproducibility.
 *
 *  (C) C. Meli 2021-2026
 */

//#include ...

#include <cassert>
#include <random>
#include <chrono>

#include "defs3.h"
#include "urandomdat.h"

namespace openpolyray {

/** @brief Construct a Urandomdat seeded from the current high-resolution clock.
 *
 *  Splits the 64-bit nanosecond timestamp into two 32-bit words and feeds
 *  them to @c std::seed_seq, ensuring different seeds even when called in
 *  rapid succession.  Initialises the uniform distribution to [0, 1].
 */
Urandomdat::Urandomdat(void)
{
    debug = 0;

    // initialize the random number generator with time-dependent seed
    uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::seed_seq ss{ uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };
    rng.seed(ss);
    // initialize a uniform distribution between 0 and 1
    unif = std::uniform_real_distribution<double>(0, 1);
}

/** @brief Construct a Urandomdat with a fixed 64-bit seed for reproducibility.
 *
 *  Seeds the Mersenne Twister directly with @p seed so that the generated
 *  sequence is deterministic across runs.  Used by unit tests that must
 *  compare against known reference values.
 *
 *  @param seed  64-bit seed value; pass the same value to reproduce a sequence.
 */
Urandomdat::Urandomdat(UWORD64 seed)
{
    debug = 0;
    //std::srand(seed);
    std::seed_seq ss{ uint32_t(seed & 0xffffffff), uint32_t(seed >> 32) };
    rng.seed(seed);
    // initialize a uniform distribution between 0 and 1
    unif = std::uniform_real_distribution<double>(0, 1);
}

/** @brief Enable debug output for subsequent random-number calls.
 *
 *  Sets the @c debug flag to 1.  When enabled, rand01() prints the generated
 *  value to @c std::cout whenever it falls outside [0, 1].
 */
void Urandomdat::setdebug(void)
{
    debug = 1;
}


/** @brief Return a uniform double in [0, RAND_MAX] scaled from the MT output.
 *
 *  Multiplies the [0, 1] variate from the internal distribution by @c RAND_MAX,
 *  matching the range of the legacy C @c rand() call this replaces.  The result
 *  may equal @c RAND_MAX (inclusive upper bound).
 *
 *  @return A uniform double in [0, RAND_MAX].
 */
double Urandomdat::randinternal(void)
{
    //return (double)rand();
    return RAND_MAX * unif(rng);
}

/** @brief Return a uniform double in [0, 1].
 *
 *  Thin alias for rand01(); provided for API compatibility.
 *  @return A uniform double in [0, 1].
 */
double Urandomdat::random(void)
{
    return rand01();
}

/** @brief Return a uniform double in [min(@p x, @p y), max(@p x, @p y)].
 *
 *  Swaps @p x and @p y if necessary so the interval is always well-formed,
 *  then linearly scales the [0, 1] output of rand01() into the requested range.
 *
 *  @param x  One endpoint of the interval (order does not matter).
 *  @param y  The other endpoint of the interval.
 *  @return   A uniform double in [min(x, y), max(x, y)].
 */
double Urandomdat::random(double x, double y)
{
    if (x > y) std::swap(x, y);
    double range = y - x;
    return rand01() * range + x;
}

/** @brief Core uniform [0, 1] variate from the Mersenne Twister.
 *
 *  Draws directly from the @c std::uniform_real_distribution member and
 *  asserts that the result lies in [0, 1].  If the value is out of range
 *  (which should never happen with a conforming implementation) a diagnostic
 *  is printed to @c std::cout before the assertion fires.
 *
 *  @return A uniform double in [0, 1].
 */
double Urandomdat::rand01(void)
{
    //double d= randinternal() /(max()+1.0); // (RAND_MAX+1.0); // random number from 0 to 1 (inclusive of 1) //rand_max was max() ?
    double d = unif(rng);
    if (d > 1 || d < 0) std::cout << "d=" << d << " randinternal()=" << randinternal() << "\n";
    assert((d >= 0) && (d <= 1));
    return d;
}


/** @brief Return a uniform random @c Flt in [0, 1] for the Polyray renderer.
 *
 *  Adapter that converts the @c double result of rand01() to the renderer's
 *  @c Flt type (typically @c double on 64-bit builds, @c float on 32-bit).
 *  Called from texture and lighting code that needs a lightweight, unbiased
 *  jitter value.
 *
 *  @return A uniform @c Flt in [0, 1].
 */
Flt Urandomdat::polyray_random(void)
{
    return rand01();
}

#ifdef TESTING
//c++ 
#endif

}//namespace