#include "Util.hpp"
#include "../Timesig.hpp"

using namespace std;
using namespace VSQ_NS;

class TimesigTest : public CppUnit::TestFixture{
public:
    void testConstruct(){
        Timesig itemA;
        CPPUNIT_ASSERT_EQUAL( itemA.numerator, 4 );
        CPPUNIT_ASSERT_EQUAL( itemA.denominator, 4 );
        CPPUNIT_ASSERT_EQUAL( itemA.barCount, 0 );

        Timesig itemB( 3, 4, 1 );
        CPPUNIT_ASSERT_EQUAL( itemB.numerator, 3 );
        CPPUNIT_ASSERT_EQUAL( itemB.denominator, 4 );
        CPPUNIT_ASSERT_EQUAL( itemB.barCount, 1 );
    }

    void testToString()
    {
        Timesig item( 3, 4, 1 );
        string expected = "{Clock=0, Numerator=3, Denominator=4, BarCount=1}";
        CPPUNIT_ASSERT_EQUAL( expected, item.toString() );
    }

    void testCompareTo()
    {
        Timesig a( 4, 4, 0 );
        Timesig b( 4, 4, 1 );
        assertEqual( -1, a.compareTo( b ) );
        assertEqual( 1, b.compareTo( a ) );
        assertEqual( 0, a.compareTo( a ) );
    }

    CPPUNIT_TEST_SUITE( TimesigTest );
    CPPUNIT_TEST( testConstruct );
    CPPUNIT_TEST( testToString );
    CPPUNIT_TEST( testCompareTo );
    CPPUNIT_TEST_SUITE_END();
};

REGISTER_TEST_SUITE( TimesigTest );
