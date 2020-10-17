#include <boost/test/unit_test.hpp>

#include "EPCT_Utils.h"

/*********************************************************************
**
**  WARNING: This file is generated.  DO NOT MODIFY
**
**********************************************************************/
BOOST_AUTO_TEST_CASE(Main_Simple_MathAdds_integers_and_return_a_value)
{
    BOOST_TEST(EPCT::EPCT_Utils::ContextSingleUseFromFile(*setup_fixture::ep, "../../../data/test/dataset0001/TC0001_T0001_Main-Simple-Math_Adds_integers_and_return_a_value.js"));
    BOOST_TEST(EPCT::EPCT_Utils::ContextSingleUseFromFile(*setup_fixture::ep, "../../../data/test/dataset0001/TC0001_T0002_Main-Simple-Math_Adds_integers_and_return_a_value.js"));
}

