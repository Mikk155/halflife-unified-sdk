/***
 *
 *    Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *    This product contains software technology licensed from Id
 *    Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *    All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#pragma once

#include <string>
#include <string_view>

#include <spdlog/common.h>

#include <EASTL/string.h>

/**
 *    @brief String type that can store an integer value up to @c (u)int64_t without dynamic memory allocations.
 */
using IntegerString = eastl::fixed_string<char, 20 + 1>;

[[nodiscard]] constexpr std::string_view ToStringView( spdlog::string_view_t view )
{
    return {view.data(), view.size()};
}

template <typename T, typename Allocator>
[[nodiscard]] constexpr std::basic_string_view<T> ToStringView( const eastl::basic_string<T, Allocator>& str )
{
    return {str.data(), str.size()};
}

/**
 *    @brief Trims whitespace at the start of the given text.
 */
std::string_view TrimStart( std::string_view text );

/**
 *    @brief Trims whitespace at the end of the given text.
 */
std::string_view TrimEnd( std::string_view text );

/**
 *    @brief Trims whitespace at the start and end of the given text.
 */
std::string_view Trim( std::string_view text );

void ToLower( std::string& text );
void ToUpper( std::string& text );

[[nodiscard]] std::string ToLower( std::string_view text );
[[nodiscard]] std::string ToUpper( std::string_view text );

[[nodiscard]] int UTIL_CompareI( std::string_view lhs, std::string_view rhs );

void UTIL_StringToVector( Vector& destination, std::string_view pString );
int UTIL_StringToInteger( std::string_view str );

// for handy use with ClientPrint params
IntegerString UTIL_ToString( int iValue );

/**
 *    @brief Parses a string that ends with an array index.
 *        The parsed index is always <tt>&gt;= 0</tt>.
 */
bool UTIL_ParseStringWithArrayIndex( std::string_view input, std::string_view& name, int& index );

std::string_view GetLine( std::string_view& text );

std::string_view::const_iterator FindWhitespace( std::string_view text );

std::string_view SkipWhitespace( std::string_view text );

std::string_view RemoveComments( std::string_view text );
