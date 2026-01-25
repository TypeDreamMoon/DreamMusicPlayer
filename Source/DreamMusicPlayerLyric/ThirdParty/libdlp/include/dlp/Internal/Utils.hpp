// Dream Lyric Parser Library CXX20
// Internal utils
// Copyright (C) 2026 Type Dream Moon. All rights reserved.

#pragma once

#include <string_view>
#include <type_traits>
#include <vector>

namespace dlp::Internal::Utils
{
    /**
     * @brief Splits a string view into a vector of lines.
     * @param in_lyric_string The input string to split.
     * @return A vector of string_views, each representing a single line.
     */
    [[nodiscard]] std::vector<std::string_view> SplitLines(std::string_view in_lyric_string);

    /**
     * @brief Trims or normalizes a line of text (e.g., removing whitespace).
     * @param s The string view to normalize.
     * @return The normalized string view.
     */
    [[nodiscard]] std::string_view NormalizeLine(std::string_view s) noexcept;

    /**
     * @brief Cast<T>(Obj) template utility.
     * Performs a safe downcast from a base class pointer to a derived class pointer
     * using dynamic_cast.
     * * @tparam To The target class type (not a pointer).
     * @tparam From The source class type.
     * @param src Pointer to the source object.
     * @return To* Pointer to the target type, or nullptr if the cast fails or src is nullptr.
     * * @note The source class 'From' must be polymorphic (i.e., contain at least one virtual function).
     */
    template <typename To, typename From>
    [[nodiscard]] To* Cast(From* src) noexcept
    {
        // Static assertion: Ensures 'To' is a class type.
        // Usage should be Cast<MyClass>(ptr) instead of Cast<MyClass*>(ptr).
        static_assert(!std::is_pointer_v<To>, "Cast template parameter 'To' must be a class type, not a pointer type.");

        return dynamic_cast<To*>(src);
    }

    /**
     * @brief Const overload of the Cast<T>(Obj) template.
     */
    template <typename To, typename From>
    [[nodiscard]] const To* Cast(const From* src) noexcept
    {
        static_assert(!std::is_pointer_v<To>, "Cast template parameter 'To' must be a class type, not a pointer type.");
        return dynamic_cast<const To*>(src);
    }
}
