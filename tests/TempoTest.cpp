#include "Util.h"
#include "../Tempo.h"

using namespace std;
using namespace VSQ_NS;

class TempoStub : public Tempo
{
public:
    TempoStub( tick_t tick, int tempo ) :
        Tempo( tick, tempo )
    {
    }

    void setTime( double time )
    {
        _time = time;
    }
};

class TempoTest : public CppUnit::TestCase
{
public:
    void testConstructor()
    {
        Tempo entry = Tempo();
        CPPUNIT_ASSERT_EQUAL( 0, entry.clock );
        CPPUNIT_ASSERT_EQUAL( 0, entry.tempo );
        CPPUNIT_ASSERT_EQUAL( 0.0, entry.getTime() );
    
        entry = Tempo( 480, 500000 );
        CPPUNIT_ASSERT_EQUAL( 480, entry.clock );
        CPPUNIT_ASSERT_EQUAL( 500000, entry.tempo );
    }
    
    void testToString()
    {
        Tempo entry = Tempo( 480, 500000 );
        CPPUNIT_ASSERT_EQUAL( string( "{Clock=480, Tempo=500000, Time=0}" ), entry.toString() );
    }
    
    void testCompareTo()
    {
        Tempo a = Tempo();
        Tempo b = Tempo( 480, 500000 );
        CPPUNIT_ASSERT( 0 < b.compareTo( a ) );
        CPPUNIT_ASSERT_EQUAL( 0, a.compareTo( a ) );
        CPPUNIT_ASSERT( 0 > a.compareTo( b ) );
    }
    
    void testEquals()
    {
        TempoStub a = TempoStub( 480, 500000 );
        TempoStub b = TempoStub( 480, 500000 );
        a.setTime( 0.5 );
        b.setTime( 0.5 );
        CPPUNIT_ASSERT( a.equals( b ) );
        // クロックは同じだがtimeが違う
        b.setTime( 1 );
        CPPUNIT_ASSERT( a.equals( b ) );
        b.clock = 1;
        b.setTime( 0.5 );
        CPPUNIT_ASSERT( false == a.equals( b ) );
    }
    
    void testCompare()
    {
        Tempo a = Tempo();
        Tempo b = Tempo( 480, 500000 );
        CPPUNIT_ASSERT( false == Tempo::compare( b, a ) );
        CPPUNIT_ASSERT( false == Tempo::compare( a, a ) );
        CPPUNIT_ASSERT( Tempo::compare( a, b ) );
    }

    CPPUNIT_TEST_SUITE( TempoTest );
    CPPUNIT_TEST( testConstructor );
    CPPUNIT_TEST( testToString );
    CPPUNIT_TEST( testCompareTo );
    CPPUNIT_TEST( testEquals );
    CPPUNIT_TEST( testCompare );
    CPPUNIT_TEST_SUITE_END();
};

REGISTER_TEST_SUITE( TempoTest );
