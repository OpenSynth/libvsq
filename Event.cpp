/**
 * Event.cpp
 * Copyright © 2012 kbinani
 *
 * This file is part of libvsq.
 *
 * libvsq is free software; you can redistribute it and/or
 * modify it under the terms of the BSD License.
 *
 * libvsq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include "Event.h"
#include "StringUtil.h"
#include "EventWriteOption.h"

using namespace std;
using namespace VSQ_NS;

void Event::init()
{
    tag = "";
    id = -1;
    clock = 0;
    index = -1;
    type = EventType::NOTE;
    _length = 0;
    note = 0;
    dynamics = 0;
    pmBendDepth = 0;
    pmBendLength = 0;
    pmbPortamentoUse = 0;
    demDecGainRate = 0;
    demAccent = 0;
    vibratoDelay = 0;
    pMeanOnsetFirstNote = 10;
    vMeanNoteTransition = 12;
    d4mean = 24;
    pMeanEndingNote = 12;
//    ustEvent = nil;
    _lyricHandleIndex = 0;
    _noteHeadHandleIndex = 0;
    _singerHandleIndex = 0;
    _vibratoHandleIndex = 0;
}

Event::Event( const string &line ) :
    vibratoHandle()
{
    init();
    vector<string> spl = StringUtil::explode( "=", line );
    clock = boost::lexical_cast<tick_t>( spl[0] );
    if( spl[1] == "EOS" ){
        index = -1;
    }
}

Event::Event()
{
    init();
    clock = 0;
    index = -1;
    id = 0;
}

Event::Event( tick_t clock, EventType::EventTypeEnum eventType )
{
    this->clock = clock;
    this->type = eventType;
    if( eventType == EventType::SINGER ){
        singerHandle = Handle( HandleType::SINGER );
    }else if( eventType == EventType::NOTE ){
        lyricHandle = Handle( HandleType::LYRIC );
        lyricHandle.setLyricAt( 0, Lyric( "a", "a" ) );
    }
    id = 0;
}

tick_t Event::getLength() const
{
    return _length;
}

void Event::setLength( tick_t value )
{
    _length = value;
}

/**
    -- @param item [VsqEvent]
    -- @return [bool]
    function this:equals( item )
        if( self.clock ~= item.clock )then
            return false;
        end
        if( self.type ~= item.type )then
            return false;
        end
        if( self.type == idType.Anote )then
            if( self.note ~= item.note )then
                return false;
            end
            if( self:getLength() ~= item:getLength() )then
                return false;
            end
            if( self.d4mean ~= item.d4mean )then
                return false;
            end
            if( self.demAccent ~= item.demAccent )then
                return false;
            end
            if( self.demDecGainRate ~= item.demDecGainRate )then
                return false;
            end
            if( self.dynamics ~= item.dynamics )then
                return false;
            end
            if( self.lyricHandle ~= nil and item.lyricHandle ~= nil )then
                return false;
            end
            if( self.lyricHandle ~= nil and item.lyricHandle == nil )then
                return false;
            end
            if( self.lyricHandle ~= nil and item.lyricHandle ~= nil )then
                if( self.lyricHandle:size() ~= item.lyricHandle:size() )then
                    return false;
                end
                local count = self.lyricHandle:size();
                local k;
                for k = 0, count - 1, 1 do
                    if( not self.lyricHandle:getLyricAt( k ):equalsForSynth( item.lyricHandle:getLyricAt( k ) ) )then
                        return false;
                    end
                end
            end
            if( self.noteHeadHandle == nil and item.noteHeadHandle ~= nil )then
                return false;
            end
            if( self.noteHeadHandle ~= nil and item.noteHeadHandle == nil )then
                return false;
            end
            if( self.noteHeadHandle ~= nil and item.noteHeadHandle ~= nil )then
                if( self.NoteHeadHandle.iconId ~= item.noteHeadHandle.iconId )then
                    return false;
                end
                if( self.noteHeadHandle:getDepth() ~= item.noteHeadHandle:getDepth() )then
                    return false;
                end
                if( self.noteHeadHandle:getDuration() ~= item.noteHeadHandle:getDuration() )then
                    return false;
                end
                if( self.noteHeadHandle:getLength() ~= item.noteHeadHandle:getLength() )then
                    return false;
                end
            end
            if( self.pmBendDepth ~= item.pmBendDepth )then
                return false;
            end
            if( self.pmBendLength ~= item.pmBendLength )then
                return false;
            end
            if( self.pmbPortamentoUse ~= item.pmbPortamentoUse )then
                return false;
            end
            if( self.pMeanEndingNote ~= item.pMeanEndingNote )then
                return false;
            end
            if( self.pMeanOnsetFirstNote ~= item.pMeanOnsetFirstNote )then
                return false;
            end
            local hVibratoThis = self.vibratoHandle;
            local hVibratoItem = item.vibratoHandle;
            if( hVibratoThis == nil and hVibratoItem ~= nil )then
                return false;
            end
            if( hVibratoThis ~= nil and hVibratoItem == nil )then
                return false;
            end
            if( hVibratoThis ~= nil and hVibratoItem ~= nil )then
                if( self.vibratoDelay ~= item.vibratoDelay )then
                    return false;
                end
                if( hVibratoThis.iconId ~= hVibratoItem.iconId )then
                    return false;
                end
                if( hVibratoThis:getStartDepth() ~= hVibratoItem:getStartDepth() )then
                    return false;
                end
                if( hVibratoThis:getStartRate() ~= hVibratoItem:getStartRate() )then
                    return false;
                end
                local vibRateThis = hVibratoThis:getRateBP();
                local vibRateItem = hVibratoItem:getRateBP();
                if( vibRateThis == nil and vibRateItem ~= nil )then
                    return false;
                end
                if( vibRateThis ~= nil and vibRateItem == nil )then
                    return false;
                end
                if( vibRateThis ~= nil and vibRateItem ~= nil )then
                    local numRateCount = vibRateThis:size();
                    if( numRateCount ~= vibRateItem:size() )then
                        return false;
                    end
                    local k;
                    for k = 0, numRateCount - 1, 1 do
                        local pThis = vibRateThis:getElement( k );
                        local pItem = vibRateItem:getElement( k );
                        if( pThis.x ~= pItem.x )then
                            return false;
                        end
                        if( pThis.y ~= pItem.y )then
                            return false;
                        end
                    end
                end
                local vibDepthThis = hVibratoThis:getDepthBP();
                local vibDepthItem = hVibratoItem:getDepthBP();
                if( vibDepthThis == nil and vibDepthItem ~= nil )then
                    return false;
                end
                if( vibDepthThis ~= nil and vibDepthItem == nil )then
                    return false;
                end
                if( vibDepthThis ~= nil and vibDepthItem ~= nil )then
                    local numDepthCount = vibDepthThis:size();
                    if( numDepthCount ~= vibDepthItem:size() )then
                        return false;
                    end
                    local k;
                    for k = 0, numDepthCount - 1, 1 do
                        local pThis = vibDepthThis:getElement( k );
                        local pItem = vibDepthItem:getElement( k );
                        if( pThis.x ~= pItem.x )then
                            return false;
                        end
                        if( pThis.y ~= pItem.y )then
                            return false;
                        end
                    end
                end
            end
            if( self.vMeanNoteTransition ~= item.vMeanNoteTransition )then
                return false;
            end
        elseif( self.type == EventTypeEnum.SINGER )then
            -- シンガーイベントの比較
            if( self.singerHandle.program ~= item.singerHandle.program )then
                return false;
            end
        elseif( self.type == EventTypeEnum.ICON )then
            if( self.iconDynamicsHandle.iconId ~= item.iconDynamicsHandle.iconId )then
                return false;
            end
            if( self.iconDynamicsHandle:isDynaffType() )then
                -- 強弱記号
            else
                -- クレッシェンド・デクレッシェンド
                if( self:getLength() ~= item:getLength() )then
                    return false;
                end
            end
        end

        return true;
    end*/

