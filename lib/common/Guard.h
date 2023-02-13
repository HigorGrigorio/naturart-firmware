#ifndef _Guard_h_
#define _Guard_h_

#include <LinkedList.h>
#include <ErrorOr.h>

#include <WString.h>

struct IGuardResult
{
    bool succeeded = false;
    String message = "";

    IGuardResult() = default;
    IGuardResult(const IGuardResult &other) = default;
    IGuardResult &operator=(const IGuardResult &other) = default;

    ~IGuardResult() = default;
};

struct IGuardArgument
{
    void *any = nullptr;
    String name = "";

    IGuardArgument() = default;
    ~IGuardArgument() = default;
};

typedef LL<IGuardResult> GuardResultCollection;
typedef LL<IGuardArgument> GuardArgumentCollection;

class Guard
{
private:
    Guard() = default;

public:
    static auto againstNull(IGuardArgument arg) -> IGuardResult
    {
        if (arg.any == nullptr)
        {
            return {
                .succeeded = false,
                .message = arg.name + " is null."};
        }
        return {.succeeded = true};
    }

    static auto againstNullBulk(GuardArgumentCollection args) -> IGuardResult
    {
        GuardResultCollection results = GuardResultCollection();

        for (auto &arg : args)
        {
            auto result = Guard::againstNull(arg);
            if (!result.succeeded)
            {
                results.add(result);
            }
        }

        return combine(results);
    }

    template <typename T>
    static auto isOneOf(IGuardArgument arg, LL<T> validValues) -> IGuardResult
    {
        auto value = static_cast<T>(arg.any);

        for (auto validValue : validValues)
        {
            if (*value == validValue)
            {
                return {.succeeded = true};
            }
        }

        return {
            .succeeded = false,
            .message = "isn't oneOf the correct types in"};
    }

    static auto inRange(
        int arg,
        int min,
        int max,
        String argumentName) -> IGuardResult
    {
        bool isInRange = arg >= min && arg <= max;
        if (!isInRange)
        {
            return {
                .succeeded = false,
                .message = argumentName + " is not in range " + min + " to " + max + ". "};
        }
        return {.succeeded = true};
    }

    static auto allInRange(
        LL<int> args,
        int min,
        int max,
        String argumentName) -> IGuardResult
    {
        IGuardResult result = {};
        GuardResultCollection results = GuardResultCollection();

        for (auto num : args)
        {
            result = Guard::inRange(num, min, max, String("") + num);
            if (!result.succeeded)
            {
                results.add({.succeeded = false,
                             .message = result.message});
            }
        }

        if (results.length() > 0)
        {
            return Guard::combine(results);
        }

        return {.succeeded = true};
    }

    static auto combine(GuardResultCollection results) -> IGuardResult
    {
        IGuardResult fusion{.succeeded = true};

        for (auto result : results)
        {
            fusion = IGuardResult{
                .succeeded = fusion.succeeded && result.succeeded,
                .message = fusion.message + (result.succeeded ? "" : ((fusion.message.length() > 0 ? ";" : "") + result.message))};
        }

        return fusion;
    }

    static auto toError(IGuardResult result) -> Error
    {
        Error error = Error::None;

        if (!result.succeeded)
        {
            error.message = strdup(result.message.c_str());
        }

        return error;
    }
};

#endif // ! _Guard_h_