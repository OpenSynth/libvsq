#include "Util.hpp"
#include "../Event.hpp"

using namespace std;
using namespace VSQ_NS;

class EventStub : public Event
{
public:
    EventStub( TextStream &stream, int value, std::string &lastLine ) :
        Event( stream, value, lastLine )
    {
    }

    int getLyricHandleIndex()
    {
        return _lyricHandleIndex;
    }

    int getVibratoHandleIndex()
    {
        return _vibratoHandleIndex;
    }

    int getNoteHeadHandleIndex()
    {
        return _noteHeadHandleIndex;
    }

    int getIconHandleIndex()
    {
        return _singerHandleIndex;
    }
};

class EventTest : public CppUnit::TestCase
{
public:
    Event getNoteEvent()
    {
        Event noteEvent( 0, EventType::NOTE );
        noteEvent.setLength( 2 );
        noteEvent.note = 6;
        noteEvent.dynamics = 21;
        noteEvent.pmBendDepth = 4;
        noteEvent.pmBendLength = 5;
        noteEvent.pmbPortamentoUse = 3;
        noteEvent.demDecGainRate = 7;
        noteEvent.demAccent = 8;
//        noteEvent.preUtterance = 9;
//        noteEvent.voiceOverlap = 10;
        noteEvent.vibratoDelay = 13;
        noteEvent.lyricHandle.index = 1;
        return noteEvent;
    }
    
    Event getSingerEvent()
    {
        Event singerEvent( 0, EventType::SINGER );
        singerEvent.singerHandle = Handle( HandleType::SINGER );
        singerEvent.singerHandle.index = 16;
        singerEvent.index = 16;
        return singerEvent;
    }
    
    Event getIconEvent()
    {
        Event iconEvent( 0, EventType::ICON );
        iconEvent.note = 19;
        iconEvent.index = 17;
        return iconEvent;
    }
    
    void testConstruct()
    {
        Event event;
        CPPUNIT_ASSERT_EQUAL( (tick_t)0, event.clock );
        CPPUNIT_ASSERT_EQUAL( 0, event.id );
    }
    
    void testConstructWithLine()
    {
        Event event( "123=ID#0001" );
        CPPUNIT_ASSERT_EQUAL( (tick_t)123, event.clock );
    
        event = Event( "1230=EOS" );
        CPPUNIT_ASSERT_EQUAL( (tick_t)1230, event.clock );
        CPPUNIT_ASSERT( event.isEOS() );
    }
    
    void testConstructWithClockAndId()
    {
        Event event( 1, EventType::NOTE );
        event.note = 60;
        event.index = 12;
    
        CPPUNIT_ASSERT_EQUAL( (tick_t)1, event.clock );
        CPPUNIT_ASSERT_EQUAL( 12, event.index );
        CPPUNIT_ASSERT_EQUAL( 60, event.note );
    }
    
    void testConstructByStream()
    {
        TextStream stream;
        stream.writeLine( "Type=Anote" );
        stream.writeLine( "Length=1" );
        stream.writeLine( "Note#=2" );
        stream.writeLine( "Dynamics=3" );
        stream.writeLine( "PMBendDepth=4" );
        stream.writeLine( "PMBendLength=5" );
        stream.writeLine( "DEMdecGainRate=6" );
        stream.writeLine( "DEMaccent=7" );
        stream.writeLine( "LyricHandle=h#0001" );
        stream.writeLine( "IconHandle=h#0002" );
        stream.writeLine( "VibratoHandle=h#0003" );
        stream.writeLine( "VibratoDelay=8" );
        stream.writeLine( "PMbPortamentoUse=3" );
        stream.writeLine( "NoteHeadHandle=h#0004" );
        stream.writeLine( "[ID#9999]" );
        stream.setPointer( -1 );
        int number = 10;
        string lastLine = "";
        EventStub event( stream, number, lastLine );

        assertEqual( EventType::NOTE, event.type );
        assertEqual( (tick_t)1, event.getLength() );
        assertEqual( 2, event.note );
        assertEqual( 3, event.dynamics );
        assertEqual( 4, event.pmBendDepth );
        assertEqual( 5, event.pmBendLength );
        assertEqual( 6, event.demDecGainRate );
        assertEqual( 7, event.demAccent );
        assertEqual( 1, event.getLyricHandleIndex() );
        assertEqual( 2, event.getIconHandleIndex() );
        assertEqual( 3, event.getVibratoHandleIndex() );
        assertEqual( 4, event.getNoteHeadHandleIndex() );
    }