void Event::write( TextStream &stream, EventWriteOption printTargets ) const
{
    stream.write( "[ID#" ).write( (boost::format( "%04d" ) % index).str() ).writeLine( "]" );
    stream.write( "Type=" ).writeLine( EventType::toString( type ) );
    if( type == EventType::NOTE ){
        if( printTargets.length ){
            stream.write( "Length=" ).writeLine( (boost::format( "%ld" ) % getLength()).str() );
        }
        if( printTargets.note ){
            stream.write( "Note#=" ).writeLine( (boost::format( "%d" ) % note).str() );
        }
        if( printTargets.dynamics ){
            stream.write( "Dynamics=" ).writeLine( (boost::format( "%d" ) % dynamics).str() );
        }
        if( printTargets.pmBendDepth ){
            stream.write( "PMBendDepth=" ).writeLine( (boost::format( "%d" ) % pmBendDepth).str() );
        }
        if( printTargets.pmBendLength ){
            stream.write( "PMBendLength=" ).writeLine( (boost::format( "%d" ) % pmBendLength).str() );
        }
        if( printTargets.pmbPortamentoUse ){
            stream.write( "PMbPortamentoUse=" ).writeLine( (boost::format( "%d" ) % pmbPortamentoUse).str() );
        }
        if( printTargets.demDecGainRate ){
            stream.write( "DEMdecGainRate=" ).writeLine( (boost::format( "%d" ) % demDecGainRate).str() );
        }
        if( printTargets.demAccent ){
            stream.write( "DEMaccent=" ).writeLine( (boost::format( "%d" ) % demAccent).str() );
        }
        if( printTargets.preUtterance ){
            //TODO:
//            stream.writeLine( "PreUtterance=" + ustEvent.preUtterance );
        }
        if( printTargets.voiceOverlap ){
            //TODO:
//            stream.writeLine( "VoiceOverlap=" + ustEvent.voiceOverlap );
        }
        if( lyricHandle.getHandleType() == HandleType::LYRIC ){
            stream.write( "LyricHandle=h#" ).writeLine( (boost::format( "%04d" ) % lyricHandle.index).str() );
        }
        if( vibratoHandle.getHandleType() == HandleType::VIBRATO ){
            stream.write( "VibratoHandle=h#" ).writeLine( (boost::format( "%04d" ) % vibratoHandle.index).str() );
            stream.write( "VibratoDelay=" ).writeLine( (boost::format( "%d" ) % vibratoDelay).str() );
        }
        if( noteHeadHandle.getHandleType() == HandleType::NOTE_HEAD ){
            stream.write( "NoteHeadHandle=h#" ).writeLine( (boost::format( "%04d" ) % noteHeadHandle.index).str() );
        }
    }else if( type == EventType::SINGER ){
        stream.write( "IconHandle=h#" ).writeLine( (boost::format( "%04d" ) % singerHandle.index).str() );
    }else if( type == EventType::ICON ){
        stream.write( "IconHandle=h#" ).writeLine( (boost::format( "%04d" ) % iconDynamicsHandle.index).str() );
        stream.write( "Note#=" ).writeLine( (boost::format( "%d" ) % note).str() );
    }
}

