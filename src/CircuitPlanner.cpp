#include "CircuitPlanner.h"

void CircuitPlanner::connect(WireColor color, Port & a, Port & b)
{
    auto plans_for_a = _get_plans(a);
    auto plans_for_b = _get_plans(b);

    Plan * plan_for_a = plans_for_a->second.plans.at(color);
    Plan * plan_for_b = plans_for_b->second.plans.at(color);

    if (plan_for_a && !plan_for_b)
    {
        assert(plan_for_a->color == color);
        plan_for_a->ports.push_back(&b);
        plans_for_b->second.plans.at(color) = plan_for_a;
    }
    else if (!plan_for_a && plan_for_b)
    {
        assert(plan_for_b->color == color);
        plan_for_b->ports.push_back(&a);
        plans_for_a->second.plans.at(color) = plan_for_b;
    }
    else if (!plan_for_a && !plan_for_b)
    {
        auto plan = _plans.emplace_front();
        plan.color = color;
        plan.ports.push_back(&a);
        plan.ports.push_back(&b);
    }
    else
    {
        assert(plan_for_a && plan_for_b);
        _merge_plans(*plan_for_a, *plan_for_b);
    }
}

void CircuitPlanner::build(CircuitManager & circuits)
{
    for (Plan const & plan : _plans)
    {
        CircuitId c = circuits.make();
        for (Port * port : plan.ports)
        {
            assert(port);
            port->connect(plan.color, c);
        }
    }
}

CircuitPlanner::PortPlans::iterator CircuitPlanner::_get_plans(Port & p)
{
    auto plans = _plans_for_ports.find(&p);
    if (plans == _plans_for_ports.end())
    {
        bool inserted;
        std::tie(plans, inserted) = _plans_for_ports.emplace(&p, PlansForColors());
        assert(inserted);
    }
    return plans;
}

void CircuitPlanner::_merge_plans(Plan & a, Plan & b)
{
    assert(a.color == b.color);
    a.ports.insert(a.ports.end(), b.ports.begin(), b.ports.end());
    for (Port * p : b.ports)
    {
        Plan* & plan_pointer = _plans_for_ports.at(p).plans.at(b.color);
        assert(plan_pointer == &b);
        plan_pointer = &a;
    }
    b.ports.clear();
}
