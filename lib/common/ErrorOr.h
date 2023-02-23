/**
 * @file ErrorOr.h
 * @brief ErrorOr class
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2023-07-10
 * 
*/
#ifndef _ErrorOr_h_
#define _ErrorOr_h_

#include <WString.h>
#include <Printable.h>
#include <Print.h>
#include <Check.h>

#include <Error.h>
#include <variant>

template <typename T = void>
struct ErrorOr
{
public:
    ErrorOr() noexcept
    {
        ok_ = false;
    }

    ErrorOr(Error error) noexcept
    {
        any_ = std::variant<Error, T>(error);
        ok_ = false;
    }

    ErrorOr(T &value) noexcept
    {
        any_ = std::variant<Error, T>(value);
        ok_ = true;
    }

    ErrorOr(T &&value) noexcept
    {
        any_ = std::variant<Error, T>(std::move(value));
        ok_ = true;
    }

    ErrorOr(const ErrorOr &other)
        : any_(other.any_),
          ok_(other.ok_)
    {
    }

    ErrorOr(ErrorOr &&other) : ErrorOr(other) {}

    ~ErrorOr() = default;

    auto ok() -> bool { return ok_; }

    // Returns the contained error.
    // REQUIRES: `ok()` is false.
    auto error() -> Error
    {
        CHECK(!ok());
        return std::get<Error>(any_);
    }

    // Returns the contained value.
    // REQUIRES: `ok()` is true.
    auto operator*() -> T &
    {
        CHECK(ok());
        return std::get<T>(any_);
    }

    // Returns the contained value.
    // REQUIRES: `ok()` is true.
    auto operator*() const -> const T &
    {
        CHECK(ok());
        return std::get<T>(any_);
    }

    // Returns the contained value.
    // REQUIRES: `ok()` is true.
    auto operator->() -> T *
    {
        CHECK(ok());
        return &std::get<T>(any_);
    }

    // Returns the contained value.
    // REQUIRES: `ok()` is true.
    auto operator->() const -> const T *
    {
        CHECK(ok());
        return &std::get<T>(any_);
    }

    inline auto unwrap() -> T
    {
        CHECK(ok()) << error().context << error().message;
        return std::get<T>(any_);
    }

    inline auto operator=(ErrorOr<T> &&other) -> ErrorOr<T> &
    {
        return *this = other;
    }

    inline auto operator=(ErrorOr<T> &other) -> ErrorOr<T> &
    {
        any_ = other.any_;
        ok_ = other.ok_;
        return *this;
    }

    template <typename U,
              typename std::enable_if<
                  std::conjunction_v<
                      std::negation<std::is_void<T>>,
                      std::is_pointer<U>,
                      std::is_convertible<U, T>>> * = nullptr>
    auto operator=(ErrorOr<U> &other) -> ErrorOr<T> &
    {
        ok_ = other.ok();
        if (ok_)
        {
            any_ = std::variant<Error, T>(static_cast<T>(other.unwrap()));
        }
        else
        {
            any_ = std::variant<Error, T>(other.error());
        }
        return *this;
    }

    template <typename U,
              typename std::enable_if<
                  std::conjunction_v<
                      std::is_pointer<U>,
                      std::is_convertible<U, T>>> * = nullptr>
    inline auto operator=(ErrorOr<U> &&other) -> ErrorOr<T> &
    {
        return *this = other;
    }

    template <typename U,
              typename std::enable_if_t<
                  std::conjunction_v<
                      std::is_pointer<T>,
                      std::is_convertible<T, U>>> * = nullptr>
    auto as() -> ErrorOr<U>
    {
        if (ok())
        {
            return ErrorOr<U>(static_cast<U>(std::get<T>(any_)));
        }
        else
        {
            return ErrorOr<U>(std::get<Error>(any_));
        }
    }

private:
    std::variant<Error, T> any_;
    bool ok_ = false;
};

struct Failure
{
    Error error;

    template <typename T>
    inline auto toError() -> ErrorOr<T>
    {
        return ErrorOr<T>(error);
    }

    template <typename T>
    inline operator ErrorOr<T>()
    {
        return toError<T>();
    }
};

template <>
struct ErrorOr<>
{
public:
    ErrorOr(Error error) noexcept
    {
        error_ = error;
        ok_ = false;
    }

    ErrorOr() noexcept
    {
        ok_ = true;
    }

    ErrorOr(const ErrorOr &other) {}

    ErrorOr(ErrorOr &&other) : ErrorOr(other) {}

    ~ErrorOr() = default;

    auto ok() -> bool { return ok_; }

    auto error() -> Error &
    {
        return error_;
    }

    template <typename U>
    inline operator ErrorOr<U>()
    {
        return ErrorOr<U>(error_);
    }

    inline auto operator=(Failure failure) -> ErrorOr<>
    {
        error_ = failure.error;
        ok_ = false;
        return *this;
    }

    inline auto operator=(ErrorOr<> &&other) -> ErrorOr<>
    {
        return *this = other;
    }

    inline auto operator=(ErrorOr<> &other) -> ErrorOr<>
    {
        ok_ = other.ok_;
        error_ = other.error_;
        return *this;
    }

private:
    bool ok_ = false;
    Error error_ = Error::None;
};

template <typename T = void>
struct Ok
{
    T value;

    inline auto toError() -> ErrorOr<T>
    {
        return ErrorOr<T>(value);
    }

    inline operator ErrorOr<T>()
    {
        return toError();
    }

    Ok(T &value) : value(value) {}
    Ok(T &&value) : value(std::move(value)) {}

private:
    Ok() = default;
};

template <>
struct Ok<>
{
    inline auto toError() -> ErrorOr<>
    {
        return ErrorOr<>();
    }

    inline operator ErrorOr<>()
    {
        return toError();
    }
};

template <typename T>
auto ok(T value) -> Ok<T>
{
    return Ok<T>{value};
}

auto ok() -> Ok<>
{
    return Ok<>{};
}

auto failure(Error error) -> Failure
{
    return {error};
}

#endif // ! _ErrorOr_h_
