#ifndef STATS_H
#define STATS_H

#include <cstdint>
#include <iostream>

// Ponlo a 0 para medir tiempos sin el coste de los contadores
#ifndef ENABLE_STATS
#define ENABLE_STATS 1
#endif

struct Stats
{
    // Rayos lanzados
    std::uint64_t closestHitRays = 0;  // llamadas a Utils::getClosestIntersection
    std::uint64_t shadowRays = 0;  // llamadas a Utils::hasIntersection

    // Tests rayo-primitiva
    std::uint64_t closestHitTests = 0;  // llamadas a Shape::rayIntersect
    std::uint64_t shadowTests = 0;  // llamadas a Shape::rayIntersectP

    static Stats& get()
    {
        static Stats instance;
        return instance;
    }

    void reset() { *this = Stats(); }

    void print(std::ostream& os = std::cout) const
    {
#if !ENABLE_STATS
        os << "\nStats disabled (ENABLE_STATS 0)\n";
        return;
#endif
        const std::uint64_t totalRays = closestHitRays + shadowRays;
        const std::uint64_t totalTests = closestHitTests + shadowTests;

        os << "\n-------- Intersection stats --------\n"
            << "Closest-hit rays:           " << closestHitRays << "\n"
            << "Shadow rays:                " << shadowRays << "\n"
            << "Closest-hit tests:          " << closestHitTests << "\n"
            << "Shadow tests:               " << shadowTests << "\n"
            << "TOTAL primitive tests:      " << totalTests << "\n";
        if (closestHitRays)
            os << "Avg tests / closest-hit ray: " << double(closestHitTests) / closestHitRays << "\n";
        if (shadowRays)
            os << "Avg tests / shadow ray:      " << double(shadowTests) / shadowRays << "\n";
        if (totalRays)
            os << "Avg tests / ray (all):       " << double(totalTests) / totalRays << "\n";
        os << "------------------------------------\n";
    }
};

#if ENABLE_STATS
#define STAT_INC(counter) (++Stats::get().counter)
#else
#define STAT_INC(counter) ((void)0)
#endif

#endif // STATS_H