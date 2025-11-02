// Placeholder for Catch2 single-header
// Download from: https://github.com/catchorg/Catch2/releases
// Replace this file with catch2/catch.hpp from the single-header release
#ifndef CATCH2_SINGLE_HEADER_HPP
#define CATCH2_SINGLE_HEADER_HPP

// Minimal test framework placeholder - replace with actual Catch2
#include <cassert>
#include <iostream>
#include <string>

#define TEST_CASE(name, tags) static void test_##name(); namespace { struct test_##name##_reg { test_##name##_reg() { test_##name(); } } test_##name##_instance; } static void test_##name()
#define SECTION(name) if (true)
#define REQUIRE(expr) do { if (!(expr)) { std::cerr << "FAIL: " << #expr << " at " << __FILE__ << ":" << __LINE__ << "\n"; std::abort(); } } while(0)
#define REQUIRE_FALSE(expr) REQUIRE(!(expr))
#define CHECK(expr) do { if (!(expr)) { std::cerr << "CHECK FAIL: " << #expr << " at " << __FILE__ << ":" << __LINE__ << "\n"; } } while(0)

// Note: Replace this file with actual Catch2 single header from:
// https://github.com/catchorg/Catch2/releases/latest
// Look for catch.hpp in the single-header download

#endif

