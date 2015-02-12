/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    main.cpp
 * @date    1/21/2015
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#define BOOST_TEST_MODULE "libjson-rpc-cpp test suite"
#if _MSC_VER
// use static boost libraries with msvc
#else
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
