#ifndef OSRM_EXTRACTOR_GUIDANCE_VALIDATION_HANDLER_HPP_
#define OSRM_EXTRACTOR_GUIDANCE_VALIDATION_HANDLER_HPP_

#include "extractor/guidance/intersection.hpp"
#include "extractor/guidance/intersection_generator.hpp"
#include "extractor/guidance/intersection_handler.hpp"
#include "extractor/query_node.hpp"

#include "util/name_table.hpp"
#include "util/node_based_graph.hpp"

#include <vector>

namespace osrm
{
namespace extractor
{
namespace guidance
{

// Runs sanity checks on intersections and dumps out suspicious ones.
class ValidationHandler final : public IntersectionHandler
{
  public:
    ValidationHandler(const IntersectionGenerator &intersection_generator,
                      const util::NodeBasedDynamicGraph &node_based_graph,
                      const std::vector<util::Coordinate> &coordinates,
                      const util::NameTable &name_table,
                      const SuffixTable &street_name_suffix_table)
        : IntersectionHandler(node_based_graph,
                              coordinates,
                              name_table,
                              street_name_suffix_table,
                              intersection_generator)
    {
    }

    ~ValidationHandler() override final = default;

    bool canProcess(const NodeID, const EdgeID, const Intersection &) const override final
    {
        return true;
    }

    Intersection operator()(const NodeID nid,
                            const EdgeID via_eid,
                            Intersection intersection) const override final
    {
        BOOST_ASSERT(intersection.size() > 2);

        checkForSharpTurnsOntoRamps(nid, via_eid, intersection);

        return intersection;
    }

  private:
    void checkForSharpTurnsOntoRamps(const NodeID,
                                     const EdgeID via_eid,
                                     const Intersection &intersection) const
    {
        for (std::size_t i = 1; i < intersection.size(); ++i)
        {
            const auto road = intersection[i];

            if (!(road.instruction.type == TurnType::OnRamp) || !road.entry_allowed)
            {
                continue;
            }

            if (road.angle <= 2 * NARROW_TURN_ANGLE)
            {
                const auto intersection_node_id = node_based_graph.GetTarget(via_eid);
                const auto where = coordinates[intersection_node_id];

                std::cout << ">>> Suspicious Intersection, angle: " << road.angle << "at: " << where
                          << std::endl;
            }
        }
    }
};

} // namespace guidance
} // namespace extractor
} // namespace osrm

#endif // OSRM_EXTRACTOR_GUIDANCE_VALIDATION_HANDLER_HPP_