    void testEquals()
    {
        //    fail();
    }
    
    void testWriteNoteWithOption()
    {
        Event event = getNoteEvent();
        event.clock = 20;
        event.index = 1;
        EventWriteOption optionAll;
        optionAll.length = true;
        optionAll.note = true;
        optionAll.dynamics = true;
        optionAll.pmBendDepth = true;
        optionAll.pmBendLength = true;
        optionAll.pmbPortamentoUse = true;
        optionAll.demDecGainRate = true;
        optionAll.demAccent = true;
        //TODO. optionAll.preUtterance = true;
        //TODO. optionAll.voiceOverlap = true;
    
        TextStream stream;
    
        // handleがどれもnilな音符イベント
        event.write( stream, optionAll );
        string expected =
                "[ID#0001]\n"
                "Type=Anote\n"
                "Length=2\n"
                "Note#=6\n"
                "Dynamics=21\n"
                "PMBendDepth=4\n"
                "PMBendLength=5\n"
                "PMbPortamentoUse=3\n"
                "DEMdecGainRate=7\n"
                "DEMaccent=8\n"
                "LyricHandle=h#0001\n";
            //TODO. "PreUtterance=9\n" ..
            //TODO. "VoiceOverlap=10\n" ..
        CPPUNIT_ASSERT_EQUAL( expected, stream.toString() );
    
        // handleに全部値が入っている音符イベント
        // 現在、PreUtteranceとVoiceOverlapは扱わないようにしているので、
        // オプション全指定と、オプションが無い場合の動作が全くおなじになってしまっている。
        // ustEventをちゃんと処理するようになったら、TODOコメントのところを外すこと
        event.lyricHandle = Handle( HandleType::LYRIC );
        event.lyricHandle.setLyricAt( 0, Lyric( "わ", "w a" ) );
        event.lyricHandle.index = 11;
        event.vibratoHandle = Handle( HandleType::VIBRATO );
        event.vibratoHandle.index = 12;
        event.noteHeadHandle = Handle( HandleType::NOTE_HEAD );
        event.noteHeadHandle.index = 14;
        stream = TextStream();
        event.write( stream, optionAll );
        expected =
            "[ID#0001]\n"
            "Type=Anote\n"
            "Length=2\n"
            "Note#=6\n"
            "Dynamics=21\n"
            "PMBendDepth=4\n"
            "PMBendLength=5\n"
            "PMbPortamentoUse=3\n"
            "DEMdecGainRate=7\n"
            "DEMaccent=8\n"
            //TODO. "PreUtterance=9\n" ..
            //TODO. "VoiceOverlap=10\n" ..
            "LyricHandle=h#0011\n"
            "VibratoHandle=h#0012\n"
            "VibratoDelay=13\n"
            "NoteHeadHandle=h#0014\n";
        CPPUNIT_ASSERT_EQUAL( expected, stream.toString() );
    
        // オプションが無い場合
        stream = TextStream();
        event.write( stream );
        expected =
            "[ID#0001]\n"
            "Type=Anote\n"
            "Length=2\n"
            "Note#=6\n"
            "Dynamics=21\n"
            "PMBendDepth=4\n"
            "PMBendLength=5\n"
            "PMbPortamentoUse=3\n"
            "DEMdecGainRate=7\n"
            "DEMaccent=8\n"
            "LyricHandle=h#0011\n"
            "VibratoHandle=h#0012\n"
            "VibratoDelay=13\n"
            "NoteHeadHandle=h#0014\n";
        CPPUNIT_ASSERT_EQUAL( expected, stream.toString() );
    
        // オプションが空の場合
        stream = TextStream();
        EventWriteOption emptyOption;
        emptyOption.demAccent = false;
        emptyOption.demDecGainRate = false;
        emptyOption.dynamics = false;
        emptyOption.length = false;
        emptyOption.note = false;
        emptyOption.pmBendDepth = false;
        emptyOption.pmBendLength = false;
        emptyOption.pmbPortamentoUse = false;
        emptyOption.preUtterance = false;
        emptyOption.voiceOverlap = false;
        event.write( stream, emptyOption );
        expected =
            "[ID#0001]\n"
            "Type=Anote\n"
            "LyricHandle=h#0011\n"
            "VibratoHandle=h#0012\n"
            "VibratoDelay=13\n"
            "NoteHeadHandle=h#0014\n";
        CPPUNIT_ASSERT_EQUAL( expected, stream.toString() );
    }
    
