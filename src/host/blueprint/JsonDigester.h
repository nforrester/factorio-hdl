#pragma once

#include "src/util.h"

#include "json.hpp"

#include <functional>

using json = nlohmann::json;

template <typename T>
class JsonDigester
{
public:
    T digest(json const & j) const;
    void digest(json const & j, T & x) const;

    using KeyHandler = std::function<void(json const & j, T & x)>;

    void require(std::string const & key, KeyHandler const & handler)
    {
        assert(_handlers.emplace(key, std::make_pair(true, handler)).second);
    }

    void optional(std::string const & key, KeyHandler const & handler)
    {
        assert(_handlers.emplace(key, std::make_pair(false, handler)).second);
    }

    ~JsonDigester()
    {
        assert(_used);
    }

private:
    std::map<std::string, std::pair<bool, KeyHandler>> _handlers;

    mutable bool _used = false;
};

#include "JsonDigester.icpp"
