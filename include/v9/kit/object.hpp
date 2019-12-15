//
// Created by kiva on 2019/12/13.
//

#pragma once

namespace v9::kit {
    class Object {
    public:
        constexpr Object() = default;
        Object(const Object&) = default;
        Object(Object&&) noexcept = default;
        virtual ~Object() = default;
        Object& operator=(const Object&) = default;
        Object& operator=(Object&&) = default;
    };

    class NoCopy {
    public:
        NoCopy() = default;
        NoCopy(const NoCopy&) = delete;
        NoCopy(NoCopy&&) noexcept = default;
        ~NoCopy() = default;
        NoCopy& operator=(const NoCopy&) = delete;
        NoCopy& operator=(NoCopy&&) = default;
    };

    class NoMove {
    public:
        NoMove() = default;
        NoMove(const NoMove&) = default;
        NoMove(NoMove&&) noexcept = delete;
        ~NoMove() = default;
        NoMove& operator=(const NoMove&) = default;
        NoMove& operator=(NoMove&&) = delete;
    };
}