    void testWriteSinger()
    {
        Event event = getSingerEvent();
        event.clock = 1;
        event.index = 15;
        TextStream stream;
        event.write( stream );
        string expected =
            "[ID#0015]\n"
            "Type=Singer\n"
            "IconHandle=h#0016\n";
        CPPUNIT_ASSERT_EQUAL( expected, stream.toString() );
    }
    
    void testWriteIcon()
    {
        Event event = getIconEvent();
        event.iconDynamicsHandle = Handle( HandleType::DYNAMICS );
        event.iconDynamicsHandle.index = 18;
        event.clock = 2;
        event.index = 17;
        TextStream stream;
        event.write( stream );
        string expected =
            "[ID#0017]\n"
            "Type=Aicon\n"
            "IconHandle=h#0018\n"
            "Note#=19\n";
        CPPUNIT_ASSERT_EQUAL( expected, stream.toString() );
    }
    
    void testCompareTo()
    {
        Event singerEvent( 1920, EventType::SINGER );
        Event noteEvent( 1920, EventType::NOTE );
        CPPUNIT_ASSERT_EQUAL( 0, singerEvent.compareTo( singerEvent ) );
        CPPUNIT_ASSERT( 0 > singerEvent.compareTo( noteEvent ) );
        CPPUNIT_ASSERT( 0 < noteEvent.compareTo( singerEvent ) );
    
        singerEvent.clock = 2000;
        noteEvent.clock = 1920;
        CPPUNIT_ASSERT( 0 < singerEvent.compareTo( noteEvent ) );
    
        singerEvent.clock = 2000;
        noteEvent.clock = 2001;
        CPPUNIT_ASSERT_EQUAL( 1, noteEvent.compareTo( singerEvent ) );
    }
    
    void testCompare()
    {
        Event singerEvent = getSingerEvent();
        singerEvent.clock = 1920;
        Event noteEvent = getNoteEvent();
        noteEvent.clock = 1920;
        CPPUNIT_ASSERT( false == Event::compare( singerEvent, singerEvent ) );
        CPPUNIT_ASSERT( Event::compare( singerEvent, noteEvent ) );
        CPPUNIT_ASSERT( false == Event::compare( noteEvent, singerEvent ) );
    
        singerEvent.clock = 2000;
        noteEvent.clock = 1920;
        CPPUNIT_ASSERT( false == Event::compare( singerEvent, noteEvent ) );
    
        singerEvent.clock = 2000;
        noteEvent.clock = 2001;
        CPPUNIT_ASSERT( false == Event::compare( noteEvent, singerEvent ) );
    }