/*
    -- テキストファイルからのコンストラクタ
    -- @param sr [TextStream] 読み込み対象
    -- @param value [int]
    -- @param last_line [ByRef<string>] 読み込んだ最後の行が返されます
    -- @return (Id)
    function this:_init_3( sr, value, last_line )
        local spl;
        self.index = value;
        self.type = EventTypeEnum.UNKNOWN;
        self._singerHandleIndex = -2;
        self._lyricHandleIndex = -1;
        self._vibratoHandleIndex = -1;
        self._noteHeadHandleIndex = -1;
        self:setLength( 0 );
        self.note = 0;
        self.dynamics = 64;
        self.pmBendDepth = 8;
        self.pmBendLength = 0;
        self.pmbPortamentoUse = 0;
        self.demDecGainRate = 50;
        self.demAccent = 50;
        self.vibratoDelay = 0;
        last_line.value = sr:readLine();
        while( last_line.value:find( "[" ) ~= 0 )do
            spl = Util.split( last_line.index, '=' );
            local search = spl[1];
            if( search == "Type" )then
                if( spl[2] == "Anote" )then
                    self.type = EventTypeEnum.NOTE;
                elseif( spl[2] == "Singer" )then
                    self.type = EventTypeEnum.SINGER;
                elseif( spl[2] == "Aicon" )then
                    self.type = EventTypeEnum.ICON;
                else
                    self.type = EventTypeEnum.UNKNOWN;
                end
            elseif( search == "Length" )then
                self:setLength( tonumber( spl[2], 10 ) );
            elseif( search == "Note#" )then
                self.note = tonumber( spl[2], 10 );
            elseif( search == "Dynamics" )then
                self.dynamics = tonumber( spl[2], 10 );
            elseif( search == "PMBendDepth" )then
                self.pmBendDepth = tonumber( spl[2], 10 );
            elseif( search == "PMBendLength" )then
                self.pmBendLength = tonumber( spl[2], 10 );
            elseif( search == "DEMdecGainRate" )then
                self.demDecGainRate = tonumber( spl[2], 10 );
            elseif( search ==  "DEMaccent" )then
                self.demAccent = tonumber( spl[2], 10 );
            elseif( search == "LyricHandle" )then
                self._lyricHandleIndex = Handle.getHandleIndexFromString( spl[2] );
            elseif( search == "IconHandle" )then
                self._singerHandleIndex = Handle.getHandleIndexFromString( spl[2] );
            elseif( search == "VibratoHandle" )then
                self._vibratoHandleIndex = Handle.getHandleIndexFromString( spl[2] );
            elseif( search == "VibratoDelay" )then
                self.vibratoDelay = tonumber( spl[2], 10 );
            elseif( search == "PMbPortamentoUse" )then
                self.pmbPortamentoUse = tonumber( spl[2], 10 );
            elseif( search == "NoteHeadHandle" )then
                self._noteHeadHandleIndex = Handle.getHandleIndexFromString( spl[2] );
            end
            if( not sr:ready() )then
                break;
            end
            last_line.value = sr:readLine();
        end
    end
*/

bool Event::isEOS() const
{
    if( index == -1 ){
        return true;
    }else{
        return false;
    }
}

int Event::compareTo( const Event &item ) const
{
    tick_t ret = clock - item.clock;
    if( ret == 0 ){
        return type - item.type;
    }else{
        return (int)ret;
    }
}

bool Event::compare( const Event &a, const Event &b )
{
    return (a.compareTo( b ) < 0);
}

const Event Event::getEOS()
{
    return Event();
}
