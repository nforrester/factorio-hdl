#pragma once

#include "Port.h"

#include <map>
#include <forward_list>

class CircuitPlanner
{
public:
    void connect(WireColor color, Port & a, Port & b);
    bool connected(Port const & p) const;

    void lock(WireColor color, Port & p);
    bool locked(WireColor color, Port const & p) const
    {
        auto plans = _plans_for_ports.find(const_cast<Port*>(&p)); // TODO const_cast? really?
        if (plans == _plans_for_ports.cend())
        {
            return false;
        }
        Plan * plan = plans->second.plans.at(color);
        return plan->locked;
    }

    void build(CircuitManager & circuits);

private:
    bool _built = false;

    struct Plan
    {
        bool locked = false;
        WireColor color;
        std::vector<Port*> ports;
    };

    // not vector because then insertion would invalidate iterators
    std::forward_list<Plan> _plans;

    struct PlansForColors
    {
        PlansForColors()
        {
            plans.fill(nullptr);
        }

        PlansForColors(PlansForColors const & other)
        {
            *this = other;
        }

        PlansForColors & operator=(PlansForColors const & other)
        {
            plans = other.plans;
            return *this;
        }

        std::array<Plan*, num_wire_colors> plans;
    };

    // not unordered_map because then insertion would invalidate iterators
    using PortPlans = std::map<Port*, PlansForColors>;

    PortPlans _plans_for_ports;

    PortPlans::iterator _get_plans(Port & p);

    void _merge_plans(Plan & a, Plan & b);
};