    void testClone(){
        Event event = getSingerEvent();
        event.clock = 40;
        event.id = 4;
        event.singerHandle = Handle( HandleType::SINGER );
        event.singerHandle.index = 12;
        Event copy = event.clone();
        CPPUNIT_ASSERT_EQUAL( (tick_t)40, copy.clock );
        CPPUNIT_ASSERT_EQUAL( 4, copy.id );
        CPPUNIT_ASSERT_EQUAL( 12, copy.singerHandle.index );

        Event id( 0, EventType::NOTE );
        id.index = 1;
        id.note = 6;
        id.dynamics = 7;
        id.pmBendDepth = 8;
        id.pmBendLength = 9;
        id.pmbPortamentoUse = 10;
        id.demDecGainRate = 11;
        id.demAccent = 12;
        id.vibratoDelay = 13;
        id.pMeanOnsetFirstNote = 14;
        id.vMeanNoteTransition = 15;
        id.d4mean = 16;
        id.pMeanEndingNote = 17;
        //assert_nil( id.singerHandle );
        //assert_not_nil( id.lyricHandle );
        //assert_nil( id.vibratoHandle );
        //assert_nil( id.noteHeadHandle );
        //assert_nil( id.iconDynamicsHandle );

        copy = id.clone();
        CPPUNIT_ASSERT_EQUAL( 1, copy.index );
        CPPUNIT_ASSERT_EQUAL( EventType::NOTE, copy.type );
        CPPUNIT_ASSERT_EQUAL( 6, copy.note );
        CPPUNIT_ASSERT_EQUAL( 7, copy.dynamics );
        CPPUNIT_ASSERT_EQUAL( 8, copy.pmBendDepth );
        CPPUNIT_ASSERT_EQUAL( 9, copy.pmBendLength );
        CPPUNIT_ASSERT_EQUAL( 10, copy.pmbPortamentoUse );
        CPPUNIT_ASSERT_EQUAL( 11, copy.demDecGainRate );
        CPPUNIT_ASSERT_EQUAL( 12, copy.demAccent );
        CPPUNIT_ASSERT_EQUAL( 13, copy.vibratoDelay );
        CPPUNIT_ASSERT_EQUAL( 14, copy.pMeanOnsetFirstNote );
        CPPUNIT_ASSERT_EQUAL( 15, copy.vMeanNoteTransition );
        CPPUNIT_ASSERT_EQUAL( 16, copy.d4mean );
        CPPUNIT_ASSERT_EQUAL( 17, copy.pMeanEndingNote );

        Handle iconHandle( HandleType::SINGER );
        iconHandle.setCaption( "foo" );
        id.singerHandle = iconHandle;
        Handle lyricHandle( HandleType::LYRIC );
        lyricHandle.index = 102;
        id.lyricHandle = lyricHandle;
        Handle vibratoHandle( HandleType::VIBRATO );
        vibratoHandle.iconId = "aho";
        id.vibratoHandle = vibratoHandle;
        Handle noteHeadHandle( HandleType::NOTE_HEAD );
        noteHeadHandle.ids = "baka";
        id.noteHeadHandle = noteHeadHandle;
        Handle iconDynamicsHandle( HandleType::DYNAMICS );
        iconDynamicsHandle.setStartDyn( 183635 );
        id.iconDynamicsHandle = iconDynamicsHandle;

        copy = id.clone();
        CPPUNIT_ASSERT_EQUAL( string( "foo" ), copy.singerHandle.getCaption() );
        CPPUNIT_ASSERT_EQUAL( 102, copy.lyricHandle.index );
        CPPUNIT_ASSERT_EQUAL( string( "aho" ), copy.vibratoHandle.iconId );
        CPPUNIT_ASSERT_EQUAL( string( "baka" ), copy.noteHeadHandle.ids );
        CPPUNIT_ASSERT_EQUAL( 183635, copy.iconDynamicsHandle.getStartDyn() );
    }

    CPPUNIT_TEST_SUITE( EventTest );
    CPPUNIT_TEST( testConstruct );
    CPPUNIT_TEST( testConstructWithLine );
    CPPUNIT_TEST( testConstructWithClockAndId );
    CPPUNIT_TEST( testConstructByStream );
    CPPUNIT_TEST( testEquals );
    CPPUNIT_TEST( testWriteNoteWithOption );
    CPPUNIT_TEST( testWriteSinger );
    CPPUNIT_TEST( testWriteIcon );
    CPPUNIT_TEST( testCompareTo );
    CPPUNIT_TEST( testCompare );
    CPPUNIT_TEST( testClone );
    CPPUNIT_TEST_SUITE_END();
};

REGISTER_TEST_SUITE( EventTest );
